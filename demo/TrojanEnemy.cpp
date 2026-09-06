#include "pch.h"
#include "TrojanEnemy.h"
#include "resource.h"
#include "RNG.h"
#include "PopUpMessage.h"

void Demo::TrojanEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"assets/trojan.png");
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 96, 96, 12), 12);
	sprite->SetOrigin(52, 52);
	sprite->SetScale(2.f);

	projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	projTexture->LoadTexture(L"assets/arrow_1.png"); //TODO: Change the projectile asset to a proper one
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
	auto desperato = 1.f - (GetHealth() / GetMaxHealth());

	if (currentTurn % 2 == 1 || desperato > 0.6f) {
		auto ability = RNG::Range(1, 2);
		if (ability == 1) {
			// Apply Weak to player
			CastAbility([this]() {
				if (auto lock = this->player.lock()) lock->AddModifier(ModifierType::Weak, 1, 0.f, false);
				}, popUpMessage, L"Trojan corrupts your systems!");
		}
		else {
			// Counter physical attack
			CastAbility([this]() {
				this->AddModifier(ModifierType::BuffDefense, 2, 60.f, true);
				}, popUpMessage, L"Trojan fortifies its defenses!");
		}
	}
}

void Demo::TrojanEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) {
	this->player = player;

	auto pattern = GetSmartRandomPattern(1, 3);
	if (pattern == 1) PatternTrojanBolt(CalculateOutgoingDamage(2.f));
	else if (pattern == 2) PatternVirusSpread(CalculateOutgoingDamage(3.f));
	else PatternFanning(CalculateOutgoingDamage(3.f));
}

void Demo::TrojanEnemy::PatternTrojanBolt(float projDamage) {
	const int WAVES = 30;
	const int BULLET_PER_BURST = 5;
	const float BURST_DELAY = 0.02f;
	const float DELAY_BETWEEN_WAVES = 0.02f;
	const float DISTANCE_FROM_PLAYER = 300.f;
	for (int j = 0; j < WAVES; j++) {
		const int side = RNG::Range(1, 4); // 1 = top, 2 = right, 3 = bottom, 4 = left
		for (int i = 0; i < BULLET_PER_BURST; i++) {
			float x;
			float y;
			switch (side) {
			case 1:
				x = RNG::Range(-100.f, 100.f);
				y = -DISTANCE_FROM_PLAYER;
				break;
			case 2:
				x = DISTANCE_FROM_PLAYER;
				y = RNG::Range(-100.f, 100.f);
				break;
			case 3:
				x = RNG::Range(-100.f, 100.f);
				y = DISTANCE_FROM_PLAYER;
				break;
			case 4:
				x = -DISTANCE_FROM_PLAYER;
				y = RNG::Range(-100.f, 100.f);
				break;
			}
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, x, y](std::function<void(void)> markFinished) {
				if (auto lock = this->player.lock()) {
					auto [px, py] = lock->GetWorldPosition();
					auto desc = ProjectileDesc(projTexture.get(), projFrames, 12, 8, 8, 16, 16, x, y)
						.SetTargetPosition(px, py)
						.SetVelocity(400.f)
						//.SetHoming(3.f)
						.SetReturnAcceleration(200.f)
						.SetDecayTime(5.f)
						.SetDamage(projDamage);
					desc.SetStatusEffect(
						ModifierType::Burn, BURN_VALUE, BURN_DURATION
					);
					projectiles.Spawn(lock, desc);
				}
				markFinished();
				}));
			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(BURST_DELAY));
		}
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(BURST_DELAY * BULLET_PER_BURST + DELAY_BETWEEN_WAVES));
	}
}

void Demo::TrojanEnemy::PatternVirusSpread(float projDamage) {
	const int BULLET_COUNT = 8;
	const int WAVE_COUNT = 15;
	const float SPAWN_DISTANCE = 350.f;
	const float JITTER_RADIUS = 70.f;

	for (int wave = 0; wave < WAVE_COUNT; wave++) {
		float baseAngleOffset = RNG::Range(0.f, 360.f) * (3.14159265f / 180.f);
		for (int i = 0; i < BULLET_COUNT; i++) {
			float angle = baseAngleOffset + (2.0f * 3.14159265f / BULLET_COUNT) * i;
			D3DXVECTOR2 dir{ std::cos(angle), std::sin(angle) };
			float waveAmp = 4.0f;
			float waveFreq = 0.5f;

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
						.SetVelocity(100.f)
						.SetWave(waveAmp, waveFreq)
						.SetDecayTime(8.5f)
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

void Demo::TrojanEnemy::PatternFanning(float projDamage)
{
	const int WAVE_COUNT = 15;
	const int BULLET_COUNT = 25;
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
							ModifierType::Burn, BURN_VALUE, BURN_DURATION
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
