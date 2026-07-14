#pragma once
#include "IBattleScene.h"
#include "BattleEncounter.h"

namespace Demo {
    class MapBattleScene : public IBattleScene {
    private:
        BattleEncounter encounter;
    public:
        MapBattleScene(Game* game, std::shared_ptr<Player> player, int screenWidth, int screenHeight, const BattleEncounter& enc);

        void Init() override;
    };
}