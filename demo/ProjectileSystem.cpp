#include "pch.h"
#include "ProjectileSystem.h"
#include "RNG.h"
#include <cmath>

namespace {
	constexpr float floatEpsilon = 0.000001f;
	constexpr float pi = 3.14159265358979323846f;
	constexpr int overlapSamples = 48;

	struct Vec2 {
		float x;
		float y;
	};

	Vec2 operator+(const Vec2& a, const Vec2& b) { return { a.x + b.x, a.y + b.y }; }
	Vec2 operator-(const Vec2& a, const Vec2& b) { return { a.x - b.x, a.y - b.y }; }
	Vec2 operator*(const Vec2& v, float s) { return { v.x * s, v.y * s }; }

	float Dot(const Vec2& a, const Vec2& b)
	{
		return a.x * b.x + a.y * b.y;
	}

	float LengthSq(const Vec2& v)
	{
		return v.x * v.x + v.y * v.y;
	}

	Vec2 Rotate(const Vec2& v, float radians)
	{
		float c = std::cos(radians);
		float s = std::sin(radians);
		return { v.x * c - v.y * s, v.x * s + v.y * c };
	}

	// Unit-circle samples shared by every ellipse test; computed once
	// instead of 96 trig calls per projectile per frame like the old
	// EllipseCollider path.
	struct UnitCircleTable {
		Vec2 samples[overlapSamples];
		UnitCircleTable() {
			for (int i = 0; i < overlapSamples; i++) {
				float theta = static_cast<float>(i) / static_cast<float>(overlapSamples) * 2.0f * pi;
				samples[i] = { std::cos(theta), std::sin(theta) };
			}
		}
	};
	const UnitCircleTable unitCircle;

	// The player's rectangle collider, resolved to world space once per
	// frame instead of once per projectile.
	struct RectShape {
		Vec2 corners[4];
		Vec2 center;
		Vec2 axisX, axisY; // normalized edge directions
		float halfX = 0.f, halfY = 0.f;
		float halfDiagonal = 0.f;
	};

	bool BuildRectShape(DX9GF::RectangleCollider& collider, RectShape& out)
	{
		const float originWorldX = collider.GetWorldX();
		const float originWorldY = collider.GetWorldY();
		const float r = collider.GetWorldRotation();
		const float sx = collider.GetWorldScaleX();
		const float sy = collider.GetWorldScaleY();
		const float ox = collider.GetOriginX();
		const float oy = collider.GetOriginY();

		const Vec2 local[4] = {
			{ 0.0f, 0.0f },
			{ collider.GetWidth(), 0.0f },
			{ collider.GetWidth(), collider.GetHeight() },
			{ 0.0f, collider.GetHeight() }
		};
		for (int i = 0; i < 4; i++) {
			Vec2 p = { (local[i].x - ox) * sx, (local[i].y - oy) * sy };
			p = Rotate(p, r);
			out.corners[i] = { originWorldX + p.x, originWorldY + p.y };
		}

		out.center = (out.corners[0] + out.corners[1] + out.corners[2] + out.corners[3]) * 0.25f;
		Vec2 edgeX = out.corners[1] - out.corners[0];
		Vec2 edgeY = out.corners[3] - out.corners[0];
		float lenX = std::sqrt((std::max)(floatEpsilon, LengthSq(edgeX)));
		float lenY = std::sqrt((std::max)(floatEpsilon, LengthSq(edgeY)));
		out.axisX = edgeX * (1.0f / lenX);
		out.axisY = edgeY * (1.0f / lenY);
		out.halfX = lenX * 0.5f;
		out.halfY = lenY * 0.5f;
		out.halfDiagonal = std::sqrt(out.halfX * out.halfX + out.halfY * out.halfY);
		return true;
	}

	bool PointInRect(const RectShape& rect, const Vec2& p)
	{
		Vec2 d = p - rect.center;
		float px = std::abs(Dot(d, rect.axisX));
		float py = std::abs(Dot(d, rect.axisY));
		return px <= (rect.halfX + floatEpsilon) && py <= (rect.halfY + floatEpsilon);
	}

	// Same overlap test the old EllipseCollider/RectangleCollider pair ran
	// (corners-in-ellipse + sampled-ellipse-boundary-in-rect), on plain
	// floats with a cheap distance broad-phase in front.
	bool EllipseOverlapsRect(
		const RectShape& rect,
		float centerX,
		float centerY,
		float rotation,
		float radiusX,
		float radiusY
	)
	{
		if (radiusX <= floatEpsilon || radiusY <= floatEpsilon) {
			return false;
		}

		const float maxRadius = (std::max)(radiusX, radiusY);
		const float reach = maxRadius + rect.halfDiagonal;
		const Vec2 center{ centerX, centerY };
		if (LengthSq(center - rect.center) > reach * reach) {
			return false;
		}

		const float c = std::cos(rotation);
		const float s = std::sin(rotation);

		for (const auto& corner : rect.corners) {
			Vec2 d = corner - center;
			// rotate by -rotation into the ellipse's local space
			float lx = d.x * c + d.y * s;
			float ly = -d.x * s + d.y * c;
			float nx = lx / radiusX;
			float ny = ly / radiusY;
			if (nx * nx + ny * ny <= 1.0f + floatEpsilon) {
				return true;
			}
		}

		for (const auto& sample : unitCircle.samples) {
			Vec2 local{ sample.x * radiusX, sample.y * radiusY };
			Vec2 world{
				centerX + local.x * c - local.y * s,
				centerY + local.x * s + local.y * c
			};
			if (PointInRect(rect, world)) {
				return true;
			}
		}

		return false;
	}

	// Separating-axis test between the player's box and a beam's oriented box.
	// Both boxes' axes are unit length, so the projections need no normalizing.
	bool ObbOverlapsRect(
		const RectShape& rect,
		float centerX,
		float centerY,
		float rotation,
		float halfLength,
		float halfThickness
	)
	{
		if (halfLength <= floatEpsilon || halfThickness <= floatEpsilon) {
			return false;
		}

		const float c = std::cos(rotation);
		const float s = std::sin(rotation);
		const Vec2 beamX{ c, s };
		const Vec2 beamY{ -s, c };
		const Vec2 delta = Vec2{ centerX, centerY } - rect.center;

		const Vec2 axes[4] = { rect.axisX, rect.axisY, beamX, beamY };
		for (const auto& axis : axes) {
			const float rectExtent = std::abs(Dot(rect.axisX, axis)) * rect.halfX
				+ std::abs(Dot(rect.axisY, axis)) * rect.halfY;
			const float beamExtent = std::abs(Dot(beamX, axis)) * halfLength
				+ std::abs(Dot(beamY, axis)) * halfThickness;
			if (std::abs(Dot(delta, axis)) > rectExtent + beamExtent + floatEpsilon) {
				return false;
			}
		}
		return true;
	}

	D3DCOLOR ScaleAlpha(D3DCOLOR color, float multiplier)
	{
		const float clamped = (std::max)(0.f, (std::min)(1.f, multiplier));
		const D3DCOLOR alpha = static_cast<D3DCOLOR>(((color >> 24) & 0xFF) * clamped);
		return (color & 0x00FFFFFF) | (alpha << 24);
	}

	// One beam quad, rotated about its own centre.
	void DrawBeam(
		DX9GF::GraphicsDevice* graphicsDevice,
		const DX9GF::Camera& camera,
		float x,
		float y,
		float angle,
		float length,
		float thickness,
		D3DCOLOR color,
		bool isFilled = true
	)
	{
		graphicsDevice->DrawRectangle(
			camera, x, y, length, thickness, angle,
			1.f, 1.f, length * 0.5f, thickness * 0.5f, color, isFilled
		);
	}
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetTrajectory(D3DXVECTOR2 trajectory)
{
	this->trajectory = trajectory;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetTargetPosition(float targetX, float targetY)
{
	D3DXVECTOR2 direction{ targetX - x, targetY - y };
	D3DXVec2Normalize(&direction, &direction);
	this->trajectory = direction;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetVelocity(float velocity)
{
	this->velocity = velocity;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetDelay(float delay)
{
	this->delay = delay;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetDecayTime(float decayTime)
{
	this->decayTime = decayTime;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetDamage(float damage)
{
	this->damage = damage;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetHoming(float turnSpeed)
{
	this->behavior = ProjectileBehavior::Homing;
	this->turnSpeed = turnSpeed;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetWave(float amplitude, float frequency)
{
	this->behavior = ProjectileBehavior::SineWave;
	this->amplitude = amplitude;
	this->frequency = frequency;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetSpiralParams(float initialAngle, float radialSpeed, float angularVelocity)
{
	this->behavior = ProjectileBehavior::Spiral;
	this->initialAngle = initialAngle;
	this->radialSpeed = radialSpeed;
	this->angularVelocity = angularVelocity;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetInitialVelocity(float velocity)
{
	this->behavior = ProjectileBehavior::Boomerang;
	this->velocity = velocity;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetReturnAcceleration(float acceleration)
{
	this->behavior = ProjectileBehavior::Boomerang;
	this->returnAcceleration = acceleration;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetSplitOnArrival(float targetX, float targetY, int count, float velocity)
{
	SetTargetPosition(targetX, targetY);
	this->splitTrigger = ProjectileSplitTrigger::OnArrival;
	this->splitTargetX = targetX;
	this->splitTargetY = targetY;
	this->splitCount = count;
	this->splitVelocity = velocity;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetSplitOnDecay(int count, float velocity)
{
	this->splitTrigger = ProjectileSplitTrigger::OnDecay;
	this->splitCount = count;
	this->splitVelocity = velocity;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetSplitSpread(float spread, float angleOffset)
{
	this->splitSpread = spread;
	this->splitAngleOffset = angleOffset;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetSplitRandomAngle(bool randomize)
{
	this->splitRandomizeAngle = randomize;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetSplitDecayTime(float decayTime)
{
	this->splitDecayTime = decayTime;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetSplitDamage(float damage)
{
	this->splitDamage = damage;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetSplitGenerations(int generations)
{
	this->splitGenerations = generations;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetGhostSprite(DX9GF::Texture* texture, RECT srcRect, float originX, float originY)
{
	this->ghostTexture = texture;
	this->ghostSrcRect = srcRect;
	this->ghostHasSrcRect = true;
	this->ghostOriginX = originX;
	this->ghostOriginY = originY;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetStatusEffect(ModifierType type, float value, int duration)
{
	this->hasStatusEffect = true;
	this->randomizeStatusEffect = false;
	this->statusEffectType = type;
	this->statusEffectValue = value;
	this->statusEffectDuration = duration;
	return *this;
}

Demo::ProjectileDesc& Demo::ProjectileDesc::SetRandomStatusEffect(
	ModifierType typeA, float valueA, int durationA,
	ModifierType typeB, float valueB, int durationB)
{
	this->hasStatusEffect = true;
	this->randomizeStatusEffect = true;
	this->statusEffectType = typeA;
	this->statusEffectValue = valueA;
	this->statusEffectDuration = durationA;
	this->altStatusEffectType = typeB;
	this->altStatusEffectValue = valueB;
	this->altStatusEffectDuration = durationB;
	return *this;
}

Demo::LaserDesc Demo::LaserDesc::Vertical(float x, float centerY, float length)
{
	return LaserDesc(x, centerY, pi * 0.5f, length);
}

Demo::LaserDesc Demo::LaserDesc::Horizontal(float y, float centerX, float length)
{
	return LaserDesc(centerX, y, 0.f, length);
}

Demo::LaserDesc& Demo::LaserDesc::SetThickness(float thickness)
{
	this->thickness = thickness;
	return *this;
}

Demo::LaserDesc& Demo::LaserDesc::SetDelay(float delay)
{
	this->delay = delay;
	return *this;
}

Demo::LaserDesc& Demo::LaserDesc::SetWarnTime(float warnTime)
{
	this->warnTime = warnTime;
	return *this;
}

Demo::LaserDesc& Demo::LaserDesc::SetFireTime(float fireTime)
{
	this->fireTime = fireTime;
	return *this;
}

Demo::LaserDesc& Demo::LaserDesc::SetDamage(float damage)
{
	this->damage = damage;
	return *this;
}

Demo::LaserDesc& Demo::LaserDesc::SetGlowThickness(float glowThickness)
{
	this->glowThickness = glowThickness;
	return *this;
}

Demo::LaserDesc& Demo::LaserDesc::SetColors(D3DCOLOR warnColor, D3DCOLOR glowColor, D3DCOLOR coreColor)
{
	this->warnColor = warnColor;
	this->glowColor = glowColor;
	this->coreColor = coreColor;
	return *this;
}

Demo::LaserDesc& Demo::LaserDesc::SetStatusEffect(ModifierType type, float value, int duration)
{
	this->hasStatusEffect = true;
	this->randomizeStatusEffect = false;
	this->statusEffectType = type;
	this->statusEffectValue = value;
	this->statusEffectDuration = duration;
	return *this;
}

Demo::LaserDesc& Demo::LaserDesc::SetRandomStatusEffect(
	ModifierType typeA, float valueA, int durationA,
	ModifierType typeB, float valueB, int durationB)
{
	this->hasStatusEffect = true;
	this->randomizeStatusEffect = true;
	this->statusEffectType = typeA;
	this->statusEffectValue = valueA;
	this->statusEffectDuration = durationA;
	this->altStatusEffectType = typeB;
	this->altStatusEffectValue = valueB;
	this->altStatusEffectDuration = durationB;
	return *this;
}

unsigned int Demo::ProjectileSystem::GetOrCreateBatch(const ProjectileDesc& desc)
{
	for (unsigned int i = 0; i < batches.size(); i++) {
		const auto& batch = batches[i];
		if (batch.texture != desc.texture
			|| batch.frameRate != desc.frameRate
			|| batch.originX != desc.spriteOriginX
			|| batch.originY != desc.spriteOriginY
			|| batch.frames.size() != desc.frames.size()) {
			continue;
		}
		bool sameFrames = true;
		for (size_t f = 0; f < batch.frames.size(); f++) {
			const RECT& a = batch.frames[f];
			const RECT& b = desc.frames[f];
			if (a.left != b.left || a.top != b.top || a.right != b.right || a.bottom != b.bottom) {
				sameFrames = false;
				break;
			}
		}
		if (sameFrames) {
			return i;
		}
	}

	SpriteBatch batch;
	batch.texture = desc.texture;
	batch.frames = desc.frames;
	batch.frameRate = (std::max)(1u, desc.frameRate);
	batch.originX = desc.spriteOriginX;
	batch.originY = desc.spriteOriginY;
	batch.sprite = std::make_unique<DX9GF::StaticSprite>(desc.texture);
	batch.sprite->SetOrigin(desc.spriteOriginX, desc.spriteOriginY);
	batches.push_back(std::move(batch));
	return static_cast<unsigned int>(batches.size() - 1);
}

void Demo::ProjectileSystem::Spawn(const std::shared_ptr<Player>& player, const ProjectileDesc& desc)
{
	target = player;

	transforms.push_back({ desc.x, desc.y, 0.f });

	MotionComponent motion{};
	motion.behavior = desc.behavior;
	motion.trajectory = desc.trajectory;
	motion.velocity = desc.velocity;
	motion.turnSpeed = desc.turnSpeed;
	motion.amplitude = desc.amplitude;
	motion.frequency = desc.frequency;
	motion.baseX = desc.x;
	motion.baseY = desc.y;
	motion.radius = 0.f;
	motion.angle = desc.initialAngle;
	motion.radialSpeed = desc.radialSpeed;
	motion.angularVelocity = desc.angularVelocity;
	motion.returnAcceleration = desc.returnAcceleration;
	motions.push_back(motion);

	lifetimes.push_back({ desc.delay, 0.f, desc.decayTime });

	const bool destroyOnHit = desc.behavior == ProjectileBehavior::Spiral
		|| desc.behavior == ProjectileBehavior::Boomerang;
	combats.push_back({
	desc.damage, desc.colliderWidth, desc.colliderHeight, destroyOnHit,
	desc.hasStatusEffect, desc.randomizeStatusEffect,
	desc.statusEffectType, desc.statusEffectValue, desc.statusEffectDuration,
	desc.altStatusEffectType, desc.altStatusEffectValue, desc.altStatusEffectDuration,
	false
		});

	RenderComponent render{};
	render.batchIndex = GetOrCreateBatch(desc);
	render.frameIndex = 0;
	render.frameDelta = 0;
	// Spiral and boomerang projectiles stayed hidden until their delay
	// expired; the other behaviors were always drawn.
	render.visibleDuringDelay = desc.behavior != ProjectileBehavior::Spiral
		&& desc.behavior != ProjectileBehavior::Boomerang;
	renders.push_back(render);

	SplitComponent split{};
	split.trigger = desc.splitCount > 0 ? desc.splitTrigger : ProjectileSplitTrigger::None;
	split.count = desc.splitCount;
	split.targetX = desc.splitTargetX;
	split.targetY = desc.splitTargetY;
	split.velocity = desc.splitVelocity;
	split.spread = desc.splitSpread;
	split.angleOffset = desc.splitAngleOffset;
	split.randomizeAngle = desc.splitRandomizeAngle;
	split.decayTime = desc.splitDecayTime;
	split.damage = desc.splitDamage;
	split.generations = desc.splitGenerations;
	splits.push_back(split);

	std::unique_ptr<DX9GF::ParticleSystem> emitter;
	if (desc.ghostTexture != nullptr) {
		emitter = std::make_unique<DX9GF::ParticleSystem>(desc.ghostTexture, 16);
		if (desc.ghostHasSrcRect) emitter->SetSrcRect(desc.ghostSrcRect);
		emitter->SetOrigin(desc.ghostOriginX, desc.ghostOriginY);
		DX9GF::ConfigureGhostTrailEmitter(*emitter);
	}
	else if (desc.behavior == ProjectileBehavior::Straight || desc.behavior == ProjectileBehavior::Homing) {
		if (!trailTexture) {
			trailTexture = std::make_shared<DX9GF::Texture>(desc.texture->GetGraphicsDevice());
			trailTexture->CreatePlainTexture(0xFFFFFFFF, 4, 4);
		}
		emitter = std::make_unique<DX9GF::ParticleSystem>(trailTexture.get(), 32);
		emitter->SetOrigin(2, 2);
		DX9GF::ConfigureTrailEmitter(*emitter);
	}
	emitters.push_back(std::move(emitter));

	dead.push_back(0);
}

void Demo::ProjectileSystem::Spawn(const std::shared_ptr<Player>& player, const LaserDesc& desc)
{
	target = player;

	LaserComponent laser{};
	laser.x = desc.x;
	laser.y = desc.y;
	laser.angle = desc.angle;
	laser.length = desc.length;
	laser.thickness = desc.thickness;
	laser.warnThickness = desc.warnThickness;
	laser.glowThickness = desc.glowThickness;
	laser.delay = desc.delay;
	laser.warnTime = desc.warnTime;
	laser.fireTime = desc.fireTime;
	laser.elapsed = 0.f;
	laser.warnColor = desc.warnColor;
	laser.glowColor = desc.glowColor;
	laser.coreColor = desc.coreColor;
	lasers.push_back(laser);

	// The beam keeps burning through a hit, so it never destroys on contact; the
	// player's own invincibility window is what limits how often it can land.
	laserCombats.push_back({
		desc.damage, desc.length, desc.thickness, false,
		desc.hasStatusEffect, desc.randomizeStatusEffect,
		desc.statusEffectType, desc.statusEffectValue, desc.statusEffectDuration,
		desc.altStatusEffectType, desc.altStatusEffectValue, desc.altStatusEffectDuration,
		false
		});

	laserDead.push_back(0);
}

void Demo::ProjectileSystem::ApplyHit(Player& player, CombatComponent& combat)
{
	player.TakeDamage(combat.damage);

	if (!combat.hasStatusEffect || combat.statusApplied) {
		return;
	}
	combat.statusApplied = true;

	ModifierType type = combat.statusEffectType;
	int duration = combat.statusEffectDuration;
	float value = combat.statusEffectValue;
	if (combat.randomizeStatusEffect) {
		bool pickAlt = (RNG::Range(1, 2) == 2);
		if (pickAlt) {
			type = combat.altStatusEffectType;
			duration = combat.altStatusEffectDuration;
			value = combat.altStatusEffectValue;
		}
	}
	switch (type) {
	case ModifierType::Burn:
	case ModifierType::Marked:
	case ModifierType::Freeze:
		player.AddStackingModifier(type, duration, value, false);
		break;
	default:
		player.AddModifier(type, duration, value, false);
	}
}

void Demo::ProjectileSystem::DestroyLaserAt(size_t index)
{
	const size_t last = lasers.size() - 1;
	if (index != last) {
		lasers[index] = lasers[last];
		laserCombats[index] = laserCombats[last];
		laserDead[index] = laserDead[last];
	}
	lasers.pop_back();
	laserCombats.pop_back();
	laserDead.pop_back();
}

void Demo::ProjectileSystem::DestroyAt(size_t index)
{
	const size_t last = transforms.size() - 1;
	if (index != last) {
		transforms[index] = transforms[last];
		motions[index] = motions[last];
		lifetimes[index] = lifetimes[last];
		combats[index] = combats[last];
		renders[index] = renders[last];
		splits[index] = splits[last];
		emitters[index] = std::move(emitters[last]);
		dead[index] = dead[last];
	}
	transforms.pop_back();
	motions.pop_back();
	lifetimes.pop_back();
	combats.pop_back();
	renders.pop_back();
	splits.pop_back();
	emitters.pop_back();
	dead.pop_back();
}

void Demo::ProjectileSystem::QueueBurst(size_t index)
{
	PendingBurst burst{};
	burst.x = transforms[index].x;
	burst.y = transforms[index].y;
	burst.baseAngle = transforms[index].rotation;
	burst.split = splits[index];
	burst.combat = combats[index];
	burst.batchIndex = renders[index].batchIndex;
	pendingBursts.push_back(burst);
}

void Demo::ProjectileSystem::EmitBurst(const PendingBurst& burst)
{
	const int count = burst.split.count;
	if (count <= 0) {
		return;
	}

	float offset = burst.split.angleOffset;
	if (burst.split.randomizeAngle) {
		offset += RNG::Range(0.f, 2.f * pi);
	}

	// A spread of 0 means an even ring; anything else is a fan of that width
	// centred on the direction the parent was travelling.
	const bool isRing = burst.split.spread <= floatEpsilon;
	const float step = isRing
		? (2.f * pi / static_cast<float>(count))
		: (count > 1 ? burst.split.spread / static_cast<float>(count - 1) : 0.f);
	const float first = isRing
		? 0.f
		: -burst.split.spread * 0.5f;

	// Children only chain if there is a decay timer for the next burst to fire on;
	// without one an OnDecay trigger would never come due.
	const bool chains = burst.split.generations > 1 && burst.split.decayTime != UNSPECIFIED;

	for (int i = 0; i < count; i++) {
		const float angle = burst.baseAngle + offset + first + step * static_cast<float>(i);

		transforms.push_back({ burst.x, burst.y, angle });

		MotionComponent motion{};
		motion.behavior = ProjectileBehavior::Straight;
		motion.trajectory = D3DXVECTOR2(std::cos(angle), std::sin(angle));
		motion.velocity = burst.split.velocity;
		motion.baseX = burst.x;
		motion.baseY = burst.y;
		motions.push_back(motion);

		lifetimes.push_back({ 0.f, 0.f, burst.split.decayTime });

		CombatComponent combat = burst.combat;
		if (burst.split.damage != UNSPECIFIED) {
			combat.damage = burst.split.damage;
		}
		combat.destroyOnHit = false;
		combat.statusApplied = false;
		combats.push_back(combat);

		RenderComponent render{};
		render.batchIndex = burst.batchIndex;
		render.frameIndex = 0;
		render.frameDelta = 0;
		render.visibleDuringDelay = true;
		renders.push_back(render);

		SplitComponent split{};
		split.trigger = chains ? ProjectileSplitTrigger::OnDecay : ProjectileSplitTrigger::None;
		split.count = burst.split.count;
		split.velocity = burst.split.velocity;
		split.spread = burst.split.spread;
		split.angleOffset = burst.split.angleOffset;
		split.randomizeAngle = burst.split.randomizeAngle;
		split.decayTime = burst.split.decayTime;
		split.damage = burst.split.damage;
		split.generations = burst.split.generations - 1;
		splits.push_back(split);

		// Children travel straight, so they get the same white trail Spawn gives
		// any straight projectile that was not handed a ghost sprite.
		std::unique_ptr<DX9GF::ParticleSystem> emitter;
		DX9GF::Texture* parentTexture = batches[burst.batchIndex].texture;
		if (parentTexture != nullptr) {
			if (!trailTexture) {
				trailTexture = std::make_shared<DX9GF::Texture>(parentTexture->GetGraphicsDevice());
				trailTexture->CreatePlainTexture(0xFFFFFFFF, 4, 4);
			}
			emitter = std::make_unique<DX9GF::ParticleSystem>(trailTexture.get(), 32);
			emitter->SetOrigin(2, 2);
			DX9GF::ConfigureTrailEmitter(*emitter);
		}
		emitters.push_back(std::move(emitter));

		dead.push_back(0);
	}
}

void Demo::ProjectileSystem::Update(unsigned long long deltaTime)
{
	if (transforms.empty() && lasers.empty()) {
		return;
	}

	const float dtSec = deltaTime / 1000.f;
	auto player = target.lock();

	RectShape playerRect;
	bool hasPlayerRect = false;
	if (player) {
		if (auto collider = player->GetCollider().lock()) {
			hasPlayerRect = BuildRectShape(*collider, playerRect);
		}
	}

	const size_t count = transforms.size();
	for (size_t i = 0; i < count; i++) {
		auto& life = lifetimes[i];
		if (life.decayTime != UNSPECIFIED && life.elapsed >= life.decayTime) {
			if (splits[i].trigger == ProjectileSplitTrigger::OnDecay) {
				QueueBurst(i);
			}
			dead[i] = 1;
			continue;
		}

		auto& tr = transforms[i];
		auto& motion = motions[i];
		if (life.elapsed >= life.delay) {
			switch (motion.behavior) {
			case ProjectileBehavior::Straight: {
				tr.x += motion.trajectory.x * motion.velocity * dtSec;
				tr.y += motion.trajectory.y * motion.velocity * dtSec;
				tr.rotation = std::atan2(motion.trajectory.y, motion.trajectory.x);
				break;
			}
			case ProjectileBehavior::Homing: {
				if (player) {
					auto [playerX, playerY] = player->GetWorldPosition();
					D3DXVECTOR2 idealTrajectory{ playerX - tr.x, playerY - tr.y };
					D3DXVec2Normalize(&idealTrajectory, &idealTrajectory);
					// gradually steer (lerp) toward the player
					motion.trajectory.x += (idealTrajectory.x - motion.trajectory.x) * motion.turnSpeed * dtSec;
					motion.trajectory.y += (idealTrajectory.y - motion.trajectory.y) * motion.turnSpeed * dtSec;
					D3DXVec2Normalize(&motion.trajectory, &motion.trajectory);
				}
				tr.x += motion.trajectory.x * motion.velocity * dtSec;
				tr.y += motion.trajectory.y * motion.velocity * dtSec;
				tr.rotation = std::atan2(motion.trajectory.y, motion.trajectory.x);
				break;
			}
			case ProjectileBehavior::SineWave: {
				motion.baseX += motion.trajectory.x * motion.velocity * dtSec;
				motion.baseY += motion.trajectory.y * motion.velocity * dtSec;
				D3DXVECTOR2 perpendicular(-motion.trajectory.y, motion.trajectory.x);
				float moveTime = life.elapsed - life.delay;
				float waveOffset = std::sin(moveTime * motion.frequency) * motion.amplitude;
				tr.x = motion.baseX + perpendicular.x * waveOffset;
				tr.y = motion.baseY + perpendicular.y * waveOffset;

				float waveDerivative = std::cos(moveTime * motion.frequency) * motion.frequency * motion.amplitude;
				D3DXVECTOR2 instantaneousVelocity = motion.trajectory * motion.velocity + perpendicular * waveDerivative;
				if (D3DXVec2LengthSq(&instantaneousVelocity) > 0.0001f) {
					tr.rotation = std::atan2(instantaneousVelocity.y, instantaneousVelocity.x);
				}
				break;
			}
			case ProjectileBehavior::Spiral: {
				motion.radius += motion.radialSpeed * dtSec;
				motion.angle += motion.angularVelocity * dtSec;
				tr.x = motion.baseX + motion.radius * std::cos(motion.angle);
				tr.y = motion.baseY + motion.radius * std::sin(motion.angle);
				tr.rotation = motion.angle;
				break;
			}
			case ProjectileBehavior::Boomerang: {
				motion.velocity -= motion.returnAcceleration * dtSec;
				D3DXVECTOR2 moveVector = motion.trajectory * motion.velocity;
				if (D3DXVec2LengthSq(&moveVector) > 0.0001f) {
					tr.rotation = std::atan2(moveVector.y, moveVector.x);
				}
				tr.x += motion.trajectory.x * motion.velocity * dtSec;
				tr.y += motion.trajectory.y * motion.velocity * dtSec;
				break;
			}
			}
		}

		// An airburst projectile bursts the frame it crosses the plane through its
		// target point, so a shot that overshoots still splits exactly on target.
		auto& split = splits[i];
		if (split.trigger == ProjectileSplitTrigger::OnArrival && life.elapsed >= life.delay) {
			const Vec2 heading{ motion.trajectory.x, motion.trajectory.y };
			const Vec2 toTarget{ split.targetX - tr.x, split.targetY - tr.y };
			if (LengthSq(heading) > floatEpsilon && Dot(toTarget, heading) <= 0.f) {
				QueueBurst(i);
				dead[i] = 1;
				continue;
			}
		}

		auto& combat = combats[i];
		if (hasPlayerRect && EllipseOverlapsRect(
			playerRect, tr.x, tr.y, tr.rotation,
			combat.colliderWidth * 0.5f, combat.colliderHeight * 0.5f)) {
			ApplyHit(*player, combat);

			if (combat.destroyOnHit) {
				dead[i] = 1;
			}
		}

		life.elapsed += dtSec;

		if (emitters[i]) {
			emitters[i]->Update(deltaTime, tr.x, tr.y, tr.rotation, 1.f, 1.f, 0xFFFFFFFF,
				!dead[i] && life.elapsed >= life.delay);
		}
	}

	for (size_t i = 0; i < transforms.size();) {
		if (dead[i]) {
			DestroyAt(i);
		}
		else {
			++i;
		}
	}

	// Deferred until the loop above is done with the component arrays; the burst
	// carries its own copy of everything the children need.
	for (const auto& burst : pendingBursts) {
		EmitBurst(burst);
	}
	pendingBursts.clear();

	// Beams only damage once they have finished telegraphing, and they keep
	// damaging for as long as they burn.
	for (size_t i = 0; i < lasers.size(); i++) {
		auto& laser = lasers[i];
		if (laser.elapsed >= laser.delay + laser.warnTime + laser.fireTime) {
			laserDead[i] = 1;
			continue;
		}

		if (laser.elapsed >= laser.delay + laser.warnTime && hasPlayerRect && ObbOverlapsRect(
			playerRect, laser.x, laser.y, laser.angle,
			laser.length * 0.5f, laser.thickness * 0.5f)) {
			ApplyHit(*player, laserCombats[i]);
		}

		laser.elapsed += dtSec;
	}

	for (size_t i = 0; i < lasers.size();) {
		if (laserDead[i]) {
			DestroyLaserAt(i);
		}
		else {
			++i;
		}
	}
}

void Demo::ProjectileSystem::Draw(DX9GF::GraphicsDevice* graphicsDevice, const DX9GF::Camera& camera, unsigned long long deltaTime)
{
	if (transforms.empty() && lasers.empty()) {
		return;
	}

	// Beams are the backdrop of a pattern, so they go under everything else.
	if (!lasers.empty() && graphicsDevice) {
		graphicsDevice->SetAlphaBlending(true);
		for (const auto& laser : lasers) {
			if (laser.elapsed < laser.delay) {
				continue;
			}
			const float phase = laser.elapsed - laser.delay;
			if (phase < laser.warnTime) {
				// A thin sight line pulsing about three times a second.
				const float pulse = std::sin(phase * 20.f) * 0.5f + 0.5f;
				DrawBeam(graphicsDevice, camera, laser.x, laser.y, laser.angle,
					laser.length, laser.warnThickness, ScaleAlpha(laser.warnColor, pulse));
			}
			else {
				DrawBeam(graphicsDevice, camera, laser.x, laser.y, laser.angle,
					laser.length, laser.glowThickness, laser.glowColor);
				DrawBeam(graphicsDevice, camera, laser.x, laser.y, laser.angle,
					laser.length, laser.thickness, laser.coreColor);
			}
		}
		graphicsDevice->SetAlphaBlending(false);
	}

	// Trails and afterimages go underneath the projectiles themselves.
	for (auto& emitter : emitters) {
		if (emitter) {
			emitter->Draw(camera, deltaTime);
		}
	}

	const size_t count = transforms.size();
	for (unsigned int b = 0; b < batches.size(); b++) {
		auto& batch = batches[b];
		bool begun = false;
		for (size_t i = 0; i < count; i++) {
			auto& render = renders[i];
			if (render.batchIndex != b) {
				continue;
			}
			if (!render.visibleDuringDelay && lifetimes[i].elapsed < lifetimes[i].delay) {
				continue;
			}
			if (!begun) {
				batch.sprite->Begin();
				begun = true;
			}
			if (!batch.frames.empty()) {
				// Same frame-stepping rule as AnimatedSprite
				render.frameDelta += deltaTime;
				const unsigned long long framePeriod = 1000 / batch.frameRate;
				if (render.frameDelta > framePeriod) {
					render.frameIndex += static_cast<unsigned int>(render.frameDelta / framePeriod);
					render.frameDelta = 0;
				}
				render.frameIndex %= batch.frames.size();
				batch.sprite->SetSrcRect(batch.frames[render.frameIndex]);
			}
			const auto& tr = transforms[i];
			batch.sprite->SetPosition(tr.x, tr.y);
			batch.sprite->SetRotation(tr.rotation);
			batch.sprite->Draw(camera, deltaTime);
		}
		if (begun) {
			batch.sprite->End();
		}
	}

	if (DX9GF::ICollider::drawCollider && graphicsDevice) {
		for (const auto& laser : lasers) {
			DrawBeam(graphicsDevice, camera, laser.x, laser.y, laser.angle,
				laser.length, laser.thickness, 0x550000FF, false);
		}
		for (size_t i = 0; i < count; i++) {
			const auto& tr = transforms[i];
			const auto& combat = combats[i];
			graphicsDevice->DrawEllipse(
				camera,
				tr.x,
				tr.y,
				combat.colliderWidth,
				combat.colliderHeight,
				tr.rotation,
				1.f,
				1.f,
				combat.colliderWidth * 0.5f,
				combat.colliderHeight * 0.5f,
				0x550000FF,
				false
			);
		}
	}
}
