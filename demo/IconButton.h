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
		std::shared_ptr<DX9GF::Texture> texture;
		std::shared_ptr<DX9GF::StaticSprite> sprite;
		//only built when slicing is enabled, draws the frame as left cap + stretched centre + right cap
		std::shared_ptr<DX9GF::NineSliceSprite> sliceSprite;
		std::vector<RECT> buttonRects;
		float spriteOffsetX = 0.0f;
		float spriteOffsetY = 0.0f;
		float spriteScaleX = 1.0f;
		int sliceLeftMargin = 0;
		int sliceRightMargin = 0;
		bool isSliced = false;

		void RebuildSliceSprite();
	public:

		IconButton(std::shared_ptr<DX9GF::TransformManager> tm, float displayX, float displayY, int imgW, int imgH,
			std::shared_ptr<DX9GF::Texture> uiSheetTex, int frames = 3);

		//An extra flag to determine the frame cutting method.
		IconButton* SetSpriteCoords(int startX, int startY, int imgW, int imgH, int spacing, bool isVertical = false);
		IconButton* SetSpriteRects(std::vector<RECT> rects);

		//Cuts the frame into three horizontal segments (in source pixels) so the button can be
		//stretched sideways without smearing its rounded caps. Pass 0/0 to go back to plain drawing.
		IconButton* SetSliceMargins(int left, int right);
		bool IsSliced() const { return this->isSliced; }
		//Smallest width the sliced sprite can render at, i.e. both caps touching.
		float GetSliceCapsWidth() const;
		void Init(DX9GF::Camera* cam) override;
		void SetSpriteScale(float scaleX, float scaleY);
		void SetSpriteColor(D3DCOLOR color);
		void Draw(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) override;
		void SetSpriteRotation(float radians);
		void SetSpriteOrigin(float x, float y);
		void SetSpriteOffset(float dx, float dy);
	};
}