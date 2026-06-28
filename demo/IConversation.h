#pragma once
#include <DX9GFExtras\DX9GFICommand.h>
#include <DX9GFSprites.h>
#include <DX9GFGraphicsDevice.h>
#include <DX9GFCamera.h>
#include <queue>
#include "DialogueLine.h"

namespace Demo {
	class IConversation : public DX9GF::ICommand {
	private:
		std::queue<DialogueLine> dialogueQueue;
		std::shared_ptr<DX9GF::FontSprite> fontSprite;
		float virtualWidth;
		float virtualHeight;
		std::wstring displayedContent;
		size_t currentCharIndex = 0;
		unsigned long long timer = 0;
		const unsigned long long MS_PER_CHAR = 30;
		bool isTyping = false;
		void ResetAnimation();
	public:
		IConversation(std::shared_ptr<DX9GF::FontSprite> fontSprite, int screenWidth, int screenHeight);

		void AddLine(const DialogueLine& line);

		void Execute(unsigned long long deltaTime) override;

		void Draw(DX9GF::GraphicsDevice* gd, DX9GF::Camera* uiCamera, unsigned long long deltaTime);
	};
}

