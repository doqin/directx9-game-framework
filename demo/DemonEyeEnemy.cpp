#include "pch.h"
#include "DemonEyeEnemy.h"
#include "resource.h"
#include "RoundProjectile.h"
#include <random>

void Demo::DemonEyeEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera)
{
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"assets/computerbug-Sheet.png");
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 12);
	sprite->SetOrigin(32, 32);
	sprite->SetScale(2.f);

	tearProjectileTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	tearProjectileTexture->LoadTexture(L"assets/bugprojectile-Sheet.png");
	tearProjectileFrames = DX9GF::Utils::CreateRectsHorizontal(0, 0, 16, 16, 4);

	SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
	InitCardSpawnTrigger(camera, 128.f, 128.f);
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

void Demo::DemonEyeEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera)
{
	(void)enemies;
	(void)popUpMessage;
	(void)graphicsDevice;
	(void)camera;
	this->player = player;
	float baseDamage = 4.f;
	float projDamage = GetOutgoingDamage(baseDamage);

	//surprise element
	int patternId = GetRandomPattern();

	//PatternBloodCross(projDamage, enemies);
	if (patternId == 1) PatternBloodRain(projDamage, enemies);
	else if (patternId == 2) PatternBloodWall(projDamage, enemies);
	else PatternBloodCross(projDamage, enemies);
	//commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(1.f));
}

void Demo::DemonEyeEnemy::PatternBloodRain(float projDamage, std::vector<std::shared_ptr<IEnemy>>* enemies)
{
	bool isAlone = enemies->size() == 1;
	const int BULLET_COUNT = isAlone ? 180 : 90;
	const float SPAWN_DELAY = isAlone ? 0.01f : 0.02f;
	const float BULLET_SPEED = isAlone ? 380.f : 190.f;
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

				auto projSprite = std::make_shared<DX9GF::AnimatedSprite>(tearProjectileTexture.get(), tearProjectileFrames, 12);
				projSprite->SetOrigin(8, 8);
				projectiles.push_back(
					RoundProjectile::Builder(transformManager, lock, projSprite, 16, 16, finalX, finalY)
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

void Demo::DemonEyeEnemy::PatternBloodWall(float projDamage, std::vector<std::shared_ptr<IEnemy>>* enemies)
{
	bool isAlone = enemies->size() == 1;
	const int WAVE_COUNT = isAlone ? 9 : 5;
	const int BULLET_PER_WAVE = 8;
	const float WAVE_DELAY = isAlone ? 0.6f : 1.2f;
	const float BULLET_SPEED = isAlone ? 200.f : 100.f;
	const float DROP_HEIGHT = 350.f;
	const float WALL_START_X = -200.f;
	const float BULLET_SPACING = isAlone ? 40.f : 80.f;
	const float BULLET_DECAY_TIME = isAlone ? 4.f : 8.f;

	std::random_device rd;
	std::mt19937 gen(rd());

	for (int wave = 0; wave < WAVE_COUNT; wave++) {
		//random hole
		std::uniform_int_distribution<int> holeDist(0, BULLET_PER_WAVE - 1);
		int emptyHole = holeDist(gen);

		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, emptyHole, BULLET_PER_WAVE, BULLET_SPEED, DROP_HEIGHT, WALL_START_X, BULLET_SPACING, BULLET_DECAY_TIME](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [playerX, playerY] = lock->GetWorldPosition();

				for (int i = 0; i < BULLET_PER_WAVE; i++) {
					if (i == emptyHole) continue;

					float offsetX = WALL_START_X + (i * BULLET_SPACING);
					float finalX = playerX + offsetX;
					float finalY = playerY - DROP_HEIGHT;

					auto projSprite = std::make_shared<DX9GF::AnimatedSprite>(tearProjectileTexture.get(), tearProjectileFrames, 12);
					projSprite->SetOrigin(8, 8);
					projectiles.push_back(
						RoundProjectile::Builder(transformManager, lock, projSprite, 16, 16, finalX, finalY)
						.SetTrajectory(D3DXVECTOR2(0, 1))
						.SetDelay(0.f)
						.SetDecayTime(BULLET_DECAY_TIME)
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

void Demo::DemonEyeEnemy::PatternBloodCross(float projDamage, std::vector<std::shared_ptr<IEnemy>>* enemies)
{
	bool isAlone = enemies->size() == 1;
	const int BULLET_COUNT = isAlone ? 180 : 70;
	const float SPAWN_DELAY = isAlone ? 0.013f : 0.050f;
	const float BULLET_SPEED = isAlone ? 300.f : 200.f;
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

			auto projSprite = std::make_shared<DX9GF::AnimatedSprite>(tearProjectileTexture.get(), tearProjectileFrames, 12);
			projSprite->SetOrigin(8, 8);
			projectiles.push_back(
				RoundProjectile::Builder(transformManager, lock, projSprite, 16, 16, finalX, finalY)
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