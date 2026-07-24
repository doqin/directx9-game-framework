#include "pch.h"
#include "VampireBatEnemy.h"
#include "PopUpMessage.h"
#include "resource.h"
#include "RNG.h"

void Demo::VampireBatEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"assets/shrimp-Sheet.png");
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 12);
	sprite->SetOrigin(32, 32);
	sprite->SetScale(2.f);

	projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	projTexture->LoadTexture(L"assets/spearprojectile.png");

	SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
	InitCardSpawnTrigger(camera, 128.f, 128.f);
}

void Demo::VampireBatEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) {
	if (sprite) {
		sprite->Begin();
		auto [x, y] = GetWorldPosition();
		sprite->SetPosition(x, y);
		sprite->Draw(*camera, deltaTime);
		sprite->End();
	}
	IEnemy::Draw(graphicsDevice, camera, deltaTime);
}

int Demo::VampireBatEnemy::GetRandomPattern() {
	return RNG::Range(1, 2);
}

void Demo::VampireBatEnemy::OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) {
	this->player = player;

	int cycle = (currentTurn - 1) / 3;

	if (currentCycle != cycle) {
		currentCycle = cycle;
		if (RNG::Range(0, 100) <= 40) {
			RNG::Range(1, 3);
		}
		else {
			skillTurnThisCycle = -1;
		}
	}

	int turnInCycle = (currentTurn - 1) % 3 + 1;

	if (turnInCycle == skillTurnThisCycle) {
		int skillType = RNG::Range(1, 2);

		if (skillType == 1) AbilityVampiricHeal();
		else AbilityVulnerablePlayer();

		if (popUpMessage) {
			popUpMessage->QueueMessage(&commandBuffer, L"Shrimp screeches aggressively!", 1.5f);
		}
	}
}

void Demo::VampireBatEnemy::AbilityVampiricHeal() {
	commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void(void)> markFinished) {
		this->Heal(15.f);
		markFinished();
		}));
	commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
}

void Demo::VampireBatEnemy::AbilityVulnerablePlayer() {
	commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			lock->AddModifier(ModifierType::Vulnerable, 2, 0.f, false);
		}
		markFinished();
		}));
	commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
}

void Demo::VampireBatEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) {
	(void)popUpMessage;
	(void)graphicsDevice;
	(void)camera;
	this->player = player;

	if (GetSmartRandomPattern(1, 2) == 1) PatternEcholocation(2.f, enemies);
	else PatternSwoopBite(4.f);
}

void Demo::VampireBatEnemy::PatternEcholocation(float baseDamage, std::vector<std::shared_ptr<IEnemy>>* enemies) {
	bool isAlone = enemies->size() == 1;
	const int BULLETS = isAlone ? 10 : 5;
	const int VELOCITY = isAlone ? 180 : 90;
	const int SPACING = 96;
	const int AMPLITUDE = isAlone ? 50 : 25;
	const int DECAY_TIME = isAlone ? 4 : 8;

	auto rightAttack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage, VELOCITY, AMPLITUDE, DECAY_TIME, BULLETS](std::function<void(void)> markFinished) {
		float finalDamage = this->CalculateOutgoingDamage(baseDamage);

		for (int i = 0; i < BULLETS; i++) {
			if (auto lock = this->player.lock()) {
				float startY = (i - BULLETS / 2.f) * SPACING;
				auto [sineProjTexWidth, sineProjTexHeight] = projTexture->GetSize();
				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), 16, 8, 16, 16, 320, startY)
					.SetTrajectory(D3DXVECTOR2(-1, 0))
					.SetWave(AMPLITUDE, 4.f)
					.SetDelay(i * 0.1f)
					.SetDecayTime(DECAY_TIME)
					.SetVelocity(VELOCITY)
					.SetDamage(finalDamage)
					.SetGhostSprite(projTexture.get(), RECT{ 0, 0, (LONG)sineProjTexWidth, (LONG)sineProjTexHeight }, 16, 8)
				);
			}
		}
		markFinished();
		});

	auto leftAttack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage, VELOCITY, AMPLITUDE, DECAY_TIME, BULLETS](std::function<void(void)> markFinished) {
		float finalDamage = this->CalculateOutgoingDamage(baseDamage);

		for (int i = 0; i < BULLETS; i++) {
			if (auto lock = this->player.lock()) {
				float startY = (i - BULLETS / 2.f) * SPACING;
				auto [sineProjTexWidth, sineProjTexHeight] = projTexture->GetSize();
				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), 16, 8, 16, 16, -320, startY)
					.SetTrajectory(D3DXVECTOR2(1, 0))
					.SetWave(AMPLITUDE, 4.f)
					.SetDelay(i * 0.1f)
					.SetDecayTime(DECAY_TIME)
					.SetVelocity(VELOCITY)
					.SetDamage(finalDamage)
					.SetGhostSprite(projTexture.get(), RECT{ 0, 0, (LONG)sineProjTexWidth, (LONG)sineProjTexHeight }, 16, 8)
				);
			}
		}
		markFinished();
		});

	for (int i = 0; i < 20; i++) {
		if (RNG::Range(1, 2) == 1) commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*leftAttack));
		else commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*rightAttack));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
	}
}

void Demo::VampireBatEnemy::PatternSwoopBite(float baseDamage) {
	const int BULLET_COUNT = 4;
	const float ANGLE_STEP = 0.02f;

	auto rightAttack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage, BULLET_COUNT, ANGLE_STEP](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			float finalDamage = this->CalculateOutgoingDamage(baseDamage);

			auto [px, py] = lock->GetWorldPosition();
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

				auto [projTexWidth, projTexHeight] = projTexture->GetSize();
				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), 16, 8, 16, 16, spawnX, spawnY)
					.SetTargetPosition(targetX, targetY)
					.SetInitialVelocity(400.f)
					.SetReturnAcceleration(180.f)
					.SetDelay(i * 0.05f)
					.SetDecayTime(8.f)
					.SetDamage(finalDamage)
					.SetGhostSprite(projTexture.get(), RECT{ 0, 0, (LONG)projTexWidth, (LONG)projTexHeight }, 16, 8)
				);
			}
		}
		markFinished();
		});

	auto leftAttack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage, BULLET_COUNT, ANGLE_STEP](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			float finalDamage = this->CalculateOutgoingDamage(baseDamage);

			auto [px, py] = lock->GetWorldPosition();
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

				auto [projTexWidth, projTexHeight] = projTexture->GetSize();
				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), 16, 8, 16, 16, spawnX, spawnY)
					.SetTargetPosition(targetX, targetY)
					.SetInitialVelocity(400.f)
					.SetReturnAcceleration(180.f)
					.SetDelay(i * 0.05f)
					.SetDecayTime(8.f)
					.SetDamage(finalDamage)
					.SetGhostSprite(projTexture.get(), RECT{ 0, 0, (LONG)projTexWidth, (LONG)projTexHeight }, 16, 8)
				);
			}
		}
		markFinished();
		});

	int dist = RNG::Range(15, 20);
	for (int i = 0; i < dist; i++) {
		if (RNG::Range(1, 2) == 1) commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*leftAttack));
		else commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*rightAttack));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
	}
}