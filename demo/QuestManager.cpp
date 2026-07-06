#include "pch.h"
#include "QuestManager.h"

namespace {
	constexpr float MARGIN_X = 20.0f;
	constexpr float MARGIN_Y = 20.0f;
	constexpr float ARROW_SIZE = 28.0f;
	constexpr float PANEL_MIN_W = 120.0f;
	constexpr float PANEL_PADDING_X = 24.0f;
	constexpr float PANEL_H = 30.0f;
	constexpr D3DCOLOR PANEL_BG = 0xCC141410;
	constexpr D3DCOLOR TEXT_COLOR = 0xFFFFD700;
}

void Demo::QuestManager::Init(DX9GF::GraphicsDevice* gd, std::shared_ptr<DX9GF::TransformManager> tm, DX9GF::Camera* uiCamera, std::shared_ptr<DX9GF::Font> font)
{
	this->font = font;
	fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());

	if (!uiTex) {
		uiTex = std::make_shared<DX9GF::Texture>(gd);
		uiTex->LoadTexture(L"assets/ui.png");
	}

	auto [sw, sh] = uiCamera->GetScreenResolution();
	float panelX = -static_cast<float>(sw) / 2.0f + MARGIN_X;
	float panelY = -static_cast<float>(sh) / 2.0f + MARGIN_Y;

	btnToggle = std::make_shared<IconButton>(tm, panelX, panelY,
		static_cast<int>(ARROW_SIZE), static_cast<int>(ARROW_SIZE), uiTex, 3);
	btnToggle->SetSpriteCoords(0, 0, static_cast<int>(ARROW_SIZE), static_cast<int>(ARROW_SIZE), 0, false);
	btnToggle->SetOnReleaseLeft([this](DX9GF::ITrigger*) {
		isExpanded = !isExpanded;
		});
	btnToggle->Init(uiCamera);
}

void Demo::QuestManager::Update(unsigned long long deltaTime)
{
	if (!isVisible) return;
	if (btnToggle) btnToggle->Update(deltaTime);
}

void Demo::QuestManager::Draw(DX9GF::GraphicsDevice* gd, DX9GF::Camera* uiCamera, unsigned long long deltaTime)
{
	if (!isVisible) return;
	if (btnToggle) btnToggle->Draw(gd, deltaTime);
	if (!isExpanded || questText.empty()) return;

	auto [sw, sh] = uiCamera->GetScreenResolution();
	float panelX = -static_cast<float>(sw) / 2.0f + MARGIN_X;
	float panelY = -static_cast<float>(sh) / 2.0f + MARGIN_Y;
	float textX = panelX + ARROW_SIZE + 8.0f;

	fontSprite->Begin();
	fontSprite->SetOutline(false);
	fontSprite->SetColor(TEXT_COLOR);
	fontSprite->SetText(std::wstring(questText));

	float textWidth = fontSprite->GetWidth();
	float panelW = std::max(PANEL_MIN_W, textWidth + PANEL_PADDING_X);

	gd->SetAlphaBlending(true);
	gd->DrawRectangle(*uiCamera, textX, panelY, panelW, PANEL_H, PANEL_BG, true);
	gd->SetAlphaBlending(false);

	fontSprite->SetPosition(textX + 6.0f, panelY + (PANEL_H - fontSprite->GetHeight()) / 2.0f);
	fontSprite->Draw(*uiCamera, deltaTime);
	fontSprite->End();
}