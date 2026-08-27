#include "pch.h"
#include "PackOpeningScene.h"
#include "ICard.h"
#include "RNG.h"
#include "SettingsManager.h"
#include <DX9GFInputManager.h>
#include "DX9GFAudioManager.h"
#include "DX9GFUtils.h"
#include <algorithm>
#include <cmath>

namespace Demo {

	namespace {
		constexpr float TWO_PI = 6.2831853f;
		constexpr float CARD_SCALE = 4.f;
		constexpr float PACK_SCALE = 4.f;
		constexpr float FADE_MS = 260.f;
		constexpr int   BURST_COUNT = 48;

		BYTE ChR(D3DCOLOR c) { return (c >> 16) & 0xFF; }
		BYTE ChG(D3DCOLOR c) { return (c >> 8) & 0xFF; }
		BYTE ChB(D3DCOLOR c) { return c & 0xFF; }
	}

	PackOpeningScene::PackOpeningScene(Game* game, int screenWidth, int screenHeight, std::vector<std::string> cardIds)
		: IScene(screenWidth, screenHeight), game(game), cardIds(std::move(cardIds))
	{
	}

	void PackOpeningScene::Init()
	{
		auto gd = game->GetGraphicsDevice();
		transformManager = std::make_shared<DX9GF::TransformManager>();

		packTex = std::make_shared<DX9GF::Texture>(gd);
		packTex->LoadTexture(L"assets/packopening-Sheet.png");
		// 992x64 sheet -> 31 frames, 32x64 each. Non-looping: it plays once and we watch for the end.
		packSprite = std::make_shared<DX9GF::AnimatedSprite>(
			packTex.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 32, 64, 31), 18, false);
		packSprite->SetOrigin(16.f, 32.f);
		packSprite->SetPosition(0.f, 0.f);
		packSprite->SetScale(PACK_SCALE, PACK_SCALE);

		uiTex = std::make_shared<DX9GF::Texture>(gd);
		uiTex->LoadTexture(L"assets/ui.png");
		cardFaceSprite = std::make_shared<DX9GF::StaticSprite>(uiTex.get());

		font = std::make_shared<DX9GF::Font>(gd, L"StatusPlz", 24);
		fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());

		particleTex = std::make_shared<DX9GF::Texture>(gd);
		particleTex->CreatePlainTexture(0xFFFFFFFF, 4, 4);
		explosionEmitter = std::make_unique<DX9GF::ParticleSystem>(particleTex.get(), 80);
		explosionEmitter->SetOrigin(2.f, 2.f);
		DX9GF::ConfigureExplosionEmitter(*explosionEmitter);

		for (const auto& id : cardIds) {
			auto card = ICard::CreateCard(id, transformManager);
			faceRects.push_back(card ? card->GetFaceRect() : RECT{ 0, 0, 0, 0 });
			rarities.push_back(CardCatalog::GetRarity(id));
		}

		transformManager->RebuildHierarchy();
	}

	void PackOpeningScene::Update(unsigned long long deltaTime)
	{
		auto inpMan = DX9GF::InputManager::GetInstance();
		inpMan->ReadMouse(deltaTime);
		inpMan->ReadKeyboard(deltaTime);
		uiCamera.Update();

		// A left click, the ACCEPT key or the INTERACT key all step the reveal forward.
		const int keyAccept = SettingsManager::GetInstance()->GetKeybind("ACCEPT");
		const int keyInteract = SettingsManager::GetInstance()->GetKeybind("INTERACT");
		bool advance = false;
		if (inpMan->MouseDown(DX9GF::InputManager::MouseButton::Left)) {
			inpMan->ConsumeMouseButton(DX9GF::InputManager::MouseButton::Left);
			advance = true;
		}
		else if (inpMan->KeyDown(keyAccept)) {
			inpMan->ConsumeKey(keyAccept);
			advance = true;
		}
		else if (inpMan->KeyDown(keyInteract)) {
			inpMan->ConsumeKey(keyInteract);
			advance = true;
		}

		const float cx = 0.f, cy = 0.f;

		if (state == State::Opening) {
			if (packSprite->IsFinished() && !burstFired) {
				burstFired = true;
				for (int i = 0; i < BURST_COUNT; ++i) {
					float ang = RNG::Range(0.f, TWO_PI);
					float speed = RNG::Range(120.f, 380.f);
					explosionEmitter->Spawn(cx, cy, ang,
						RNG::Range(0.8f, 1.7f), RNG::Range(0.8f, 1.7f), 0xFFFFFFFF,
						std::cos(ang) * speed, std::sin(ang) * speed);
				}
				DX9GF::AudioManager::GetInstance()->Play("shop_buy", false, 0.7f);
				holdTimer = 320.f;
				state = State::Revealing;
				revealIndex = 0;
				cardAlpha = 0.f;
				pendingCardSound = true;
			}
		}
		else if (state == State::Revealing) {
			if (holdTimer > 0.f) {
				holdTimer -= (float)deltaTime;
			}
			else {
				if (pendingCardSound) {
					DX9GF::AudioManager::GetInstance()->PlayRandom("card_draw", 0.5f);
					pendingCardSound = false;
				}
				cardAlpha = (std::min)(1.f, cardAlpha + (float)deltaTime / FADE_MS);
				if (advance) {
					if (cardAlpha >= 1.f) {
						++revealIndex;
						if (revealIndex >= (int)cardIds.size()) {
							state = State::Done;
						}
						else {
							cardAlpha = 0.f;
							pendingCardSound = true;
						}
					}
					else {
						cardAlpha = 1.f; // snap the current card in on an early input
					}
				}
			}
		}
		else { // Done
			if (advance) {
				shouldLeave = true;
			}
		}

		// Age/fade the burst every frame (active=false: never auto-emit).
		explosionEmitter->Update(deltaTime, cx, cy, 0.f, 1.f, 1.f, 0xFFFFFFFF, false);

		if (shouldLeave) {
			auto sceMan = game->GetSceneManager();
			sceMan->RemoveScene(sceMan->GetIndex());
			sceMan->GoToPrevious();
		}
	}

	void PackOpeningScene::DrawWorld(unsigned long long deltaTime)
	{
		// The scene underneath still renders its own world.
	}

	void PackOpeningScene::DrawUI(unsigned long long deltaTime)
	{
		auto gd = game->GetGraphicsDevice();
		const float vw = (float)game->GetVirtualWidth();
		const float vh = (float)game->GetVirtualHeight();

		if (FAILED(gd->BeginDraw())) return;

		gd->SetAlphaBlending(true);
		gd->DrawRectangle(uiCamera, -vw / 2.f, -vh / 2.f, vw, vh, D3DCOLOR_ARGB(210, 0, 0, 0), true);

		if (state == State::Opening) {
			packSprite->SetPosition(0.f, 0.f);
			packSprite->SetScale(PACK_SCALE, PACK_SCALE);
			packSprite->Begin();
			packSprite->Draw(uiCamera, deltaTime);
			packSprite->End();
		}

		explosionEmitter->Draw(uiCamera, deltaTime);

		if (state == State::Revealing || state == State::Done) {
			const int idx = (std::min)(revealIndex, (int)cardIds.size() - 1);
			const RECT& r = faceRects[idx];
			const float fw = (float)(r.right - r.left);
			const float fh = (float)(r.bottom - r.top);
			const BYTE a = (BYTE)((std::clamp)(cardAlpha, 0.f, 1.f) * 255.f);
			const D3DCOLOR rc = CardCatalog::RarityColor(rarities[idx]);

			if (fw > 0.f && fh > 0.f) {
				const float gw = fw * CARD_SCALE + 48.f;
				const float gh = fh * CARD_SCALE + 48.f;
				gd->DrawRectangle(uiCamera, -gw / 2.f, -gh / 2.f - 10.f, gw, gh,
					D3DCOLOR_ARGB((BYTE)(a * 0.45f), ChR(rc), ChG(rc), ChB(rc)), true);

				cardFaceSprite->SetSrcRect(r);
				cardFaceSprite->SetOrigin(fw / 2.f, fh / 2.f);
				cardFaceSprite->SetScale(CARD_SCALE, CARD_SCALE);
				cardFaceSprite->SetPosition(0.f, -10.f);
				cardFaceSprite->SetColor(D3DCOLOR_ARGB(a, 255, 255, 255));
				cardFaceSprite->Begin();
				cardFaceSprite->Draw(uiCamera, deltaTime);
				cardFaceSprite->End();
			}

			fontSprite->Begin();

			fontSprite->SetScale(1.2f, 1.2f);
			fontSprite->SetOutline(true, 0xFF000000, 2.f);
			fontSprite->SetColor(D3DCOLOR_ARGB(a, ChR(rc), ChG(rc), ChB(rc)));
			fontSprite->SetText(std::wstring(CardCatalog::RarityName(rarities[idx])));
			const float labelW = fontSprite->GetWidth() * 1.2f;
			fontSprite->SetPosition(-labelW / 2.f, fh * CARD_SCALE / 2.f + 8.f);
			fontSprite->Draw(uiCamera, deltaTime);

			fontSprite->SetScale(0.9f, 0.9f);
			fontSprite->SetOutline(true, 0xFF000000, 2.f);
			fontSprite->SetColor(0xFFFFFFFF);
			const std::wstring hint = state == State::Done
				? std::wstring(L"Click or press a key to close")
				: L"Card " + std::to_wstring(revealIndex + 1) + L" / " + std::to_wstring(cardIds.size())
				+ L"   -   click or press a key for next";
			fontSprite->SetText(hint);
			const float hintW = fontSprite->GetWidth() * 0.9f;
			fontSprite->SetPosition(-hintW / 2.f, vh / 2.f - 60.f);
			fontSprite->Draw(uiCamera, deltaTime);

			fontSprite->End();
		}

		gd->SetAlphaBlending(false);
		gd->EndDraw();
	}
}
