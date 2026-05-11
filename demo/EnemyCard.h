#pragma once
#include "IExpressionCard.h"

namespace Demo {
	class EnemyCard : public IExpressionCard {
	private:
		std::shared_ptr<IEnemy> enemy;
		std::shared_ptr<DX9GF::Font> nameFont;
		std::shared_ptr<DX9GF::FontSprite> nameFontSprite;
		std::shared_ptr<DX9GF::Texture> arrowTexture;
		std::shared_ptr<DX9GF::StaticSprite> arrowSprite;
		std::shared_ptr<DX9GF::Texture> uiTexture;
		std::shared_ptr<DX9GF::StaticSprite> targetSprite;
		std::shared_ptr<DX9GF::StaticSprite> untargetedSprite;
		std::shared_ptr<DX9GF::StaticSprite> targetedSprite;
	public:
		EnemyCard(std::weak_ptr<DX9GF::TransformManager> tm, std::shared_ptr<IEnemy> enemy, float x = 0, float y = 0)
			: IGameObject(tm, x, y), IExpressionCard(tm, 96, 32, x, y), enemy(enemy) {}
		std::shared_ptr<IEnemy> GetValue() override;
		void Draw(unsigned long long deltaTime) override;
	};
}
