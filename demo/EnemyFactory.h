#pragma once
#include "IEnemy.h"
#include "TestEnemy.h"
#include "DemonEyeEnemy.h"
#include "VampireBatEnemy.h"
#include "MimicEnemy.h"
#include "WarlockEnemy.h"
#include "CupidEnemy.h"
#include "KeyeEnemy.h"
#include "KeyeproEnemy.h"
#include <string>
#include <memory>
#include <random>

namespace Demo {
	// Struct lưu trữ thông số hình ảnh ngoài map
	struct OverworldSpriteData {
		std::wstring texturePath;
		float width;
		float height;
		int frameCount;
	};
	class EnemyFactory {
	public:
		static OverworldSpriteData GetOverworldSprite(const std::string& enemyType) {
			if (enemyType == "KeyeEnemy") return { L"assets/minion-Sheet.png", 64.f, 64.f, 12 }; // Thay bằng path/size thật của sếp
			if (enemyType == "DemonEyeEnemy") return { L"assets/computerbug-Sheet.png", 64.f, 64.f, 12 };
			if (enemyType == "MimicEnemy") return { L"assets/notresponding-Sheet.png", 64.f, 64.f, 12 };
			if (enemyType == "CupidEnemy") return { L"assets/bubble-Sheet.png", 64.f, 64.f, 12 };
			if (enemyType == "VampireBatEnemy") return { L"assets/shrimp-Sheet.png", 64.f, 64.f, 12 };

			// Mặc định trả về dấu chấm hỏi nếu là quái Random hoặc không tìm thấy
			return { L"assets/computerbug-Sheet.png", 64.f, 64.f, 12 }; //TODO: change this random sprite vro
		}

		static std::shared_ptr<IEnemy> Create(const std::string& type, std::weak_ptr<DX9GF::TransformManager> tm, DX9GF::GraphicsDevice* gd, DX9GF::Camera* cam) {
			// Khởi tạo engine random tĩnh để tối ưu bộ nhớ
			static std::random_device rd;
			static std::mt19937 gen(rd());

			if (type == "TestEnemy") {
				std::uniform_real_distribution<float> hp(40.0f, 60.0f);
				auto enemy = std::make_shared<TestEnemy>(tm, hp(gen));
				enemy->Init(gd, cam);
				return enemy;
			}
			else if (type == "DemonEyeEnemy") {
				std::uniform_real_distribution<float> hp(25.0f, 45.0f);
				auto enemy = std::make_shared<DemonEyeEnemy>(tm, hp(gen));
				enemy->Init(gd, cam);
				return enemy;
			}
			else if (type == "VampireBatEnemy") {
				std::uniform_real_distribution<float> hp(60.0f, 80.0f);
				auto enemy = std::make_shared<VampireBatEnemy>(tm, hp(gen));
				enemy->Init(gd, cam);
				return enemy;
			}
			else if (type == "MimicEnemy") {
				std::uniform_real_distribution<float> hp(60.0f, 80.0f);
				auto enemy = std::make_shared<MimicEnemy>(tm, hp(gen));
				enemy->Init(gd, cam);
				return enemy;
			}
			else if (type == "WarlockEnemy") {
				std::uniform_real_distribution<float> hp(70.0f, 90.0f);
				auto enemy = std::make_shared<WarlockEnemy>(tm, hp(gen));
				enemy->Init(gd, cam);
				return enemy;
			}
			else if (type == "KeyeEnemy") {
				std::uniform_real_distribution<float> hp(20.0f, 40.0f);
				auto enemy = std::make_shared<KeyeEnemy>(tm, hp(gen));
				enemy->Init(gd, cam);
				return enemy;
			}
			else if (type == "CupidEnemy") {
				auto enemy = std::make_shared<CupidEnemy>(tm, 200.0f); // Pac-man Boss
				enemy->Init(gd, cam);
				return enemy;
			}
			else if (type == "KeyeproEnemy") {
				auto enemy = std::make_shared<KeyeproEnemy>(tm, 500.0f); // Final Boss
				enemy->Init(gd, cam);
				return enemy;
			}

			return nullptr;
		}
	};
}