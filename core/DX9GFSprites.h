#pragma once
#include <string>
#include <vector>
#include "DX9GFISprite.h"
#include "DX9GFFont.h"
#include "DX9GFTexture.h"

namespace DX9GF {

	class StaticSprite : public ISprite {
	private:
		RECT* p_src = nullptr;
		Texture* texture;
		UINT width = 0, height = 0;
	public:
		StaticSprite(Texture* texture);
		~StaticSprite();

		
		void Begin() override;
		void Draw(const Camera& camera, unsigned long long deltaTime) override;
		void End() override;
		void SetSrcRect(RECT srcRect);
	};

	class AnimatedSprite : public ISprite {
	private:
		std::vector<RECT> frames;
		Texture* texture;

		UINT frame_index = 0;
		unsigned int frameRate = 24;
		unsigned long long delta = 0;
		bool isLooped;
	public:
		AnimatedSprite(Texture* spritesheet, std::vector<RECT> frames, UINT frameRate = 24, bool isLooped = true);
		~AnimatedSprite();
		void SetFrame(unsigned int frameIndex);
		void SetFrameRate(unsigned int frameRate);
		void SetLooped(bool isLooped);
		bool IsFinished() const; // returns true if the animation has finished playing (only relevant if isLooped is false)
		void Begin() override;
		void Draw(const Camera& camera, unsigned long long deltaTime) override;
		void End() override;
	};

	class FontSprite : public ISprite {
	private:
		RECT* p_src = nullptr;
		Font* font = nullptr;
		std::wstring text;
		bool isBold = false;
		bool isOutline = false;
		D3DCOLOR outlineColor = 0xFF000000;
		float outlineThickness = 1.0f;
	public:
		FontSprite(Font* font);
		~FontSprite();
		void SetText(std::wstring&& text);
		void SetColor(D3DCOLOR color);
		void SetBold(bool bold);
		void SetOutline(bool outline, D3DCOLOR color = 0xFF000000, float thickness = 1.0f);
	  LONG GetWidth() const;
		LONG GetHeight() const;
		void Begin() override;
		void Draw(const Camera& camera, unsigned long long deltaTime) override;
		void End() override;
	};
};