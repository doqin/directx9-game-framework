#include "pch.h"
#include "MapBattleScene.h"
#include "EnemyFactory.h"
#include "RNG.h"

namespace Demo {

    MapBattleScene::MapBattleScene(Game* game, std::shared_ptr<Player> player, int screenWidth, int screenHeight, const BattleEncounter& enc)
        : IBattleScene(game, player, screenWidth, screenHeight), encounter(enc) {
    }

    void MapBattleScene::Init() {
        IBattleScene::Init();
        std::string finalBGM = encounter.bgmName;

        if (finalBGM == "battle_loop") {
            std::vector<std::string> bgmPool = {
                "battle_loop1", "battle_loop2",
                "battle_loop3", "battle_loop4"
            };
            finalBGM = bgmPool[RNG::Range(0, bgmPool.size() - 1)];
        }

        this->SetCustomBGM(finalBGM);
        if (encounter.bgDrawFunc) {
            this->SetCustomBackgroundDraw(encounter.bgDrawFunc);
        }

        const float spacingY = 140.f;
        const size_t enemyCount = encounter.enemyTypes.size();

        for (size_t i = 0; i < enemyCount; ++i) {
            auto enemy = EnemyFactory::Create(encounter.enemyTypes[i], transformManager, game->GetGraphicsDevice(), &camera);
            if (enemy) {
                enemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> e) {
                    this->CreateEnemyCard(e);
                    });
                this->enemies.push_back(enemy);
            }
        }

        transformManager->RebuildHierarchy();
        this->StartBattle();
    }
}