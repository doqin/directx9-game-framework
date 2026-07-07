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

void Demo::QuestManager::Init(DX9GF::GraphicsDevice* gd, std::shared_ptr<DX9GF::TransformManager> tm,
	DX9GF::Camera* uiCamera, std::shared_ptr<DX9GF::Font> font)
{
	if (!fontSprite || this->font.get() != font.get()) {
		this->font = font;
		fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
	}
	if (!uiTex) {
		uiTex = std::make_shared<DX9GF::Texture>(gd);
		uiTex->LoadTexture(L"assets/ui.png");
	}
	if (!uiTransformManager) {
		uiTransformManager = std::make_shared<DX9GF::TransformManager>();
	}

	if (!btnToggle) {
		btnToggle = std::make_shared<IconButton>(uiTransformManager, 0, 0,
			static_cast<int>(ARROW_SIZE), static_cast<int>(ARROW_SIZE), uiTex, 3);
		btnToggle->SetSpriteCoords(240, 96, 16, 16, 0);
		btnToggle->SetSpriteScale(ARROW_SIZE / 16.0f, ARROW_SIZE / 16.0f);
		btnToggle->SetOnReleaseLeft([this](DX9GF::ITrigger*) {
			isExpanded = !isExpanded;
			});
	}

	SetUICamera(uiCamera);
	uiTransformManager->RebuildHierarchy();
}
void Demo::QuestManager::Update(unsigned long long deltaTime)
{
	if (!isVisible) return;
	float panelX = -virtualWidth / 2.0f + MARGIN_X;
	float panelY = -virtualHeight / 2.0f + MARGIN_Y;
	if (btnToggle) {
		btnToggle->SetLocalPosition(panelX, panelY);
		btnToggle->Update(deltaTime);
	}
	if (uiTransformManager) {
		uiTransformManager->UpdateAll();
	}
}

void Demo::QuestManager::Draw(DX9GF::GraphicsDevice* gd, DX9GF::Camera* uiCamera, unsigned long long deltaTime)
{
	if (!isVisible) return;
	if (btnToggle) btnToggle->Draw(gd, deltaTime);
	if (!isExpanded || questText.empty()) return;

	float panelX = -virtualWidth / 2.0f + MARGIN_X;
	float panelY = -virtualHeight / 2.0f + MARGIN_Y;
	static int qmLogCounter = 0;
	if (qmLogCounter++ % 60 == 0) {
		char buf[256];
		sprintf_s(buf, "[QM DEBUG] virtualW=%.1f virtualH=%.1f panelX=%.1f panelY=%.1f\n",
			virtualWidth, virtualHeight, panelX, panelY);
		OutputDebugStringA(buf);
	}
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