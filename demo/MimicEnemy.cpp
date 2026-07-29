#include "pch.h"
#include "MimicEnemy.h"
#include "PopUpMessage.h"
#include "resource.h"
#include "RNG.h"

void Demo::MimicEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"assets/notresponding-Sheet.png");
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 12);
	sprite->SetOrigin(32, 32);
	sprite->SetScale(2.f, 2.f);

	projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	projTexture->LoadTexture(L"assets/xprojectile.png");

	SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
	InitCardSpawnTrigger(camera, 128.f, 128.f);
}

void Demo::MimicEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) {
	if (sprite) {
		sprite->Begin();
		auto [x, y] = GetWorldPosition();
		sprite->SetPosition(x, y);
		sprite->Draw(*camera, deltaTime);
		sprite->End();
	}
	IEnemy::Draw(graphicsDevice, camera, deltaTime);
}

int Demo::MimicEnemy::GetRandomPattern() {
	return RNG::Range(1, 2);
}

void Demo::MimicEnemy::OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) {
	this->player = player;

	int cycle = (currentTurn - 1) / 3;

	if (currentCycle != cycle) {
		currentCycle = cycle;

		if (RNG::Range(1, 100) <= 70) {
			skillTurnThisCycle = RNG::Range(1, 3);
		}
		else {
			skillTurnThisCycle = -1;
		}
	}

	int turnInCycle = (currentTurn - 1) % 3 + 1;

	if (turnInCycle == skillTurnThisCycle) {
		if (RNG::Range(1, 2) == 1) {
			CastAbility([this]() { this->AddModifier(ModifierType::BuffDefense, 2, 15.0f, true); }, popUpMessage, L"A giant pop-up blocks the way!");
		}
		else {
			CastAbility([this]() {
				if (auto lock = this->player.lock()) lock->AddModifier(ModifierType::Poison, 2, 2.0f, false);
				}, popUpMessage, L"Mimic installs malicious adware!");
		}
	}
}

void Demo::MimicEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) {
	this->player = player;

	float baseDamage = 4.f;

	int patternId = GetSmartRandomPattern(1, 2);
	if (patternId == 1) PatternCoinCyclone(baseDamage, enemies);
	else PatternJunkVomit(baseDamage, enemies);

	commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(4.f));
}

void Demo::MimicEnemy::PatternCoinCyclone(float baseDamage, std::vector<std::shared_ptr<IEnemy>>* enemies) {
	bool isAlone = enemies->size() == 1;
	const int ATTACK_COUNT = isAlone ? 10 : 5;
	const float RADICAL_SPEED = isAlone ? 100.f : 150.f;
	const float ANGULAR_SPEED = isAlone ? 1.2f : 0.6f;

	auto attack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage, RADICAL_SPEED, ANGULAR_SPEED](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			float finalDamage = this->CalculateOutgoingDamage(baseDamage);

			for (int i = 0; i < 12; i++) {
				float angle = i * (3.14159f * 2.f / 12.f);
				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), 8, 8, 16, 16, 256.f, 0)
					.SetSpiralParams(angle, RADICAL_SPEED, ANGULAR_SPEED)
					.SetDelay(i * 0.05f)
					.SetDecayTime(6.f)
					.SetDamage(finalDamage) 
				);
			}
		}
		markFinished();
		});

	for (int i = 0; i < ATTACK_COUNT; i++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*attack));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(1.f));
	}
}

void Demo::MimicEnemy::PatternJunkVomit(float baseDamage, std::vector<std::shared_ptr<IEnemy>>* enemies) {
	bool isAlone = enemies->size() == 1;
	const int JUNK_COUNT = isAlone ? 100 : 50;
	const float ATTACK_DELAY = isAlone ? 0.05f : 0.1f;

	for (int i = 0; i < JUNK_COUNT; i++) {
		float randY = RNG::Range(-196.f, 196.f);

		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, baseDamage, randY](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				float finalDamage = this->CalculateOutgoingDamage(baseDamage);

				auto [px, py] = lock->GetWorldPosition();
				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), 8, 8, 16, 16, 512.f, 0.f)
					.SetTargetPosition(px, py + randY)

					.SetInitialVelocity(450.f)
					.SetReturnAcceleration(150.f)

					.SetDelay(0.f)
					.SetDecayTime(6.f)
					.SetDamage(finalDamage)
				);
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(ATTACK_DELAY));
	}
}