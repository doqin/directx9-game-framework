#pragma once
#include "IEnemy.h"

namespace Demo {
	class VampireBatEnemy : public EnemyBase<VampireBatEnemy> {
	private:
		std::shared_ptr<DX9GF::Texture> texture;
		std::shared_ptr<DX9GF::AnimatedSprite> sprite;
		std::shared_ptr<DX9GF::Texture> projTexture;
		std::weak_ptr<Player> player;

		//ability vars
		int currentCycle = -1;
		int skillTurnThisCycle = -1;

		int GetRandomPattern();
		void PatternEcholocation(float projDamage, std::vector<std::shared_ptr<IEnemy>>* enemies);
		void PatternSwoopBite(float projDamage);
	public:
		using EnemyBase<VampireBatEnemy>::EnemyBase;
		void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera);
		void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) override;

		void OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) override;
		void StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) override;
	};
}