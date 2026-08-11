#include "pch.h"
#include "TrojanEnemy.h"
#include "resource.h"
#include "RNG.h"
#include "PopUpMessage.h"

void Demo::TrojanEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"assets/placeholder.png"); //TODO: Change the enemy asset to a proper one
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 12);
	sprite->SetOrigin(32, 32);
	sprite->SetScale(2.f);

	projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	projTexture->LoadTexture(L"assets/placeholder-round-projectile.png"); //TODO: Change the projectile asset to a proper one
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
	for (int i = 0; i < 5; i++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [px, py] = lock->GetWorldPosition();
				auto desc = ProjectileDesc(projTexture.get(), projFrames, 12, 8, 8, 16, 16, RNG::Range(-100.f, 100.f), -200.f)
					.SetTargetPosition(px, py)
					.SetVelocity(180.f)
					.SetHoming(3.f)
					.SetDecayTime(6.f)
					.SetDamage(projDamage);
				desc.SetRandomStatusEffect(
					ModifierType::Freeze, FREEZE_VALUE, FREEZE_DURATION,
					ModifierType::Burn, BURN_VALUE, BURN_DURATION
				);
				projectiles.Spawn(lock, desc);
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.7f));
	}
}

void Demo::TrojanEnemy::PatternVirusSpread(float projDamage) {
	const int BULLET_COUNT = 4;
	const int WAVE_COUNT = 2;
	const float SPAWN_DISTANCE = 150.f;
	const float JITTER_RADIUS = 70.f;

	for (int wave = 0; wave < WAVE_COUNT; wave++) {
		float baseAngleOffset = RNG::Range(0.f, 360.f) * (3.14159265f / 180.f);
		for (int i = 0; i < BULLET_COUNT; i++) {
			float angle = baseAngleOffset + (2.0f * 3.14159265f / BULLET_COUNT) * i;
			D3DXVECTOR2 dir{ std::cos(angle), std::sin(angle) };
			float waveAmp = RNG::Range(20.f, 40.f);
			float waveFreq = RNG::Range(3.f, 5.f);

			float jitterAngle = RNG::Range(0.f, 360.f) * (3.14159265f / 180.f);
			float jitterDist = std::sqrt(RNG::Range(0.f, 1.f)) * JITTER_RADIUS;
			float jitterX = std::cos(jitterAngle) * jitterDist;
			float jitterY = std::sin(jitterAngle) * jitterDist;

			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, dir, waveAmp, waveFreq, jitterX, jitterY, SPAWN_DISTANCE](std::function<void(void)> markFinished) {
				if (auto lock = this->player.lock()) {
					auto [px, py] = lock->GetWorldPosition();

					float spawnX = px - dir.x * SPAWN_DISTANCE + jitterX;
					float spawnY = py - dir.y * SPAWN_DISTANCE + jitterY;

					auto desc = ProjectileDesc(projTexture.get(), projFrames, 12, 8, 8, 16, 16, spawnX, spawnY)
						.SetTrajectory(dir)
						.SetVelocity(150.f)
						.SetWave(waveAmp, waveFreq)
						.SetDecayTime(5.5f)
						.SetDamage(projDamage);
					desc.SetRandomStatusEffect(
						ModifierType::Freeze, FREEZE_VALUE, FREEZE_DURATION,
						ModifierType::Burn, BURN_VALUE, BURN_DURATION
					);
					projectiles.Spawn(lock, desc);
				}
				markFinished();
				}));
		}
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(wave == 0 ? 0.35f : 1.f));
	}
}