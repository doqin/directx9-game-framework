#pragma once
#include <string>
#include <vector>
#include <functional> 
#include "DX9GF.h"

namespace Demo {
    struct BattleEncounter {
        std::string mapEnemyID;
        std::vector<std::string> enemyTypes;
        std::string bgmName = "battle_loop1";

        std::function<void(DX9GF::GraphicsDevice*, unsigned long long)> bgDrawFunc = nullptr;
    };
}