#include "pch.h"
#include "EnemyFactory.h"
#include "EncounterGenerator.h"
#include "TestEnemy.h"
#include "DemonEyeEnemy.h"
#include "VampireBatEnemy.h"
#include "MimicEnemy.h"
#include "WarlockEnemy.h"
#include "CupidEnemy.h"
#include "KeyeEnemy.h"
#include "KeyeproEnemy.h"
#include "KernelEnemy.h"
#include "TrojanEnemy.h"
#include "RNG.h"

namespace Demo {

	OverworldSpriteData EnemyFactory::GetOverworldSprite(const std::string& enemyType) {
		if (enemyType == "KeyeEnemy")       return { L"assets/minion-Sheet.png", 64.f, 64.f, 12, 38.f, 45.f };
		if (enemyType == "DemonEyeEnemy")   return { L"assets/computerbug-Sheet.png", 64.f, 64.f, 12, 38.f, 45.f };
		if (enemyType == "MimicEnemy")      return { L"assets/notresponding-Sheet.png", 64.f, 64.f, 12, 38.f, 45.f };
		if (enemyType == "CupidEnemy")      return { L"assets/bubble-Sheet.png", 64.f, 64.f, 12, 38.f, 45.f };
		if (enemyType == "VampireBatEnemy") return { L"assets/shrimp-Sheet.png", 64.f, 64.f, 12, 38.f, 45.f };
		if (enemyType == "WarlockEnemy")    return { L"assets/crawler-Sheet.png", 64.f, 64.f, 12, 38.f, 45.f };
		if (enemyType == "KernelEnemy")      return { L"assets/kernel_2.png", 64.f, 64.f, 12, 38.f, 45.f };
		if (enemyType == "TrojanEnemy")     return { L"assets/trojan-Sheet.png", 96.f, 96.f, 13, 48.f, 48.f };

		return { L"assets/random-Sheet.png", 64.f, 64.f,8, 38.f, 45.f };
	}

	std::shared_ptr<IEnemy> EnemyFactory::Create(const std::string& type, std::weak_ptr<DX9GF::TransformManager> tm, DX9GF::GraphicsDevice* gd, DX9GF::Camera* cam) {
		if (type == "TestEnemy") {
			auto enemy = std::make_shared<TestEnemy>(tm, RNG::Range(40.0f, 60.0f));
			enemy->Init(gd, cam);
			return enemy;
		}
		else if (type == "DemonEyeEnemy") {
			auto enemy = std::make_shared<DemonEyeEnemy>(tm, RNG::Range(25.0f, 45.0f));
			enemy->Init(gd, cam);
			return enemy;
		}
		else if (type == "VampireBatEnemy") {
			auto enemy = std::make_shared<VampireBatEnemy>(tm, RNG::Range(60.0f, 80.0f));
			enemy->Init(gd, cam);
			return enemy;
		}
		else if (type == "MimicEnemy") {
			auto enemy = std::make_shared<MimicEnemy>(tm, RNG::Range(60.0f, 80.0f));
			enemy->Init(gd, cam);
			return enemy;
		}
		else if (type == "WarlockEnemy") {
			auto enemy = std::make_shared<WarlockEnemy>(tm, RNG::Range(70.0f, 90.0f));
			enemy->Init(gd, cam);
			return enemy;
		}
		else if (type == "KeyeEnemy") {
			//auto enemy = std::make_shared<KeyeEnemy>(tm, RNG::Range(20.0f, 40.0f));
			auto enemy = std::make_shared<KeyeEnemy>(tm, 20);
			enemy->Init(gd, cam);
			return enemy;
		}
		else if (type == "KernelEnemy") {
			auto enemy = std::make_shared<KernelEnemy>(tm, RNG::Range(40.0f, 95.0f)); //TODO: Balance this pls
			enemy->Init(gd, cam);
			return enemy;
		}
		else if (type == "CupidEnemy") {
			auto enemy = std::make_shared<CupidEnemy>(tm, 200.0f);
			enemy->Init(gd, cam);
			return enemy;
		}
		else if (type == "KeyeproEnemy") {
			auto enemy = std::make_shared<KeyeproEnemy>(tm, 500.0f);
			enemy->Init(gd, cam);
			return enemy;
		}
		else if (type == "TrojanEnemy") {
			auto enemy = std::make_shared<TrojanEnemy>(tm, RNG::Range(40.0f, 60.0f));
			enemy->Init(gd, cam);
			return enemy;
		}
		return nullptr;
	}

	std::shared_ptr<MapEnemy> EnemyFactory::CreateMapEnemy(
		float x, float y, const std::string& id,
		const std::vector<std::string>& types, bool isRandomPool, bool useGlobalPool,
		const std::string& bgmName,
		std::function<void(DX9GF::GraphicsDevice*, unsigned long long)> bgDrawFunc,
		std::shared_ptr<DX9GF::TransformManager> tm, Game* game, DX9GF::ColliderManager* colMan, std::shared_ptr<Player> player)
	{
		BattleEncounter enc;
		enc.mapEnemyID = id;
		enc.useGlobalPool = useGlobalPool;

		if (useGlobalPool) enc.enemyTypes = EncounterGenerator::GenerateNormalEncounter();
		else if (isRandomPool) {
			enc.randomPool = types;
			enc.enemyTypes = EncounterGenerator::GenerateFromTypes(enc.randomPool);
		}
		else enc.enemyTypes = types;

		enc.bgmName = bgmName;
		enc.bgDrawFunc = bgDrawFunc;

		std::string spriteType = (isRandomPool || useGlobalPool) ? "Random" : enc.enemyTypes[0];
		auto sprite = GetOverworldSprite(spriteType);
		enc.mapTexturePath = sprite.texturePath;
		enc.spriteWidth = static_cast<int>(sprite.width);
		enc.spriteHeight = static_cast<int>(sprite.height);
		enc.frameCount = sprite.frameCount;
		enc.hitBoxWidth = sprite.hitBoxWidth;
		enc.hitBoxHeight = sprite.hitBoxHeight;

		auto enemy = std::make_shared<MapEnemy>(tm, x, y, enc);
		enemy->Init(game, game->GetGraphicsDevice(), colMan, player);
		return enemy;
	}
}