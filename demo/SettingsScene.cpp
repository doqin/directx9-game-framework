#include "pch.h"
#include "SettingsScene.h"
#include "resource.h"
#include "IconButton.h"
#include "SettingsManager.h"
#include <algorithm>
#include <cmath>
#include <dinput.h>
namespace Demo
{
	//Extra helpers, use to get keyname and string
	std::wstring ToWString(const std::string& s)
	{
		return std::wstring(s.begin(), s.end());
	}
	std::string GetKeyName(int dikCode)
	{
		if (dikCode <= 0 || dikCode > 255) return "None";
		char name[64];

		LONG lParam = (dikCode & 0x7F) << 16;

		if (dikCode == DIK_UP || dikCode == DIK_DOWN || dikCode == DIK_LEFT || dikCode == DIK_RIGHT ||
			dikCode == DIK_INSERT || dikCode == DIK_DELETE || dikCode == DIK_HOME || dikCode == DIK_END ||
			dikCode == DIK_PRIOR || dikCode == DIK_NEXT || dikCode == DIK_DIVIDE || dikCode == DIK_RALT ||
			dikCode == DIK_RMENU) {
			lParam |= 0x01000000;
		}

		if (GetKeyNameTextA(lParam, name, 64)) return std::string(name);
		return "Key " + std::to_string(dikCode);
	}

	//Draw functions for scene (to avoid code duplication)
	void SettingsScene::DrawString(std::wstring text, float x, float y, D3DCOLOR color, DWORD format)
	{
		int screenX = (int)(lastScreenWidth / 2.0f + x);
		int screenY = (int)(lastScreenHeight / 2.0f + y);
		fontSprite->Begin();
		fontSprite->SetColor(color);
		fontSprite->SetPosition(x, y);
		fontSprite->SetText(std::move(text));
		fontSprite->Draw(camera, 0); // deltaTime is not really used for anything :P
		fontSprite->End();
	}

	void SettingsScene::DrawBackground(unsigned long long deltaTime)
	{
		const D3DCOLOR polyColor = 0xFF9cdb43;
		auto [screenWidth, screenHeight] = camera.GetScreenResolution();

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

	void SettingsScene::DrawVolumeTrack(std::shared_ptr<DX9GF::StaticSprite> bg, std::shared_ptr<DX9GF::StaticSprite> fill, float vol, RECT originalRect, unsigned long long deltaTime)
	{
		if (bg)
		{
			bg->Begin();
			bg->Draw(camera, deltaTime);
			bg->End();
		}
		if (fill && bg)
		{
			RECT r = originalRect;
			float fullWidth = (float)(originalRect.right - originalRect.left);
			r.right = r.left + (LONG)(fullWidth * vol);
			fill->SetSrcRect(r);
			fill->SetScale(SLIDER_SCALE_X, 1.0f);
			fill->Begin(); fill->Draw(camera, deltaTime); fill->End();
		}
	}

	void SettingsScene::DrawKeybindButton(const std::string& action, std::shared_ptr<IconButton> btn, bool listening)
	{
		auto sm = SettingsManager::GetInstance();
		std::wstring text = listening ? L"..." : ToWString(GetKeyName(sm->GetKeybind(action)));
		D3DCOLOR color = listening ? D3DCOLOR_XRGB(5, 5, 5) : D3DCOLOR_XRGB(0, 0, 0);

		fontSprite->SetText(std::move(text));
		auto textWidth = fontSprite->GetWidth();
		auto textHeight = fontSprite->GetHeight();
		auto btnW = btn->GetWidth();
		auto btnH = btn->GetHeight();
		auto btnX = btn->GetLocalX();
		auto btnY = btn->GetLocalY();
		auto x = btnX + btnW / 2.f - textWidth / 2.f;
		auto y = btnY + btnH / 2.f - textHeight / 1.25f;

		fontSprite->Begin();
		fontSprite->SetPosition(x, y);
		fontSprite->SetColor(color);

		fontSprite->Draw(camera, 0);
		fontSprite->End();
	}

	//Update functions for component
	void SettingsScene::ResetListening()
	{
		isListeningUp = isListeningDown = isListeningLeft = isListeningRight = false;
		if (btnUp) btnUp->SetState(Demo::IButton::ButtonState::IDLE);
		if (btnDown) btnDown->SetState(Demo::IButton::ButtonState::IDLE);
		if (btnLeft) btnLeft->SetState(Demo::IButton::ButtonState::IDLE);
		if (btnRight) btnRight->SetState(Demo::IButton::ButtonState::IDLE);
	}

	void SettingsScene::UpdateLayout(int screenW, int screenH)
	{
		//Background
		float bgImageW = (float)bgTex->GetWidth();
		float bgImageH = (float)bgTex->GetHeight();
		if (!bgSprite)
		{
			bgSprite = std::make_shared<DX9GF::StaticSprite>(bgTex.get());
			bgSprite->SetSrcRect({ 0, 0, (LONG)bgImageW, (LONG)bgImageH });
		}
		bgSprite->SetScale(std::max(screenW / bgImageW, screenH / bgImageH));
		bgSprite->SetOrigin(bgImageW / 2.0f, bgImageH / 2.0f);
		bgSprite->SetPosition(0, 0);

		//UI Elements
		float startY = -screenH / 2.f + 64.f;

		//LOCAL FUNCTION to set Row Slider positions to keep the code clean
		auto SetVolumeRowPosition = [&](std::shared_ptr<DX9GF::StaticSprite> track, std::shared_ptr<DX9GF::StaticSprite> fill,
			std::shared_ptr<IconButton> btnD, std::shared_ptr<IconButton> btnI, float y)
			{
				float gap = 5.0f; //gap between track and inc,desc buttons
				float btnWidth = 12.0f;

				if (track)
				{
					track->SetPosition(SLIDER_COLUMN_X, y + ALIGN_OFFSET_Y);
					track->SetScale(SLIDER_SCALE_X, 1.0f);
				}

				if (fill) fill->SetPosition(SLIDER_COLUMN_X, y + ALIGN_OFFSET_Y);

				if (btnD) btnD->SetLocalPosition(SLIDER_COLUMN_X - btnWidth - gap, y + ALIGN_OFFSET_Y - 2.0f);

				if (btnI) btnI->SetLocalPosition(SLIDER_COLUMN_X + SLIDER_DESIRED_WIDTH + gap, y + ALIGN_OFFSET_Y - 2.0f);
			};

		SetVolumeRowPosition(trackMaster, trackMasterFill, btnMasterDec, btnMasterInc, startY);
		SetVolumeRowPosition(trackMusic, trackMusicFill, btnMusicDec, btnMusicInc, startY + SPACING_Y);
		SetVolumeRowPosition(trackSFX, trackSFXFill, btnSFXDec, btnSFXInc, startY + SPACING_Y * 2);

		backButton->SetLocalPosition(-screenW / 2.0f + 32.f, -screenH / 2.0f + 32.f);

		btnUp->SetLocalPosition(SLIDER_COLUMN_X, startY + SPACING_Y * 3.5f - 3.0f);
		btnDown->SetLocalPosition(SLIDER_COLUMN_X, startY + SPACING_Y * 5.0f - 3.0f);
		btnLeft->SetLocalPosition(SLIDER_COLUMN_X, startY + SPACING_Y * 6.5f - 3.0f);
		btnRight->SetLocalPosition(SLIDER_COLUMN_X, startY + SPACING_Y * 8.0f - 3.0f);
	}

	void SettingsScene::Init()
	{
		transformManager = std::make_shared<DX9GF::TransformManager>();
		auto app = DX9GF::Application::GetInstance();
		lastScreenWidth = app->GetScreenWidth();
		lastScreenHeight = app->GetScreenHeight();

		//Load assets
		font = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 16);
		fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
		fontSprite->SetColor(0xFF000000);

		placeholderTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		placeholderTex->LoadTexture(L"ui-pack.png");
		uiSheetTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		uiSheetTex->LoadTexture(L"ui.png");
		bgTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		bgTex->LoadTexture(IDB_PNG2);

		//disable custom cursor to use the Windows default system cursor.
		//DX9GF::InputManager::GetInstance()->EnableCustomCursor(false);

		//to hide the cursor during cutscenes, enable the custom cursor but do not call the draw function.
		//DX9GF::InputManager::GetInstance()->EnableCustomCursor(true);

		//LOCAL FUNCTION to init track and trackfill
		auto InitTrack = [&](std::shared_ptr<DX9GF::StaticSprite>& track, std::shared_ptr<DX9GF::StaticSprite>& fill, RECT trackR, RECT fillR) {
			track = std::make_shared<DX9GF::StaticSprite>(placeholderTex.get()); track->SetSrcRect(trackR);
			fill = std::make_shared<DX9GF::StaticSprite>(placeholderTex.get()); fill->SetSrcRect(fillR);
			};

		InitTrack(trackMaster, trackMasterFill, { 113, 483, 160, 490 }, { 65, 483, 112, 490 });
		InitTrack(trackMusic, trackMusicFill, { 113, 499, 160, 506 }, { 65, 499, 112, 506 });
		InitTrack(trackSFX, trackSFXFill, { 113, 515, 160, 522 }, { 65, 515, 112, 522 });

		//LOCAL FUNCTION to init decs,inc buttons
		auto CreateVolBtn = [&](int srcX, int srcY, const std::function<void(DX9GF::ITrigger*)>& action) {
			auto btn = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 12, 11, placeholderTex, 1);
			btn->SetSpriteCoords(srcX, srcY, 12, 11, 0);

			btn->SetOnReleaseLeft([action](DX9GF::ITrigger* t)
				{
					action(t);
					SettingsManager::GetInstance()->SaveSettings();
				});
			return btn;
			};

		auto sm = SettingsManager::GetInstance();
		btnMasterDec = CreateVolBtn(82, 82, [](DX9GF::ITrigger*) { SettingsManager::GetInstance()->SetMasterVolume(std::max(0.0f, SettingsManager::GetInstance()->GetMasterVolume() - 0.2f)); });
		btnMasterInc = CreateVolBtn(82, 71, [](DX9GF::ITrigger*) { SettingsManager::GetInstance()->SetMasterVolume(std::min(1.0f, SettingsManager::GetInstance()->GetMasterVolume() + 0.2f)); });

		btnMusicDec = CreateVolBtn(82, 82, [](DX9GF::ITrigger*) { SettingsManager::GetInstance()->SetMusicVolume(std::max(0.0f, SettingsManager::GetInstance()->GetMusicVolume() - 0.2f)); });
		btnMusicInc = CreateVolBtn(82, 71, [](DX9GF::ITrigger*) { SettingsManager::GetInstance()->SetMusicVolume(std::min(1.0f, SettingsManager::GetInstance()->GetMusicVolume() + 0.2f)); });

		btnSFXDec = CreateVolBtn(82, 82, [](DX9GF::ITrigger*) { SettingsManager::GetInstance()->SetSfxVolume(std::max(0.0f, SettingsManager::GetInstance()->GetSfxVolume() - 0.2f)); });
		btnSFXInc = CreateVolBtn(82, 71, [](DX9GF::ITrigger*) { SettingsManager::GetInstance()->SetSfxVolume(std::min(1.0f, SettingsManager::GetInstance()->GetSfxVolume() + 0.2f)); });

		backButton = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 96, 32, uiSheetTex, 3);
		backButton->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(96, 48, 48, 16, 3));
		backButton->SetOnReleaseLeft([this](DX9GF::ITrigger*) { this->isGoingBack = true; });
		backButton->SetSpriteScale(2.f, 2.f);

		//Use the same placeholder image for all control buttons for now.
		btnUp = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 32, 32, uiSheetTex, 3);
		btnUp->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(0, 0, 16, 16, 3));
		btnUp->SetOnReleaseLeft([this](DX9GF::ITrigger*) {
			this->ResetListening();
			this->isListeningUp = true;
			btnUp->SetState(Demo::IButton::ButtonState::LISTENING);
			});
		btnUp->SetSpriteScale(2.f, 2.f);

		btnDown = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 32, 32, uiSheetTex, 3);
		btnDown->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(0, 0, 16, 16, 3));
		btnDown->SetOnReleaseLeft([this](DX9GF::ITrigger*) {
			this->ResetListening();
			this->isListeningDown = true;
			btnDown->SetState(Demo::IButton::ButtonState::LISTENING);
			});
		btnDown->SetSpriteScale(2.f, 2.f);

		btnLeft = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 32, 32, uiSheetTex, 3);
		btnLeft->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(0, 0, 16, 16, 3));
		btnLeft->SetOnReleaseLeft([this](DX9GF::ITrigger*) {
			this->ResetListening();
			this->isListeningLeft = true;
			btnLeft->SetState(Demo::IButton::ButtonState::LISTENING);
			});
		btnLeft->SetSpriteScale(2.f, 2.f);

		btnRight = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 32, 32, uiSheetTex, 3);
		btnRight->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(0, 0, 16, 16, 3));
		btnRight->SetOnReleaseLeft([this](DX9GF::ITrigger*) {
			this->ResetListening();
			this->isListeningRight = true;
			btnRight->SetState(Demo::IButton::ButtonState::LISTENING);
			});
		btnRight->SetSpriteScale(2.f, 2.f);

		// Active Buttons
		std::shared_ptr<Demo::IButton> buttons[] = { backButton, btnUp, btnDown, btnLeft,btnRight, btnMasterDec, btnMasterInc, btnMusicDec, btnMusicInc, btnSFXDec, btnSFXInc };
		for (auto& btn : buttons)
		{
			if (btn)
			{
				btn->Init(&camera);
				uiButtons.push_back(btn);
			}
		}

		UpdateLayout(lastScreenWidth, lastScreenHeight);
		transformManager->RebuildHierarchy();
	}

	void SettingsScene::Update(unsigned long long deltaTime)
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

		transformManager->UpdateAll();
		camera.Update();

		//LOCAL FUNCTION to handle key presses
		auto HandleKeybind = [&](bool& isListeningFlag, const std::string& actionName, std::shared_ptr<IconButton> btn)
			{
				if (!isListeningFlag) return;

				for (int i = 1; i < 256; i++)
				{
					if (i == DIK_ESCAPE) continue;

					if (inpMan->KeyPress(i))
					{
						SettingsManager::GetInstance()->SetKeybind(actionName, i);
						btn->SetState(IButton::ButtonState::IDLE);
						isListeningFlag = false;
						SettingsManager::GetInstance()->SaveSettings();

						inpMan->ConsumeKey(i);
						break;
					}
				}
			};

		HandleKeybind(isListeningUp, "MOVE_UP", btnUp);
		HandleKeybind(isListeningDown, "MOVE_DOWN", btnDown);
		HandleKeybind(isListeningLeft, "MOVE_LEFT", btnLeft);
		HandleKeybind(isListeningRight, "MOVE_RIGHT", btnRight);

		if (this->isGoingBack)
		{
			auto sm = this->game->GetSceneManager();
			sm->GoToPrevious();
			sm->PopScene();
			return;
		}
	}

	void SettingsScene::Draw(unsigned long long deltaTime)
	{
		auto gd = game->GetGraphicsDevice();
		gd->Clear(0xFF000000);
		if (SUCCEEDED(gd->BeginDraw())) {
			DrawBackground(deltaTime);

			gd->SetAlphaBlending(true);
			gd->DrawRectangle(camera, -lastScreenWidth / 2.f, -lastScreenHeight / 2.f, lastScreenWidth, lastScreenHeight, D3DCOLOR_ARGB(200, 0, 0, 0), true);
			gd->SetAlphaBlending(false);

			float startY = -lastScreenHeight / 2.f + 64.f;

			//Draw label
			DrawString(L"Master Volume", LABEL_COLUMN_X, startY, 0xFFFFFFFF);
			DrawString(L"Music Volume", LABEL_COLUMN_X, startY + SPACING_Y, 0xFFFFFFFF);
			DrawString(L"Sfx Volume", LABEL_COLUMN_X, startY + SPACING_Y * 2, 0xFFFFFFFF);
			DrawString(L"Move up", LABEL_COLUMN_X, startY + SPACING_Y * 3.5f, 0xFFFFFFFF);
			DrawString(L"Move down", LABEL_COLUMN_X, startY + SPACING_Y * 5.0f, 0xFFFFFFFF);
			DrawString(L"Move left", LABEL_COLUMN_X, startY + SPACING_Y * 6.5f, 0xFFFFFFFF);
			DrawString(L"Move right", LABEL_COLUMN_X, startY + SPACING_Y * 8.0f, 0xFFFFFFFF);

			//draw tracks
			auto sm = SettingsManager::GetInstance();
			DrawVolumeTrack(trackMaster, trackMasterFill, sm->GetMasterVolume(), { 65, 483, 112, 490 }, deltaTime);
			DrawVolumeTrack(trackMusic, trackMusicFill, sm->GetMusicVolume(), { 65, 499, 112, 506 }, deltaTime);
			DrawVolumeTrack(trackSFX, trackSFXFill, sm->GetSfxVolume(), { 65, 515, 112, 522 }, deltaTime);

			//draw buttons
			for (auto& btn : uiButtons) btn->Draw(gd, deltaTime);

			//draw text onto keybind
			DrawKeybindButton("MOVE_UP", btnUp, isListeningUp);
			DrawKeybindButton("MOVE_DOWN", btnDown, isListeningDown);
			DrawKeybindButton("MOVE_LEFT", btnLeft, isListeningLeft);
			DrawKeybindButton("MOVE_RIGHT", btnRight, isListeningRight);

			//remember to call drawcursor if want to use custom cursor
			DX9GF::InputManager::GetInstance()->DrawCursor(&this->camera, deltaTime);

			gd->EndDraw();
		}
		gd->Present();
	}
}