#include "pch.h"
#include "KeyeEnemy.h"
#include "resource.h"
#include "RNG.h"
#include "PopUpMessage.h"

void Demo::KeyeEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"assets/minion-Sheet.png");
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 12);
	sprite->SetOrigin(32, 32);
	sprite->SetScale(2.f);

	projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	projTexture->LoadTexture(L"assets/minionprojectile-Sheet.png");
	projFrames = DX9GF::Utils::CreateRectsHorizontal(0, 0, 32, 32, 5);
	SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
	InitCardSpawnTrigger(camera, 128.f, 128.f);
}

void Demo::KeyeEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) {
	if (sprite) {
		sprite->Begin();
		auto [x, y] = GetWorldPosition();
		sprite->SetPosition(x, y);
		sprite->Draw(*camera, deltaTime);
		sprite->End();
	}
	IEnemy::Draw(graphicsDevice, camera, deltaTime);
}

int Demo::KeyeEnemy::GetRandomPattern() {
	return RNG::Range(1, 2);
}

void Demo::KeyeEnemy::OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) {
	this->player = player; //saved player's ref

	int cycle = (currentTurn - 1) / 3;

	if (currentCycle != cycle) {
		currentCycle = cycle;

		if (RNG::Range(1, 100) <= 50) {
			skillTurnThisCycle = RNG::Range(1, 3);
		}
		else {
			skillTurnThisCycle = -1;
		}
	}

	int turnInCycle = (currentTurn - 1) % 3 + 1;

	if (turnInCycle == skillTurnThisCycle) {
		if (RNG::Range(1, 2) == 1) {
			CastAbility([this]() { this->AddModifier(ModifierType::BuffDamage, 2, 5.0f, true); }, popUpMessage, L"Keye focuses its gaze!");
		}
		else {
			CastAbility([this]() {
				if (auto lock = this->player.lock()) lock->AddModifier(ModifierType::Weak, 2, 0.f, false);
				}, popUpMessage, L"Keye finds your blind spot!");
		}
	}
}

void Demo::KeyeEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) {
	this->player = player;

	float baseDamage = 2.f;
	float finalDamage = CalculateOutgoingDamage(baseDamage);

	if (GetSmartRandomPattern(1, 2) == 1) PatternBoomerangCross(finalDamage);
	else PatternRoundCircle(finalDamage);
}


void Demo::KeyeEnemy::PatternBoomerangCross(float projDamage) {
	auto attack = std::make_shared<DX9GF::CustomCommand>([this, projDamage](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			auto [px, py] = lock->GetWorldPosition();

			for (int i = 0; i < 5; i++) {
				float offset = i * 2.0f;
				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), projFrames, 12, 16, 16, 16, 16, RNG::Range(-128.f, 128.f), -256.f + offset)
					.SetTargetPosition(px, py)
					.SetInitialVelocity(300.f)
					.SetReturnAcceleration(100.f)
					.SetDelay(i * 0.1f)
					.SetDecayTime(10.f)
					.SetDamage(projDamage)
				);
			}
		}
		markFinished();
		});
	for (int i = 0; i < 5; i++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*attack));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(1.5f));
	}
}

void Demo::KeyeEnemy::PatternRoundCircle(float projDamage) {
	for (int i = 0; i < 25; i++) {
		float randX = RNG::Range(-120.f, 120.f);
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, randX](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), projFrames, 12, 16, 16, 16, 16, randX, -220.f)
					.SetTargetPosition(lock->GetCollider().lock()->GetWorldX(), lock->GetCollider().lock()->GetWorldY())
					.SetVelocity(220.f)
					.SetDecayTime(6.f)
					.SetDamage(projDamage)
				);
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
	}
}