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

	int cycle = (currentTurn - 1) / 3;

	if (currentCycle != cycle) {
		currentCycle = cycle;
		if (RNG::Range(1, 100) <= 60) {
			skillTurnThisCycle = RNG::Range(1, 3);
		}
		else {
			skillTurnThisCycle = -1;
		}
	}

	int turnInCycle = (currentTurn - 1) % 3 + 1;

	if (turnInCycle == skillTurnThisCycle) {
		if (RNG::Range(1, 2) == 1) {
			CastAbility([this]() { this->Heal(15.f); }, popUpMessage, L"Pacman heals 15 HP!");
		}
		else {
			CastAbility([this]() {
				if (auto lock = this->player.lock()) lock->AddModifier(ModifierType::Vulnerable, 2, 0.f, false);
				}, popUpMessage, L"Pacman breaks your guard!");
		}
	}
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
	const int WAVE_COUNT = 8;
	const int ARROW_COUNT = 15;
	const float SPAWN_DELAY = 0.05f;
	const float WAVE_DELAY = 0.4f;
	const float BULLET_SPEED = 600.f;
	const float TURN_SPEED = 2.f;		// sharpness of turn
	const float OFFSET_RANGE = 300.f;
	const float DROP_HEIGHT = 350.f;

	for (int wave = 0; wave < WAVE_COUNT; wave++) {
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
						.SetDecayTime(3.f)
						.SetVelocity(BULLET_SPEED)
						.SetDamage(projDamage)
						.SetStatusEffect(ModifierType::Poison, 0.f, 1)
					);
				}
				markFinished();
				}));
			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(SPAWN_DELAY));
		}
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(WAVE_DELAY));
	}
}

void Demo::CupidEnemy::PatternHeartNova(float projDamage)
{
	const int WAVE_COUNT = 20;
	const int BURST_COUNT = 5;
	const int BULLET_PER_ARC = 7;
	const float BURST_DELAY = 0.01f;
	const float BULLET_SPEED = 250.f;
	const float SPAWN_OFFSET_Y = -200.f;
	const float DECAY_TIME = 1.5f;
	std::shared_ptr<float> angleShift = std::make_shared<float>(-PI / 2);

	for (int wave = 0; wave < WAVE_COUNT; wave++) {
		for (int burst = 0; burst < BURST_COUNT; burst++) {
			float angleOffset = (burst - (BURST_COUNT / 2)) * (PI / (BULLET_PER_ARC - 1)) * 0.25f;
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, BULLET_PER_ARC, BULLET_SPEED, SPAWN_OFFSET_Y, angleOffset, DECAY_TIME, angleShift](std::function<void(void)> markFinished) {
				if (auto lock = this->player.lock()) {
					auto [playerX, playerY] = lock->GetWorldPosition();
					float originX = 0;
					float originY = SPAWN_OFFSET_Y;

					float baseAngle = std::atan2(playerY - originY, playerX - originX);
					float angleStep = PI / (BULLET_PER_ARC - 1);
					float startAngle = baseAngle - (PI * 0.5f);

					for (int i = 0; i < BULLET_PER_ARC; i++) {
						float currentAngle = startAngle + i * angleStep + angleOffset + *angleShift;
						D3DXVECTOR2 dir(std::cos(currentAngle), std::sin(currentAngle));

						projectiles.Spawn(
							lock,
							ProjectileDesc(heartTexture.get(), 8, 8, 16, 16, originX, originY)
							.SetTrajectory(dir)
							.SetDelay(1.f)
							.SetDecayTime(DECAY_TIME + 1.f)
							.SetVelocity(BULLET_SPEED)
							.SetDamage(projDamage)
						);
					}
					*angleShift += PI / 20.f; // Increment the angle shift for the next burst
					if (*angleShift > PI / 2) {
						*angleShift = -PI / 2; // Keep the angle shift within the range of -PI/2 to PI/2
					}
				}
				markFinished();
				}));
			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(BURST_DELAY));
		}
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.2f)); // delay between waves
	}
}