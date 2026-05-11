#include "pch.h"
#include "VampireBatEnemy.h"
#include "resource.h"
#include "SineWaveProjectile.h"
#include "BoomerangProjectile.h"
#include <random>

void Demo::VampireBatEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(IDB_PNG4);
	sprite = std::make_shared<DX9GF::StaticSprite>(texture.get());
	sprite->SetOrigin(32, 32);

	projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	projTexture->LoadTexture(IDB_PNG5);
	projSprite = std::make_shared<DX9GF::StaticSprite>(projTexture.get());
	projSprite->SetOrigin(8, 8);

   SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
	InitCardSpawnTrigger(camera, 64.f, 64.f);
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
	float projDamage = GetOutgoingDamage(4.f);

	if (GetRandomPattern() == 1) PatternEcholocation(projDamage);
	else PatternSwoopBite(projDamage);

	commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(3.f));
}

void Demo::VampireBatEnemy::PatternEcholocation(float projDamage) {
	for (int i = 0; i < 8; i++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, i](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				float startY = GetWorldY() - 100.f + (i * 30.f);
				projectiles.push_back(
					SineWaveProjectile::Builder(transformManager, lock, projSprite, 16, 16, GetWorldX(), startY)
					.SetTrajectory(D3DXVECTOR2(-1, 0))
					.SetWave(50.f, 4.f)
					.SetDelay(0.f)
					.SetDecayTime(4.f)
					.SetVelocity(160.f)
					.SetDamage(projDamage)
					.Build()
				);
				projectiles.back()->Init();
				transformManager.lock()->RebuildHierarchy();
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.2f));
	}
}

void Demo::VampireBatEnemy::PatternSwoopBite(float projDamage) {
	const int BULLET_COUNT = 7;

	commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, BULLET_COUNT](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			auto [px, py] = lock->GetWorldPosition();

			float dx = px - GetWorldX();
			float dy = py - GetWorldY();
			float baseAngle = std::atan2(dy, dx);

			float angleStep = 0.15f;

			for (int i = 0; i < BULLET_COUNT; i++) {
				float offsetAngle = (i - (BULLET_COUNT / 2)) * angleStep;
				float finalAngle = baseAngle + offsetAngle;

				float targetX = GetWorldX() + std::cos(finalAngle) * 500.f;
				float targetY = GetWorldY() + std::sin(finalAngle) * 500.f;

				float spawnX = GetWorldX() + (i - (BULLET_COUNT / 2)) * 15.f;
				float spawnY = GetWorldY() - 10.f;

				projectiles.push_back(
					BoomerangProjectile::Builder(transformManager, lock, projSprite, 32, 32, spawnX, spawnY)
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
		}));
}