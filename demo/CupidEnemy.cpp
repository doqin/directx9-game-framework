#include "pch.h"
#include "CupidEnemy.h"
#include "resource.h"
#include "RoundProjectile.h"
#include "SineWaveProjectile.h"
#include "TargetedProjectile.h"
#include <random>

const float PI = 3.14159265359f;

void Demo::CupidEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera)
{
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(IDB_PNG4); // TODO: enemy img
	sprite = std::make_shared<DX9GF::StaticSprite>(texture.get());
	sprite->SetOrigin(32, 32);

	heartTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	heartTexture->LoadTexture(IDB_PNG5); // TODO: Heart img
	heartSprite = std::make_shared<DX9GF::StaticSprite>(heartTexture.get());
	heartSprite->SetOrigin(8, 8);

	arrowTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	arrowTexture->LoadTexture(IDB_PNG5); // TODO: Arrow img
	arrowSprite = std::make_shared<DX9GF::StaticSprite>(arrowTexture.get());
	arrowSprite->SetOrigin(8, 8);

	SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
	InitCardSpawnTrigger(camera, 128.f, 128.f);
}

void Demo::CupidEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime)
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

int Demo::CupidEnemy::GetRandomPattern()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(1, 3);
	return dist(gen);
}

void Demo::CupidEnemy::StartAttack(std::shared_ptr<Player> player)
{
	this->player = player;
	float baseDamage = 4.f;
	float projDamage = GetOutgoingDamage(baseDamage);

	int patternId = GetRandomPattern();
	if (patternId == 1) PatternHeartWave(projDamage);
	else if (patternId == 2) PatternHomingArrow(projDamage);
	else PatternHeartNova(projDamage);

	commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(4.f));
}

void Demo::CupidEnemy::PatternHeartWave(float projDamage)
{
	const int BULLET_COUNT = 20;
	const float SPAWN_DELAY = 0.25f;
	const float BULLET_SPEED = 140.f;
	const float AMPLITUDE = 100.f;
	const float FREQUENCY = 6.f;
	const float START_X = -400.f;
	const float OFFSET_RANGE = 170.f;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> offsetDist(-OFFSET_RANGE, OFFSET_RANGE);

	for (int i = 0; i < BULLET_COUNT; i++) {
		float offsetY = offsetDist(gen);

		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, offsetY, BULLET_SPEED, AMPLITUDE, FREQUENCY, START_X](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [playerX, playerY] = lock->GetWorldPosition();
				float finalY = playerY + offsetY;

				projectiles.push_back(
					SineWaveProjectile::Builder(transformManager, lock, heartSprite, 16, 16, START_X, finalY)
					.SetTrajectory(D3DXVECTOR2(1, 0))
					.SetWave(AMPLITUDE, FREQUENCY)
					.SetDelay(0.f)
					.SetDecayTime(5.f)
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

void Demo::CupidEnemy::PatternHomingArrow(float projDamage)
{
	const int ARROW_COUNT = 11;
	const float SPAWN_DELAY = 0.50f;
	const float BULLET_SPEED = 180.f;	// should set this slightly higher than player's speed
	const float TURN_SPEED = 3.f;		// sharpness of turn
	const float OFFSET_RANGE = 250.f;
	const float DROP_HEIGHT = 350.f; 

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> offsetDist(-OFFSET_RANGE, OFFSET_RANGE);

	for (int i = 0; i < ARROW_COUNT; i++) {
		float offsetX = offsetDist(gen);

		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, offsetX, BULLET_SPEED, TURN_SPEED, DROP_HEIGHT](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [playerX, playerY] = lock->GetWorldPosition();
				float finalX = playerX + offsetX;
				float finalY = playerY - DROP_HEIGHT;

				projectiles.push_back(
					TargetedProjectile::Builder(transformManager, lock, arrowSprite, 16, 16, finalX, finalY)
					.SetTrajectory(D3DXVECTOR2(0, 1))
					.SetHoming(TURN_SPEED)
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

void Demo::CupidEnemy::PatternHeartNova(float projDamage)
{
	const int BURST_COUNT = 9;
	const int BULLET_PER_RING = 29;
	const float BURST_DELAY = 0.3f; 
	const float BULLET_SPEED = 130.f;
	const float SPAWN_OFFSET_Y = -200.f; 

	for (int wave = 0; wave < BURST_COUNT; wave++) {
		//simple trick to avoid blindspot
		float angleOffset = (wave % 2 == 0) ? 0.f : (PI / BULLET_PER_RING);
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, BULLET_PER_RING, BULLET_SPEED, SPAWN_OFFSET_Y, angleOffset](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [playerX, playerY] = lock->GetWorldPosition();
				float originX = playerX;
				float originY = playerY + SPAWN_OFFSET_Y;

				float angleStep = (2.f * PI) / BULLET_PER_RING;

				for (int i = 0; i < BULLET_PER_RING; i++) {
					float currentAngle = i * angleStep + angleOffset;
					D3DXVECTOR2 dir(std::cos(currentAngle), std::sin(currentAngle));

					projectiles.push_back(
						RoundProjectile::Builder(transformManager, lock, heartSprite, 16, 16, originX, originY)
						.SetTrajectory(dir)
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
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(BURST_DELAY));
	}
}