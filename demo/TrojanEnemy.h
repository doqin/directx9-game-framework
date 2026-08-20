#pragma once
#include "IEnemy.h"
namespace Demo {
	class TrojanEnemy : public IEnemy {
	private:
		std::shared_ptr<DX9GF::Texture> texture;
		std::shared_ptr<DX9GF::AnimatedSprite> sprite;
		std::shared_ptr<DX9GF::Texture> projTexture;
		std::vector<RECT> projFrames;
		std::weak_ptr<Player> player;
		
		int currentCycle = -1;
		int skillTurnThisCycle = -1;
		int GetRandomPattern();
		void PatternTrojanBolt(float projDamage);
		void PatternVirusSpread(float projDamage);

		bool hasPendingStatus = false;
		static constexpr float FREEZE_VALUE = 0.4f;
		static constexpr float BURN_VALUE = 3.f;
		static constexpr int FREEZE_DURATION = 1;
		static constexpr int BURN_DURATION = 1;
	public:
		using IEnemy::IEnemy;
		// 96px frames drawn at 2x.
		float GetBodyWidth() const override { return 192.f; }
		float GetBodyHeight() const override { return 192.f; }
		void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera);
		void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) override;
		void OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) override;
		void StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) override;
	};
}