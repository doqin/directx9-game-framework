#pragma once
#include "IEnemy.h"

namespace Demo {
	class MimicEnemy : public IEnemy {
	private:
		std::shared_ptr<DX9GF::Texture> texture;
		std::shared_ptr<DX9GF::AnimatedSprite> sprite;
		std::shared_ptr<DX9GF::Texture> projTexture;
		std::weak_ptr<Player> player;

		int GetRandomPattern();
		void PatternCoinCyclone(float projDamage, std::vector<std::shared_ptr<IEnemy>>* enemies);
		void PatternJunkVomit(float projDamage, std::vector<std::shared_ptr<IEnemy>>* enemies);

	public:
		using IEnemy::IEnemy;
		void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera);
		void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) override;
		void StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) override;
	};
}