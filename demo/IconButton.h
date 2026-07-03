#pragma once
#include "IButton.h"
#include <DX9GF.h>
#include "DX9GFTexture.h"
#include <functional>
#include <vector>
namespace Demo
{
	class IconButton : public IButton
	{
	private:
		std::shared_ptr<DX9GF::StaticSprite> sprite;
		std::vector<RECT> buttonRects;
		float spriteOffsetX = 0.0f;
		float spriteOffsetY = 0.0f;
	public:

		IconButton(std::shared_ptr<DX9GF::TransformManager> tm, float displayX, float displayY, int imgW, int imgH,
			std::shared_ptr<DX9GF::Texture> uiSheetTex, int frames = 3);

		//An extra flag to determine the frame cutting method.
		IconButton* SetSpriteCoords(int startX, int startY, int imgW, int imgH, int spacing, bool isVertical = false);
		IconButton* SetSpriteRects(std::vector<RECT> rects);
		void Init(DX9GF::Camera* cam) override;
		void SetSpriteScale(float scaleX, float scaleY);
		void SetSpriteColor(D3DCOLOR color);
		void Draw(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) override;
		void SetSpriteRotation(float radians);
		void SetSpriteOrigin(float x, float y);
		void SetSpriteOffset(float dx, float dy);
	};
}