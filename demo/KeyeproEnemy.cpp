#include "pch.h"
#include "KeyeproEnemy.h"
#include "resource.h"
#include "KeyeEnemy.h"
#include "PopUpMessage.h"
#include "RNG.h"
#include <algorithm>

void Demo::KeyeproEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"assets/boss-Sheet.png");
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 128, 128, 29), 6);
	sprite->SetOrigin(64, 64);
	sprite->SetScale(2.f);

	projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	projTexture->LoadTexture(L"assets/bossprojectile-Sheet.png");
	projFrames = DX9GF::Utils::CreateRectsHorizontal(0, 0, 32, 32, 5);
	shardTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	shardTexture->LoadTexture(L"assets/bossshard-Sheet.png");
	shardFrames = DX9GF::Utils::CreateRectsHorizontal(0, 0, 16, 16, 6);

	SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
	InitCardSpawnTrigger(camera, 128.f, 128.f);
}

void Demo::KeyeproEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) {
	if (sprite) {
		sprite->Begin();
		auto [x, y] = GetWorldPosition();
		sprite->SetPosition(x, y);
		sprite->Draw(*camera, deltaTime);
		sprite->End();
	}
	IEnemy::Draw(graphicsDevice, camera, deltaTime);
}

int Demo::KeyeproEnemy::GetRandomPattern() {
	return RNG::Range(1, 6);
}

void Demo::KeyeproEnemy::OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn, std::vector<std::shared_ptr<IEnemy>>* enemies, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	this->player = player;

	float desperato = 1.f - this->GetHealth() / this->GetMaxHealth(); // Desperation factor, ranges from 0 to 1 as health decreases

	if (enemies != nullptr && graphicsDevice != nullptr && camera != nullptr) {
		constexpr float MINION_SPAWN_CHANCE = 0.5f;
		if (RNG::Range(0.f, 1.f) <= std::max(MINION_SPAWN_CHANCE, desperato)) {
			size_t keyeCount = 0;
			for (const auto& enemy : *enemies) {
				if (!enemy || enemy->IsDead())
				{
					continue;
				}
				if (std::dynamic_pointer_cast<KeyeEnemy>(enemy))
				{
					++keyeCount;
				}
			}

			constexpr size_t MAX_KEYE_ENEMIES = 4;
			const size_t spawnCount = (keyeCount < MAX_KEYE_ENEMIES)
				? (std::min)(static_cast<size_t>(1), MAX_KEYE_ENEMIES - keyeCount)
				: 0;

			if (spawnCount > 0 && onRequestSpawnEnemy) {
				// This runs as a deferred command, which the scene drains from inside its loop over
				// the enemy list. Hand the minion to the scene instead of appending to that list
				// here: a push_back would reallocate the vector the loop is iterating and leave it
				// reading freed memory. The scene inserts the minion once the loop has finished.
				CastAbility([this, spawnCount, graphicsDevice, camera]() {
					for (size_t i = 0; i < spawnCount; ++i) {
						float spawnX = 1200;
						float spawnY = 0;
						auto minion = std::make_shared<KeyeEnemy>(this->transformManager, 40.0f, spawnX, spawnY);
						minion->Init(graphicsDevice, camera);
						minion->SetOnRequestEnemyCard(this->onRequestEnemyCard);
						this->onRequestSpawnEnemy(minion);
					}
				}, popUpMessage, L"The boss spawned a minion!");
			}
		}
	}

	constexpr float LOCK_CHANCE = 0.4f;
	if (RNG::Range(0.f, 1.f) <= std::max(LOCK_CHANCE, desperato)) {
		CastAbility([this]() {
			if (this->onRequestLockCard) this->onRequestLockCard(2);
			}, popUpMessage, L"Boss locks your mind!");
	}

	if (RNG::Range(0.f, 1.f) <= desperato) {
		CastAbility([this]() {
			this->Heal(45.f);
			}, popUpMessage, L"Boss regenerates 45HP!");
	}

	if (RNG::Range(0.f, 1.f) <= desperato) {
		CastAbility([this]() { 
			this->AddModifier(ModifierType::BuffDefense, 2, 30.0f, true); 
			}, popUpMessage, L"Boss fortifies itself with 30DEF!");
	}

	if (currentTurn % 2 == 1 || desperato > 0.6f) {
		auto abilityRoll = RNG::Range(1, 3);
		if (abilityRoll == 1) {
			CastAbility([this]() { this->player.lock()->AddModifier(ModifierType::Vulnerable, 1, 5.0f, false); }, popUpMessage, L"Boss lowers your defenses!");
		}
		else if (abilityRoll == 2) {
			CastAbility([this]() { this->player.lock()->AddModifier(ModifierType::Weak, 2, 5.0f, false); }, popUpMessage, L"Boss weakens your attacks!");
		}
		else {
			CastAbility([this]() { this->AddModifier(ModifierType::BuffDamage, 2, 3.0f, true); }, popUpMessage, L"Boss enters a frenzy!");
		}
	}
}

void Demo::KeyeproEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) {
	this->player = player;

	float baseDamage = 5.f;

	if (enemies != nullptr && graphicsDevice != nullptr && camera != nullptr) {
		int patternId = GetSmartRandomPattern(1, 5);
		if (patternId == 1) PatternTargetedSniping(baseDamage);
		else if (patternId == 2) PatternEcholocation(baseDamage);
		else if (patternId == 3) PatternSwoopBite(baseDamage);
		else if (patternId == 4) PatternShatterVolley(baseDamage);
		else if (patternId == 5) PatternFanning(CalculateOutgoingDamage(1.f));
	}
}

void Demo::KeyeproEnemy::PatternTargetedSniping(float baseDamage) {
	auto currentPosition = std::make_shared<int>(0);

	for (int i = 0; i < 100; i++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, baseDamage, currentPosition](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				float finalDamage = this->CalculateOutgoingDamage(baseDamage);
				*currentPosition = *currentPosition % 4 + 1; // Cycle through positions 1 to 4
				float x, y;
				switch (*currentPosition) {
				case 1: // Top
					x = 0;
					y = -350.f;
					break;
				case 2: // Bottom
					x = 0;
					y = 350.f;
					break;
				case 3: // Left
					x = -350.f;
					y = 0;
					break;
				case 4: // Right
					x = 350.f;
					y = 0;
					break;
				}

				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), projFrames, 12, 16, 16, 16, 16, x, y)
					.SetTargetPosition(lock->GetCollider().lock()->GetWorldX(), lock->GetCollider().lock()->GetWorldY())
					.SetVelocity(200.f)
					.SetDelay(0.f)
					.SetDecayTime(4.f)
					.SetDamage(finalDamage)
					.SetStatusEffect(ModifierType::Poison, 0.f, 1)
				);
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.1f));
	}
}

void Demo::KeyeproEnemy::PatternEcholocation(float baseDamage) {
	const int BULLETS = 10;
	const int VELOCITY = 180;
	const int SPACING = 96;
	std::shared_ptr<int> yOffset = std::make_shared<int>(0);
	auto rightAttack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage, yOffset, SPACING](std::function<void(void)> markFinished) {
		float finalDamage = this->CalculateOutgoingDamage(baseDamage);
		for (int i = 0; i < BULLETS; i++) {
			if (auto lock = this->player.lock()) {
				float startY = (i - BULLETS / 2.f) * SPACING + *yOffset;
				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), projFrames, 12, 16, 16, 16, 16, 320, startY)
					.SetTrajectory(D3DXVECTOR2(-1, 0))
					.SetWave(50.f, 4.f)
					.SetDelay(i * 0.1f)
					.SetDecayTime(4.f)
					.SetVelocity(VELOCITY)
					.SetDamage(finalDamage)
					.SetGhostSprite(projTexture.get(), projFrames.front(), 16, 16)
					.SetStatusEffect(ModifierType::Freeze, 0.05f, 1)
				);
			}
		}
		*yOffset += 16;
		*yOffset %= SPACING;
		markFinished();
		});
	auto leftAttack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage, yOffset](std::function<void(void)> markFinished) {
		float finalDamage = this->CalculateOutgoingDamage(baseDamage);
		for (int i = 0; i < BULLETS; i++) {
			if (auto lock = this->player.lock()) {
				float startY = (i - BULLETS / 2.f) * SPACING + *yOffset;
				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), projFrames, 12, 16, 16, 16, 16, -320, startY)
					.SetTrajectory(D3DXVECTOR2(1, 0))
					.SetWave(50.f, 4.f)
					.SetDelay(i * 0.1f)
					.SetDecayTime(4.f)
					.SetVelocity(VELOCITY)
					.SetDamage(finalDamage)
					.SetGhostSprite(projTexture.get(), projFrames.front(), 16, 16)
				);
			}
		}
		*yOffset += 16;
		*yOffset %= SPACING;
		markFinished();
		});
	for (int i = 0; i < 40; i++) {
		if (RNG::Range(1, 2) == 1) commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*leftAttack));
		else commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*rightAttack));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
	}
}

void Demo::KeyeproEnemy::PatternSwoopBite(float baseDamage) {
	const int BULLET_COUNT = 8;
	const float ANGLE_STEP = 0.3f;
	const float INITIAL_VELOCITY = 370.f;
	auto rightAttack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage, BULLET_COUNT, ANGLE_STEP, INITIAL_VELOCITY](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			float finalDamage = this->CalculateOutgoingDamage(baseDamage);
			auto [px, py] = lock->GetCollider().lock()->GetWorldPosition();
            float x = 320.f;
            float y = 0;
            float dx = px - x;
            float dy = py - y;
			float baseAngle = std::atan2(dy, dx);

			for (int i = 0; i < BULLET_COUNT; i++) {
				float offsetAngle = (i - (BULLET_COUNT / 2)) * ANGLE_STEP;
				float finalAngle = baseAngle + offsetAngle;
				float targetX = x + std::cos(finalAngle) * 500.f;
				float targetY = y + std::sin(finalAngle) * 500.f;
				float spawnX = x + (i - (BULLET_COUNT / 2)) * 15.f;
				float spawnY = y - 10.f;

				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), projFrames, 12, 16, 16, 16, 16, spawnX, spawnY)
					.SetTargetPosition(targetX, targetY)
					.SetInitialVelocity(INITIAL_VELOCITY)
					.SetReturnAcceleration(180.f)
					.SetDelay(i * 0.05f)
					.SetDecayTime(8.f)
					.SetDamage(finalDamage)
				);
			}
		}
		markFinished();
		});
	auto leftAttack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage, BULLET_COUNT, ANGLE_STEP, INITIAL_VELOCITY](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			float finalDamage = this->CalculateOutgoingDamage(baseDamage);
			auto [px, py] = lock->GetCollider().lock()->GetWorldPosition();
            float x = -320.f;
            float y = 0;
            float dx = px - x;
            float dy = py - y;
			float baseAngle = std::atan2(dy, dx);

			for (int i = 0; i < BULLET_COUNT; i++) {
				float offsetAngle = (i - (BULLET_COUNT / 2)) * ANGLE_STEP;
				float finalAngle = baseAngle + offsetAngle;
				float targetX = x + std::cos(finalAngle) * 500.f;
				float targetY = y + std::sin(finalAngle) * 500.f;
				float spawnX = x + (i - (BULLET_COUNT / 2)) * 15.f;
				float spawnY = y - 10.f;

				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), projFrames, 12, 16, 16, 16, 16, spawnX, spawnY)
					.SetTargetPosition(targetX, targetY)
					.SetInitialVelocity(INITIAL_VELOCITY)
					.SetReturnAcceleration(180.f)
					.SetDelay(i * 0.05f)
					.SetDecayTime(8.f)
					.SetDamage(finalDamage)
				);
			}
		}
		markFinished();
		});

	int dist = RNG::Range(30, 40);
	for (int i = 0; i < dist; i++) {
		if (RNG::Range(1, 2) == 1) commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*leftAttack));
		else commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*rightAttack));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
	}
}

// Shells cross the arena from one side and burst at a random point near its centre,
// throwing a ring of shards outward. The burst point moves every wave, so no corner
// stays safe and the player has to keep reading the incoming shell instead of settling.
void Demo::KeyeproEnemy::PatternShatterVolley(float baseDamage) {
	constexpr int SHELLS_PER_WAVE = 3;
	// Wide enough that the three shells in a wave land as three separate pops the
	// player can read one at a time, rather than one wall of shards.
	constexpr float SHELL_STAGGER = 0.35f;
	constexpr float WAVE_GAP = 1.3f;
	constexpr float SHELL_VELOCITY = 180.f;
	constexpr float SPAWN_X = 320.f;
	constexpr float SPAWN_Y_RANGE = 120.f;
	// The battle box is 256px square around the origin. Keeping burst points well
	// inside it means the shards sweep across the arena rather than off its edge.
	constexpr float BURST_RANGE = 90.f;
	constexpr int SHARDS = 8;
	constexpr float SHARD_VELOCITY = 100.f;
	// Shards clear the 256px box roughly 0.7s after a centre burst; 1.6s carries them
	// past the visible border without leaving dead sprites padding the phase out.
	constexpr float SHARD_DECAY = 1.6f;
	constexpr float SHARD_DAMAGE_SCALE = 0.6f;

	// Only the wave gap costs command time - the shells inside a wave are staggered by
	// their own spawn delay - so the barrage runs waves * WAVE_GAP, about 10.5-12s.
	// The last wave then needs its stagger, ~1s of shell flight and the shard decay to
	// play out: ~3.3s more, for 14-15s under fire. Enough that the player runs out of
	// easy dodges, and short of the 20s Echolocation spends, which is the point where
	// the pressure starts reading as a slog instead.
	const int waves = RNG::Range(8, 9);
	for (int w = 0; w < waves; w++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, baseDamage](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				const float shellDamage = this->CalculateOutgoingDamage(baseDamage);
				const float shardDamage = this->CalculateOutgoingDamage(baseDamage * SHARD_DAMAGE_SCALE);
				for (int i = 0; i < SHELLS_PER_WAVE; i++) {
					const float spawnX = (RNG::Range(1, 2) == 1) ? -SPAWN_X : SPAWN_X;
					const float spawnY = RNG::Range(-SPAWN_Y_RANGE, SPAWN_Y_RANGE);
					const float burstX = RNG::Range(-BURST_RANGE, BURST_RANGE);
					const float burstY = RNG::Range(-BURST_RANGE, BURST_RANGE);

					projectiles.Spawn(
						lock,
						ProjectileDesc(projTexture.get(), projFrames, 12, 16, 16, 16, 16, spawnX, spawnY)
						.SetSplitOnArrival(burstX, burstY, SHARDS, SHARD_VELOCITY)
						.SetSplitRandomAngle()
						.SetSplitDecayTime(SHARD_DECAY)
						.SetSplitDamage(shardDamage)
						.SetShardTexture(shardTexture.get(), shardFrames, 12, 8, 8, 4, 4)
						.SetVelocity(SHELL_VELOCITY)
						.SetDelay(i * SHELL_STAGGER)
						.SetDecayTime(4.f)
						.SetDamage(shellDamage)
						.SetGhostSprite(projTexture.get(), projFrames.front(), 16, 16)
						.SetStatusEffect(ModifierType::Burn, 1.f, 1)
					);
				}
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(WAVE_GAP));
	}
}

void Demo::KeyeproEnemy::PatternFanning(float projDamage)
{
	const int WAVE_COUNT = 15;
	const int BULLET_COUNT = 30;
	const float SPAWN_DISTANCE = 400.f;
	const float SPREAD_ANGLE = 60.f * (3.14159265f / 180.f); // Convert degrees to radians
	const float BASE_ANGLE = RNG::Range(0.f, 360.f) * (3.14159265f / 180.f);
	const float ANGLE_INCREMENT = SPREAD_ANGLE / (BULLET_COUNT - 1);
	const float DELAY_BETWEEN_BULLETS = 0.01f;
	const float DELAY_BETWEEN_WAVES = 0.03f;
	const float JITTER_RADIUS = 250.f;
	for (int wave = 0; wave < WAVE_COUNT; wave++) {
		for (int i = 0; i < BULLET_COUNT; i++) {
			float angle = BASE_ANGLE - SPREAD_ANGLE / 2 + ANGLE_INCREMENT * i;
			D3DXVECTOR2 dir{ std::cos(angle), std::sin(angle) };
			float jitterAngle = RNG::Range(0.f, 360.f) * (3.14159265f / 180.f);
			float jitterDist = std::sqrt(RNG::Range(0.f, 1.f)) * JITTER_RADIUS;
			float jitterX = std::cos(jitterAngle) * jitterDist;
			float jitterY = std::sin(jitterAngle) * jitterDist;
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, dir, SPAWN_DISTANCE, jitterX, jitterY](std::function<void(void)> markFinished) {
				if (auto lock = this->player.lock()) {
					auto [px, py] = lock->GetWorldPosition();
					float spawnX = px - dir.x * SPAWN_DISTANCE + jitterX;
					float spawnY = py - dir.y * SPAWN_DISTANCE + jitterY;
					auto desc = ProjectileDesc(projTexture.get(), projFrames, 12, 8, 8, 16, 16, spawnX, spawnY)
						.SetTrajectory(dir)
						.SetVelocity(100.f)
						.SetDecayTime(8.f)
						.SetReturnAcceleration(200.f)
						.SetWave(100.f, 0.5f)
						.SetDamage(projDamage)
						.SetStatusEffect(
							ModifierType::Burn, 1.f, 1
						);
					projectiles.Spawn(lock, desc);
				}
				markFinished();
				}));
			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(DELAY_BETWEEN_BULLETS));
		}
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(DELAY_BETWEEN_WAVES));
	}

}