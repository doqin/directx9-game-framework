#pragma once
#include "IEnemy.h"

namespace Demo {
	class DemonEyeEnemy : public IEnemy {
	private:
		std::shared_ptr<DX9GF::Texture> texture;
		std::shared_ptr<DX9GF::AnimatedSprite> sprite;
		std::shared_ptr<DX9GF::Texture> tearProjectileTexture;
		std::shared_ptr<DX9GF::AnimatedSprite> tearProjectileSprite;
		std::weak_ptr<Player> player;

		int GetRandomPattern();
		void PatternBloodRain(float projDamage);
		void PatternBloodWall(float projDamage);
		void PatternBloodCross(float projDamage);

	public:
		using IEnemy::IEnemy;
		void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera);
		void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) override;
		void StartAttack(std::shared_ptr<Player> player) override;
	};
}