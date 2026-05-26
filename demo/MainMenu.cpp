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
#include "TransitionCommand.h"

namespace Demo
{
	std::shared_ptr<SaveGameState> MainMenu::gameSaveState = nullptr;
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
		fontSprite->SetPosition(64.f, y - height * 3 * 2.f - 32.f);
	}

	void MainMenu::DrawBackground(unsigned long long deltaTime)
	{
		const D3DCOLOR polyColor = 0xFF9cdb43;
		//const D3DCOLOR crackColor = 0xFF9cdb43;
		auto [screenWidth, screenHeight] = camera.GetScreenResolution();

		//shared static timer variable for background processes
		static float timeAcc = 0.0f;
		timeAcc += static_cast<float>(deltaTime) * 0.001f;

		for (int i = 0; i < 15; ++i) {
			float size = 100.0f + (i % 3) * 50.0f;
			float margin = size * 2.0f;
			float bx = std::fmod((i * 123.0f) + timeAcc * 10.0f, static_cast<float>(screenWidth) + margin * 2.0f) - margin;
			float by = std::fmod((i * 456.0f) + timeAcc * 5.0f, static_cast<float>(screenHeight) + margin * 2.0f) - margin;
			float angle = timeAcc * 0.1f + i;

			float glitchSize = size + std::sinf(timeAcc * 2.0f + i) * 5.0f;

			float prevX = bx + std::cosf(angle) * glitchSize;
			float prevY = by + std::sinf(angle) * glitchSize;

			for (int v = 1; v <= 5; ++v) {
				float vAngle = angle + (v * 72.0f * 3.14159f / 180.0f);
				float vx = bx + std::cosf(vAngle) * glitchSize;
				float vy = by + std::sinf(vAngle) * glitchSize;

				game->GetGraphicsDevice()->DrawLine(prevX, prevY, vx, vy, polyColor);
				prevX = vx;
				prevY = vy;
			}

			//if (static_cast<int>(timeAcc * 2.0f + i) % 7 == 0) {
			//	float crackX = bx + (rand() % static_cast<int>(size)) - size / 2.0f;
			//	float crackY = by + (rand() % static_cast<int>(size)) - size / 2.0f;
			//	game->GetGraphicsDevice()->DrawLine(crackX - 20, crackY - 20, crackX + 20, crackY + 20, crackColor);
			//}
		}
	}

	void MainMenu::Init()
	{
		drawBuffer = std::make_shared<DX9GF::CommandBuffer>();
		commandBuffer = std::make_shared<DX9GF::CommandBuffer>();
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
			if (isTransitioning) return;
			isTransitioning = true;
			auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), 1.f, true);
			drawBuffer->PushCommand(transitionInCommand);
			commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this, transitionInCommand](std::function<void(void)> markFinished) {
				if (!transitionInCommand->IsFinished()) {
					return;
				}
				gameSaveState = SaveGameState::LoadSavedGame(game, saveManager);
				isTransitioning = false;
				markFinished();
				}));
			drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), 1.f, false));
		});

		//New Game Button
		newGameButton = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 96, 32, buttonSheetTex, 3);
		newGameButton->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(144, 48, 48, 16, 3));
        newGameButton->SetOnReleaseLeft([this](DX9GF::ITrigger* t) { 
			if (isTransitioning) return;
			isTransitioning = true;
			auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), 1.f, true);
			drawBuffer->PushCommand(transitionInCommand);
			commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this, transitionInCommand](std::function<void(void)> markFinished) {
				if (!transitionInCommand->IsFinished()) {
					return;
				}
				std::remove("savegame.json");
				gameSaveState = SaveGameState::StartNewGame(game, saveManager);
				isTransitioning = false;
				markFinished();
			}));
			drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), 1.f, false));
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

		//i removed the timer check for savegame.json here becuz it keeps spamming the audio. i think init() did a good job checking on savegame already

		UpdateLayout(currW, currH);

		for (auto& button : uiButtons)
		{
			button->Update(deltaTime);
		}

		transformManager->UpdateAll();
		camera.Update();
		commandBuffer->Update(deltaTime);
	}

	void MainMenu::Draw(unsigned long long deltaTime)
	{
		auto gd = game->GetGraphicsDevice();
		gd->Clear(0xFF242234);
		if (SUCCEEDED(gd->BeginDraw())) {
			//if (bgSprite)
			//{
			//	bgSprite->Begin();
			//	bgSprite->Draw(camera, deltaTime);
			//	bgSprite->End();
			//}
			DrawBackground(deltaTime);
			fontSprite->Begin();
			fontSprite->SetScale(2.f, 2.f);
			auto prevPos = fontSprite->GetPosition();
			auto height = fontSprite->GetHeight() * 2.f;
			fontSprite->SetColor(0xFFFFFFFF);
			fontSprite->SetOutline(true, 0xFF000000, 2.0f);
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
			drawBuffer->Update(deltaTime);
			DX9GF::InputManager::GetInstance()->DrawCursor(&this->uiCamera, deltaTime);
			gd->EndDraw();
		}
		gd->Present();
	}

}
