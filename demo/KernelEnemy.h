#pragma once
#include "IEnemy.h"
#include "DX9GFExtras.h"
#include <vector>

namespace Demo {
	class KernelEnemy : public IEnemy {
	private:
		std::shared_ptr<DX9GF::Texture> texture;
		std::shared_ptr<DX9GF::AnimatedSprite> sprite;

		std::shared_ptr<DX9GF::Texture> mineTexture;
		std::shared_ptr<DX9GF::Texture> bulletTexture;

		std::weak_ptr<Player> player;

		int laserState = 0;
		float storedProjDamage = 0.f;

		std::vector<float> laserVerticals;
		std::vector<float> laserHorizontals;

		void PatternDefrag(float projDamage);
		void PatternPing999(float projDamage);
		void PatternBadSector(float projDamage);

	public:
		using IEnemy::IEnemy;

		void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera);
		void Update(unsigned long long deltaTime) override;
		void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) override;
		void StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) override;
	};
}