#include "pch.h"
#include "CustomBattleScene.h"
#include <random>
#include "TestEnemy.h"
#include "DemonEyeEnemy.h"
#include "VampireBatEnemy.h"
#include "MimicEnemy.h"
#include "WarlockEnemy.h"
#include "CupidEnemy.h"
#include "KeyeEnemy.h"
#include "KeyeproEnemy.h"
#include "RNG.h"
namespace Demo {
    void CustomBattleScene::Init() {
        IBattleScene::Init(); // Initialize hand etc. using base class base init

        bool enemySpawned = false;

        for (const auto& [enemyName, chance] : possibleEnemies) {
            int roll = RNG::Range(1, 100);;
            if (roll <= chance) {
                enemySpawned = true;
                if (enemyName == "TestEnemy") {
					auto enemy = std::make_shared<TestEnemy>(transformManager, 50.0f);
					enemy->Init(game->GetGraphicsDevice(), &camera);
					enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
					enemies.push_back(enemy);
                } else if (enemyName == "DemonEyeEnemy") { // bug
					auto enemy = std::make_shared<DemonEyeEnemy>(transformManager, 35.0f);
					enemy->Init(game->GetGraphicsDevice(), &camera);
					enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
					enemies.push_back(enemy);
                } else if (enemyName == "VampireBatEnemy") { // shrimp
					auto enemy = std::make_shared<VampireBatEnemy>(transformManager, 70.0f);
					enemy->Init(game->GetGraphicsDevice(), &camera);
					enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
					enemies.push_back(enemy);
                } else if (enemyName == "MimicEnemy") { // popup
					auto enemy = std::make_shared<MimicEnemy>(transformManager, 70.0f);
					enemy->Init(game->GetGraphicsDevice(), &camera);
					enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
					enemies.push_back(enemy);
                } else if (enemyName == "WarlockEnemy") { // crawler
					auto enemy = std::make_shared<WarlockEnemy>(transformManager, 80.0f);
					enemy->Init(game->GetGraphicsDevice(), &camera);
					enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
					enemies.push_back(enemy);
                } else if (enemyName == "CupidEnemy") {
					auto enemy = std::make_shared<CupidEnemy>(transformManager, 200.0f);
					enemy->Init(game->GetGraphicsDevice(), &camera);
					enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
					enemies.push_back(enemy);
                } 
                else if (enemyName == "KeyeproEnemy") {
                    auto enemy = std::make_shared<KeyeproEnemy>(transformManager, 600.0f);
                    enemy->Init(game->GetGraphicsDevice(), &camera);
                    enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
                    enemies.push_back(enemy);
                }
                else if (enemyName == "KeyeEnemy") {
                    auto enemy = std::make_shared<KeyeEnemy>(transformManager, 50.0f);
                    enemy->Init(game->GetGraphicsDevice(), &camera);
                    enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
                    enemies.push_back(enemy);
                }
            }
        }

        // Force spawn at least one enemy if none succeeded the chance roll
        if (!enemySpawned && !possibleEnemies.empty()) {
            int atIndex = RNG::Range(0, possibleEnemies.size() - 1);
            auto it = possibleEnemies.begin();
            std::advance(it, atIndex);
            std::string enemyName = it->first;

            if (enemyName == "TestEnemy") {
                auto enemy = std::make_shared<TestEnemy>(transformManager, 50.0f);
                enemy->Init(game->GetGraphicsDevice(), &camera);
                enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
                enemies.push_back(enemy);
            } else if (enemyName == "DemonEyeEnemy") {
                auto enemy = std::make_shared<DemonEyeEnemy>(transformManager, 35.0f);
                enemy->Init(game->GetGraphicsDevice(), &camera);
                enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
                enemies.push_back(enemy);
            } else if (enemyName == "VampireBatEnemy") {
                auto enemy = std::make_shared<VampireBatEnemy>(transformManager, 70.0f);
                enemy->Init(game->GetGraphicsDevice(), &camera);
                enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
                enemies.push_back(enemy);
            } else if (enemyName == "MimicEnemy") {
                auto enemy = std::make_shared<MimicEnemy>(transformManager, 70.0f);
                enemy->Init(game->GetGraphicsDevice(), &camera);
                enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
                enemies.push_back(enemy);
            } else if (enemyName == "WarlockEnemy") {
                auto enemy = std::make_shared<WarlockEnemy>(transformManager, 80.0f);
                enemy->Init(game->GetGraphicsDevice(), &camera);
                enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
                enemies.push_back(enemy);
            } else if (enemyName == "CupidEnemy") {
                auto enemy = std::make_shared<CupidEnemy>(transformManager, 200.0f);
                enemy->Init(game->GetGraphicsDevice(), &camera);
                enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
                enemies.push_back(enemy);
            } 
            else if (enemyName == "KeyeproEnemy") {
                auto enemy = std::make_shared<KeyeproEnemy>(transformManager, 500.0f);
                enemy->Init(game->GetGraphicsDevice(), &camera);
                enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
                enemies.push_back(enemy);
            }
            else if (enemyName == "KeyeEnemy") {
                auto enemy = std::make_shared<KeyeEnemy>(transformManager, 30.0f);
                enemy->Init(game->GetGraphicsDevice(), &camera);
                enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
                enemies.push_back(enemy);
            }
            
        }
        transformManager->RebuildHierarchy();
        StartBattle();
    }
}