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
		fontSprite->Draw(uiCamera, 0);
		fontSprite->End();
	}

	void SettingsScene::DrawBackground(unsigned long long deltaTime)
	{
		auto [screenWidth, screenHeight] = camera.GetScreenResolution();

		game->GetGraphicsDevice()->DrawRectangle(0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0xFF000000, true);

		const D3DCOLOR polyColor = 0xFF9cdb43;

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

	void SettingsScene::DrawVolumeTrack(std::shared_ptr<DX9GF::NineSliceSprite> bg, std::shared_ptr<DX9GF::NineSliceSprite> fill, float vol, RECT originalRect, unsigned long long deltaTime)
	{
		if (bg)
		{
			bg->Begin();
			bg->Draw(uiCamera, deltaTime);
			bg->End();
		}
		//Only draw the fill bar if volume > 0 to prevent visual glitches
		if (fill && bg && vol > 0.001f)
		{
			float fillWidth = SLIDER_DESIRED_WIDTH * vol;
			fill->SetTargetSize(fillWidth, 7.0f);

			fill->Begin();
			fill->Draw(uiCamera, deltaTime);
			fill->End();
		}
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
		float rowSpacing = SPACING_Y * 1.5f;

		//LOCAL FUNCTION to set Row Slider positions to keep the code clean
		auto SetVolumeRowPosition = [&](std::shared_ptr<DX9GF::NineSliceSprite> track, std::shared_ptr<DX9GF::NineSliceSprite> fill,
			std::shared_ptr<IconButton> btnD, std::shared_ptr<IconButton> btnI, float y)
			{
				float gap = 5.0f;
				float btnWidth = 12.0f;
				float trackAlignY = y + 5.0f;

				if (track)
				{
					track->SetPosition(SLIDER_COLUMN_X, trackAlignY);
					track->SetTargetSize(SLIDER_DESIRED_WIDTH, 7.0f);
				}

				if (fill) fill->SetPosition(SLIDER_COLUMN_X, trackAlignY);

				if (btnD) btnD->SetLocalPosition(SLIDER_COLUMN_X - btnWidth - gap, trackAlignY + (7.0f - btnD->GetHeight()) / 2.0f);
				if (btnI) btnI->SetLocalPosition(SLIDER_COLUMN_X + SLIDER_DESIRED_WIDTH + gap, trackAlignY + (7.0f - btnI->GetHeight()) / 2.0f);
			};

		SetVolumeRowPosition(trackMaster, trackMasterFill, btnMasterDec, btnMasterInc, startY);
		SetVolumeRowPosition(trackMusic, trackMusicFill, btnMusicDec, btnMusicInc, startY + rowSpacing);
		SetVolumeRowPosition(trackSFX, trackSFXFill, btnSFXDec, btnSFXInc, startY + rowSpacing * 2);

		backButton->SetLocalPosition(-screenW / 2.0f + 32.f, -screenH / 2.0f + 32.f);

		float resRowY = startY + rowSpacing * 3.5f;
		float fineTuneY = -6.0f;

		btnResPrev->SetLocalPosition(SLIDER_COLUMN_X, resRowY + fineTuneY);
		btnResNext->SetLocalPosition(SLIDER_COLUMN_X + 180.f, resRowY + fineTuneY);

		btnUp->SetLocalPosition(SLIDER_COLUMN_X, startY + rowSpacing * 5.0f + fineTuneY);
		btnDown->SetLocalPosition(SLIDER_COLUMN_X, startY + rowSpacing * 6.5f + fineTuneY);
		btnLeft->SetLocalPosition(SLIDER_COLUMN_X, startY + rowSpacing * 8.0f + fineTuneY);
		btnRight->SetLocalPosition(SLIDER_COLUMN_X, startY + rowSpacing * 9.5f + fineTuneY);
	}

	void SettingsScene::Init()
	{
		transformManager = std::make_shared<DX9GF::TransformManager>();
		auto [camW, camH] = camera.GetScreenResolution();
		lastScreenWidth = camW;
		lastScreenHeight = camH;

		//Load assets
		font = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 16);
		fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
		fontSprite->SetColor(0xFF000000);

		placeholderTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		placeholderTex->LoadTexture(L"assets/ui-pack.png");
		uiSheetTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		uiSheetTex->LoadTexture(L"assets/ui.png");
		bgTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		bgTex->LoadTexture(IDB_PNG2);

		//disable custom cursor to use the Windows default system cursor.
		//DX9GF::InputManager::GetInstance()->EnableCustomCursor(false);

		//to hide the cursor during cutscenes, enable the custom cursor but do not call the draw function.
		//DX9GF::InputManager::GetInstance()->EnableCustomCursor(true);

		//LOCAL FUNCTION to init track and trackfill
		auto InitTrack = [&](std::shared_ptr<DX9GF::NineSliceSprite>& track, std::shared_ptr<DX9GF::NineSliceSprite>& fill, RECT trackR, RECT fillR) {
			track = std::make_shared<DX9GF::NineSliceSprite>(placeholderTex.get(), trackR, 4, 2, 4, 2);
			fill = std::make_shared<DX9GF::NineSliceSprite>(placeholderTex.get(), fillR, 4, 2, 4, 2);
			};

		InitTrack(trackMaster, trackMasterFill, { 113, 483, 160, 490 }, { 65, 483, 112, 490 });
		InitTrack(trackMusic, trackMusicFill, { 113, 499, 160, 506 }, { 65, 499, 112, 506 });
		InitTrack(trackSFX, trackSFXFill, { 113, 515, 160, 522 }, { 65, 515, 112, 522 });

		//LOCAL FUNCTION to init decs,inc buttons
		auto CreateVolBtn = [&](int srcX, int srcY, const std::function<void(DX9GF::ITrigger*)>& action) {
			auto btn = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 12, 11, uiSheetTex);
			btn->SetSpriteCoords(srcX, srcY, 16, 16, 0);

			btn->SetOnReleaseLeft([action](DX9GF::ITrigger* t)
				{
					action(t);
					SettingsManager::GetInstance()->SaveSettings();
				});
			return btn;
			};

		auto sm = SettingsManager::GetInstance();
		btnMasterDec = CreateVolBtn(240, 96, [](DX9GF::ITrigger*) { SettingsManager::GetInstance()->SetMasterVolume(std::max(0.0f, SettingsManager::GetInstance()->GetMasterVolume() - 0.2f)); });
		btnMasterInc = CreateVolBtn(240, 112, [](DX9GF::ITrigger*) { SettingsManager::GetInstance()->SetMasterVolume(std::min(1.0f, SettingsManager::GetInstance()->GetMasterVolume() + 0.2f)); });

		btnMusicDec = CreateVolBtn(240, 96, [](DX9GF::ITrigger*) { SettingsManager::GetInstance()->SetMusicVolume(std::max(0.0f, SettingsManager::GetInstance()->GetMusicVolume() - 0.2f)); });
		btnMusicInc = CreateVolBtn(240, 112, [](DX9GF::ITrigger*) { SettingsManager::GetInstance()->SetMusicVolume(std::min(1.0f, SettingsManager::GetInstance()->GetMusicVolume() + 0.2f)); });

		btnSFXDec = CreateVolBtn(240, 96, [](DX9GF::ITrigger*) { SettingsManager::GetInstance()->SetSfxVolume(std::max(0.0f, SettingsManager::GetInstance()->GetSfxVolume() - 0.2f)); });
		btnSFXInc = CreateVolBtn(240, 112, [](DX9GF::ITrigger*) { SettingsManager::GetInstance()->SetSfxVolume(std::min(1.0f, SettingsManager::GetInstance()->GetSfxVolume() + 0.2f)); });

		backButton = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 96, 32, uiSheetTex, 3);
		backButton->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(96, 48, 48, 16, 3));
		backButton->SetOnReleaseLeft([this](DX9GF::ITrigger*) { this->isGoingBack = true; });
		backButton->SetSpriteScale(2.f, 2.f);

		btnResPrev = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 32, 32, uiSheetTex, 3);
		btnResPrev->SetSpriteCoords(240, 96, 16, 16, 0);
		btnResPrev->SetSpriteScale(2.f, 2.f);
		btnResPrev->SetSpriteOrigin(8.f, 8.f); //center of sprite
		btnResPrev->SetSpriteRotation(1.5708f);
		btnResPrev->SetSpriteOffset(16.f, 16.f);
		btnResPrev->SetOnReleaseLeft([this](DX9GF::ITrigger*) {
			auto sm = SettingsManager::GetInstance();
			int idx = sm->GetCurrentResolutionIndex();
			if (idx > 0) {
				sm->SetResolutionIndex(idx - 1);
				sm->SaveSettings();
				sm->ApplyResolution();
			}
			});
		btnResPrev->Init(&uiCamera);

		btnResNext = std::make_shared<Demo::IconButton>(transformManager, 0, 0, 32, 32, uiSheetTex, 3);
		btnResNext->SetSpriteCoords(240, 112, 16, 16, 0);
		btnResNext->SetSpriteScale(2.f, 2.f);
		btnResNext->SetSpriteOrigin(8.f, 8.f);
		btnResNext->SetSpriteRotation(1.5708f);
		btnResNext->SetSpriteOffset(16.f, 16.f);
		btnResNext->SetOnReleaseLeft([this](DX9GF::ITrigger*) {
			auto sm = SettingsManager::GetInstance();
			int idx = sm->GetCurrentResolutionIndex();
			if (idx < sm->GetSupportedResolutions().size() - 1) {
				sm->SetResolutionIndex(idx + 1);
				sm->SaveSettings();
				sm->ApplyResolution();
			}
			});
		btnResNext->Init(&uiCamera);

		auto SetupKeybindBtn = [&](std::shared_ptr<Demo::TextIconButton>& btn, const std::string& action, bool& listeningFlag) {
			btn = std::make_shared<Demo::TextIconButton>(transformManager, 0, 0, 32, 32, uiSheetTex, font.get(), L"", 3);
			btn->SetSpriteRects(DX9GF::Utils::CreateRectsVertical(0, 0, 16, 16, 3));
			btn->SetSpriteScale(2.f, 2.f);

			btn->SetTextScale(1.0f, 1.0f);
			btn->SetTextColor(D3DCOLOR_XRGB(0, 0, 0));

			Demo::TextIconButton* rawBtn = btn.get();

			btn->SetDynamicTextGetter([rawBtn, sm, action]() {
				if (rawBtn->GetState() == Demo::IButton::ButtonState::LISTENING) {
					return std::wstring(L"...");
				}
				return ToWString(GetKeyName(sm->GetKeybind(action)));
				});

			btn->SetOnReleaseLeft([this, rawBtn, &listeningFlag](DX9GF::ITrigger*) {
				this->ResetListening();
				listeningFlag = true;
				rawBtn->SetState(Demo::IButton::ButtonState::LISTENING);
				});
			};

		SetupKeybindBtn(btnUp, "MOVE_UP", isListeningUp);
		SetupKeybindBtn(btnDown, "MOVE_DOWN", isListeningDown);
		SetupKeybindBtn(btnLeft, "MOVE_LEFT", isListeningLeft);
		SetupKeybindBtn(btnRight, "MOVE_RIGHT", isListeningRight);

		// Active Buttons
		std::shared_ptr<Demo::IButton> buttons[] = { backButton, btnUp, btnDown, btnLeft, btnRight, btnMasterDec, btnMasterInc, btnMusicDec, btnMusicInc, btnSFXDec, btnSFXInc };
		for (auto& btn : buttons)
		{
			if (btn)
			{
				btn->Init(&uiCamera);
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

		for (auto& button : uiButtons) button->Update(deltaTime);
		int resIdx = SettingsManager::GetInstance()->GetCurrentResolutionIndex();
		int maxIdx = SettingsManager::GetInstance()->GetSupportedResolutions().size() - 1;
		if (resIdx > 0) btnResPrev->Update(deltaTime);
		if (resIdx < maxIdx) btnResNext->Update(deltaTime);
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
			sm->RemoveScene(sm->GetIndex()); //remove self from scene manager
			sm->GoToPrevious();
			return;
		}
	}

	void SettingsScene::DrawWorld(unsigned long long deltaTime)
	{
		auto gd = game->GetGraphicsDevice();
		if (SUCCEEDED(gd->BeginDraw())) {
			DrawBackground(deltaTime);
			gd->EndDraw();
		}
	}

	void SettingsScene::DrawUI(unsigned long long deltaTime)
	{
		auto gd = game->GetGraphicsDevice();
		if (SUCCEEDED(gd->BeginDraw())) {

			gd->SetAlphaBlending(true);
			gd->DrawRectangle(uiCamera, -lastScreenWidth / 2.f, -lastScreenHeight / 2.f, lastScreenWidth, lastScreenHeight, D3DCOLOR_ARGB(200, 0, 0, 0), true);
			gd->SetAlphaBlending(false);

			float startY = -lastScreenHeight / 2.f + 64.f;
			float rowSpacing = SPACING_Y * 1.5f;

			//Draw label
			DrawString(L"Master Volume", LABEL_COLUMN_X, startY, 0xFFFFFFFF);
			DrawString(L"Music Volume", LABEL_COLUMN_X, startY + rowSpacing, 0xFFFFFFFF);
			DrawString(L"Sfx Volume", LABEL_COLUMN_X, startY + rowSpacing * 2, 0xFFFFFFFF);
			DrawString(L"Resolution", LABEL_COLUMN_X, startY + rowSpacing * 3.5f, 0xFFFFFFFF);
			DrawString(L"Move up", LABEL_COLUMN_X, startY + rowSpacing * 5.0f, 0xFFFFFFFF);
			DrawString(L"Move down", LABEL_COLUMN_X, startY + rowSpacing * 6.5f, 0xFFFFFFFF);
			DrawString(L"Move left", LABEL_COLUMN_X, startY + rowSpacing * 8.0f, 0xFFFFFFFF);
			DrawString(L"Move right", LABEL_COLUMN_X, startY + rowSpacing * 9.5f, 0xFFFFFFFF);

			//draw tracks
			auto sm = SettingsManager::GetInstance();
			DrawVolumeTrack(trackMaster, trackMasterFill, sm->GetMasterVolume(), { 65, 483, 112, 490 }, deltaTime);
			DrawVolumeTrack(trackMusic, trackMusicFill, sm->GetMusicVolume(), { 65, 499, 112, 506 }, deltaTime);
			DrawVolumeTrack(trackSFX, trackSFXFill, sm->GetSfxVolume(), { 65, 515, 112, 522 }, deltaTime);

			//draw buttons
			for (auto& btn : uiButtons) btn->Draw(gd, deltaTime);

			int resIdx = sm->GetCurrentResolutionIndex();
			auto resList = sm->GetSupportedResolutions();
			float fineTuneY = -6.0f;
			float resRowY = startY + rowSpacing * 3.5f + fineTuneY;
			if (resIdx > 0)
				btnResPrev->Draw(gd, deltaTime);
			//draw resolution text
			std::string resStr = resList[resIdx].label;
			fontSprite->SetPosition(SLIDER_COLUMN_X + 45.f, resRowY + 8.f);
			fontSprite->SetColor(0xFFFFFFFF);
			fontSprite->SetText(ToWString(resStr));
			fontSprite->Begin(); fontSprite->Draw(uiCamera, 0); fontSprite->End();
			if (resIdx < resList.size() - 1)
				btnResNext->Draw(gd, deltaTime);
			DX9GF::InputManager::GetInstance()->DrawCursor(&this->uiCamera, deltaTime);
			gd->EndDraw();
		}
	}

}