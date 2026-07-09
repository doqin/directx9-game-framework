#include "pch.h"
#include "VampireBatEnemy.h"
#include "resource.h"
#include "SineWaveProjectile.h"
#include "BoomerangProjectile.h"
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

void Demo::VampireBatEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	(void)enemies;
	(void)popUpMessage;
	(void)graphicsDevice;
	(void)camera;
	this->player = player;

	if (GetRandomPattern() == 1) PatternEcholocation(GetOutgoingDamage(2.f), enemies);
	else PatternSwoopBite(GetOutgoingDamage(4.f));

	//commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(1.f));
}

void Demo::VampireBatEnemy::PatternEcholocation(float projDamage, std::vector<std::shared_ptr<IEnemy>>* enemies) {
	bool isAlone = enemies->size() == 1;
	const int BULLETS = isAlone ? 10 : 5;
	const int VELOCITY = isAlone ? 180 : 90;
	const int SPACING = 96;
	const int AMPLITUDE = isAlone ? 50 : 25;
	const int DECAY_TIME = isAlone ? 4 : 8;
	auto rightAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage, VELOCITY, AMPLITUDE, DECAY_TIME, BULLETS](std::function<void(void)> markFinished) {
		for (int i = 0; i < BULLETS; i++) {
			if (auto lock = this->player.lock()) {
				float startY = (i - BULLETS / 2.f) * SPACING;
				auto projSprite = std::make_shared<DX9GF::StaticSprite>(projTexture.get());
				projSprite->SetOrigin(16, 8);
				projectiles.push_back(
					SineWaveProjectile::Builder(transformManager, lock, projSprite, 16, 16, 320, startY)
					.SetTrajectory(D3DXVECTOR2(-1, 0))
					.SetWave(AMPLITUDE, 4.f)
					.SetDelay(i * 0.1f)
					.SetDecayTime(DECAY_TIME)
					.SetVelocity(VELOCITY)
					.SetDamage(projDamage)
					.Build()
				);
				projectiles.back()->Init();
				transformManager.lock()->RebuildHierarchy();
			}
		}
		markFinished();
	});
	auto leftAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage, VELOCITY, AMPLITUDE, DECAY_TIME, BULLETS](std::function<void(void)> markFinished) {
		for (int i = 0; i < BULLETS; i++) {
			if (auto lock = this->player.lock()) {
				float startY = (i - BULLETS / 2.f) * SPACING;
				auto projSprite = std::make_shared<DX9GF::StaticSprite>(projTexture.get());
				projSprite->SetOrigin(16, 8);
				projectiles.push_back(
					SineWaveProjectile::Builder(transformManager, lock, projSprite, 16, 16, -320, startY)
					.SetTrajectory(D3DXVECTOR2(1, 0))
					.SetWave(AMPLITUDE, 4.f)
					.SetDelay(i * 0.1f)
					.SetDecayTime(DECAY_TIME)
					.SetVelocity(VELOCITY)
					.SetDamage(projDamage)
					.Build()
				);
				projectiles.back()->Init();
				transformManager.lock()->RebuildHierarchy();
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

void Demo::VampireBatEnemy::PatternSwoopBite(float projDamage) {
	const int BULLET_COUNT = 4;
	const float ANGLE_STEP = 0.02f;
	auto rightAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage, BULLET_COUNT, ANGLE_STEP](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
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

				auto projSprite = std::make_shared<DX9GF::StaticSprite>(projTexture.get());
				projSprite->SetOrigin(16, 8);
				projectiles.push_back(
					BoomerangProjectile::Builder(transformManager, lock, projSprite, 16, 16, spawnX, spawnY)
					.SetTargetPosition(targetX, targetY)
					.SetInitialVelocity(400.f)
					.SetReturnAcceleration(180.f)
					.SetDelay(i * 0.05f)
					.SetDecayTime(8.f)
					.SetDamage(projDamage)
					.Build()
				);
				projectiles.back()->Init();
			}
			transformManager.lock()->RebuildHierarchy();
		}
		markFinished();
	});
	auto leftAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage, BULLET_COUNT, ANGLE_STEP](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
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

				auto projSprite = std::make_shared<DX9GF::StaticSprite>(projTexture.get());
				projSprite->SetOrigin(16, 8);
				projectiles.push_back(
					BoomerangProjectile::Builder(transformManager, lock, projSprite, 16, 16, spawnX, spawnY)
					.SetTargetPosition(targetX, targetY)
					.SetInitialVelocity(400.f)
					.SetReturnAcceleration(180.f)
					.SetDelay(i * 0.05f)
					.SetDecayTime(8.f)
					.SetDamage(projDamage)
					.Build()
				);
				projectiles.back()->Init();
			}
			transformManager.lock()->RebuildHierarchy();
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