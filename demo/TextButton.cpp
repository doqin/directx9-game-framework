#include "pch.h"
#include "TextButton.h"

Demo::TextButton::TextButton(std::shared_ptr<DX9GF::TransformManager> tm,
	float x, float y, float w, float h,
	std::string txtContent, DX9GF::Font* f,
	std::function<void(DX9GF::ITrigger*)> onClick)
	: IButton(tm, x, y, w, h, 4)
{
	this->text = txtContent;
	this->wText = std::wstring(txtContent.begin(), txtContent.end());
	this->fontSprite = std::make_shared<DX9GF::FontSprite>(f);

	//Color & bg init
	this->textColors =
	{
		0xFFFFFFFF, // 0: IDLE (white)
		0xFFFFFF00, // 1: HOVER (yellow)
		0xFFFF0000, // 2: CLICKED (red)
		0xFF888888  // 3: DISABLED (grey)
	};

	this->bgColors = {
		D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f), // 0: IDLE (dark grey)
		D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f), // 1: HOVER (light grey)
		D3DXCOLOR(0.1f, 0.8f, 0.1f, 1.0f), // 2: CLICKED (green)
		D3DXCOLOR(0.2f, 0.2f, 0.2f, 0.5f)  // 3: DISABLED (slightly transparent)
	};

	this->hasOutline = false;
	this->outlineThickness = 0.0f;
	this->paddingX = 0.0f;
	this->paddingY = 0.0f;
	this->outlineColors = { D3DXCOLOR(0,0,0,1), D3DXCOLOR(0,0,0,1), D3DXCOLOR(0,0,0,1), D3DXCOLOR(0,0,0,0.5f) };

	this->SetOnReleaseLeft(onClick);
}


Demo::TextButton* Demo::TextButton::SetText(std::string newText)
{
	this->text = newText;
	this->wText = std::wstring(newText.begin(), newText.end());
	return this;
}

Demo::TextButton* Demo::TextButton::SetBackgroundColors(D3DXCOLOR idle, D3DXCOLOR hover, D3DXCOLOR click, D3DXCOLOR disabled)
{
	bgColors[0] = idle; bgColors[1] = hover; bgColors[2] = click; bgColors[3] = disabled;
	return this;
}

Demo::TextButton* Demo::TextButton::SetTextColors(DWORD idle, DWORD hover, DWORD click, DWORD disabled)
{
	textColors[0] = idle; textColors[1] = hover; textColors[2] = click; textColors[3] = disabled;
	return this;
}

Demo::TextButton* Demo::TextButton::SetOutline(float thickness, D3DXCOLOR idle, D3DXCOLOR hover, D3DXCOLOR click, D3DXCOLOR disabled)
{
	this->hasOutline = true;
	this->outlineThickness = thickness;
	this->outlineColors[0] = idle;
	this->outlineColors[1] = hover;
	this->outlineColors[2] = click;
	this->outlineColors[3] = disabled;
	return this;
}

Demo::TextButton* Demo::TextButton::SetPadding(float px, float py)
{
	this->paddingX = px;
	this->paddingY = py;
	return this;
}

void Demo::TextButton::Init(DX9GF::Camera* cam)
{
	this->trigger = std::make_shared<DX9GF::RectangleTrigger>
		(
			this->GetTransformManager(), shared_from_this(),
			this->width + this->paddingX * 2, this->height + this->paddingY * 2, 0, 0, 0, 1, 1
		);

	//lock the trigger
	this->trigger->SetLocalPosition(0, 0);

	this->uiCamera = cam;
	this->trigger->Init(cam);
	this->trigger->SetOnReleaseLeft(this->callback);
}

void Demo::TextButton::Draw(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
{

	if (!this->fontSprite) return;

	int stateIndex = 0; // IDLE default
	if (this->currentState == ButtonState::HOVER) stateIndex = 1;

	else if (this->currentState == ButtonState::CLICKED || this->currentState == ButtonState::LISTENING) stateIndex = 2;

	else if (this->currentState == ButtonState::DISABLED) stateIndex = 3;

	auto currX = this->GetWorldX();
	auto currY = this->GetWorldY();

	if (hasOutline && gd && uiCamera) {
		float outW = (this->width + paddingX * 2 + outlineThickness * 2) * GetWorldScaleX();
		float outH = (this->height + paddingY * 2 + outlineThickness * 2) * GetWorldScaleY();
		float offX = outlineThickness * GetWorldScaleX();
		float offY = outlineThickness * GetWorldScaleY();

		gd->DrawRectangle(
			*uiCamera,
			currX, currY, outW, outH,
			0, 1, 1, offX, offY,
			outlineColors[stateIndex],
			true
		);
	}

	//draw button's background
	if (gd && uiCamera) {
		gd->DrawRectangle(
			*uiCamera,
			currX, currY, (this->width + paddingX * 2) * GetWorldScaleX(), (this->height + paddingY * 2) * GetWorldScaleY(),
			0, 1, 1, 0, 0,
			bgColors[stateIndex],
			true
		);
	}

	//draw button's text
	if (fontSprite && uiCamera) {
		fontSprite->Begin();
		fontSprite->SetScale(GetWorldScaleX(), GetWorldScaleY());
		fontSprite->SetPosition(currX + paddingX * GetWorldScaleX(), currY + paddingY * GetWorldScaleY());
		fontSprite->SetColor(textColors[stateIndex]);
		fontSprite->SetText(std::wstring(this->wText));
		fontSprite->Draw(*uiCamera, deltaTime);
		fontSprite->End();
	}

}