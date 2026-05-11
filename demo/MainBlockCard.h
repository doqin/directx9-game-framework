#pragma once
#include "IBlockCard.h"

namespace Demo {
	class MainBlockCard : public IBlockCard {
 private:
		std::shared_ptr<DX9GF::Font> nameFont;
		std::shared_ptr<DX9GF::FontSprite> nameFontSprite;
		std::shared_ptr<DX9GF::Texture> blockTexture;
		std::shared_ptr<DX9GF::StaticSprite> blockSprite;
		std::shared_ptr<DX9GF::StaticSprite> pointerSprite;
	public:
	   MainBlockCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), IBlockCard(tm, 160, 32, x, y) {}
      void Draw(unsigned long long deltaTime) override;
	};
}
