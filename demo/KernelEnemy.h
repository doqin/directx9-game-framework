#pragma once
#include "IEnemy.h"
#include "DX9GFExtras.h"
#include <utility>
#include <vector>

namespace Demo {
	class KernelEnemy : public EnemyBase<KernelEnemy> {
	private:
		std::shared_ptr<DX9GF::Texture> texture;
		std::shared_ptr<DX9GF::AnimatedSprite> sprite;

		std::shared_ptr<DX9GF::Texture> mineTexture;
		std::shared_ptr<DX9GF::Texture> bulletTexture;

		std::weak_ptr<Player> player;

		// The beams themselves live in the projectile system; Bad Sector only keeps the
		// lines it picked so it can work out which cell the player is boxed into, and
		// whether that cell is currently being telegraphed.
		bool telegraphingCell = false;
		std::vector<float> laserVerticals;
		std::vector<float> laserHorizontals;

		std::pair<float, float> GetSafeCellCenter(float playerX, float playerY) const;

		//ability vars
		int currentCycle = -1;
		int skillTurnThisCycle = -1;

		void PatternDefrag(float projDamage);
		void PatternPing999(float projDamage);
		void PatternZigzag(float projDamage);
		void PatternBadSector(float projDamage);

	public:
		using EnemyBase<KernelEnemy>::EnemyBase;

		void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera);
		void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) override;

		void OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) override;
		void StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) override;
	};
}