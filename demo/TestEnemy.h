#pragma once
#include "IEnemy.h"

namespace Demo {
	class TestEnemy : public IEnemy {
	private:
		std::shared_ptr<DX9GF::Texture> texture;
		std::shared_ptr<DX9GF::StaticSprite> sprite;
		std::shared_ptr<DX9GF::Texture> roundProjectileTexture;
		std::weak_ptr<Player> player;
		int currentCycle = -1;
		int skillTurnThisCycle = -1;
		void AbilityTestHeal();
		void AbilityTestBuff();
		void AbilityTestDebuff();
		void AbilityTestLockCard();

	public:
		using IEnemy::IEnemy;
		void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera);
		void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) override;

		void OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) override;
		void StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) override;
	};
}