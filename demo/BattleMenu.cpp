#include "pch.h"
#include "BattleMenu.h"
#include "SettingsScene.h"
#include "DX9GFAudioManager.h"

namespace Demo {

	BattleMenu::BattleMenu(Game* g, std::shared_ptr<DX9GF::TransformManager> tm, DX9GF::Camera* cam)
		: game(g), transformManager(tm), uiCamera(cam)
	{
	}

	void BattleMenu::Init()
	{
		float sw = static_cast<float>(game->GetVirtualWidth());
		float sh = static_cast<float>(game->GetVirtualHeight());

		float bottomGap = 20.0f;
		float buttonW = 48.0f * 2;
		float resumeX = -(buttonW + bottomGap + buttonW + bottomGap + buttonW) / 2.0f;
		float optionsX = resumeX + buttonW + bottomGap;
		float leaveX = optionsX + buttonW + bottomGap;

		uiTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		uiTex->LoadTexture(L"assets/ui.png");

		btnResume = std::make_shared<IconButton>(transformManager, resumeX, 0, 48.0f * 2, 32.0f * 2, uiTex);
		btnResume->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(144, 176, 48, 32, 3));
		btnResume->SetOnReleaseLeft([this](DX9GF::ITrigger* t) { this->Toggle(); });
		btnResume->SetSpriteScale(2.f, 2.f);
		btnResume->Init(uiCamera);

		btnOptions = std::make_shared<IconButton>(transformManager, optionsX, 0, 48.0f * 2, 32.0f * 2, uiTex);
		btnOptions->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(0, 208, 48, 32, 3));
		btnOptions->SetOnReleaseLeft([this, sw, sh](DX9GF::ITrigger* t) {
			auto sceMan = this->game->GetSceneManager();
			sceMan->InsertScene(sceMan->GetIndex() + 1, new SettingsScene(this->game, sw, sh));
			sceMan->GoToNext();
			});
		btnOptions->SetSpriteScale(2.f, 2.f);
		btnOptions->Init(uiCamera);

		btnLeaveGame = std::make_shared<IconButton>(transformManager, leaveX, 0, 48.0f * 2, 32.0f * 2, uiTex);
		btnLeaveGame->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(144, 208, 48, 32, 3));
		btnLeaveGame->SetOnReleaseLeft([this](DX9GF::ITrigger* t) {
			this->pendingLeave = true;
			});
		btnLeaveGame->SetSpriteScale(2.f, 2.f);
		btnLeaveGame->Init(uiCamera);
	}

	void BattleMenu::Toggle()
	{
		isOpen = !isOpen;
		if (isOpen) {
			DX9GF::AudioManager::GetInstance()->Play("open_inv");
		}
		else {
			DX9GF::AudioManager::GetInstance()->Play("close_inv");
		}
	}

	std::vector<KeyboardNavigator::Candidate> BattleMenu::CollectKeyboardCandidates()
	{
		std::vector<KeyboardNavigator::Candidate> candidates;

		auto addButton = [&](std::shared_ptr<IconButton> button) {
			if (!button || button->GetState() == IButton::ButtonState::DISABLED) {
				return;
			}
			candidates.push_back({
				button,
				button->GetWorldX(),
				button->GetWorldY(),
				(float)button->GetWidth(),
				(float)button->GetHeight(),
				[button]() { button->Activate(); }
				});
			};

		addButton(btnResume);
		addButton(btnOptions);
		addButton(btnLeaveGame);

		return candidates;
	}

	void BattleMenu::Update(unsigned long long deltaTime)
	{
		if (!isOpen) return;

		float sw = static_cast<float>(game->GetVirtualWidth());
		float sh = static_cast<float>(game->GetVirtualHeight());

		float bottomGap = 20.0f;
		float buttonW = 48.0f * 2;
		float resumeX = -(buttonW + bottomGap + buttonW + bottomGap + buttonW) / 2.0f;
		float optionsX = resumeX + buttonW + bottomGap;
		float leaveX = optionsX + buttonW + bottomGap;

		float centerY = 0.0f;
		btnResume->SetLocalPosition(resumeX, centerY);
		btnOptions->SetLocalPosition(optionsX, centerY);
		btnLeaveGame->SetLocalPosition(leaveX, centerY);

		btnResume->Update(deltaTime);
		btnOptions->Update(deltaTime);
		btnLeaveGame->Update(deltaTime);

		keyboardNavigator.Update(deltaTime, CollectKeyboardCandidates());
	}

	void BattleMenu::Draw(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
	{
		if (!isOpen) return;

		float sw = static_cast<float>(game->GetVirtualWidth());
		float sh = static_cast<float>(game->GetVirtualHeight());

		float leftEdge = -sw / 2.0f;
		float topEdge = -sh / 2.0f;

		gd->SetAlphaBlending(true);
		gd->DrawRectangle(*uiCamera, leftEdge, topEdge, sw, sh, D3DCOLOR_ARGB(165, 0, 0, 0), true);
		gd->SetAlphaBlending(false);

		btnResume->Draw(gd, deltaTime);
		btnOptions->Draw(gd, deltaTime);
		btnLeaveGame->Draw(gd, deltaTime);
	}

	void BattleMenu::DrawKeyboardReticle(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
	{
		if (!isOpen) return;
		keyboardNavigator.Draw(gd, *uiCamera, CollectKeyboardCandidates());
	}
}
