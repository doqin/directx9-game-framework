#include "pch.h"
#include "KeyeproEnemy.h"
#include "resource.h"
#include "SineWaveProjectile.h"
#include "RoundProjectile.h"
#include "TargetedProjectile.h"
#include "BoomerangProjectile.h"
#include "KeyeEnemy.h"
#include "PopUpMessage.h"
#include <random>
#include <algorithm>

void Demo::KeyeproEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {

    texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
    texture->LoadTexture(L"Keyepro-Sheet.png");
    sprite = std::make_shared<DX9GF::StaticSprite>(texture.get());
    sprite->SetOrigin(32, 32);
    sprite->SetScale(2.f); 

    projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
    projTexture->LoadTexture(L"Keye-bullet.png");
    projSprite = std::make_shared<DX9GF::StaticSprite>(projTexture.get());
    projSprite->SetOrigin(8, 8);

    SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
    InitCardSpawnTrigger(camera, 128.f, 128.f);
}

void Demo::KeyeproEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) {
    if (sprite) {
        sprite->Begin();
        auto [x, y] = GetWorldPosition();
        sprite->SetPosition(x, y);
        sprite->Draw(*camera, deltaTime); 
        sprite->End();
    }
    IEnemy::Draw(graphicsDevice, camera, deltaTime);
}

int Demo::KeyeproEnemy::GetRandomPattern() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 3);
    return dist(gen);
}

void Demo::KeyeproEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
    this->player = player;
    float projDamage = 5.f; 

    std::random_device rd;
    std::mt19937 gen(rd());

    if (enemies != nullptr && graphicsDevice != nullptr && camera != nullptr) {
        std::uniform_real_distribution<float> spawnChanceDist(0.f, 1.f);
        constexpr float MINION_SPAWN_CHANCE = 0.35f;
        if (spawnChanceDist(gen) <= MINION_SPAWN_CHANCE) {
            size_t keyeCount = 0;
            for (const auto& enemy : *enemies) {
                if (!enemy || enemy->IsDead()) {
                    continue;
                }
                if (std::dynamic_pointer_cast<KeyeEnemy>(enemy)) {
                    ++keyeCount;
                }
            }

            constexpr size_t MAX_KEYE_ENEMIES = 3;
            const size_t spawnCount = (keyeCount < MAX_KEYE_ENEMIES)
                ? (std::min)(static_cast<size_t>(1), MAX_KEYE_ENEMIES - keyeCount)
                : 0;

            if (spawnCount > 0) {
                auto [bossX, bossY] = GetWorldPosition();
                for (size_t i = 0; i < spawnCount; ++i) {
                    float spawnX = 1200;
                    float spawnY = 0;
                    auto minion = std::make_shared<KeyeEnemy>(transformManager, 25.0f, spawnX, spawnY);
                    minion->Init(graphicsDevice, camera);
                    minion->SetOnRequestEnemyCard(onRequestEnemyCard);
                    enemies->push_back(minion);
                }

                if (auto tm = transformManager.lock()) {
                    tm->RebuildHierarchy();
                }

                if (popUpMessage) {
                    popUpMessage->QueueMessage(&commandBuffer, L"The boss had spawned minions");
                }
            }
        }
        else {
            std::uniform_int_distribution<int> dist(1, 3);
            if (dist(gen) == 1) {
                PatternTargetedSniping(projDamage);
            }
            else if (dist(gen) == 2) {
                PatternEcholocation(projDamage);
            }
            else {
                PatternSwoopBite(projDamage);
            }

            commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(2.f));
        }

    }
}

void Demo::KeyeproEnemy::PatternSineWaveStorm(float projDamage) {
    const int BULLETS =10;
    const int SPACING = 80;
	const int WAVE_COUNT = 10;

    auto leftAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage, BULLETS, SPACING, WAVE_COUNT](std::function<void(void)> markFinished) {
        if (auto lock = this->player.lock()) {
            for (int waveCount = 0; waveCount < WAVE_COUNT; waveCount++) {
                for (int i = 0; i < BULLETS; i++) {
                    float startY = (i - BULLETS / 2.f) * SPACING;

                    projectiles.push_back(
                        SineWaveProjectile::Builder(transformManager, lock, projSprite, 16, 16, -350.f, startY)
                        .SetTrajectory(D3DXVECTOR2(1, 0)) 
                        .SetWave(20.f, 4.0f)
                        .SetDelay((waveCount * 0.25f) + (i * 0.05f)) 
                        .SetDecayTime(10.f)
                        .SetVelocity(100.f)
                        .SetDamage(projDamage)
                        .Build()
                    );
                    projectiles.back()->Init();
                }
            }
            transformManager.lock()->RebuildHierarchy();
        }
        markFinished();
        });
	//auto topAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage, BULLETS, SPACING, WAVE_COUNT](std::function<void(void)> markFinished) {
	//	if (auto lock = this->player.lock()) {
	//		for (int waveCount = 0; waveCount < WAVE_COUNT; waveCount++) {
	//			for (int i = 0; i < BULLETS; i++) {
	//				float startX = (i - BULLETS / 2.f) * SPACING;
	//				projectiles.push_back(
	//					SineWaveProjectile::Builder(transformManager, lock, projSprite, 16, 16, startX, -350.f)
	//					.SetTrajectory(D3DXVECTOR2(0, 1))
	//					.SetWave(20.f, 4.0f)
	//					.SetDelay((waveCount * 0.25f) + (i * 0.05f))
	//					.SetDecayTime(10.f)
	//					.SetVelocity(100.f)
	//					.SetDamage(projDamage)
	//					.Build()
	//				);
	//				projectiles.back()->Init();
	//			}
	//		}
	//		transformManager.lock()->RebuildHierarchy();
	//	}
	//	markFinished();
	//	});

    commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*leftAttack));
	//commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*topAttack));
}

void Demo::KeyeproEnemy::PatternTargetedSniping(float projDamage) {
    auto currentPosition = std::make_shared<int>(0);

    for (int i = 0; i < 100; i++) {
        commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, currentPosition](std::function<void(void)> markFinished) {
            if (auto lock = this->player.lock()) {
                std::uniform_int_distribution<int> positionDist(1, 4);
				*currentPosition = *currentPosition % 4 + 1; // Cycle through positions 1 to 4
				float x, y;
				switch (*currentPosition) {
				case 1: // Top
					x = 0;
					y = -350.f;
					break;
				case 2: // Bottom
					x = 0;
					y = 350.f;
					break;
				case 3: // Left
					x = -350.f;
					y = 0;
					break;
				case 4: // Right
					x = 350.f;
					y = 0;
					break;
				}

                projectiles.push_back(
                    RoundProjectile::Builder(transformManager, lock, projSprite, 16, 16, x, y)
                    .SetTargetPosition(lock->GetCollider().lock()->GetWorldX(), lock->GetCollider().lock()->GetWorldY())
                    .SetVelocity(200.f) 
                    .SetDelay(0.f)
                    .SetDecayTime(4.f)
                    .SetDamage(projDamage)
                    .Build()
                );
                projectiles.back()->Init();
                transformManager.lock()->RebuildHierarchy();
            }
            markFinished();
            }));

        commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.1f));
    }
}

void Demo::KeyeproEnemy::PatternEcholocation(float projDamage)
{
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
    for (int i = 0; i < 40; i++) {
        if (dist(gen) == 1) commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*leftAttack));
        else commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*rightAttack));
        commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
    }
}

void Demo::KeyeproEnemy::PatternSwoopBite(float projDamage)
{
    const int BULLET_COUNT = 8;
    const float ANGLE_STEP = 0.3f;
	const float INITIAL_VELOCITY = 370.f;
    auto rightAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage, BULLET_COUNT, ANGLE_STEP, INITIAL_VELOCITY](std::function<void(void)> markFinished) {
        if (auto lock = this->player.lock()) {
            auto [px, py] = lock->GetCollider().lock()->GetWorldPosition();
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
                    .SetInitialVelocity(INITIAL_VELOCITY)
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
    auto leftAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage, BULLET_COUNT, ANGLE_STEP, INITIAL_VELOCITY](std::function<void(void)> markFinished) {
        if (auto lock = this->player.lock()) {
            auto [px, py] = lock->GetCollider().lock()->GetWorldPosition();
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
                    .SetInitialVelocity(INITIAL_VELOCITY)
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
    std::uniform_int_distribution<int> dist(30, 40);
    std::uniform_int_distribution<int> sideDist(1, 2);
    for (int i = 0; i < dist(gen); i++) {
        if (sideDist(gen) == 1) commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*leftAttack));
        else commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*rightAttack));
        commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
    }
}
