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
#include "CreditsScene.h"
#include "PopupManager.h"

namespace Demo
{
	std::shared_ptr<SaveGameState> MainMenu::gameSaveState = nullptr;
	void MainMenu::UpdateLayout(int screenW, int screenH)
	{
		// BACKGROUND - use aspect fill
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

		// Draw UI: Anchor to the left edge of the virtual screen to keep it clear of the black bars
		float spacingY = 10.0f;
		float startY = -screenH * 0.10f;
		float currentY = startY;

		// -screenW / 2.f is the left edge of the gameplay area, +64.f offsets it inward
		float leftAnchorX = -screenW / 2.f + 64.f;

		std::shared_ptr<Demo::IButton> buttons[] = { continueButton, newGameButton, optionsButton, creditsButton, quitButton };

		for (auto& btn : buttons)
		{
			if (btn)
			{
				btn->SetLocalPosition(leftAnchorX, currentY);
				currentY += btn->GetHeight() + spacingY;
			}
		}

		// TITLE
		auto [_, y] = continueButton->GetLocalPosition();
		auto height = fontSprite->GetHeight();
		fontSprite->SetPosition(leftAnchorX, y - height * 3 * 2.f - 32.f);
	}

	void MainMenu::DrawBackground(unsigned long long deltaTime)
	{
		auto [screenWidth, screenHeight] = camera.GetScreenResolution();

		game->GetGraphicsDevice()->DrawRectangle(0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0xFF242234, true);

		const D3DCOLOR polyColor = 0xFF9cdb43;
		//const D3DCOLOR crackColor = 0xFF9cdb43;

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
		}
	}

	void MainMenu::Init()
	{
		drawBuffer = std::make_shared<DX9GF::CommandBuffer>();
		commandBuffer = std::make_shared<DX9GF::CommandBuffer>();
		transformManager = std::make_shared<DX9GF::TransformManager>();
		saveManager = std::make_shared<DX9GF::SaveManager>();
		gameSaveState = std::make_shared<SaveGameState>(game, saveManager);

		auto [camW, camH] = camera.GetScreenResolution();
		lastScreenWidth = camW;
		lastScreenHeight = camH;

		//load textures
		buttonSheetTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		buttonSheetTex->LoadTexture(L"assets/ui.png");

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

		auto borderTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		borderTex->LoadTexture(L"assets/popup-borders.png");

		auto uiTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		uiTex->LoadTexture(L"assets/ui.png");

		PopupManager::GetInstance()->Init(game->GetGraphicsDevice(), borderTex, uiTex, font);

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
			auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, true);
			drawBuffer->PushCommand(transitionInCommand);
			commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this, transitionInCommand](std::function<void(void)> markFinished) {
				if (!transitionInCommand->IsFinished()) {
					return;
				}
				gameSaveState = SaveGameState::LoadSavedGame(game, saveManager);
				isTransitioning = false;
				markFinished();
				}));
			drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));			});

		//New Game Button
		newGameButton = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 96, 32, buttonSheetTex, 3);
		newGameButton->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(144, 48, 48, 16, 3));
		newGameButton->SetOnReleaseLeft([this](DX9GF::ITrigger* t) {
			if (isTransitioning) return;

			// Kiểm tra xem có file save không
			std::ifstream f("savegame.json");
			bool hasSave = f.good();
			f.close();

			// GÓI GỌN LOGIC TẠO GAME MỚI VÀO LAMBDA ĐỂ TÁI SỬ DỤNG
			auto startNewGameLogic = [this]() {
				this->isTransitioning = true;
				auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, true);

				this->drawBuffer->PushCommand(transitionInCommand);
				this->commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this, transitionInCommand](std::function<void(void)> markFinished) {
					if (!transitionInCommand->IsFinished()) return;

					std::remove("savegame.json");
					gameSaveState = SaveGameState::StartNewGame(this->game, this->saveManager);
					this->isTransitioning = false;

					this->commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void(void)> markFinished1) {
						std::ifstream f2("savegame.json");
						if (f2.good()) this->continueButton->SetState(IButton::ButtonState::IDLE);
						else this->continueButton->SetState(IButton::ButtonState::DISABLED);
						f2.close();

						markFinished1();
						}));
					markFinished();
					}));
				this->drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
				};

			// QUYẾT ĐỊNH FLOW: CÓ SAVE THÌ POPUP, KHÔNG THÌ CHẠY LUÔN
			if (hasSave) {
				std::vector<std::pair<std::wstring, std::function<void()>>> popupBtns = {
					{ L"Yes", startNewGameLogic }, // Bấm Yes thì chạy logic
					{ L"No", []() {} }             // Bấm No thì bỏ qua, Popup tự tắt
				};
				PopupManager::GetInstance()->Show("basic_bluepurple", L"WARNING", L"Overwrite existing save?", popupBtns);
			}
			else {
				startNewGameLogic();
			}
			});
		newGameButton->SetSpriteScale(2.f, 2.f);

		//Options Button
		optionsButton = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 96, 32, buttonSheetTex, 3);
		optionsButton->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(192, 48, 48, 16, 3));
		optionsButton->SetOnReleaseLeft([this](DX9GF::ITrigger* t) {
			auto app = DX9GF::Application::GetInstance();
			//push Settings Scene
			auto sceMan = this->game->GetSceneManager();
			sceMan->InsertScene(sceMan->GetIndex() + 1,
				new SettingsScene(this->game, app->GetScreenWidth(), app->GetScreenHeight())
			);
			this->game->GetSceneManager()->GoToNext();
			});
		optionsButton->SetSpriteScale(2.f, 2.f);

		//Credits Button
		creditsButton = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 96, 32, buttonSheetTex, 3);
		creditsButton->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(192, 96, 48, 16, 3));
		creditsButton->SetOnReleaseLeft([this](DX9GF::ITrigger* t) {
			auto app = DX9GF::Application::GetInstance();
			auto sceMan = this->game->GetSceneManager();
			sceMan->InsertScene(sceMan->GetIndex() + 1,
				new CreditsScene(this->game, app->GetScreenWidth(), app->GetScreenHeight())
			);
			this->game->GetSceneManager()->GoToNext();
			});
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
				btn->Init(&uiCamera);
				uiButtons.push_back(btn);
			}
		}

		auto audio = DX9GF::AudioManager::GetInstance();

		audio->Load("bgm_sky", IDR_BGM_SKY);
		audio->PlayBGM_Fade("bgm_sky", 0.9f, 1.5f);

		//call it to setup the update layout
		UpdateLayout(lastScreenWidth, lastScreenHeight);
		transformManager->RebuildHierarchy();
	}

	void MainMenu::Update(unsigned long long deltaTime)
	{
		PopupManager::GetInstance()->SetUICamera(&this->uiCamera);

		auto inpMan = DX9GF::InputManager::GetInstance();
		inpMan->ReadMouse(deltaTime);
		inpMan->ReadKeyboard(deltaTime);

		auto [currW, currH] = camera.GetScreenResolution();
		UpdateLayout(currW, currH);

		if (!PopupManager::GetInstance()->IsActive()) {
			for (auto& button : uiButtons)
			{
				if (button->GetState() == IButton::ButtonState::DISABLED) continue;
				button->Update(deltaTime);
			}
		}		
		PopupManager::GetInstance()->Update(deltaTime, &this->uiCamera);
		transformManager->UpdateAll();
		camera.Update();
		commandBuffer->Update(deltaTime);
	}

	void MainMenu::DrawWorld(unsigned long long deltaTime)
	{
		auto gd = game->GetGraphicsDevice();

		if (SUCCEEDED(gd->BeginDraw())) {
			DrawBackground(deltaTime);
			gd->EndDraw();
		}
	}

	void MainMenu::DrawUI(unsigned long long deltaTime)
	{
		auto gd = game->GetGraphicsDevice();

		if (SUCCEEDED(gd->BeginDraw())) {

			fontSprite->Begin();
			fontSprite->SetScale(2.f, 2.f);
			auto prevPos = fontSprite->GetPosition();
			auto height = fontSprite->GetHeight() * 2.f;

			fontSprite->SetColor(0xFFFFFFFF);
			fontSprite->SetOutline(true, 0xFF000000, 2.0f);

			fontSprite->SetText(L"Toi, sinh vien nam 6 UIT,");
			fontSprite->Draw(uiCamera, deltaTime);

			fontSprite->SetPosition(prevPos.x, prevPos.y + height);
			fontSprite->SetText(L"bi hut vao cyberspace");
			fontSprite->Draw(uiCamera, deltaTime);

			fontSprite->SetPosition(prevPos.x, prevPos.y + height * 2);
			fontSprite->SetText(L"vi click vao link doc");
			fontSprite->Draw(uiCamera, deltaTime);

			fontSprite->SetPosition(prevPos.x, prevPos.y);
			fontSprite->End();

			for (auto& btn : uiButtons)
			{
				btn->Draw(gd, deltaTime);
			}

			drawBuffer->Update(deltaTime);
			PopupManager::GetInstance()->DrawUI(deltaTime, &this->uiCamera);
			DX9GF::InputManager::GetInstance()->DrawCursor(&this->uiCamera, deltaTime);
			gd->EndDraw();
		}
	}


}
