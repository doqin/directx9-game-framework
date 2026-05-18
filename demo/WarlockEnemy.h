#pragma once
#include "IEnemy.h"

namespace Demo {
	class WarlockEnemy : public IEnemy {
	private:
		std::shared_ptr<DX9GF::Texture> texture;
		std::shared_ptr<DX9GF::AnimatedSprite> sprite;
		std::shared_ptr<DX9GF::Texture> projTexture;
		std::shared_ptr<DX9GF::StaticSprite> projSprite;
		std::weak_ptr<Player> player;

		int GetRandomPattern();
		void PatternDarkVortex(float projDamage);
		void PatternHomingCurse(float projDamage);

	public:
		using IEnemy::IEnemy;
		void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera);
		void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) override;
		void StartAttack(std::shared_ptr<Player> player) override;
	};
}