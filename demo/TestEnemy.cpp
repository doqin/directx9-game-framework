#include "pch.h"
#include "TestEnemy.h"
#include "resource.h"
#include "RoundProjectile.h"

void Demo::TestEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera)
{
    texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
    texture->LoadTexture(IDB_PNG4);
    sprite = std::make_shared<DX9GF::StaticSprite>(texture.get());
    sprite->SetOrigin(32, 32);
    roundProjectileTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
    roundProjectileTexture->LoadTexture(IDB_PNG5);
    SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
    InitCardSpawnTrigger(camera, 64.f, 64.f);
}

void Demo::TestEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime)
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

void Demo::TestEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
    (void)enemies;
    (void)popUpMessage;
    (void)graphicsDevice;
    (void)camera;
    this->player = player;
    float baseDamage = 5.f;
    float projDamage = GetOutgoingDamage(baseDamage);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(-200, 200);
    std::uniform_real_distribution<float> yDist(-200, 200);
    
    for (int i = 0; i < 40; i++) {
        auto x = xDist(gen);
        auto y = yDist(gen);
        commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, x, y, projDamage](std::function<void(void)> markFinished) {
            if (auto lock = this->player.lock()) {
                auto projSprite = std::make_shared<DX9GF::StaticSprite>(roundProjectileTexture.get());
                projSprite->SetOrigin(8, 8);
                projectiles.push_back(
                    RoundProjectile::Builder(
                        transformManager,
                        lock,
                        projSprite,
                        16, 16,
                        x, y
                    )
                    .SetTargetPosition(lock->GetCollider().lock()->GetWorldX(), lock->GetCollider().lock()->GetWorldY())
                    .SetDelay(.2f)
                    .SetDecayTime(4.f)
                    .SetVelocity(200.f)
                    .SetDamage(projDamage)
                    .Build()
                );
                projectiles.back()->Init();
                transformManager.lock()->RebuildHierarchy();
            }
            else {
                throw std::runtime_error("player is null");
            }
            markFinished();
        }));
        commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.1f));
    }
    commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(5.f));
}
