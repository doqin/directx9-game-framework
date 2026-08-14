#include "pch.h"
#include "IconButton.h"
#include <algorithm>


Demo::IconButton::IconButton(std::shared_ptr<DX9GF::TransformManager> tm, float displayX, float displayY, int imgW, int imgH,
	std::shared_ptr<DX9GF::Texture> uiSheetTex, int frames)
	: IButton(tm, displayX, displayY, imgW, imgH, frames), texture(uiSheetTex)
{
	if (uiSheetTex) {
		this->sprite = std::make_shared<DX9GF::StaticSprite>(uiSheetTex.get());
	}
	this->buttonRects.resize(frames);
}

Demo::IconButton* Demo::IconButton::SetSpriteCoords(int startX, int startY, int imgW, int imgH, int spacing, bool isVertical)
{
	for (int i = 0; i < this->frameCount; ++i) {
		//Vertical = x fixed, y moves
		//Horizontal = x moves, y fixed
		this->buttonRects[i].left = startX + (isVertical ? 0 : i * (imgW + spacing));
		this->buttonRects[i].top = startY + (isVertical ? i * (imgH + spacing) : 0);
		this->buttonRects[i].right = this->buttonRects[i].left + imgW;
		this->buttonRects[i].bottom = this->buttonRects[i].top + imgH;
	}
	return this;
}

Demo::IconButton* Demo::IconButton::SetSpriteRects(std::vector<RECT> rects)
{
	this->buttonRects = rects;
	return this;
}

Demo::IconButton* Demo::IconButton::SetSliceMargins(int left, int right)
{
	this->sliceLeftMargin = std::max(0, left);
	this->sliceRightMargin = std::max(0, right);
	this->isSliced = (this->sliceLeftMargin > 0 || this->sliceRightMargin > 0);

	this->RebuildSliceSprite();
	return this;
}

float Demo::IconButton::GetSliceCapsWidth() const
{
	if (!this->isSliced) return 0.0f;
	return (this->sliceLeftMargin + this->sliceRightMargin) * this->spriteScaleX;
}

void Demo::IconButton::RebuildSliceSprite()
{
	if (!this->isSliced || !this->texture) {
		this->sliceSprite.reset();
		return;
	}

	//top/bottom margins stay at 0, so the nine slice degenerates into the three horizontal segments we want
	RECT src = this->buttonRects.empty() ? RECT{} : this->buttonRects[0];
	this->sliceSprite = std::make_shared<DX9GF::NineSliceSprite>(
		this->texture.get(), src, this->sliceLeftMargin, 0, this->sliceRightMargin, 0);

	//the caps keep the sprite's own zoom, only the centre segment gets stretched
	this->sliceSprite->SetScale(this->spriteScaleX);
	if (this->sprite) this->sliceSprite->SetColor(this->sprite->GetColor());
}

void Demo::IconButton::Init(DX9GF::Camera* uiCamera)
{
	if (!this->trigger) {
		this->trigger = std::make_shared<DX9GF::RectangleTrigger>
			(
				this->GetTransformManager(), shared_from_this(),
				this->width, this->height, 0, 0, 0, 1, 1
			);
		//lock the trigger
		this->trigger->SetLocalPosition(0, 0);
	}
	this->trigger->SetLocalPosition(0, 0);
	this->uiCamera = uiCamera;
	this->trigger->Init(uiCamera);
	this->trigger->SetOnReleaseLeft(this->callback);
}

void Demo::IconButton::SetSpriteScale(float scaleX, float scaleY)
{
	this->spriteScaleX = scaleX;
	if (this->sprite) this->sprite->SetScale(scaleX, scaleY);
	if (this->sliceSprite) this->sliceSprite->SetScale(scaleX);
}

void Demo::IconButton::SetSpriteColor(D3DCOLOR color)
{
	if (this->sprite) this->sprite->SetColor(color);
	if (this->sliceSprite) this->sliceSprite->SetColor(color);
}

void Demo::IconButton::Draw(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
{
	//prevent from crashing
	if (!this->sprite || !this->uiCamera || buttonRects.empty()) return;

	//mapping index
	int expectedIndex = 0;

	if (this->currentState == ButtonState::HOVER) expectedIndex = 1;
	else if (this->currentState == ButtonState::CLICKED || this->currentState == ButtonState::LISTENING) expectedIndex = 2;
	else if (this->currentState == ButtonState::DISABLED) expectedIndex = 3;

	//force min index when expectedIndex > frame counts
	int finalIndex = std::min(expectedIndex, this->frameCount - 1);

	if (this->isSliced && this->sliceSprite) {
		//stretch the frame to the button's current size instead of drawing it 1:1
		this->sliceSprite->SetSrcRect(this->buttonRects[finalIndex]);
		this->sliceSprite->SetTargetSize(this->width, this->height);
		this->sliceSprite->Begin();
		this->sliceSprite->SetPosition(GetWorldX() + spriteOffsetX, GetWorldY() + spriteOffsetY);
		this->sliceSprite->Draw(*uiCamera, deltaTime);
		this->sliceSprite->End();
		return;
	}

	this->sprite->SetSrcRect(this->buttonRects[finalIndex]);
	this->sprite->Begin();
	//Add offset to prevent the sprite from shifting out of the collision box after changing the pivot
	this->sprite->SetPosition(GetWorldX() + spriteOffsetX, GetWorldY() + spriteOffsetY);
	this->sprite->Draw(*uiCamera, deltaTime);
	this->sprite->End();

}

void Demo::IconButton::SetSpriteRotation(float radians) {
	if (this->sprite) this->sprite->SetRotation(radians);
	if (this->sliceSprite) this->sliceSprite->SetRotation(radians);
}

void Demo::IconButton::SetSpriteOrigin(float x, float y) {
	if (this->sprite) this->sprite->SetOrigin(x, y);
	if (this->sliceSprite) this->sliceSprite->SetOrigin(x, y);
}

void Demo::IconButton::SetSpriteOffset(float dx, float dy) {
	this->spriteOffsetX = dx;
	this->spriteOffsetY = dy;
}