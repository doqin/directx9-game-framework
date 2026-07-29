#include "pch.h"
#include "WarlockEnemy.h"
#include "PopUpMessage.h"
#include "resource.h"
#include "RNG.h"

void Demo::WarlockEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"assets/crawler-Sheet.png");
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 12);
	sprite->SetOrigin(32, 32);
	sprite->SetScale(2.f, 2.f);

	projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	projTexture->LoadTexture(L"assets/shardprojectile.png");

	SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
	InitCardSpawnTrigger(camera, 128.f, 128.f);
}

void Demo::WarlockEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) {
	if (sprite) {
		sprite->Begin();
		auto [x, y] = GetWorldPosition();
		sprite->SetPosition(x, y);
		sprite->Draw(*camera, deltaTime);
		sprite->End();
	}
	IEnemy::Draw(graphicsDevice, camera, deltaTime);
}

int Demo::WarlockEnemy::GetRandomPattern() {
	return RNG::Range(1, 2);
}

void Demo::WarlockEnemy::OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) {
	this->player = player;

	int cycle = (currentTurn - 1) / 3;

	if (currentCycle != cycle) {
		currentCycle = cycle;

		if (RNG::Range(1, 100) <= 65) {
			skillTurnThisCycle = RNG::Range(1, 3);
		}
		else {
			skillTurnThisCycle = -1;
		}
	}

	int turnInCycle = (currentTurn - 1) % 3 + 1;

	if (turnInCycle == skillTurnThisCycle) {
		if (RNG::Range(1, 2) == 1) {
			CastAbility([this]() {
				if (auto lock = this->player.lock()) lock->AddModifier(ModifierType::Weak, 2, 0.f, false);
				}, popUpMessage, L"Crawler slows your connection!");
		}
		else {
			CastAbility([this]() { this->AddModifier(ModifierType::BuffDamage, 2, 4.0f, true); }, popUpMessage, L"Crawler gathers corrupted data!");
		}
	}
}

void Demo::WarlockEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) {
	(void)enemies;
	(void)popUpMessage;
	(void)graphicsDevice;
	(void)camera;
	this->player = player;

	float baseDamage = 4.f;

	if (GetSmartRandomPattern(1, 2) == 1) PatternDarkVortex(baseDamage, enemies);
	else PatternHomingCurse(baseDamage, enemies);
}

void Demo::WarlockEnemy::PatternDarkVortex(float baseDamage, std::vector<std::shared_ptr<IEnemy>>* enemies) {
	bool isAlone = enemies->size() == 1;
	const int RING_COUNT = isAlone ? 15 : 8;
	const int PROJECTILES_PER_RING = isAlone ? 16 : 12;
	const float VELOCITY = isAlone ? 320.f : 160.f;
	const float DECAY_TIME = isAlone ? 2.2f : 4.2f;
	const float DELAY_BETWEEN_RINGS = isAlone ? 0.7f : 1.4f;

	for (int ring = 0; ring < RING_COUNT; ring++) {
		float gapRatio = RNG::Range(0.0f, 1.0f);

		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, baseDamage, ring, gapRatio, PROJECTILES_PER_RING, VELOCITY, DECAY_TIME](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				float finalDamage = this->CalculateOutgoingDamage(baseDamage);

				int numProjectiles = PROJECTILES_PER_RING;
				float radius = 640.f;
				float gapAngle = 3.14159f * 2.f * gapRatio;
				float gapSize = 3.14159f / 4.f;

				for (int i = 0; i < numProjectiles; ++i) {
					float angle = i * (2.f * 3.14159f / numProjectiles);

					// define gap
					float angleDiff = std::abs(angle - gapAngle);
					if (angleDiff > 3.14159f) {
						angleDiff = 2.f * 3.14159f - angleDiff;
					}

					if (angleDiff < gapSize) {
						continue;
					}

					projectiles.Spawn(
						lock,
						ProjectileDesc(projTexture.get(), 16, 8, 16, 16, radius * std::cos(angle), radius * std::sin(angle))
						.SetDelay(0.2f)
						.SetDecayTime(DECAY_TIME)
						.SetVelocity(VELOCITY)
						.SetTrajectory(D3DXVECTOR2(-std::cos(angle), -std::sin(angle)))
						.SetDamage(finalDamage)
					);
				}
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(DELAY_BETWEEN_RINGS));
	}
}

void Demo::WarlockEnemy::PatternHomingCurse(float baseDamage, std::vector<std::shared_ptr<IEnemy>>* enemies) {
	bool isAlone = enemies->size() == 1;
	const float VELOCITY = isAlone ? 250.f : 150.f;
	const float TURN_SPEED = isAlone ? 2.f : 1.f;

	auto topRightAttack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage, VELOCITY, TURN_SPEED](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			float finalDamage = this->CalculateOutgoingDamage(baseDamage);
			auto [px, py] = lock->GetWorldPosition();
			projectiles.Spawn(
				lock,
				ProjectileDesc(projTexture.get(), 16, 8, 16, 16, 512, 128)
				.SetTrajectory(D3DXVECTOR2(-1, 1))
				.SetHoming(TURN_SPEED)
				.SetDelay(0.f)
				.SetDecayTime(5.f)
				.SetVelocity(VELOCITY)
				.SetDamage(finalDamage)
			);
		}
		markFinished();
		});

	auto topLeftAttack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage, VELOCITY, TURN_SPEED](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			float finalDamage = this->CalculateOutgoingDamage(baseDamage);
			auto [px, py] = lock->GetWorldPosition();
			projectiles.Spawn(
				lock,
				ProjectileDesc(projTexture.get(), 16, 8, 16, 16, -512, 128)
				.SetTrajectory(D3DXVECTOR2(1, 1))
				.SetHoming(TURN_SPEED)
				.SetDelay(0.f)
				.SetDecayTime(5.f)
				.SetVelocity(VELOCITY)
				.SetDamage(finalDamage)
			);
		}
		markFinished();
		});

	auto bottomRightAttack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage, VELOCITY, TURN_SPEED](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			float finalDamage = this->CalculateOutgoingDamage(baseDamage);
			auto [px, py] = lock->GetWorldPosition();
			projectiles.Spawn(
				lock,
				ProjectileDesc(projTexture.get(), 16, 8, 16, 16, 512, -128)
				.SetTrajectory(D3DXVECTOR2(-1, -1))
				.SetHoming(TURN_SPEED)
				.SetDelay(0.f)
				.SetDecayTime(5.f)
				.SetVelocity(VELOCITY)
				.SetDamage(finalDamage)
			);
		}
		markFinished();
		});

	auto bottomLeftAttack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage, VELOCITY, TURN_SPEED](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			float finalDamage = this->CalculateOutgoingDamage(baseDamage);
			auto [px, py] = lock->GetWorldPosition();
			projectiles.Spawn(
				lock,
				ProjectileDesc(projTexture.get(), 16, 8, 16, 16, -512, -128)
				.SetTrajectory(D3DXVECTOR2(1, -1))
				.SetHoming(TURN_SPEED)
				.SetDelay(0.f)
				.SetDecayTime(5.f)
				.SetVelocity(VELOCITY)
				.SetDamage(finalDamage)
			);
		}
		markFinished();
		});

	for (int i = 0; i < 40; i++) {
		int attackIndex = RNG::Range(0, 3);
		switch (attackIndex) {
		case 0:
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*topRightAttack));
			break;
		case 1:
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*topLeftAttack));
			break;
		case 2:
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*bottomRightAttack));
			break;
		case 3:
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*bottomLeftAttack));
			break;
		}
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.3f));
	}
}