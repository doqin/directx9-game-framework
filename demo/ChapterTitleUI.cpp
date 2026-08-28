#include "pch.h"
#include "ChapterTitleUI.h"
#include <algorithm>

namespace Demo {
	ChapterTitleUI::ChapterTitleUI(std::shared_ptr<DX9GF::Font> font)
		: state(State::Hidden), timer(0), maxTime(0), alpha(0),
		titleColor(0xFFFFFFFF), subColor(0xFFFFFFFF), shadowColor(0xFF000000)
	{
		if (font) {
			fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
		}
	}

	void ChapterTitleUI::Show(const std::wstring& title, const std::wstring& sub, float duration, D3DCOLOR tColor, D3DCOLOR sColor, D3DCOLOR shColor) {
		mainTitle = title;
		subTitle = sub;
		maxTime = duration;
		titleColor = tColor;
		subColor = sColor;
		shadowColor = shColor;

		timer = 0.0f;
		alpha = 0.0f;
		state = State::FadeIn;
	}

	void ChapterTitleUI::Update(unsigned long long deltaTime) {
		if (state == State::Hidden) return;

		float dt = static_cast<float>(deltaTime) / 1000.0f;
		const float fadeSpeed = 1.5f;

		switch (state) {
		case State::FadeIn:
			alpha += fadeSpeed * dt;
			if (alpha >= 1.0f) {
				alpha = 1.0f;
				state = State::Visible;
			}
			break;
		case State::Visible:
			timer += dt;
			if (timer >= maxTime) {
				state = State::FadeOut;
			}
			break;
		case State::FadeOut:
			alpha -= fadeSpeed * dt;
			if (alpha <= 0.0f) {
				alpha = 0.0f;
				state = State::Hidden;
			}
			break;
		default:
			break;
		}
	}

	void ChapterTitleUI::Draw(DX9GF::Camera* uiCam, unsigned long long deltaTime) {
		if (state == State::Hidden || !fontSprite || !uiCam) return;

		int a = static_cast<int>((std::max)(0.f, (std::min)(255.f, alpha * 255.f)));
		if (a <= 0) return;

		D3DCOLOR alphaBits = static_cast<D3DCOLOR>(a) << 24;
		D3DCOLOR finalTitleCol = alphaBits | (titleColor & 0x00FFFFFF);
		D3DCOLOR finalSubCol = alphaBits | (subColor & 0x00FFFFFF);
		D3DCOLOR finalShadowCol = alphaBits | (shadowColor & 0x00FFFFFF);

		fontSprite->Begin();
		float scaleRat = 1.3f;
		fontSprite->SetScale(scaleRat);

		fontSprite->SetText(mainTitle);
		float titleW = static_cast<float>(fontSprite->GetWidth() * scaleRat);
		float titleH = static_cast<float>(fontSprite->GetHeight() * scaleRat);

		fontSprite->SetText(subTitle);
		float subW = static_cast<float>(fontSprite->GetWidth() * scaleRat);
		float subH = static_cast<float>(fontSprite->GetHeight() * scaleRat);

		const float gap = 12.0f;
		const float totalH = titleH + gap + subH;
		const float startY = -totalH / 2.0f;

		fontSprite->SetText(mainTitle);
		fontSprite->SetColor(finalTitleCol);
		fontSprite->SetOutline(true, finalShadowCol, 2.f);
		fontSprite->SetPosition(-titleW / 2.0f, startY);
		fontSprite->Draw(*uiCam, deltaTime);

		fontSprite->SetText(subTitle);
		fontSprite->SetColor(finalSubCol);
		fontSprite->SetOutline(true, finalShadowCol, 2.f);
		fontSprite->SetPosition(-subW / 2.0f, startY + titleH + gap);
		fontSprite->Draw(*uiCam, deltaTime);

		fontSprite->End();
		fontSprite->SetOutline(false);
	}
}