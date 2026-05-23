#pragma once
#include "IEnemy.h"

namespace Demo {
	class CupidEnemy : public IEnemy {
	private:
		std::shared_ptr<DX9GF::Texture> texture;
		std::shared_ptr<DX9GF::AnimatedSprite> sprite;

		std::shared_ptr<DX9GF::Texture> heartTexture;
		std::shared_ptr<DX9GF::StaticSprite> heartSprite;

		std::shared_ptr<DX9GF::Texture> arrowTexture;
		std::shared_ptr<DX9GF::StaticSprite> arrowSprite;

		std::weak_ptr<Player> player;

		int GetRandomPattern();
		void PatternHeartWave(float projDamage); 
		void PatternHomingArrow(float projDamage);
		void PatternHeartNova(float projDamage);

	public:
		using IEnemy::IEnemy;
		void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera);
		void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) override;
		void StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) override;
	};
}