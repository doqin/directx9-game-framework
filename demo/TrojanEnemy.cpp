#include "pch.h"
#include "TrojanEnemy.h"
#include "resource.h"
#include "RNG.h"
#include "PopUpMessage.h"

void Demo::TrojanEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"assets/placeholder.png");
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 12);
	sprite->SetOrigin(32, 32);
	sprite->SetScale(2.f);

	projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	projTexture->LoadTexture(L"assets/placeholder-round-projectile.png");
	projFrames = DX9GF::Utils::CreateRectsHorizontal(0, 0, 16, 16, 4);

	SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
	InitCardSpawnTrigger(camera, 128.f, 128.f);
}

void Demo::TrojanEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) {
	if (sprite) {
		sprite->Begin();
		auto [x, y] = GetWorldPosition();
		sprite->SetPosition(x, y);
		sprite->Draw(*camera, deltaTime);
		sprite->End();
	}
	IEnemy::Draw(graphicsDevice, camera, deltaTime);
}

int Demo::TrojanEnemy::GetRandomPattern() {
	return RNG::Range(1, 2);
}

void Demo::TrojanEnemy::OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) {
	this->player = player;

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
		CastAbility([this]() {
			if (auto lock = this->player.lock()) lock->AddModifier(ModifierType::Weak, 1, 0.f, false);
			}, popUpMessage, L"Trojan corrupts your systems!");
	}
}

void Demo::TrojanEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) {
	this->player = player;

	float baseDamage = 3.f;
	float finalDamage = CalculateOutgoingDamage(baseDamage);

	if (GetSmartRandomPattern(1, 2) == 1) PatternTrojanBolt(finalDamage);
	else PatternVirusSpread(finalDamage);
}

void Demo::TrojanEnemy::PatternTrojanBolt(float projDamage) {
	for (int i = 0; i < 3; i++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [px, py] = lock->GetWorldPosition();
				float burnValue = lock->GetMaxHealth() * 0.01f;
				auto desc = ProjectileDesc(projTexture.get(), projFrames, 12, 8, 8, 16, 16, RNG::Range(-100.f, 100.f), -200.f)
					.SetTargetPosition(px, py)
					.SetVelocity(180.f)
					.SetHoming(1.5f)
					.SetDecayTime(6.f)
					.SetDamage(projDamage);
				desc.SetRandomStatusEffect(
					ModifierType::Freeze, FREEZE_VALUE, FREEZE_DURATION,
					ModifierType::Burn, burnValue, BURN_DURATION
				);
				projectiles.Spawn(lock, desc);
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.8f));
	}
}

void Demo::TrojanEnemy::PatternVirusSpread(float projDamage) {
	const int BULLET_COUNT = 4;
	for (int i = 0; i < BULLET_COUNT; i++) {
		float angle = (2.0f * 3.14159265f / BULLET_COUNT) * i;
		D3DXVECTOR2 dir{ std::cos(angle), std::sin(angle) };

		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, dir](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [px, py] = lock->GetWorldPosition();
				float burnValue = lock->GetMaxHealth() * 0.01f;
				auto desc = ProjectileDesc(projTexture.get(), projFrames, 12, 8, 8, 16, 16, px - dir.x * 150.f, py - dir.y * 150.f)
					.SetTrajectory(dir)
					.SetVelocity(160.f)
					.SetDecayTime(5.f)
					.SetDamage(projDamage);
				desc.SetRandomStatusEffect(
					ModifierType::Freeze, FREEZE_VALUE, FREEZE_DURATION,
					ModifierType::Burn, burnValue, BURN_DURATION
				);
				projectiles.Spawn(lock, desc);
			}
			markFinished();
			}));
	}
	commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(1.f));
}