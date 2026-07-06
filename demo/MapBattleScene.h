#pragma once
#include "IBattleScene.h"
#include "BattleEncounter.h"
#include "EnemyFactory.h"

namespace Demo {
    class MapBattleScene : public IBattleScene {
    private:
        BattleEncounter encounter;
    public:
        MapBattleScene(Game* game, std::shared_ptr<Player> player, int screenWidth, int screenHeight, const BattleEncounter& enc)
            : IBattleScene(game, player, screenWidth, screenHeight), encounter(enc) {
        }

        void Init() override {
            IBattleScene::Init();

            this->SetCustomBGM(encounter.bgmName);

            // FIX: Set background
            if (encounter.bgDrawFunc) {
                this->SetCustomBackgroundDraw(encounter.bgDrawFunc);
            }

            const float spacingY = 140.f;
            const size_t enemyCount = encounter.enemyTypes.size();
            for (size_t i = 0; i < enemyCount; ++i) {
                auto enemy = EnemyFactory::Create(encounter.enemyTypes[i], transformManager, game->GetGraphicsDevice(), &camera);
                if (enemy) {
                    // FIX Z-INDEX & CLICK: Bắt sự kiện click để sinh EnemyCard
                    enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> e) {
                        this->CreateEnemyCard(e);
                        });
                    this->enemies.push_back(enemy);
                }
            }

            // FIX INPUT: Bắt buộc phải Rebuild lại cây Transform để nhận diện nút bấm
            transformManager->RebuildHierarchy();
            this->StartBattle();
        }
    };
}