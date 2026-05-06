#pragma once
#include "IBattleScene.h"
#include <map>
#include <string>

namespace Demo {
    class CustomBattleScene : public IBattleScene {
    private:
        std::map<std::string, int> possibleEnemies;

    public:
        CustomBattleScene(Game* game, std::shared_ptr<Player> player, int screenWidth, int screenHeight, const std::map<std::string, int>& possibleEnemies)
            : IBattleScene(game, player, screenWidth, screenHeight), possibleEnemies(possibleEnemies) {}

        void Init() override;
    };
}
