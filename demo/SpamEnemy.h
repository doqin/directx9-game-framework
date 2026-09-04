#pragma once
#include "IEnemy.h"
#include <memory>

namespace Demo {
	// Entry-level boss: a junk-mail process that got smart, guarding the tutorial exit portal.
	// 200 HP, five medium-difficulty bullet patterns picked from a shuffle bag, plus a light
	// heal / debuff on a three-turn cycle. Structurally a trimmed-down CupidEnemy (one
	// projectile texture, no minions).
	class SpamEnemy : public EnemyBase<SpamEnemy> {
	private:
		std::shared_ptr<DX9GF::Texture> texture;      // spam-Sheet.png body
		std::shared_ptr<DX9GF::AnimatedSprite> sprite;
		std::shared_ptr<DX9GF::Texture> projTexture;  // spamprojectile.png (single frame)

		std::weak_ptr<Player> player;

		// ability vars
		int currentCycle = -1;
		int skillTurnThisCycle = -1;

		void PatternInboxFlood(float projDamage);
		void PatternChainLetter(float projDamage);
		void PatternMassMailer(float projDamage);
		void PatternPopupAds(float projDamage);
		void PatternUnsubscribe(float projDamage);

	public:
		using EnemyBase<SpamEnemy>::EnemyBase;
		void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera);
		void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) override;

		void OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) override;
		void StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) override;
	};
}
