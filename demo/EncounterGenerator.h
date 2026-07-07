#pragma once
#include <vector>
#include <string>
#include <random>

namespace Demo {
    class EncounterGenerator {
    public:
        // bể quái chung cho toàn bộ mạch chạy roguelike
        static std::vector<std::string> GenerateNormalEncounter() {
            std::vector<std::string> normalPool = {
                "DemonEyeEnemy", "VampireBatEnemy",
                "MimicEnemy", "WarlockEnemy", "KeyeEnemy"
            };

            static std::random_device rd;
            static std::mt19937 gen(rd());

            std::uniform_int_distribution<> sizeDis(1, 2);
            std::uniform_int_distribution<> poolDis(0, normalPool.size() - 1);

            int enemyCount = sizeDis(gen);
            std::vector<std::string> result;

            for (int i = 0; i < enemyCount; ++i) {
                result.push_back(normalPool[poolDis(gen)]);
            }

            return result;
        }

        // dùng riêng cho các trường hợp đặc biệt cần giới hạn loại quái dễ
        static std::vector<std::string> GenerateFromTypes(const std::vector<std::string>& allowedTypes) {
            std::vector<std::string> result;
            if (allowedTypes.empty()) return result;

            static std::random_device rd;
            static std::mt19937 gen(rd());

            std::uniform_int_distribution<> sizeDis(1, 2);
            std::uniform_int_distribution<> poolDis(0, allowedTypes.size() - 1);

            int enemyCount = sizeDis(gen);

            for (int i = 0; i < enemyCount; ++i) {
                result.push_back(allowedTypes[poolDis(gen)]);
            }

            return result;
        }
    };
}