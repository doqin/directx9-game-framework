#include "pch.h"
#include "CreditsScene.h"
#include "resource.h"
#include "DX9GFInputManager.h"
#include <algorithm>
#include <dinput.h>
#include "DX9GFIScene.h"

namespace Demo
{
	void CreditsScene::UpdateLayout(int screenW, int screenH)
	{
		//BACKGROUND - use aspect fill
		//float bgImageW = (float)bgTex->GetWidth();
		//float bgImageH = (float)bgTex->GetHeight();

		//if (!bgSprite)
		//{
		//	bgSprite = std::make_shared<DX9GF::StaticSprite>(bgTex.get());
		//	bgSprite->SetSrcRect({ 0, 0, (LONG)bgImageW, (LONG)bgImageH });
		//}

		//float bgScaleX = screenW / bgImageW;
		//float bgScaleY = screenH / bgImageH;
		//float bgFinalScale = std::max(bgScaleX, bgScaleY);

		//bgSprite->SetScale(bgFinalScale);
		//bgSprite->SetOrigin(bgImageW / 2.0f, bgImageH / 2.0f);
		//bgSprite->SetPosition(0, 0);

		backButton->SetLocalPosition(-screenW / 2.0f + 32.f, -screenH / 2.0f + 32.f);
		float bottomY = screenH / 2.0f - 48.f;
		btnPrevPage->SetLocalPosition(-160.f - 18.f, bottomY);
		btnNextPage->SetLocalPosition(160.f - 18.f, bottomY);
	}

	void CreditsScene::DrawBackground(unsigned long long deltaTime)
	{
		const D3DCOLOR polyColor = 0xFF9cdb43;
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

		}
	}

	void CreditsScene::DrawOverlay(unsigned long long deltaTime)
	{
		auto gd = game->GetGraphicsDevice();
		DrawBackground(deltaTime);

		gd->SetAlphaBlending(true);
		gd->DrawRectangle(camera, -lastScreenWidth / 2.f, -lastScreenHeight / 2.f, lastScreenWidth, lastScreenHeight, D3DCOLOR_ARGB(200, 0, 0, 0), true);
		gd->SetAlphaBlending(false);
	}

	void CreditsScene::DrawCreditsText(unsigned long long deltaTime)
	{
		if (creditsPages.empty() || currentPage < 0 || currentPage >= creditsPages.size()) return;

		fontSprite->Begin();
		fontSprite->SetOutline(true, 0xFF000000, 2.f);

		float currentY = -lastScreenHeight * 0.42f; //start y
		const auto& lines = creditsPages[currentPage];

		for (size_t i = 0; i < lines.size(); i++)
		{
			if (lines[i].empty()) {
				currentY += 20.f;
				continue;
			}

			bool isTitle = (i == 0);
			float scale = isTitle ? 2.2f : 1.8f; //type font scale

			fontSprite->SetScale(scale, scale);
			fontSprite->SetText(std::wstring(lines[i]));

			float textWidth = fontSprite->GetWidth() * scale;
			float textHeight = fontSprite->GetHeight() * scale;

			float xPos = -textWidth / 2.0f;

			if (isTitle) fontSprite->SetColor(0xFFfce74c);
			else fontSprite->SetColor(0xFFFFFFFF);

			fontSprite->SetPosition(xPos, currentY);
			fontSprite->Draw(camera, deltaTime);

			float padding = isTitle ? 32.f : 16.f;
			currentY += textHeight + padding;
		}
		fontSprite->End();
	}

	void CreditsScene::DrawPagination(unsigned long long deltaTime)
	{
		fontSprite->Begin();
		fontSprite->SetScale(1.5f, 1.5f);
		std::wstring pageText = L"Page " + std::to_wstring(currentPage + 1) + L" / " + std::to_wstring(creditsPages.size());
		fontSprite->SetText(std::move(pageText));

		float pWidth = fontSprite->GetWidth() * 1.5f;
		float pHeight = fontSprite->GetHeight() * 1.5f;
		float bottomY = lastScreenHeight / 2.0f - 48.f;

		fontSprite->SetPosition(-pWidth / 2.f, bottomY - pHeight / 2.f + 16.f);
		fontSprite->Draw(camera, deltaTime);
		fontSprite->End();
	}

	void CreditsScene::Init()
	{
		transformManager = std::make_shared<DX9GF::TransformManager>();
		auto app = DX9GF::Application::GetInstance();
		lastScreenWidth = app->GetScreenWidth();
		lastScreenHeight = app->GetScreenHeight();

		// Load Assets
		font = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 16);
		fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
		fontSprite->SetColor(0xFFFFFFFF);

		//bgTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		//bgTex->LoadTexture(IDB_PNG2);

		uiSheetTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		uiSheetTex->LoadTexture(L"ui.png");

		placeholderTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		placeholderTex->LoadTexture(L"ui-pack.png");

		backButton = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 96, 32, uiSheetTex, 3);
		backButton->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(96, 48, 48, 16, 3));
		backButton->SetOnReleaseLeft([this](DX9GF::ITrigger*) { this->isGoingBack = true; });
		backButton->SetSpriteScale(2.f, 2.f);

		auto CreatePageBtn = [&](int srcX, int srcY, const std::function<void(DX9GF::ITrigger*)>& action) {

			auto btn = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 36, 33, placeholderTex, 1);

			btn->SetSpriteCoords(srcX, srcY, 12, 11, 0);
			btn->SetSpriteScale(3.f, 3.f);
			btn->SetOnReleaseLeft(action);
			return btn;
			};

		btnPrevPage = CreatePageBtn(82, 82, [this](DX9GF::ITrigger*) {
			if (currentPage > 0) currentPage--;
			});

		btnNextPage = CreatePageBtn(82, 71, [this](DX9GF::ITrigger*) {
			if (currentPage < creditsPages.size() - 1) currentPage++;
			});

		std::shared_ptr<Demo::IButton> buttons[] = { backButton, btnPrevPage, btnNextPage };
		for (auto& btn : buttons)
		{
			if (btn)
			{
				btn->Init(&camera);
				uiButtons.push_back(btn);
			}
		}

		//check and change this if theres misconception
		creditsPages.push_back({
			L"-- GAME DESIGN & CONCEPT --",
			L"",
			L"24520763 - Khang Tuan Nguyen",
			L"",
			L"23521334 - Quynh Pham Truc Ngoc",
			L"",
			L"25520200 - Chi Tran To Viet",
			L"",
			L"24521166 - Ngoc Tran Thi Hoai",
			L"",
			L"24520743 - Khang Le Nguyen Huu"
			});

		creditsPages.push_back({
			L"-- PROGRAMMING & ENGINEERING --",
			L"",
			L"DX9GF Custom Engine",
			L"C++ / DirectX 9",
			L"",
			L"Lead Programmer: Khang Tuan Nguyen",
			L"",
			L"Gameplay Mechanics:",
			L"Quynh Pham Truc Ngoc",
			L"Chi Tran To Viet",
			L"Ngoc Tran Thi Hoai",
			L"Khang Le Nguyen Huu"
			});
		creditsPages.push_back({
			L"-- ART & VISUAL RESOURCES --",
			L"",
			L"Tilesets & Animations: Khang Tuan Nguyen",
			L"",
			L"UI & Icons: Khang Tuan Nguyen",
			L"",
			L"Character Sprites: Khang Tuan Nguyen",
			L"",
			L"Enemy Sprites: Chi Tran To Viet",
			L"",
			L"Fonts: StatusPlz, Arcade Among 2"
			});
		creditsPages.push_back({
			L"-- AUDIO RESOURCES --",
			L"Inventory Open: LittleRobotSoundFactory",
			L"- https://freesound.org/s/270393/",
			L"Close Bag: by TriqyStudio",
			L"- https://freesound.org/s/467605/",
			L"Character Footsteps: Nebula Audio",
			L"- https://nebula-audio.itch.io",
			L"Sounds: JDSherbert",
			L"– https://jdsherbert.itch.io",
			L"Card Deck: Geoff-Bremner-Audio",
			L"- https://freesound.org/s/682449/"
			});
		creditsPages.push_back({
			L"-- AUDIO RESOURCES --",
			L"Checkpoint: by Vicces1212",
			L"- https://freesound.org/s/123751/",
			L"Arcade Music Loop: by joshuaempyre",
			L"- https://freesound.org/s/251461/",
			L"Not Jam Music Pack 1 + 2 - https://not-jam.itch.io/",
			L"Battle: BloodPixelHero",
			L"- https://freesound.org/s/724507/",
			L"Battle: levelclearer",
			L"- https://freesound.org/s/346200/",
			L"",
			L"",
			L"Licensed under CC BY 4.0 & Public Domain"
			});

		creditsPages.push_back({
			L"-- SPECIAL THANKS --",
			L"",
			L"Supervisor: Hai Vu Tuan",
			L"",
			L"My eyes becuz it didn't give up at 2A.M", //just a joke, delete this shjet if we need to
			L"",
			L"You guys, for playing this game!"
			});
		UpdateLayout(lastScreenWidth, lastScreenHeight);
		transformManager->RebuildHierarchy();
	}

	void CreditsScene::Update(unsigned long long deltaTime)
	{
		auto inpMan = DX9GF::InputManager::GetInstance();
		inpMan->ReadMouse(deltaTime);
		inpMan->ReadKeyboard(deltaTime);

		auto app = DX9GF::Application::GetInstance();
		if (app->GetScreenWidth() != lastScreenWidth || app->GetScreenHeight() != lastScreenHeight)
		{
			lastScreenWidth = app->GetScreenWidth();
			lastScreenHeight = app->GetScreenHeight();
			UpdateLayout(lastScreenWidth, lastScreenHeight);
		}

		for (auto& button : uiButtons) button->Update(deltaTime);

		//keyboard navigation for page switching
		if (inpMan->KeyDown(DIK_UP) || inpMan->KeyDown(DIK_W) || inpMan->KeyDown(DIK_LEFT) || inpMan->KeyDown(DIK_A)) {
			if (currentPage > 0) currentPage--;
		}
		if (inpMan->KeyDown(DIK_DOWN) || inpMan->KeyDown(DIK_S) || inpMan->KeyDown(DIK_RIGHT) || inpMan->KeyDown(DIK_D)) {
			if (currentPage < creditsPages.size() - 1) currentPage++;
		}

		transformManager->UpdateAll();
		camera.Update();

		if (this->isGoingBack)
		{
			auto sm = this->game->GetSceneManager();
			sm->GoToPrevious();
			sm->PopScene();
			return;
		}
	}

	void CreditsScene::Draw(unsigned long long deltaTime)
	{
		auto gd = game->GetGraphicsDevice();
		gd->Clear(0xFF000000);

		if (SUCCEEDED(gd->BeginDraw())) {
			DrawOverlay(deltaTime);

			for (auto& btn : uiButtons) btn->Draw(gd, deltaTime);

			DrawCreditsText(deltaTime);
			DrawPagination(deltaTime);

			DX9GF::InputManager::GetInstance()->DrawCursor(&this->camera, deltaTime);
			gd->EndDraw();
		}
		gd->Present();
	}
}