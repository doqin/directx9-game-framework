#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "DX9GFIScene.h"
#include "Game.h"
#include "TextButton.h"
#include "IconButton.h"

namespace Demo
{
   class SaveGameState;
	class MainMenu : public DX9GF::IScene
	{
	private:
		Game* game;
		std::shared_ptr<DX9GF::TransformManager> transformManager;
		std::shared_ptr<DX9GF::SaveManager> saveManager;
      std::shared_ptr<SaveGameState> gameSaveState;
		std::shared_ptr<DX9GF::Font> font;
		std::shared_ptr<DX9GF::FontSprite> fontSprite;
		std::shared_ptr<DX9GF::StaticSprite> bgSprite;
		std::shared_ptr<DX9GF::Texture> bgTex;
		std::shared_ptr<DX9GF::Texture> titleTex;
		DX9GF::Camera uiCamera;

		//button component
		std::shared_ptr<DX9GF::Texture> buttonSheetTex;
		std::vector<std::shared_ptr<Demo::IButton>> uiButtons;

		//temporarily comment, only uncomment when needed. Ex: That button changes affect another component in the scene.

		std::shared_ptr<IconButton> continueButton;
		std::shared_ptr<IconButton> newGameButton;
		std::shared_ptr<IconButton> optionsButton;
		std::shared_ptr<IconButton> creditsButton;
		std::shared_ptr<IconButton> quitButton;

		//use to scale or relocate the sprite/object
		int lastScreenWidth;
		int lastScreenHeight;

	public:

		MainMenu(Game* game, int screenWidth, int screenHeight)
			: IScene(screenWidth, screenHeight),
			game(game), uiCamera(screenWidth, screenHeight) {}
		void Init() override;
		void Update(unsigned long long deltaTime) override;
		void Draw(unsigned long long deltaTime) override;
		void UpdateLayout(int width, int height);

	};
}

