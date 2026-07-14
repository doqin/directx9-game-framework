#pragma once
#include "IEnemy.h"
#include "MapEnemy.h"
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include "DX9GF.h"

namespace Demo {
	struct OverworldSpriteData {
		std::wstring texturePath;
		float width;
		float height;
		int frameCount;
		float hitBoxWidth;
		float hitBoxHeight;
	};

	class EnemyFactory {
	public:
		static OverworldSpriteData GetOverworldSprite(const std::string& enemyType);

		static std::shared_ptr<IEnemy> Create(const std::string& type, std::weak_ptr<DX9GF::TransformManager> tm, DX9GF::GraphicsDevice* gd, DX9GF::Camera* cam);

		static std::shared_ptr<MapEnemy> CreateMapEnemy(
			float x, float y, const std::string& id,
			const std::vector<std::string>& types, bool isRandomPool, bool useGlobalPool,
			const std::string& bgmName,
			std::function<void(DX9GF::GraphicsDevice*, unsigned long long)> bgDrawFunc,
			std::shared_ptr<DX9GF::TransformManager> tm,
			Game* game,
			DX9GF::ColliderManager* colMan,
			std::shared_ptr<Player> player
		);
	};
}