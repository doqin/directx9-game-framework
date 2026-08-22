#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Game.h"
#include <functional>
#include <string>
#include <vector>

namespace Demo {
	// A modal 4-digit code breaker. The player types a guess and submits it; every submitted guess is
	// scored Wordle-style and kept on screen so the code can be deduced from the history. There is no
	// attempt limit - the menu stays open until the code is cracked or the player walks away.
	class PasswordMenu {
	public:
		enum class DigitState { Absent, Misplaced, Correct };

		struct ScoredGuess {
			std::wstring digits;
			std::vector<DigitState> states;
		};

	private:
		static constexpr int CODE_LENGTH = 4;
		// Older guesses scroll off the top so the panel can never outgrow the screen.
		static constexpr size_t MAX_VISIBLE_ROWS = 8;

		Game* game = nullptr;
		DX9GF::Camera* uiCamera = nullptr;
		std::shared_ptr<DX9GF::FontSprite> fontSprite;

		bool isOpen = false;
		std::wstring password;
		std::wstring currentGuess;
		std::vector<ScoredGuess> history;

		std::wstring statusMessage;
		float caretTimer = 0.0f;

		std::function<void()> onSolved;

		ScoredGuess Score(const std::wstring& guess) const;
		void Submit();

	public:
		void Init(DX9GF::Camera* uiCam, std::shared_ptr<DX9GF::Font> font, Game* game);

		// password must be CODE_LENGTH characters, each '0'-'9'.
		void Open(const std::wstring& password);
		void Close();
		bool IsOpen() const { return isOpen; }

		void SetOnSolved(std::function<void()> cb) { onSolved = std::move(cb); }

		void Update(unsigned long long deltaTime);
		void Draw(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime);
	};
}
