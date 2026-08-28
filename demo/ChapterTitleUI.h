#pragma once
#include "DX9GF.h"
#include <string>
#include <memory>

namespace Demo {
	class ChapterTitleUI {
	private:
		std::wstring mainTitle;
		std::wstring subTitle;
		float timer;
		float maxTime;
		float alpha;

		enum class State { Hidden, FadeIn, Visible, FadeOut };
		State state;

		std::shared_ptr<DX9GF::FontSprite> fontSprite;

		D3DCOLOR titleColor;
		D3DCOLOR subColor;
		D3DCOLOR shadowColor;

	public:
		ChapterTitleUI(std::shared_ptr<DX9GF::Font> font);

		void Show(const std::wstring& title, const std::wstring& sub, float duration, D3DCOLOR tColor, D3DCOLOR sColor, D3DCOLOR shColor);
		void Update(unsigned long long deltaTime);

		void Draw(DX9GF::Camera* uiCam, unsigned long long deltaTime);

		bool IsActive() const { return state != State::Hidden; }
	};
}