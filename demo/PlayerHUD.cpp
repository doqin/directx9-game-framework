#include "pch.h"
#include "PlayerHUD.h"
#include "resource.h"

namespace {
	constexpr float PANEL_MIN_SIZE = 150.0f;
	constexpr float PANEL_PADDING = 18.0f;
	constexpr float PANEL_BOTTOM_MARGIN = 16.0f;

	// Border layers, outermost first
	constexpr float OUTLINE_T = 2.0f;
	constexpr float FRAME_T = 6.0f;
	constexpr float INNER_T = 1.0f;
	constexpr float CHECKER_SIZE = 16.0f;

	constexpr float ICON_SCALE = 2.0f;
	constexpr float ICON_SIZE = 32.0f * ICON_SCALE;

	// Boxed digit fields
	constexpr float CELL_W = 18.0f;
	constexpr float CELL_H = 24.0f;
	constexpr float CELL_BORDER = 2.0f;
	constexpr float ROW_H = CELL_H + 2.0f * CELL_BORDER;
	constexpr float ROW_GAP = 6.0f;
	constexpr float LABEL_GAP = 8.0f;
	constexpr float CONTENT_GAP = 10.0f;

	constexpr float BUTTON_SCALE = 1.5f;
	constexpr float BUTTON_W = 48.0f * BUTTON_SCALE;
	constexpr float BUTTON_H = 32.0f * BUTTON_SCALE;

	constexpr D3DCOLOR PANEL_OUTLINE = 0xFF262620;
	constexpr D3DCOLOR PANEL_FRAME = 0xFFd8d5a8;
	constexpr D3DCOLOR PANEL_BG = 0xFF7e7ec6;
	constexpr D3DCOLOR PANEL_BG_ALT = 0xFF8d8dd4;

	constexpr D3DCOLOR LABEL_COLOR = 0xFFFFFFFF;
	constexpr D3DCOLOR LABEL_OUTLINE = 0xFF54317c;
	constexpr D3DCOLOR CELL_FRAME = 0xFFd8d5a8;
	constexpr D3DCOLOR CELL_BG = 0xFF141410;
	constexpr D3DCOLOR DIGIT_COLOR = 0xFFf0eeda;
	constexpr D3DCOLOR DIGIT_COLOR_LOW = 0xFFff5a3c;
}

Demo::PlayerHUD::PlayerHUD(Game* game, std::shared_ptr<Player> player, std::shared_ptr<DX9GF::TransformManager> transformManager, DX9GF::Camera* uiCamera, DX9GF::Font* font)
	: game(game), player(player), transformManager(transformManager), uiCamera(uiCamera), font(font)
{
	fontSprite = std::make_shared<DX9GF::FontSprite>(font);
}

void Demo::PlayerHUD::Init()
{
	playerTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	playerTex->LoadTexture(IDB_PNG1);
	playerIcon = std::make_shared<DX9GF::StaticSprite>(playerTex.get());
	playerIcon->SetSrcRect({ .left = 0, .top = 0, .right = 32, .bottom = 32 });
	playerIcon->SetScale(ICON_SCALE);

	uiTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	uiTex->LoadTexture(L"assets/ui.png");

	btnInventory = std::make_shared<IconButton>(transformManager, 0.0f, 0.0f, static_cast<int>(BUTTON_W), static_cast<int>(BUTTON_H), uiTex);
	btnInventory->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(0, 144, 48, 32, 3));
	btnInventory->SetSpriteScale(BUTTON_SCALE, BUTTON_SCALE);
	btnInventory->SetOnReleaseLeft([this](DX9GF::ITrigger*) {
		if (onInventoryOpen) onInventoryOpen();
		});
	btnInventory->Init(uiCamera);

	UpdateLayout();
	btnInventory->SetLocalPosition(contentX + contentW - BUTTON_W, contentY + (statsRowH - BUTTON_H) / 2.0f);
}

void Demo::PlayerHUD::UpdateLayout()
{
	float sh = static_cast<float>(game->GetVirtualHeight());

	hpValue = std::to_wstring(static_cast<int>(player->GetHealth()));
	goldValue = std::to_wstring(player->GetGold());
	size_t maxHpDigits = std::to_wstring(static_cast<int>(player->GetMaxHealth())).size();

	cellCount = static_cast<int>((std::max)({ static_cast<size_t>(2), hpValue.size(), goldValue.size(), maxHpDigits }));

	fontSprite->SetText(L"HP");
	labelW = static_cast<float>(fontSprite->GetWidth());
	fontSprite->SetText(L"G");
	labelW = (std::max)(labelW, static_cast<float>(fontSprite->GetWidth()));

	groupW = cellCount * CELL_W + (cellCount + 1) * CELL_BORDER;

	statsRowH = (std::max)(ICON_SIZE, BUTTON_H);
	contentW = (std::max)(labelW + LABEL_GAP + groupW, ICON_SIZE + CONTENT_GAP + BUTTON_W);
	contentH = statsRowH + CONTENT_GAP + 2.0f * ROW_H + ROW_GAP;

	// Roughly a square, expands when the content gets too big
	panelW = (std::max)(PANEL_MIN_SIZE, contentW + 2.0f * PANEL_PADDING);
	panelH = (std::max)(PANEL_MIN_SIZE, contentH + 2.0f * PANEL_PADDING);
	panelX = -panelW / 2.0f;
	panelY = sh / 2.0f - PANEL_BOTTOM_MARGIN - panelH;

	contentX = panelX + (panelW - contentW) / 2.0f;
	contentY = panelY + (panelH - contentH) / 2.0f;
}

void Demo::PlayerHUD::Update(unsigned long long deltaTime)
{
	if (!visible) return;
	UpdateLayout();
	btnInventory->SetLocalPosition(contentX + contentW - BUTTON_W, contentY + (statsRowH - BUTTON_H) / 2.0f);
	btnInventory->Update(deltaTime);
}

void Demo::PlayerHUD::DrawPanel(DX9GF::GraphicsDevice* gd)
{
	// Dark outline -> cream frame -> dark inner line -> checkered interior
	gd->DrawRectangle(*uiCamera, panelX, panelY, panelW, panelH, PANEL_OUTLINE, true);
	gd->DrawRectangle(*uiCamera, panelX + OUTLINE_T, panelY + OUTLINE_T, panelW - 2.0f * OUTLINE_T, panelH - 2.0f * OUTLINE_T, PANEL_FRAME, true);

	float innerX = panelX + OUTLINE_T + FRAME_T;
	float innerY = panelY + OUTLINE_T + FRAME_T;
	float innerW = panelW - 2.0f * (OUTLINE_T + FRAME_T);
	float innerH = panelH - 2.0f * (OUTLINE_T + FRAME_T);
	gd->DrawRectangle(*uiCamera, innerX, innerY, innerW, innerH, PANEL_OUTLINE, true);

	float bgX = innerX + INNER_T;
	float bgY = innerY + INNER_T;
	float bgW = innerW - 2.0f * INNER_T;
	float bgH = innerH - 2.0f * INNER_T;
	gd->DrawRectangle(*uiCamera, bgX, bgY, bgW, bgH, PANEL_BG, true);

	int cols = static_cast<int>(std::ceil(bgW / CHECKER_SIZE));
	int rows = static_cast<int>(std::ceil(bgH / CHECKER_SIZE));
	for (int row = 0; row < rows; ++row) {
		for (int col = (row % 2 == 0) ? 1 : 0; col < cols; col += 2) {
			float x = bgX + col * CHECKER_SIZE;
			float y = bgY + row * CHECKER_SIZE;
			float w = (std::min)(CHECKER_SIZE, bgX + bgW - x);
			float h = (std::min)(CHECKER_SIZE, bgY + bgH - y);
			if (w > 0.0f && h > 0.0f) {
				gd->DrawRectangle(*uiCamera, x, y, w, h, PANEL_BG_ALT, true);
			}
		}
	}
}

void Demo::PlayerHUD::DrawStatRow(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime, float rowY, const wchar_t* label, const std::wstring& value, D3DCOLOR digitColor)
{
	float groupX = contentX + contentW - groupW;

	// Cream backing with one black field per digit, odometer style
	gd->DrawRectangle(*uiCamera, groupX, rowY, groupW, ROW_H, CELL_FRAME, true);
	gd->DrawRectangle(*uiCamera, groupX, rowY, groupW, ROW_H, PANEL_OUTLINE, false);
	for (int i = 0; i < cellCount; ++i) {
		float cellX = groupX + CELL_BORDER + i * (CELL_W + CELL_BORDER);
		gd->DrawRectangle(*uiCamera, cellX, rowY + CELL_BORDER, CELL_W, CELL_H, CELL_BG, true);
	}

	fontSprite->Begin();

	fontSprite->SetOutline(true, LABEL_OUTLINE, 2.0f);
	fontSprite->SetColor(LABEL_COLOR);
	fontSprite->SetText(std::wstring(label));
	fontSprite->SetPosition(contentX, rowY + (ROW_H - fontSprite->GetHeight()) / 2.0f);
	fontSprite->Draw(*uiCamera, deltaTime);
	fontSprite->SetOutline(false);

	// Digits fill the cells from the right, leading cells stay blank
	fontSprite->SetColor(digitColor);
	int blanks = cellCount - static_cast<int>(value.size());
	for (size_t j = 0; j < value.size(); ++j) {
		float cellX = groupX + CELL_BORDER + static_cast<float>(blanks + j) * (CELL_W + CELL_BORDER);
		fontSprite->SetText(std::wstring(1, value[j]));
		fontSprite->SetPosition(
			cellX + (CELL_W - fontSprite->GetWidth()) / 2.0f,
			rowY + CELL_BORDER + (CELL_H - fontSprite->GetHeight()) / 2.0f
		);
		fontSprite->Draw(*uiCamera, deltaTime);
	}

	fontSprite->End();
}

void Demo::PlayerHUD::Draw(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
{
	if (!visible) return;

	DrawPanel(gd);

	float hpRowY = contentY + statsRowH + CONTENT_GAP;
	float goldRowY = hpRowY + ROW_H + ROW_GAP;

	bool lowHealth = player->GetHealth() <= player->GetMaxHealth() * 0.25f;
	DrawStatRow(gd, deltaTime, hpRowY, L"HP", hpValue, lowHealth ? DIGIT_COLOR_LOW : DIGIT_COLOR);
	DrawStatRow(gd, deltaTime, goldRowY, L"G", goldValue, DIGIT_COLOR);

	playerIcon->SetPosition(contentX, contentY + (statsRowH - ICON_SIZE) / 2.0f);
	playerIcon->Begin();
	playerIcon->Draw(*uiCamera, deltaTime);
	playerIcon->End();

	btnInventory->Draw(gd, deltaTime);
}
