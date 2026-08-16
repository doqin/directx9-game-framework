#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Game.h"
#include "IconButton.h"
#include "KeyboardNavigator.h"

namespace Demo {
	class BattleMenu {
	private:
		Game* game;
		std::shared_ptr<DX9GF::TransformManager> transformManager;
		DX9GF::Camera* uiCamera;

		bool isOpen = false;

		// UI Buttons
		std::shared_ptr<IconButton> btnResume;
		std::shared_ptr<IconButton> btnOptions;
		std::shared_ptr<IconButton> btnLeaveGame;

		std::shared_ptr<DX9GF::Texture> uiTex;

		bool pendingLeave = false;

		KeyboardNavigator keyboardNavigator;
		std::vector<KeyboardNavigator::Candidate> CollectKeyboardCandidates();

	public:
		BattleMenu(Game* g, std::shared_ptr<DX9GF::TransformManager> tm, DX9GF::Camera* cam);

		void Init();
		void Update(unsigned long long deltaTime);
		void Draw(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime);
		void DrawKeyboardReticle(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime);

		void Toggle();
		bool IsOpen() const { return isOpen; }
		bool IsInKeyboardMode() const { return isOpen && keyboardNavigator.IsInKeyboardMode(); }
		bool IsPendingLeave() const { return pendingLeave; }
	};
}
