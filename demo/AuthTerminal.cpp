#include "pch.h"
#include "SettingsManager.h"
#include "AuthTerminal.h"
#include <cmath>

namespace {
	constexpr RECT SRC_GREEN = { 32, 0, 48, 32 };
	constexpr RECT SRC_OK = { 80, 0, 96, 32 };
}

namespace Demo {
	AuthTerminal::AuthTerminal(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y)
		: IGameObject(tm, x, y), transformManager(tm) {
	}

	void AuthTerminal::Init(Game* game, DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, DX9GF::Camera* uiCamera,
		std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm,
		std::shared_ptr<DX9GF::Font> font)
	{
		this->gd = gd;
		this->worldCamera = camera;
		this->player = p;
		fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());

		collider = std::make_shared<DX9GF::RectangleCollider>(transformManager, 32.f, 32.f, GetWorldX(), GetWorldY());
		collider->SetOriginCenter();
		cm->Add(collider);

		spritesheet = std::make_shared<DX9GF::Texture>(gd);
		spritesheet->LoadTexture(L"assets/terminals.png");

		sprite = std::make_shared<DX9GF::StaticSprite>(spritesheet.get());
		sprite->SetSrcRect(isSolved ? SRC_OK : SRC_GREEN);
		sprite->SetOrigin(8.f, 16.f);
		sprite->SetPosition(GetWorldX(), GetWorldY());

		menu.Init(uiCamera, font, game);
		menu.SetOnSolved([this]() {
			SetSolved(true);
			if (onSolved) onSolved();
			});
	}

	void AuthTerminal::Update(unsigned long long deltaTime)
	{
		if (!isVisible) return;

		if (menu.IsOpen()) {
			menu.Update(deltaTime);
			return;
		}

		auto pLock = player.lock();
		if (!pLock) return;

		auto [px, py] = pLock->GetWorldPosition();
		auto [sx, sy] = GetWorldPosition();

		float distance = std::sqrt((px - sx) * (px - sx) + (py - sy) * (py - sy));
		isPlayerNear = (distance <= INTERACTION_DISTANCE);

		auto inpMan = DX9GF::InputManager::GetInstance();
		int interactKey = SettingsManager::GetInstance()->GetKeybind("INTERACT");

		if (isPlayerNear && !isSolved && inpMan->KeyDown(interactKey)) {
			inpMan->ConsumeKey(interactKey);
			menu.Open(password);
		}
	}

	void AuthTerminal::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime)
	{
		if (!isVisible) return;
		sprite->Begin();
		sprite->Draw(camera, deltaTime);
		sprite->End();

		DrawPosition(deltaTime, gd, camera);
	}

	void AuthTerminal::DrawUI(DX9GF::Camera* uiCamera, unsigned long long deltaTime)
	{
		if (!isVisible || !uiCamera || !fontSprite || !worldCamera) return;

		if (menu.IsOpen()) {
			menu.Draw(gd, deltaTime);
			return;
		}

		if (!isPlayerNear || isSolved) return;

		auto [worldX, worldY] = GetWorldPosition();
		float zoom = worldCamera->GetZoom();

		float uiX = (worldX - worldCamera->GetPosition().x) * zoom;
		float uiY = (worldY - worldCamera->GetPosition().y) * zoom;

		float scale = 1.0f * zoom;
		fontSprite->Begin();
		fontSprite->SetText(SettingsManager::GetInstance()->GetKeybindDisplayName("INTERACT"));
		fontSprite->SetScale(scale);
		fontSprite->SetColor(0xFFFFFFFF);

		float textW = fontSprite->GetWidth() * scale;
		float textH = fontSprite->GetHeight() * scale;
		fontSprite->SetPosition(uiX - textW / 2.f, uiY - 30.f * zoom - textH / 2.f);

		fontSprite->SetOutline(true, 0xFF000000);
		fontSprite->Draw(*uiCamera, deltaTime);
		fontSprite->End();
	}

	void AuthTerminal::SetSolved(bool solved)
	{
		isSolved = solved;
		// Init may not have run yet when a save is restored, so guard the sprite swap.
		if (sprite) sprite->SetSrcRect(isSolved ? SRC_OK : SRC_GREEN);
	}
}
