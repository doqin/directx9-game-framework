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

void Demo::KeyeproEnemy::OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) {
	this->player = player;

	int cycle = (currentTurn - 1) / 3;

	if (currentCycle != cycle) {
		currentCycle = cycle;
		skillTurnThisCycle = RNG::Range(1, 3);
	}

	int turnInCycle = (currentTurn - 1) % 3 + 1;

	if (turnInCycle == skillTurnThisCycle) {
		if (RNG::Range(1, 2) == 1) {
			CastAbility([this]() {
				if (this->onRequestLockCard) this->onRequestLockCard(2);
				}, popUpMessage, L"Boss locks your mind!");
		}
		else {
			CastAbility([this]() { this->AddModifier(ModifierType::BuffDamage, 2, 5.0f, true); }, popUpMessage, L"Boss enters a frenzy!");
		}
	}
}

void Demo::KeyeproEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) {
	this->player = player;

	float baseDamage = 5.f;

	if (enemies != nullptr && graphicsDevice != nullptr && camera != nullptr) {
		constexpr float MINION_SPAWN_CHANCE = 0.35f;
		if (RNG::Range(0.f, 1.f) <= MINION_SPAWN_CHANCE) {
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

			constexpr size_t MAX_KEYE_ENEMIES = 1;
			const size_t spawnCount = (keyeCount < MAX_KEYE_ENEMIES)
				? (std::min)(static_cast<size_t>(1), MAX_KEYE_ENEMIES - keyeCount)
				: 0;

			if (spawnCount > 0) {
				auto [bossX, bossY] = GetWorldPosition();
				for (size_t i = 0; i < spawnCount; ++i) {
					float spawnX = 1200;
					float spawnY = 0;
					auto minion = std::make_shared<KeyeEnemy>(transformManager, 25.0f, spawnX, spawnY);
					minion->Init(graphicsDevice, camera);
					minion->SetOnRequestEnemyCard(onRequestEnemyCard);
					enemies->push_back(minion);
				}

				if (auto tm = transformManager.lock())
				{
					tm->RebuildHierarchy();
				}

				if (popUpMessage) {
					popUpMessage->QueueMessage(&commandBuffer, L"The boss spawned a minion!", 1.5f);
				}
				return;
			}
		}

		int patternId = GetSmartRandomPattern(1, 6);
		if (patternId == 1) PatternTargetedSniping(baseDamage);
		else if (patternId == 2) PatternEcholocation(baseDamage);
		else if (patternId == 3) PatternSwoopBite(baseDamage);
		else if (patternId == 4) PatternSpiralBloom(baseDamage);
		else if (patternId == 5) PatternCrossfireSweep(baseDamage);
		else PatternHomingConstellation(baseDamage);
	}
}

void Demo::KeyeproEnemy::PatternSineWaveStorm(float baseDamage) {
	const int BULLETS = 10;
	const int SPACING = 80;
	const int WAVE_COUNT = 10;

	auto leftAttack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage, BULLETS, SPACING, WAVE_COUNT](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			float finalDamage = this->CalculateOutgoingDamage(baseDamage);

			for (int waveCount = 0; waveCount < WAVE_COUNT; waveCount++) {
				for (int i = 0; i < BULLETS; i++) {
					float startY = (i - BULLETS / 2.f) * SPACING;

					projectiles.Spawn(
						lock,
						ProjectileDesc(projTexture.get(), projFrames, 12, 16, 16, 16, 16, -350.f, startY)
						.SetTrajectory(D3DXVECTOR2(1, 0))
						.SetWave(20.f, 4.0f)
						.SetDelay((waveCount * 0.25f) + (i * 0.05f))
						.SetDecayTime(10.f)
						.SetVelocity(100.f)
						.SetDamage(finalDamage)
						.SetGhostSprite(projTexture.get(), projFrames.front(), 16, 16)
					);
				}
			}
		}
		markFinished();
		});
	//auto topAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage, BULLETS, SPACING, WAVE_COUNT](std::function<void(void)> markFinished) {
	//	if (auto lock = this->player.lock()) {
	//		for (int waveCount = 0; waveCount < WAVE_COUNT; waveCount++) {
	//			for (int i = 0; i < BULLETS; i++) {
	//				float startX = (i - BULLETS / 2.f) * SPACING;
	//				projectiles.push_back(
	//					SineWaveProjectile::Builder(transformManager, lock, projSprite, 16, 16, startX, -350.f)
	//					.SetTrajectory(D3DXVECTOR2(0, 1))
	//					.SetWave(20.f, 4.0f)
	//					.SetDelay((waveCount * 0.25f) + (i * 0.05f))
	//					.SetDecayTime(10.f)
	//					.SetVelocity(100.f)
	//					.SetDamage(projDamage)
	//					.Build()
	//				);
	//				projectiles.back()->Init();
	//			}
	//		}
	//		transformManager.lock()->RebuildHierarchy();
	//	}
	//	markFinished();
	//	});

	commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*leftAttack));
	//commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*topAttack));
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
	auto rightAttack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage](std::function<void(void)> markFinished) {
		float finalDamage = this->CalculateOutgoingDamage(baseDamage);
		for (int i = 0; i < BULLETS; i++) {
			if (auto lock = this->player.lock()) {
				float startY = (i - BULLETS / 2.f) * SPACING;
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
				);
			}
		}
		markFinished();
		});
	auto leftAttack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage](std::function<void(void)> markFinished) {
		float finalDamage = this->CalculateOutgoingDamage(baseDamage);
		for (int i = 0; i < BULLETS; i++) {
			if (auto lock = this->player.lock()) {
				float startY = (i - BULLETS / 2.f) * SPACING;
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

void Demo::KeyeproEnemy::PatternSpiralBloom(float baseDamage) {
	const int WAVE_COUNT = 4;
	const int BULLETS_PER_WAVE = 8;
	const float WAVE_DELAY = 0.55f;
	const float RADIAL_SPEED = 65.f;
	const float ANGULAR_SPEED = 1.35f;
	const float DECAY_TIME = 5.5f;
	const float ORIGIN_X = 240.f;
	const float ORIGIN_Y = -160.f;

	for (int wave = 0; wave < WAVE_COUNT; wave++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, baseDamage, wave, BULLETS_PER_WAVE, RADIAL_SPEED, ANGULAR_SPEED, DECAY_TIME, ORIGIN_X, ORIGIN_Y](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				float finalDamage = this->CalculateOutgoingDamage(baseDamage);
				float spinDirection = (wave % 2 == 0) ? 1.f : -1.f;
				float waveOffset = wave * 0.35f;

				for (int i = 0; i < BULLETS_PER_WAVE; i++) {
					float angle = waveOffset + i * (2.f * 3.14159265359f / BULLETS_PER_WAVE);

					projectiles.Spawn(
						lock,
						ProjectileDesc(projTexture.get(), projFrames, 12, 16, 16, 16, 16, ORIGIN_X, ORIGIN_Y)
						.SetSpiralParams(angle, RADIAL_SPEED, spinDirection * ANGULAR_SPEED)
						.SetDelay(0.f)
						.SetDecayTime(DECAY_TIME)
						.SetDamage(finalDamage)
					);
				}
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(WAVE_DELAY));
	}
}

void Demo::KeyeproEnemy::PatternCrossfireSweep(float baseDamage) {
	constexpr float PI = 3.14159265359f;
	const int PAIR_COUNT = 56;
	const float SPAWN_DELAY = 0.08f;
	const float BULLET_SPEED = 220.f;
	const float DECAY_TIME = 4.f;
	const float START_X = 0.f;
	const float TOP_Y = -350.f;
	const float BOTTOM_Y = 350.f;
	const float SWEEP_RANGE = PI * 0.38f;

	for (int i = 0; i < PAIR_COUNT; i++) {
		float sweep = std::sin(i * 0.65f) * SWEEP_RANGE;
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, baseDamage, BULLET_SPEED, DECAY_TIME, START_X, TOP_Y, BOTTOM_Y, sweep](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				float finalDamage = this->CalculateOutgoingDamage(baseDamage);

				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), projFrames, 12, 16, 16, 16, 16, START_X, TOP_Y)
					.SetTrajectory(D3DXVECTOR2(std::sin(sweep), 1.f))
					.SetVelocity(BULLET_SPEED)
					.SetDelay(0.f)
					.SetDecayTime(DECAY_TIME)
					.SetDamage(finalDamage)
				);

				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), projFrames, 12, 16, 16, 16, 16, -START_X, BOTTOM_Y)
					.SetTrajectory(D3DXVECTOR2(-std::sin(sweep), -1.f))
					.SetVelocity(BULLET_SPEED)
					.SetDelay(0.f)
					.SetDecayTime(DECAY_TIME)
					.SetDamage(finalDamage)
				);
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(SPAWN_DELAY));
	}
}

void Demo::KeyeproEnemy::PatternHomingConstellation(float baseDamage) {
	constexpr float PI = 3.14159265359f;
	const int POINT_COUNT = 7;
	const int WAVE_COUNT = 4;
	const float RADIUS = 360.f;
	const float ARC_WIDTH = PI * 0.85f;
	const float WAVE_DELAY = 0.8f;
	const float BULLET_SPEED = 115.f;
	const float TURN_SPEED = 0.8f;
	const float DECAY_TIME = 5.5f;
	const float BULLET_DELAY = 0.12f;

	for (int wave = 0; wave < WAVE_COUNT; wave++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, baseDamage, wave, POINT_COUNT, RADIUS, ARC_WIDTH, BULLET_SPEED, TURN_SPEED, DECAY_TIME, BULLET_DELAY](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				float finalDamage = this->CalculateOutgoingDamage(baseDamage);
				float centerAngle = (wave % 2 == 0) ? 0.f : 3.14159265359f;
				float waveOffset = (wave / 2) * 0.35f;

				for (int i = 0; i < POINT_COUNT; i++) {
					float arcRatio = (POINT_COUNT == 1) ? 0.5f : static_cast<float>(i) / (POINT_COUNT - 1);
					float angle = centerAngle - ARC_WIDTH * 0.5f + arcRatio * ARC_WIDTH + waveOffset;
					float spawnX = std::cos(angle) * RADIUS;
					float spawnY = std::sin(angle) * RADIUS;
					float tangentDirection = (wave % 2 == 0) ? 1.f : -1.f;
					D3DXVECTOR2 tangent(-std::sin(angle) * tangentDirection, std::cos(angle) * tangentDirection);

					projectiles.Spawn(
						lock,
						ProjectileDesc(projTexture.get(), projFrames, 12, 16, 16, 16, 16, spawnX, spawnY)
						.SetTrajectory(tangent)
						.SetHoming(TURN_SPEED)
						.SetVelocity(BULLET_SPEED)
						.SetDelay(i * BULLET_DELAY)
						.SetDecayTime(DECAY_TIME)
						.SetDamage(finalDamage)
					);
				}
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(WAVE_DELAY));
	}
}