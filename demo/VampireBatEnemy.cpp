#include "pch.h"
#include "VampireBatEnemy.h"
#include "resource.h"
#include "SineWaveProjectile.h"
#include "BoomerangProjectile.h"
#include <random>

void Demo::VampireBatEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"shrimp-Sheet.png");
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 12);
	sprite->SetOrigin(32, 32);
	sprite->SetScale(2.f);

	projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	projTexture->LoadTexture(L"spearprojectile.png");
	projSprite = std::make_shared<DX9GF::StaticSprite>(projTexture.get());
	projSprite->SetOrigin(16, 8);

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
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(1, 2);
	return dist(gen);
}

void Demo::VampireBatEnemy::StartAttack(std::shared_ptr<Player> player) {
	this->player = player;

	if (GetRandomPattern() == 1) PatternEcholocation(GetOutgoingDamage(2.f));
	else PatternSwoopBite(GetOutgoingDamage(4.f));

	commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(1.f));
}

void Demo::VampireBatEnemy::PatternEcholocation(float projDamage) {
	const int BULLETS = 10;
	const int VELOCITY = 180;
	const int SPACING = 96;
	auto rightAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage](std::function<void(void)> markFinished) {
		for (int i = 0; i < BULLETS; i++) {
			if (auto lock = this->player.lock()) {
				float startY = (i - BULLETS / 2.f) * SPACING;
				projectiles.push_back(
					SineWaveProjectile::Builder(transformManager, lock, projSprite, 16, 16, 320, startY)
					.SetTrajectory(D3DXVECTOR2(-1, 0))
					.SetWave(50.f, 4.f)
					.SetDelay(i * 0.1f)
					.SetDecayTime(4.f)
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
	auto leftAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage](std::function<void(void)> markFinished) {
		for (int i = 0; i < BULLETS; i++) {
			if (auto lock = this->player.lock()) {
				float startY = (i - BULLETS / 2.f) * SPACING;
				projectiles.push_back(
					SineWaveProjectile::Builder(transformManager, lock, projSprite, 16, 16, -320, startY)
					.SetTrajectory(D3DXVECTOR2(1, 0))
					.SetWave(50.f, 4.f)
					.SetDelay(i * 0.1f)
					.SetDecayTime(4.f)
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
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(1, 2);
	for (int i = 0; i < 20; i++) {
		if (dist(gen) == 1) commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*leftAttack));
		else commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*rightAttack));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
	}
	
}

void Demo::VampireBatEnemy::PatternSwoopBite(float projDamage) {
	const int BULLET_COUNT = 8;
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
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(15, 20);
	std::uniform_int_distribution<int> sideDist(1, 2);
	for (int i = 0; i < dist(gen); i++) {
		if (sideDist(gen) == 1) commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*leftAttack));
		else commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*rightAttack));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
	}
	
}