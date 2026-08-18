#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Player.h"
#include "GameItems.h"
#include <memory>
#include <vector>

namespace Demo {
	constexpr float UNSPECIFIED = -1;

	enum class ProjectileBehavior : unsigned char {
		Straight,
		Homing,
		SineWave,
		Spiral,
		Boomerang
	};

	// When an airburst projectile tears itself into a ring of children.
	// OnArrival fires the moment the projectile crosses its target point,
	// OnDecay fires when its decay timer runs out (also the trigger every
	// chained generation uses, since children have no target of their own).
	enum class ProjectileSplitTrigger : unsigned char {
		None,
		OnArrival,
		OnDecay
	};

	// Plain-data spawn description. The fluent setters mirror the old
	// per-class builders so spawn sites read the same as before.
	struct ProjectileDesc {
		// Render
		DX9GF::Texture* texture = nullptr;
		std::vector<RECT> frames; // empty = draw the whole texture
		unsigned int frameRate = 12;
		float spriteOriginX = 0.f;
		float spriteOriginY = 0.f;
		// Placement / collision (ellipse, origin at center)
		float x = 0.f;
		float y = 0.f;
		float colliderWidth = 0.f;
		float colliderHeight = 0.f;
		// Common
		ProjectileBehavior behavior = ProjectileBehavior::Straight;
		D3DXVECTOR2 trajectory{ 1.f, 0.f };
		float velocity = 0.f;
		float delay = 0.f;
		float decayTime = UNSPECIFIED;
		float damage = 0.f;
		// Homing
		float turnSpeed = 0.f;
		// Sine wave
		float amplitude = 0.f;
		float frequency = 0.f;
		// Spiral
		float initialAngle = 0.f;
		float radialSpeed = 0.f;
		float angularVelocity = 0.f;
		// Boomerang
		float returnAcceleration = 0.f;
		// Airburst: the projectile bursts into a ring of children that inherit its
		// sprite, collider and status effect, then travel straight outward.
		ProjectileSplitTrigger splitTrigger = ProjectileSplitTrigger::None;
		int splitCount = 0;
		float splitTargetX = 0.f;
		float splitTargetY = 0.f;
		float splitVelocity = 0.f;
		float splitSpread = 0.f;       // arc in radians; 0 = evenly spaced full circle
		float splitAngleOffset = 0.f;  // rotates the ring off the parent's heading
		bool splitRandomizeAngle = false;
		float splitDecayTime = UNSPECIFIED;
		float splitDamage = UNSPECIFIED; // UNSPECIFIED = inherit the parent's damage
		int splitGenerations = 1;      // >1 lets the children burst again on decay
		// Ghost trail (afterimages reusing the projectile's own texture)
		DX9GF::Texture* ghostTexture = nullptr;
		RECT ghostSrcRect{};
		bool ghostHasSrcRect = false;
		float ghostOriginX = 0.f;
		float ghostOriginY = 0.f;
		// Status effect on hit
		bool hasStatusEffect = false;
		bool randomizeStatusEffect = false;
		ModifierType statusEffectType = ModifierType::Freeze;
		float statusEffectValue = 0.f;
		int statusEffectDuration = 0;
		ModifierType altStatusEffectType = ModifierType::Burn;
		float altStatusEffectValue = 0.f;
		int altStatusEffectDuration = 0;

		ProjectileDesc(
			DX9GF::Texture* texture,
			float spriteOriginX,
			float spriteOriginY,
			float colliderWidth,
			float colliderHeight,
			float x,
			float y
		) : texture(texture),
			spriteOriginX(spriteOriginX),
			spriteOriginY(spriteOriginY),
			colliderWidth(colliderWidth),
			colliderHeight(colliderHeight),
			x(x),
			y(y) {}
		ProjectileDesc(
			DX9GF::Texture* texture,
			std::vector<RECT> frames,
			unsigned int frameRate,
			float spriteOriginX,
			float spriteOriginY,
			float colliderWidth,
			float colliderHeight,
			float x,
			float y
		) : texture(texture),
			frames(std::move(frames)),
			frameRate(frameRate),
			spriteOriginX(spriteOriginX),
			spriteOriginY(spriteOriginY),
			colliderWidth(colliderWidth),
			colliderHeight(colliderHeight),
			x(x),
			y(y) {}

		ProjectileDesc& SetTrajectory(D3DXVECTOR2 trajectory);
		ProjectileDesc& SetTargetPosition(float targetX, float targetY);
		ProjectileDesc& SetVelocity(float velocity);
		ProjectileDesc& SetDelay(float delay);
		ProjectileDesc& SetDecayTime(float decayTime);
		ProjectileDesc& SetDamage(float damage);
		ProjectileDesc& SetHoming(float turnSpeed);
		ProjectileDesc& SetWave(float amplitude, float frequency);
		ProjectileDesc& SetSpiralParams(float initialAngle, float radialSpeed, float angularVelocity);
		ProjectileDesc& SetInitialVelocity(float velocity);
		ProjectileDesc& SetReturnAcceleration(float acceleration);
		// Aims the projectile at (targetX, targetY) and arms it to burst there.
		ProjectileDesc& SetSplitOnArrival(float targetX, float targetY, int count, float velocity);
		// Bursts wherever the projectile happens to be when its decay timer expires.
		ProjectileDesc& SetSplitOnDecay(int count, float velocity);
		ProjectileDesc& SetSplitSpread(float spread, float angleOffset = 0.f);
		ProjectileDesc& SetSplitRandomAngle(bool randomize = true);
		ProjectileDesc& SetSplitDecayTime(float decayTime);
		ProjectileDesc& SetSplitDamage(float damage);
		ProjectileDesc& SetSplitGenerations(int generations);
		ProjectileDesc& SetGhostSprite(DX9GF::Texture* texture, RECT srcRect, float originX, float originY);
		ProjectileDesc& SetStatusEffect(ModifierType type, float value, int duration);
		ProjectileDesc& SetRandomStatusEffect(
			ModifierType typeA, float valueA, int durationA,
			ModifierType typeB, float valueB, int durationB
		);
	};

	// Data-oriented replacement for the old IProjectile class hierarchy.
	// Entities are dense indices into the parallel component arrays below;
	// destruction swap-pops, so indices never escape this class. Behaviors
	// are data (MotionComponent + switch), not virtual dispatch, and
	// rendering batches all projectiles that share a texture/frame set into
	// a single sprite Begin/End instead of one per projectile.
	class ProjectileSystem {
	private:
		struct TransformComponent {
			float x, y, rotation;
		};
		struct MotionComponent {
			ProjectileBehavior behavior;
			D3DXVECTOR2 trajectory;
			float velocity;
			float turnSpeed;
			float amplitude, frequency;
			float baseX, baseY;      // sine wave carrier / spiral origin
			float radius, angle;     // spiral state
			float radialSpeed, angularVelocity;
			float returnAcceleration;
		};
		struct LifetimeComponent {
			float delay, elapsed, decayTime;
		};
		struct CombatComponent {
			float damage;
			float colliderWidth, colliderHeight;
			bool destroyOnHit;
			bool hasStatusEffect;
			bool randomizeStatusEffect;
			ModifierType statusEffectType;
			float statusEffectValue;
			int statusEffectDuration;
			ModifierType altStatusEffectType;
			float altStatusEffectValue;
			int altStatusEffectDuration;
			bool statusApplied = false;
		};
		struct SplitComponent {
			ProjectileSplitTrigger trigger;
			int count;
			float targetX, targetY;
			float velocity;
			float spread, angleOffset;
			bool randomizeAngle;
			float decayTime;
			float damage;
			int generations;
		};
		struct RenderComponent {
			unsigned int batchIndex;
			unsigned int frameIndex;
			unsigned long long frameDelta; // ms accumulator, mirrors AnimatedSprite
			bool visibleDuringDelay;
		};

		// One batch per unique (texture, frames, frame rate, origin); every
		// projectile in a batch is drawn through the same sprite object.
		struct SpriteBatch {
			DX9GF::Texture* texture = nullptr;
			std::vector<RECT> frames;
			unsigned int frameRate = 12;
			float originX = 0.f;
			float originY = 0.f;
			std::unique_ptr<DX9GF::StaticSprite> sprite;
		};

		// Components (parallel arrays; index = entity)
		std::vector<TransformComponent> transforms;
		std::vector<MotionComponent> motions;
		std::vector<LifetimeComponent> lifetimes;
		std::vector<CombatComponent> combats;
		std::vector<RenderComponent> renders;
		std::vector<SplitComponent> splits;
		std::vector<std::unique_ptr<DX9GF::ParticleSystem>> emitters;
		std::vector<unsigned char> dead;

		// A burst snapshots everything its children need, so the children can be
		// created after the update loop has finished. Spawning them inline would
		// push onto the component arrays the loop is iterating and invalidate the
		// references it holds.
		struct PendingBurst {
			float x, y;
			float baseAngle;
			SplitComponent split;
			CombatComponent combat;
			unsigned int batchIndex;
		};
		std::vector<PendingBurst> pendingBursts;

		std::vector<SpriteBatch> batches;
		std::weak_ptr<Player> target;
		std::shared_ptr<DX9GF::Texture> trailTexture; // shared 4x4 white square

		unsigned int GetOrCreateBatch(const ProjectileDesc& desc);
		void DestroyAt(size_t index);
		void QueueBurst(size_t index);
		void EmitBurst(const PendingBurst& burst);
	public:
		void Spawn(const std::shared_ptr<Player>& player, const ProjectileDesc& desc);
		void Update(unsigned long long deltaTime);
		void Draw(DX9GF::GraphicsDevice* graphicsDevice, const DX9GF::Camera& camera, unsigned long long deltaTime);
		bool IsEmpty() const { return transforms.empty(); }
		size_t Count() const { return transforms.size(); }
	};
}
