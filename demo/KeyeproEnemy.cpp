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

void Demo::KeyeproEnemy::OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn, std::vector<std::shared_ptr<IEnemy>>* enemies, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	this->player = player;

	if (enemies != nullptr && graphicsDevice != nullptr && camera != nullptr) {
		constexpr float MINION_SPAWN_CHANCE = 0.5f;
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

	if (currentTurn % 2 == 1) {
		auto abilityRoll = RNG::Range(1, 6);
		if (abilityRoll == 1) {
			CastAbility([this]() {
				if (this->onRequestLockCard) this->onRequestLockCard(2);
				}, popUpMessage, L"Boss locks your mind!");
		}
		else if (abilityRoll == 2) {
			CastAbility([this]() { this->player.lock()->AddModifier(ModifierType::Vulnerable, 1, 5.0f, false); }, popUpMessage, L"Boss lowers your defenses!");
		}
		else if (abilityRoll == 3) {
			CastAbility([this]() { this->player.lock()->AddModifier(ModifierType::Weak, 2, 5.0f, false); }, popUpMessage, L"Boss weakens your attacks!");
		}
		else if (abilityRoll == 4) {
			CastAbility([this]() { this->AddModifier(ModifierType::BuffDefense, 2, 30.0f, true); }, popUpMessage, L"Boss fortifies itself!");
		}
		else if (abilityRoll == 5) {
			CastAbility([this]() { this->AddModifier(ModifierType::HealHP, 0, 50.0f, true); }, popUpMessage, L"Boss regenerates!");
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
		int patternId = GetSmartRandomPattern(1, 3);
		if (patternId == 1) PatternTargetedSniping(baseDamage);
		else if (patternId == 2) PatternEcholocation(baseDamage);
		else if (patternId == 3) PatternSwoopBite(baseDamage);
		//else if (patternId == 4) PatternSpiralBloom(baseDamage);
		//else if (patternId == 5) PatternCrossfireSweep(baseDamage);
		//else PatternHomingConstellation(baseDamage);
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