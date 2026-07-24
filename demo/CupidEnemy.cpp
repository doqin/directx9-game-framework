#include "pch.h"
#include "CupidEnemy.h"
#include "resource.h"
#include "RNG.h"
#include "PopUpMessage.h"
const float PI = 3.14159265359f;

void Demo::CupidEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera)
{
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"assets/bubble-Sheet.png");
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 11), 12);
	sprite->SetOrigin(32, 32);
	sprite->SetScale(2.f);

	heartTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	heartTexture->LoadTexture(L"assets/bubbleprojectile.png");

	arrowTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	arrowTexture->LoadTexture(L"assets/bubbleprojectile.png");

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
	return RNG::Range(1, 3);
}

void Demo::CupidEnemy::OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) {
	this->player = player;

	int cycle = (currentTurn - 1) / 4;

	if (currentCycle != cycle) {
		currentCycle = cycle;
		if (RNG::Range(1, 100) <= 40) {
			skillTurnThisCycle = RNG::Range(1, 4); 
		}
		else {
			skillTurnThisCycle = -1;
		}
	}

	int turnInCycle = (currentTurn - 1) % 4 + 1;

	if (turnInCycle == skillTurnThisCycle) {
		int skillType = RNG::Range(1,2);

		if (skillType == 1) AbilityHealSelf();
		else AbilityVulnerablePlayer();

		if (popUpMessage) {
			popUpMessage->QueueMessage(&commandBuffer, L"Pacman is hungry for data!", 1.5f);
		}
	}
}

void Demo::CupidEnemy::AbilityHealSelf() {
	commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void(void)> markFinished) {
		this->Heal(15.f);
		markFinished();
		}));
	commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
}

void Demo::CupidEnemy::AbilityVulnerablePlayer() {
	commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			lock->AddModifier(ModifierType::Vulnerable, 2, 0.f, false);
		}
		markFinished();
		}));
	commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
}

void Demo::CupidEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn)
{
	(void)enemies;
	(void)graphicsDevice;
	(void)camera;
	this->player = player;
	//remember to calc the finaldmg
	float baseDamage = 4.f;
	float finalDamage = CalculateOutgoingDamage(baseDamage);

	int patternId = GetSmartRandomPattern(1, 3);
	if (patternId == 1) PatternHeartWave(finalDamage);
	else if (patternId == 2) PatternHomingArrow(finalDamage);
	else PatternHeartNova(finalDamage);
}

void Demo::CupidEnemy::PatternHeartWave(float projDamage)
{
	const int BULLET_COUNT = 126;
	const float SPAWN_DELAY = 0.01f;
	const float BULLET_SPEED = 300.f;
	const float START_X = -200.f;
	const float START_Y = 0.f;
	const float DECAY_TIME = 4.f;
	const float MIN_ANGLE = -PI * 0.5f;
	const float MAX_ANGLE = PI * 0.5f;
	const float MIN_STEP = PI * 0.08f;
	const float MAX_STEP = PI * 0.22f;
	float angle = MIN_ANGLE;
	float direction = 1.f;
	for (int i = 0; i < BULLET_COUNT; i++) {
		float currentAngle = angle;

		angle += direction * RNG::Range(MIN_STEP, MAX_STEP);
		if (angle > MAX_ANGLE) {
			angle = MAX_ANGLE - (angle - MAX_ANGLE);
			direction = -1.f;
		}
		else if (angle < MIN_ANGLE) {
			angle = MIN_ANGLE + (MIN_ANGLE - angle);
			direction = 1.f;
		}

		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, BULLET_SPEED, START_X, START_Y, DECAY_TIME, currentAngle](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				D3DXVECTOR2 dir(std::cos(currentAngle), std::sin(currentAngle));

				projectiles.Spawn(
					lock,
					ProjectileDesc(heartTexture.get(), 8, 8, 16, 16, START_X, START_Y)
					.SetTrajectory(dir)
					.SetDelay(0.f)
					.SetDecayTime(DECAY_TIME)
					.SetVelocity(BULLET_SPEED)
					.SetDamage(projDamage)
				);
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

	for (int i = 0; i < ARROW_COUNT; i++) {
		float offsetX = RNG::Range(-OFFSET_RANGE, OFFSET_RANGE);

		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, offsetX, BULLET_SPEED, TURN_SPEED, DROP_HEIGHT](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [playerX, playerY] = lock->GetWorldPosition();
				float finalX = playerX + offsetX;
				float finalY = playerY - DROP_HEIGHT;

				projectiles.Spawn(
					lock,
					ProjectileDesc(arrowTexture.get(), 8, 8, 16, 16, finalX, finalY)
					.SetTrajectory(D3DXVECTOR2(0, 1))
					.SetHoming(TURN_SPEED)
					.SetDelay(0.f)
					.SetDecayTime(4.f)
					.SetVelocity(BULLET_SPEED)
					.SetDamage(projDamage)
				);
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(SPAWN_DELAY));
	}
}

void Demo::CupidEnemy::PatternHeartNova(float projDamage)
{
	const int WAVE_COUNT = 10;
	const int BURST_COUNT = 5;
	const int BULLET_PER_ARC = 8;
	const float BURST_DELAY = 0.05f;
	const float BULLET_SPEED = 200.f;
	const float SPAWN_OFFSET_Y = -200.f;
	const float DECAY_TIME = 2.f;

	for (int wave = 0; wave < WAVE_COUNT; wave++) {
		for (int burst = 0; burst < BURST_COUNT; burst++) {
			float angleOffset = (burst - (BURST_COUNT / 2)) * (PI / (BULLET_PER_ARC - 1)) * 0.5f;
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, BULLET_PER_ARC, BULLET_SPEED, SPAWN_OFFSET_Y, angleOffset, DECAY_TIME](std::function<void(void)> markFinished) {
				if (auto lock = this->player.lock()) {
					auto [playerX, playerY] = lock->GetWorldPosition();
					float originX = 0;
					float originY = SPAWN_OFFSET_Y;

					float baseAngle = std::atan2(playerY - originY, playerX - originX);
					float angleStep = PI / (BULLET_PER_ARC - 1);
					float startAngle = baseAngle - (PI * 0.5f);

					for (int i = 0; i < BULLET_PER_ARC; i++) {
						float currentAngle = startAngle + i * angleStep + angleOffset;
						D3DXVECTOR2 dir(std::cos(currentAngle), std::sin(currentAngle));

						projectiles.Spawn(
							lock,
							ProjectileDesc(heartTexture.get(), 8, 8, 16, 16, originX, originY)
							.SetTrajectory(dir)
							.SetDelay(0.f)
							.SetDecayTime(DECAY_TIME)
							.SetVelocity(BULLET_SPEED)
							.SetDamage(projDamage)
						);
					}
				}
				markFinished();
				}));
			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(BURST_DELAY));
		}
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.2f)); // delay between waves
	}
}