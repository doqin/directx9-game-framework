#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "IEnemy.h"

namespace Demo {
	class TuitionFeeEnemy : public EnemyBase<TuitionFeeEnemy> {
	private:
		std::shared_ptr<DX9GF::Texture> texture;
		std::shared_ptr<DX9GF::AnimatedSprite> sprite;

		std::shared_ptr<DX9GF::Texture> bulletTexture;
		std::vector<RECT> bulletFrames;

		std::weak_ptr<Player> player;

		bool hasSpawnTurn = false;
		int spawnTurn = 0;
		static constexpr int BUFF_WIPE_INTERVAL = 5;

		static constexpr int VISION_DEBUFF_DURATION = 1;

		static constexpr float ARENA_HALF_SIZE = 128.f;

		void BanRandomItems(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage);
		void PatternSideWall(float baseDamage, std::vector<std::shared_ptr<IEnemy>>* enemies);
		void PatternDualSpiral(float baseDamage, std::vector<std::shared_ptr<IEnemy>>* enemies);
		void PatternDebtCollector(float baseDamage, std::vector<std::shared_ptr<IEnemy>>* enemies);

	public:
		using EnemyBase<TuitionFeeEnemy>::EnemyBase;

		void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera);
		void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) override;
		void OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) override;
		void StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies,
			std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice,
			DX9GF::Camera* camera, int currentTurn) override;
	};
}