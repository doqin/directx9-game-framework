#include "pch.h"
#include "KeyeproEnemy.h"
#include "resource.h"
#include "SineWaveProjectile.h"
#include "RoundProjectile.h"
#include "TargetedProjectile.h"
#include <random>

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
    std::uniform_int_distribution<int> dist(1, 2);
    return dist(gen);
}

void Demo::KeyeproEnemy::StartAttack(std::shared_ptr<Player> player) {
    this->player = player;
    float projDamage = 5.f; 

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 2);

    if (dist(gen) == 1) {
        PatternSineWaveStorm(projDamage);
    }
    else {
        PatternTargetedSniping(projDamage);
    }

    commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(4.f));
}

void Demo::KeyeproEnemy::PatternSineWaveStorm(float projDamage) {
    const int BULLETS =15;
    const int SPACING = 45;

    auto sineAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage, BULLETS, SPACING](std::function<void(void)> markFinished) {
        if (auto lock = this->player.lock()) {
            for (int waveCount = 0; waveCount < 6; waveCount++) {
                for (int i = 0; i < BULLETS; i++) {
                    float startY = (i - BULLETS / 2.f) * SPACING;

                    projectiles.push_back(
                        SineWaveProjectile::Builder(transformManager, lock, projSprite, 16, 16, -350.f, startY)
                        .SetTrajectory(D3DXVECTOR2(1, 0)) 
                        .SetWave(65.f, 4.0f)
                        .SetDelay((waveCount * 0.25f) + (i * 0.05f)) 
                        .SetDecayTime(5.f)
                        .SetVelocity(220.f)
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

    commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*sineAttack));
}

void Demo::KeyeproEnemy::PatternTargetedSniping(float projDamage) {
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < 100; i++) {
        commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, &gen](std::function<void(void)> markFinished) {
            if (auto lock = this->player.lock()) {
                std::uniform_real_distribution<float> spawnXDist(-300.f, 300.f);

                projectiles.push_back(
                    RoundProjectile::Builder(transformManager, lock, projSprite, 16, 16, spawnXDist(gen), -260.f)
                    .SetTargetPosition(lock->GetCollider().lock()->GetWorldX(), lock->GetCollider().lock()->GetWorldY())
                    .SetVelocity(320.f) 
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

        commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.04f));
    }
}