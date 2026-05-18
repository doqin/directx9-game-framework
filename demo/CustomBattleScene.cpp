#include "pch.h"
#include "CustomBattleScene.h"
#include <random>
#include "TestEnemy.h"
#include "DemonEyeEnemy.h"
#include "VampireBatEnemy.h"
#include "MimicEnemy.h"
#include "WarlockEnemy.h"
#include "CupidEnemy.h"

namespace Demo {
    void CustomBattleScene::Init() {
        IBattleScene::Init(); // Initialize hand etc. using base class base init

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 100);

        bool enemySpawned = false;

        for (const auto& [enemyName, chance] : possibleEnemies) {
            int roll = dis(gen);
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
					auto enemy = std::make_shared<VampireBatEnemy>(transformManager, 45.0f);
					enemy->Init(game->GetGraphicsDevice(), &camera);
					enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
					enemies.push_back(enemy);
                } else if (enemyName == "MimicEnemy") { // popup
					auto enemy = std::make_shared<MimicEnemy>(transformManager, 70.0f);
					enemy->Init(game->GetGraphicsDevice(), &camera);
					enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
					enemies.push_back(enemy);
                } else if (enemyName == "WarlockEnemy") { // crawler
					auto enemy = std::make_shared<WarlockEnemy>(transformManager, 55.0f);
					enemy->Init(game->GetGraphicsDevice(), &camera);
					enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
					enemies.push_back(enemy);
                } else if (enemyName == "CupidEnemy") {
					auto enemy = std::make_shared<CupidEnemy>(transformManager, 60.0f);
					enemy->Init(game->GetGraphicsDevice(), &camera);
					enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
					enemies.push_back(enemy);
                }
            }
        }

        // Force spawn at least one enemy if none succeeded the chance roll
        if (!enemySpawned && !possibleEnemies.empty()) {
            std::uniform_int_distribution<> indexDis(0, possibleEnemies.size() - 1);
            int atIndex = indexDis(gen);
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
                auto enemy = std::make_shared<VampireBatEnemy>(transformManager, 45.0f);
                enemy->Init(game->GetGraphicsDevice(), &camera);
                enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
                enemies.push_back(enemy);
            } else if (enemyName == "MimicEnemy") {
                auto enemy = std::make_shared<MimicEnemy>(transformManager, 70.0f);
                enemy->Init(game->GetGraphicsDevice(), &camera);
                enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
                enemies.push_back(enemy);
            } else if (enemyName == "WarlockEnemy") {
                auto enemy = std::make_shared<WarlockEnemy>(transformManager, 55.0f);
                enemy->Init(game->GetGraphicsDevice(), &camera);
                enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
                enemies.push_back(enemy);
            } else if (enemyName == "CupidEnemy") {
                auto enemy = std::make_shared<CupidEnemy>(transformManager, 60.0f);
                enemy->Init(game->GetGraphicsDevice(), &camera);
                enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemyCardOwner) { CreateEnemyCard(enemyCardOwner); });
                enemies.push_back(enemy);
            }
        }
        transformManager->RebuildHierarchy();
        StartBattle();
    }
}