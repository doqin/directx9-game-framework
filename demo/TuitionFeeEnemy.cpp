#include "pch.h"
#include "TuitionFeeEnemy.h"
#include "PopUpMessage.h"
#include "GameItems.h"
#include "RNG.h"
#include <cmath>

void Demo::TuitionFeeEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera)
{
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"assets/placeholder.png"); //TODO: change to the real asset path
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 12);
	sprite->SetOrigin(32, 32);
	sprite->SetScale(2.f);

	bulletTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	bulletTexture->LoadTexture(L"assets/placeholder-round-projectile.png"); //TODO: change to the real asset path
	bulletFrames = DX9GF::Utils::CreateRectsHorizontal(0, 0, 16, 16, 4);

	SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
	InitCardSpawnTrigger(camera, 128.f, 128.f);
}

void Demo::TuitionFeeEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime)
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

void Demo::TuitionFeeEnemy::OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn)
{
	this->player = player;

	if (!hasSpawnTurn) {
		hasSpawnTurn = true;
		spawnTurn = currentTurn;

		CastAbility([this, player, popUpMessage]() {
			this->BanRandomItems(player, popUpMessage);
			}, popUpMessage, L"");
		return;
	}

	if (player) player->GetInventoryItems().TickLocks();

	const int turnsSinceSpawn = currentTurn - spawnTurn;

	if (turnsSinceSpawn > 0 && turnsSinceSpawn % BUFF_WIPE_INTERVAL == 0) {
		CastAbility([player]() {
			if (player) player->ClearBuffs();
			}, popUpMessage, L"Tuition Fee wipes out all your buffs!");
	}

	if (turnsSinceSpawn > 0 && turnsSinceSpawn % 2 == 1) {
		CastAbility([this]() {
			if (onRequestVisionDebuff) onRequestVisionDebuff(VISION_DEBUFF_DURATION);
			}, popUpMessage, L"Tuition Fee blurs your sight!");
	}
}

void Demo::TuitionFeeEnemy::BanRandomItems(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage)
{
	if (!player) return;

	constexpr int ITEM_POOL_MIN = 0;
	constexpr int ITEM_POOL_MAX = 9;
	constexpr int BAN_DURATION = 3;

	std::vector<int> pool;
	for (int id = ITEM_POOL_MIN; id <= ITEM_POOL_MAX; ++id) pool.push_back(id);

	std::wstring bannedNames;
	int pickCount = (std::min)(2, static_cast<int>(pool.size()));
	for (int i = 0; i < pickCount; ++i) {
		int idx = RNG::Range(0, static_cast<int>(pool.size()) - 1);
		int itemID = pool[idx];
		pool.erase(pool.begin() + idx);

		player->GetInventoryItems().LockItem(itemID, BAN_DURATION);

		auto blueprint = Demo::ItemData::GetInstance()->GetItemBlueprint(itemID);
		if (blueprint) {
			if (!bannedNames.empty()) bannedNames += L", ";
			bannedNames += blueprint->GetName();
		}
	}

	if (popUpMessage && !bannedNames.empty()) {
		popUpMessage->ShowMessage(L"Tuition Fee bans your access to: " + bannedNames + L"!", 3.f);
	}
}

void Demo::TuitionFeeEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies,
	std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice,
	DX9GF::Camera* camera, int currentTurn)
{
	(void)popUpMessage;
	(void)graphicsDevice;
	(void)camera;
	(void)currentTurn;
	this->player = player;

	float baseDamage = 4.f;
	int patternId = GetSmartRandomPattern(1, 3);

	if (patternId == 1) PatternSideWall(baseDamage, enemies);
	else if (patternId == 2) PatternDualSpiral(baseDamage, enemies);
	else PatternDebtCollector(baseDamage, enemies);
}


void Demo::TuitionFeeEnemy::PatternSideWall(float baseDamage, std::vector<std::shared_ptr<IEnemy>>* enemies)
{
	bool isAlone = !enemies || enemies->size() == 1;
	const int WAVE_COUNT = isAlone ? 9 : 5;
	const int BULLET_PER_WAVE = 8;
	const float WAVE_DELAY = isAlone ? 0.6f : 1.2f;
	const float BULLET_SPEED = isAlone ? 200.f : 100.f;
	const float SPAWN_X = 350.f;
	const float BULLET_SPACING = isAlone ? 40.f : 80.f;
	const float BULLET_DECAY_TIME = isAlone ? 4.f : 8.f;
	const float WALL_START_Y = -(BULLET_PER_WAVE / 2.f) * BULLET_SPACING;
	std::shared_ptr<float> bulletOffset = std::make_shared<float>(0.f);

	for (int wave = 0; wave < WAVE_COUNT; wave++) {
		int emptyHole = RNG::Range(0, BULLET_PER_WAVE - 1);

		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, baseDamage, emptyHole, BULLET_PER_WAVE, BULLET_SPEED, SPAWN_X, WALL_START_Y, BULLET_SPACING, BULLET_DECAY_TIME, bulletOffset](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				float finalDamage = this->CalculateOutgoingDamage(baseDamage);

				for (int i = 0; i < BULLET_PER_WAVE; i++) {
					if (i == emptyHole) continue;

					float offsetY = WALL_START_Y + (i * BULLET_SPACING);
					float finalY = offsetY + *bulletOffset;
					float finalX = SPAWN_X;

					projectiles.Spawn(
						lock,
						ProjectileDesc(bulletTexture.get(), bulletFrames, 12, 8, 8, 16, 16, finalX, finalY)
						.SetTrajectory(D3DXVECTOR2(-1.f, 0.f))
						.SetDelay(0.f)
						.SetDecayTime(BULLET_DECAY_TIME)
						.SetVelocity(BULLET_SPEED)
						.SetDamage(finalDamage)
					);
				}
				*bulletOffset += 10.f;
				*bulletOffset = std::fmod(*bulletOffset, BULLET_SPACING);
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(WAVE_DELAY));
	}
}


void Demo::TuitionFeeEnemy::PatternDualSpiral(float baseDamage, std::vector<std::shared_ptr<IEnemy>>* enemies)
{
	bool isAlone = !enemies || enemies->size() == 1;
	const int ATTACK_COUNT = isAlone ? 10 : 5;
	const float RADIAL_SPEED = isAlone ? 100.f : 150.f;
	const float ANGULAR_SPEED = isAlone ? 1.2f : 0.6f;
	const float SPAWN_DIST = 220.f;

	auto attack = std::make_shared<DX9GF::CustomCommand>([this, baseDamage, RADIAL_SPEED, ANGULAR_SPEED, SPAWN_DIST](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			float finalDamage = this->CalculateOutgoingDamage(baseDamage);

			struct SpawnPoint { float x, y; float spinDir; };
			SpawnPoint points[2] = {
				{ 0.f, -SPAWN_DIST,  1.f }, 
				{ 0.f,  SPAWN_DIST, -1.f } 
			};

			for (auto& sp : points) {
				for (int i = 0; i < 12; i++) {
					float angle = i * (3.14159f * 2.f / 12.f);
					projectiles.Spawn(
						lock,
						ProjectileDesc(bulletTexture.get(), bulletFrames, 12, 8, 8, 16, 16, sp.x, sp.y)
						.SetSpiralParams(angle, RADIAL_SPEED, ANGULAR_SPEED * sp.spinDir)
						.SetDelay(i * 0.05f)
						.SetDecayTime(6.f)
						.SetDamage(finalDamage)
					);
				}
			}
		}
		markFinished();
		});

	for (int i = 0; i < ATTACK_COUNT; i++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*attack));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(1.f));
	}
}


void Demo::TuitionFeeEnemy::PatternDebtCollector(float baseDamage, std::vector<std::shared_ptr<IEnemy>>* enemies)
{
	(void)enemies;

	constexpr int VOLLEY_COUNT = 8;
	constexpr float VOLLEY_INTERVAL = 1.f;
	constexpr float LANE_SPACING = 28.f;
	const float bulletSpeed = 180.f;
	const float bulletWidth = 16.f;
	const float bulletHeight = 16.f;
	const float spawnOvershoot = 60.f;
	const float travelDistance = (ARENA_HALF_SIZE + spawnOvershoot) * 2.f;
	const float decayTime = travelDistance / bulletSpeed + 0.3f;

	for (int volley = 0; volley < VOLLEY_COUNT; ++volley) {
		int dirA = RNG::Range(0, 3);
		int dirB = RNG::Range(0, 2);
		if (dirB >= dirA) dirB++; 

		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(
			[this, baseDamage, bulletSpeed, bulletWidth, bulletHeight, decayTime, spawnOvershoot, dirA, dirB]
			(std::function<void(void)> markFinished) {
				if (auto lock = this->player.lock()) {
					float finalDamage = this->CalculateOutgoingDamage(baseDamage);
					auto [px, py] = lock->GetWorldPosition();

					auto fireDirection = [&](int dir) {
						if (dir == 0) {
							for (int i = -1; i <= 1; ++i) {
								const float laneY = py + i * LANE_SPACING;
								projectiles.Spawn(
									lock,
									ProjectileDesc(bulletTexture.get(), bulletFrames, 12, 8, 8, bulletWidth, bulletHeight,
										ARENA_HALF_SIZE + spawnOvershoot, laneY)
									.SetTrajectory(D3DXVECTOR2(-1.f, 0.f))
									.SetVelocity(bulletSpeed)
									.SetDecayTime(decayTime)
									.SetDamage(finalDamage)
									.SetStatusEffect(ModifierType::Freeze, 0.1f, 1)
								);
							}
						}
						else if (dir == 1) {
							for (int i = -1; i <= 1; ++i) {
								const float laneY = py + i * LANE_SPACING;
								projectiles.Spawn(
									lock,
									ProjectileDesc(bulletTexture.get(), bulletFrames, 12, 8, 8, bulletWidth, bulletHeight,
										-ARENA_HALF_SIZE - spawnOvershoot, laneY)
									.SetTrajectory(D3DXVECTOR2(1.f, 0.f))
									.SetVelocity(bulletSpeed)
									.SetDecayTime(decayTime)
									.SetDamage(finalDamage)
									.SetStatusEffect(ModifierType::Freeze, 0.1f, 1)
								);
							}
						}
						else if (dir == 2) {
							for (int i = -1; i <= 1; ++i) {
								const float laneX = px + i * LANE_SPACING;
								projectiles.Spawn(
									lock,
									ProjectileDesc(bulletTexture.get(), bulletFrames, 12, 8, 8, bulletWidth, bulletHeight,
										laneX, -ARENA_HALF_SIZE - spawnOvershoot)
									.SetTrajectory(D3DXVECTOR2(0.f, 1.f))
									.SetVelocity(bulletSpeed)
									.SetDecayTime(decayTime)
									.SetDamage(finalDamage)
									.SetStatusEffect(ModifierType::Freeze, 0.1f, 1)
								);
							}
						}
						else {
							for (int i = -1; i <= 1; ++i) {
								const float laneX = px + i * LANE_SPACING;
								projectiles.Spawn(
									lock,
									ProjectileDesc(bulletTexture.get(), bulletFrames, 12, 8, 8, bulletWidth, bulletHeight,
										laneX, ARENA_HALF_SIZE + spawnOvershoot)
									.SetTrajectory(D3DXVECTOR2(0.f, -1.f))
									.SetVelocity(bulletSpeed)
									.SetDecayTime(decayTime)
									.SetDamage(finalDamage)
									.SetStatusEffect(ModifierType::Freeze, 0.1f, 1)
								);
							}
						}
						};

					fireDirection(dirA);
					fireDirection(dirB);
				}
				markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(VOLLEY_INTERVAL));
	}
}