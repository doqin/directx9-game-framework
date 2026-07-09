#pragma once
#include <vector>
#include <string>
#include "RNG.h"

namespace Demo {
	class EncounterGenerator {
	public:
		// bể quái chung cho toàn bộ mạch chạy roguelike
		static std::vector<std::string> GenerateNormalEncounter() {
			std::vector<std::string> normalPool = {
				"DemonEyeEnemy", "VampireBatEnemy",
				"MimicEnemy", "WarlockEnemy", "KeyeEnemy"
			};

			std::vector<std::string> result;
			int  enemyCount = RNG::Range(1, 2);
			for (int i = 0; i < enemyCount; ++i) {
				int poolDis = RNG::Range(0, normalPool.size() - 1);
				result.push_back(normalPool[poolDis]);
			}

			return result;
		}

		// dùng riêng cho các trường hợp đặc biệt cần giới hạn loại quái dễ
		static std::vector<std::string> GenerateFromTypes(const std::vector<std::string>& allowedTypes) {
			std::vector<std::string> result;
			if (allowedTypes.empty()) return result;

			int enemyCount = RNG::Range(1, 2);

			for (int i = 0; i < enemyCount; ++i) {
				result.push_back(allowedTypes[RNG::Range(0, allowedTypes.size() - 1)]);
			}

			return result;
		}
	};
}