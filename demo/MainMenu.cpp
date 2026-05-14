#include "pch.h"
#include "MainMenu.h"
#include "resource.h"
#include "IconButton.h"
#include "SettingsScene.h"
#include "ThreadAlleyScene.h"
#include "TutorialWorldScene.h"
#include "SaveGameState.h"
#include <fstream>
#include <cstdio>

namespace Demo
{
	void MainMenu::UpdateLayout(int screenW, int screenH)
	{
		camera.SetPosition({ screenW / 2.f, 0 });

		//BACKGROUND - use aspect fill
		float bgImageW = (float)bgTex->GetWidth();
		float bgImageH = (float)bgTex->GetHeight();

		if (!bgSprite)
		{
			bgSprite = std::make_shared<DX9GF::StaticSprite>(bgTex.get());
			bgSprite->SetSrcRect({ 0, 0, (LONG)bgImageW, (LONG)bgImageH });
		}

		float bgScaleX = screenW / bgImageW;
		float bgScaleY = screenH / bgImageH;
		float bgFinalScale = std::max(bgScaleX, bgScaleY);

		bgSprite->SetScale(bgFinalScale);
		bgSprite->SetOrigin(bgImageW / 2.0f, bgImageH / 2.0f);
		bgSprite->SetPosition(0, 0);

		//spacing between buttons
		float spacingY = 10.0f;
		//start drawing from 40% of the screen height
		float startY = -screenH * 0.10f;

		std::shared_ptr<Demo::IButton> buttons[] = { continueButton, newGameButton, optionsButton, creditsButton, quitButton };
		float currentY = startY;

		for (auto& btn : buttons)
		{
			if (btn)
			{
				//center X
				float posX = 64.f;
				btn->SetLocalPosition(posX, currentY);

				//button spacing
				currentY += btn->GetHeight() + spacingY;
			}
		}

		//TITLE
		auto [_, y] = continueButton->GetLocalPosition();
		auto height = fontSprite->GetHeight();
		fontSprite->SetPosition(64.f, y - height * 3 - 32.f);
	}

	void MainMenu::Init()
	{

		transformManager = std::make_shared<DX9GF::TransformManager>();
       saveManager = std::make_shared<DX9GF::SaveManager>();
		gameSaveState = std::make_shared<SaveGameState>(game, saveManager);

		auto app = DX9GF::Application::GetInstance();
		lastScreenWidth = app->GetScreenWidth();
		lastScreenHeight = app->GetScreenHeight();

		//load textures
		buttonSheetTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		buttonSheetTex->LoadTexture(L"ui.png");

		bgTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		bgTex->LoadTexture(IDB_PNG2);

		titleTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		titleTex->LoadTexture(IDB_PNG3);


		// Sprites
		font = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 16);
		fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
		fontSprite->SetColor(0xFF000000);

		//BUTTONS INIT
		//Continue Button
		continueButton = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 96, 32, buttonSheetTex, 4);
		continueButton->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(144, 96, 48, 16, 4));
		//continueButton->SetOnReleaseLeft([](DX9GF::ITrigger* t) { /* Logic */ });
		continueButton->SetSpriteScale(2.f, 2.f);
		/*continueButton->SetState(IButton::ButtonState::DISABLED);*/

		std::ifstream f("savegame.json");
		if (f.good()) {
			continueButton->SetState(IButton::ButtonState::IDLE);
		}
		else {
			continueButton->SetState(IButton::ButtonState::DISABLED);
		}
		f.close();

        continueButton->SetOnReleaseLeft([this](DX9GF::ITrigger* t) {
			gameSaveState = SaveGameState::LoadSavedGame(game, saveManager);
		});

		//New Game Button
		newGameButton = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 96, 32, buttonSheetTex, 3);
		newGameButton->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(144, 48, 48, 16, 3));
        newGameButton->SetOnReleaseLeft([this](DX9GF::ITrigger* t) { 
			std::remove("savegame.json");
          gameSaveState = SaveGameState::StartNewGame(game, saveManager);
		});
		newGameButton->SetSpriteScale(2.f, 2.f);

		//Options Button
		optionsButton = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 96, 32, buttonSheetTex, 3);
		optionsButton->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(192, 48, 48, 16, 3));
		optionsButton->SetOnReleaseLeft([this](DX9GF::ITrigger* t) {
			auto app = DX9GF::Application::GetInstance();
			//push Settings Scene
			this->game->GetSceneManager()->PushScene(
				new SettingsScene(this->game, app->GetScreenWidth(), app->GetScreenHeight())
			);
			this->game->GetSceneManager()->GoToNext();
		});
		optionsButton->SetSpriteScale(2.f, 2.f);

		//Credits Button
		creditsButton = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 96, 32, buttonSheetTex, 3);
		creditsButton->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(192, 96, 48, 16, 3));
		creditsButton->SetOnReleaseLeft([](DX9GF::ITrigger* t) { /* Logic */ });
		creditsButton->SetSpriteScale(2.f, 2.f);

		//Quit Button
		quitButton = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 96, 32, buttonSheetTex, 3);
		quitButton->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(240, 48, 48, 16, 3));
		quitButton->SetOnReleaseLeft([](DX9GF::ITrigger* t) { PostQuitMessage(0); });
		quitButton->SetSpriteScale(2.f, 2.f);

		//active buttons
		std::shared_ptr<Demo::IButton> buttons[] = { continueButton, newGameButton, optionsButton, creditsButton, quitButton };
		for (auto& btn : buttons)
		{
			if (btn)
			{
				btn->Init(&camera);
				uiButtons.push_back(btn);
			}
		}

		//call it to setup the update layout
		UpdateLayout(lastScreenWidth, lastScreenHeight);

		transformManager->RebuildHierarchy();
	}

	void MainMenu::Update(unsigned long long deltaTime)
	{
		auto [currW, currH] = camera.GetScreenResolution();
		auto [lastWidth, lastHeight] = uiCamera.GetScreenResolution();
		if (currW != lastWidth || currH != lastHeight) {
			uiCamera.SetScreenResolution(currW, currH);
		}
		auto inpMan = DX9GF::InputManager::GetInstance();
		inpMan->ReadMouse(deltaTime);
		inpMan->ReadKeyboard(deltaTime);

		auto app = DX9GF::Application::GetInstance();
		int currentWidth = app->GetScreenWidth();
		int currentHeight = app->GetScreenHeight();

		static float timer = 0;
		timer += deltaTime;
		if (timer > 0) {
			std::ifstream f("savegame.json");
			if (f.good()) {
				continueButton->SetState(IButton::ButtonState::IDLE);
			}
			else {
				continueButton->SetState(IButton::ButtonState::DISABLED);
			}
			f.close();
			timer = 0;
		}

		UpdateLayout(currW, currH);

		for (auto& button : uiButtons)
		{
			button->Update(deltaTime);
		}

		transformManager->UpdateAll();
		camera.Update();
	}

	void MainMenu::Draw(unsigned long long deltaTime)
	{
		auto gd = game->GetGraphicsDevice();
		gd->Clear(0xFFFFFFFF);
		if (SUCCEEDED(gd->BeginDraw())) {
			//if (bgSprite)
			//{
			//	bgSprite->Begin();
			//	bgSprite->Draw(camera, deltaTime);
			//	bgSprite->End();
			//}
			fontSprite->Begin();
			auto prevPos = fontSprite->GetPosition();
			auto height = fontSprite->GetHeight();
			fontSprite->SetText(L"Toi, sinh vien nam 6 UIT,");
			fontSprite->Draw(camera, deltaTime);
			fontSprite->SetPosition(prevPos.x, prevPos.y + height);
			fontSprite->SetText(L"bi hut vao cyberspace");
			fontSprite->Draw(camera, deltaTime);
			fontSprite->SetPosition(prevPos.x, prevPos.y + height * 2);
			fontSprite->SetText(L"vi click vao link doc");
			fontSprite->Draw(camera, deltaTime);
			fontSprite->SetPosition(prevPos.x, prevPos.y);
			fontSprite->End();

			for (auto& btn : uiButtons)
			{
				btn->Draw(gd, deltaTime);
			}
			DX9GF::InputManager::GetInstance()->DrawCursor(&this->uiCamera, deltaTime);
			gd->EndDraw();
		}
		gd->Present();
	}

}
