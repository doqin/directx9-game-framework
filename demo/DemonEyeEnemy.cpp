#include "pch.h"
#include "DemonEyeEnemy.h"
#include "resource.h"
#include "RoundProjectile.h"
#include <random>

void Demo::DemonEyeEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera)
{
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(IDB_PNG4); // TODO: enemy img
	sprite = std::make_shared<DX9GF::StaticSprite>(texture.get());
	sprite->SetOrigin(32, 32);

	tearProjectileTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	tearProjectileTexture->LoadTexture(IDB_PNG5); // TODO: blood drop img
	tearProjectileSprite = std::make_shared<DX9GF::StaticSprite>(tearProjectileTexture.get());
	tearProjectileSprite->SetOrigin(8, 8);

   SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
	InitCardSpawnTrigger(camera, 64.f, 64.f);
}

void Demo::DemonEyeEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime)
{
	if (sprite) {
		sprite->Begin();
		auto [x, y] = GetWorldPosition();
		sprite->SetPosition(x, y);
		sprite->Draw(*camera, deltaTime);
		sprite->End();
	}
	IEnemy::Draw(graphicsDevice, camera, deltaTime);
}

int Demo::DemonEyeEnemy::GetRandomPattern()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(1, 3);
	return dist(gen);
}

void Demo::DemonEyeEnemy::StartAttack(std::shared_ptr<Player> player)
{
	this->player = player;
	float baseDamage = 4.f;
	float projDamage = GetOutgoingDamage(baseDamage);

	//surprise element
	int patternId = GetRandomPattern();

	if (patternId == 1) PatternBloodRain(projDamage);
	else if (patternId == 2) PatternBloodWall(projDamage);
	else PatternBloodCross(projDamage);
	commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(5.f));
}

void Demo::DemonEyeEnemy::PatternBloodRain(float projDamage)
{
	const int BULLET_COUNT = 180;
	const float SPAWN_DELAY = 0.01f;
	const float BULLET_SPEED = 380.f;
	const float OFFSET_RANGE = 300.f;
	const float DROP_HEIGHT = 350.f;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> offsetDist(-OFFSET_RANGE, OFFSET_RANGE);

	for (int i = 0; i < BULLET_COUNT; i++) {
		float offsetX = offsetDist(gen);

		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, offsetX, DROP_HEIGHT, BULLET_SPEED](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [playerX, playerY] = lock->GetWorldPosition();

				float finalX = playerX + offsetX;
				float finalY = playerY - DROP_HEIGHT;

				projectiles.push_back(
					RoundProjectile::Builder(transformManager, lock, tearProjectileSprite, 16, 16, finalX, finalY)
					.SetTrajectory(D3DXVECTOR2(0, 1))
					.SetDelay(0.f)
					.SetDecayTime(4.f)
					.SetVelocity(BULLET_SPEED)
					.SetDamage(projDamage)
					.Build()
				);
				projectiles.back()->Init();
				transformManager.lock()->RebuildHierarchy();
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(SPAWN_DELAY));
	}
}

void Demo::DemonEyeEnemy::PatternBloodWall(float projDamage)
{
	const int WAVE_COUNT = 9;
	const int BULLET_PER_WAVE = 15;
	const float WAVE_DELAY = 0.6f;
	const float BULLET_SPEED = 200.f;
	const float DROP_HEIGHT = 350.f;
	const float WALL_START_X = -200.f;
	const float BULLET_SPACING = 40.f;

	std::random_device rd;
	std::mt19937 gen(rd());

	for (int wave = 0; wave < WAVE_COUNT; wave++) {
		//random hole
		std::uniform_int_distribution<int> holeDist(0, BULLET_PER_WAVE - 1);
		int emptyHole = holeDist(gen);

		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, emptyHole, BULLET_PER_WAVE, BULLET_SPEED, DROP_HEIGHT, WALL_START_X, BULLET_SPACING](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [playerX, playerY] = lock->GetWorldPosition();

				for (int i = 0; i < BULLET_PER_WAVE; i++) {
					if (i == emptyHole) continue;

					float offsetX = WALL_START_X + (i * BULLET_SPACING);
					float finalX = playerX + offsetX;
					float finalY = playerY - DROP_HEIGHT;

					projectiles.push_back(
						RoundProjectile::Builder(transformManager, lock, tearProjectileSprite, 16, 16, finalX, finalY)
						.SetTrajectory(D3DXVECTOR2(0, 1))
						.SetDelay(0.f)
						.SetDecayTime(4.f)
						.SetVelocity(BULLET_SPEED)
						.SetDamage(projDamage)
						.Build()
					);
					projectiles.back()->Init();
				}
				transformManager.lock()->RebuildHierarchy();
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(WAVE_DELAY));
	}
}

void Demo::DemonEyeEnemy::PatternBloodCross(float projDamage)
{
	const int BULLET_COUNT = 180;
	const float SPAWN_DELAY = 0.013f;
	const float BULLET_SPEED = 300.f;
	const float DROP_HEIGHT = 350.f;
	const float OFFSET_MIN = -450.f;
	const float OFFSET_MAX = 150.f;

	std::random_device rd;
	std::mt19937 gen(rd());

	//since the bullets veer to the RIGHT(0.5, 1.0), shift the spawn area further to the left to ensure they hit the Player.
	std::uniform_real_distribution<float> offsetDist(OFFSET_MIN, OFFSET_MAX);

	for (int i = 0; i < BULLET_COUNT; i++) {
		float offsetX = offsetDist(gen);

		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, offsetX, BULLET_SPEED, DROP_HEIGHT](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [playerX, playerY] = lock->GetWorldPosition();

				float finalX = playerX + offsetX;
				float finalY = playerY - DROP_HEIGHT;

				projectiles.push_back(
					RoundProjectile::Builder(transformManager, lock, tearProjectileSprite, 16, 16, finalX, finalY)
					.SetTrajectory(D3DXVECTOR2(0.5f, 1.0f))
					.SetDelay(0.f)
					.SetDecayTime(4.f)
					.SetVelocity(BULLET_SPEED)
					.SetDamage(projDamage)
					.Build()
				);
				projectiles.back()->Init();
				transformManager.lock()->RebuildHierarchy();
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(SPAWN_DELAY));
	}
}