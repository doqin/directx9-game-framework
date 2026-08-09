#include "pch.h"
#include "IShopScene.h"
#include "IconButton.h"
#include "DX9GFUtils.h"
#include <algorithm>
#include <cmath>

namespace {
	// Panel geometry as a fraction of the virtual resolution, so the shop keeps its
	// proportions if the virtual resolution ever changes.
	constexpr float PANEL_W_RATIO = 0.84f;
	constexpr float PANEL_H_RATIO = 0.80f;
	constexpr float PANEL_PAD = 24.0f;
	// Tall enough that the 64px LEAVE button clears the divider drawn just above the list.
	constexpr float HEADER_H = 80.0f;
	constexpr float FOOTER_H = 104.0f;
	constexpr float ROW_H = 64.0f;
	constexpr float ROW_GAP = 10.0f;
	constexpr float ROW_INNER_PAD = 14.0f;
	// The footer stacks the description box on top of the page bar; these must sum to
	// no more than FOOTER_H or the arrows would sit on top of the description.
	constexpr float DESC_BOX_H = 56.0f;
	constexpr float PAGE_BAR_OFFSET = 64.0f;
	constexpr float PAGE_BTN_SIZE = 32.0f;
	// Scales only the nine-slice border margins, keeping the pixel-art frame crisp
	// while the centre stretches to the target size.
	constexpr float PANEL_BORDER_SCALE = 3.0f;
	constexpr float CARD_BORDER_SCALE = 2.0f;

	constexpr float BUY_BTN_SIZE = 64.0f;
	constexpr float ICON_SCALE = 1.4f;
	// Card faces are authored for 2x, same as IStatementCard::DrawSheetFace uses in battle.
	constexpr float CARD_FACE_SCALE = 2.0f;
	// Kept modest so the widest title ("--- PREMIUM CARD SHOP ---") still clears the
	// LEAVE button on the left and the gold readout on the right.
	constexpr float TITLE_SCALE = 1.25f;
	constexpr float GOLD_SCALE = 1.25f;

	constexpr D3DCOLOR COLOR_GOLD = 0xFFFFD700;
	constexpr D3DCOLOR COLOR_TEXT = 0xFFFFFFFF;
	constexpr D3DCOLOR COLOR_TEXT_DIM = 0xFF909090;
	constexpr D3DCOLOR COLOR_PRICE_BAD = 0xFFFF5555;
	constexpr D3DCOLOR COLOR_STATUS = 0xFF00FFFF;
	constexpr D3DCOLOR COLOR_DIVIDER = 0x66FFFFFF;
	constexpr D3DCOLOR COLOR_BTN_DISABLED = 0xFF707070;
	constexpr D3DCOLOR COLOR_FACE_DISABLED = 0xFF808080;

	// Source rects into assets/popup-borders.png, the sheet PopupManager registers its
	// styles against. Its tiles are saturated popup colours, so the ones used as dark
	// UI surfaces are the plain white tile modulated to this screen's palette --
	// NineSliceSprite multiplies by its colour, and the tiles' black borders stay black.
	constexpr RECT PANEL_FRAME_SRC = { 128, 128, 160, 160 }; // stepped_gold, used as trim
	constexpr RECT PANEL_FILL_SRC = { 0, 0, 32, 32 };        // basic_blackwhite, tinted
	constexpr RECT ROW_SRC = { 32, 0, 64, 32 };              // basic_blackgrey
	constexpr RECT ROW_HOVER_SRC = { 0, 0, 32, 32 };         // basic_blackwhite, tinted
	constexpr RECT DESC_SRC = { 0, 0, 32, 32 };              // basic_blackwhite, tinted

	constexpr D3DCOLOR TINT_PANEL_FILL = 0xFF232331;
	constexpr D3DCOLOR TINT_ROW_HOVER = 0xFF4E6FA8;
	constexpr D3DCOLOR TINT_DESC = 0xFF15151F;
	// How much of the gold frame stays visible around the panel fill.
	constexpr float PANEL_FRAME_INSET = 10.0f;

	struct ShopLayout {
		float panelX, panelY, panelW, panelH;
		float listTop, listBottom;
		float rowX, rowW;
		float footerTop;
	};

	ShopLayout ComputeLayout(float sw, float sh)
	{
		ShopLayout l{};
		l.panelW = sw * PANEL_W_RATIO;
		l.panelH = sh * PANEL_H_RATIO;
		l.panelX = -l.panelW / 2.0f;
		l.panelY = -l.panelH / 2.0f;
		l.listTop = l.panelY + HEADER_H;
		l.footerTop = l.panelY + l.panelH - FOOTER_H;
		l.listBottom = l.footerTop;
		l.rowX = l.panelX + PANEL_PAD;
		l.rowW = l.panelW - PANEL_PAD * 2.0f;
		return l;
	}

	int ComputeRowsPerPage(const ShopLayout& l)
	{
		const float usable = l.listBottom - l.listTop;
		const int rows = static_cast<int>(std::floor((usable + ROW_GAP) / (ROW_H + ROW_GAP)));
		return (std::max)(1, rows);
	}

	// Greedy space-split word wrap. Measures each candidate line with the caller's font
	// sprite, so it matches whatever scale that sprite is currently set to.
	std::vector<std::wstring> WrapText(DX9GF::FontSprite* fontSprite, const std::wstring& text,
		float maxWidth, size_t maxLines)
	{
		std::vector<std::wstring> lines;
		if (!fontSprite || text.empty() || maxLines == 0) return lines;

		std::wstring current;
		size_t pos = 0;
		while (pos <= text.size()) {
			const size_t space = text.find(L' ', pos);
			const std::wstring word = text.substr(pos, space == std::wstring::npos ? std::wstring::npos : space - pos);

			if (!word.empty()) {
				const std::wstring candidate = current.empty() ? word : current + L" " + word;
				fontSprite->SetText(std::wstring(candidate));
				if (static_cast<float>(fontSprite->GetWidth()) > maxWidth && !current.empty()) {
					lines.push_back(current);
					if (lines.size() == maxLines) {
						// Ran out of room: mark the last line as truncated.
						lines.back() += L"...";
						return lines;
					}
					current = word;
				}
				else {
					current = candidate;
				}
			}

			if (space == std::wstring::npos) break;
			pos = space + 1;
		}

		if (!current.empty() && lines.size() < maxLines) {
			lines.push_back(current);
		}
		return lines;
	}
}

Demo::IShopScene::IShopScene(Game* game, Player* player, int sw, int sh, std::string title)
	: IScene(sw, sh), game(game), player(player), shopTitle(title)
{
}

void Demo::IShopScene::Init()
{
	transformManager = std::make_shared<DX9GF::TransformManager>();
	myFont = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 24);
	myFontSprite = std::make_shared<DX9GF::FontSprite>(myFont.get());

	uiSheetTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	uiSheetTex->LoadTexture(L"assets/ui.png");

	borderTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	borderTex->LoadTexture(L"assets/popup-borders.png");

	itemsTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	itemsTex->LoadTexture(L"assets/items.png");

	LoadItems();

	BuildUI();

	transformManager->RebuildHierarchy();
}

void Demo::IShopScene::BuildUI()
{
	const float sw = static_cast<float>(game->GetVirtualWidth());
	const float sh = static_cast<float>(game->GetVirtualHeight());
	const ShopLayout l = ComputeLayout(sw, sh);

	// --- Panel and row chrome ---------------------------------------------------
	panelFrameSprite = std::make_shared<DX9GF::NineSliceSprite>(borderTex.get(), PANEL_FRAME_SRC, 6, 6, 6, 6);
	panelFrameSprite->SetScale(PANEL_BORDER_SCALE);
	panelFrameSprite->SetTargetSize(l.panelW, l.panelH);
	panelFrameSprite->SetPosition(l.panelX, l.panelY);

	panelSprite = std::make_shared<DX9GF::NineSliceSprite>(borderTex.get(), PANEL_FILL_SRC, 2, 2, 2, 2);
	panelSprite->SetScale(PANEL_BORDER_SCALE);
	panelSprite->SetColor(TINT_PANEL_FILL);
	panelSprite->SetTargetSize(l.panelW - PANEL_FRAME_INSET * 2.0f, l.panelH - PANEL_FRAME_INSET * 2.0f);
	panelSprite->SetPosition(l.panelX + PANEL_FRAME_INSET, l.panelY + PANEL_FRAME_INSET);

	rowSprite = std::make_shared<DX9GF::NineSliceSprite>(borderTex.get(), ROW_SRC, 2, 2, 2, 2);
	rowSprite->SetScale(CARD_BORDER_SCALE);
	rowSprite->SetTargetSize(l.rowW, ROW_H);

	rowHoverSprite = std::make_shared<DX9GF::NineSliceSprite>(borderTex.get(), ROW_HOVER_SRC, 2, 2, 2, 2);
	rowHoverSprite->SetScale(CARD_BORDER_SCALE);
	rowHoverSprite->SetColor(TINT_ROW_HOVER);
	rowHoverSprite->SetTargetSize(l.rowW, ROW_H);

	descSprite = std::make_shared<DX9GF::NineSliceSprite>(borderTex.get(), DESC_SRC, 2, 2, 2, 2);
	descSprite->SetScale(CARD_BORDER_SCALE);
	descSprite->SetColor(TINT_DESC);
	descSprite->SetTargetSize(l.rowW, DESC_BOX_H);

	itemIconSprite = std::make_shared<DX9GF::StaticSprite>(itemsTex.get());
	itemIconSprite->SetScale(ICON_SCALE, ICON_SCALE);

	// Card faces live on the UI sheet and are drawn at 2x in battle; matching that here
	// keeps them pixel-exact rather than resampled.
	cardFaceSprite = std::make_shared<DX9GF::StaticSprite>(uiSheetTex.get());
	cardFaceSprite->SetScale(CARD_FACE_SCALE, CARD_FACE_SCALE);

	goldIconSprite = std::make_shared<DX9GF::StaticSprite>(uiSheetTex.get());
	goldIconSprite->SetSrcRect({ 0, 240, 32, 272 });
	goldIconSprite->SetScale(1.0f, 1.0f);

	// --- One buy button per item, across every page -----------------------------
	// Positions are assigned by RefreshPage; only the current page's slice is ever
	// updated or drawn.
	for (size_t i = 0; i < itemsForSale.size(); ++i) {
		auto item = itemsForSale[i];

		auto buyBtn = std::make_shared<Demo::IconButton>(
			transformManager,
			0.0f, 0.0f, static_cast<int>(BUY_BTN_SIZE), static_cast<int>(BUY_BTN_SIZE),
			uiSheetTex, 3
		);
		buyBtn->SetSpriteCoords(0, 240, 32, 32, 0, false);
		buyBtn->SetSpriteScale(2, 2);
		buyBtn->SetOnReleaseLeft([this, item](DX9GF::ITrigger* t) {
			if (this->player->GetGold() >= item.cost) {
				this->player->AddGold(-item.cost);

				if (item.onBuyAction) {
					item.onBuyAction();
				}

				DX9GF::AudioManager::GetInstance()->Play("shop_buy", false, 0.8f);
				this->ShowMessage(L"Bought " + item.name + L"!");
			}
			else {
				DX9GF::AudioManager::GetInstance()->Play("error", false, 0.3f);
				this->ShowMessage(L"Not enough gold!");
			}
			});
		buyBtn->Init(&uiCamera);
		buyButtons.push_back(buyBtn);
	}

	// --- LEAVE ------------------------------------------------------------------
	leaveButton = std::make_shared<Demo::IconButton>(
		transformManager,
		l.panelX + PANEL_PAD, l.panelY + (HEADER_H - 64.0f) / 2.0f, 96, 64,
		uiSheetTex, 3
	);
	leaveButton->SetSpriteCoords(144, 240, 48, 32, 0, false);
	leaveButton->SetSpriteScale(2, 2);
	leaveButton->SetOnReleaseLeft([this](DX9GF::ITrigger* t) {
		this->shouldLeave = true;
		});
	leaveButton->Init(&uiCamera);

	// --- Page arrows ------------------------------------------------------------
	auto makePageButton = [&](const std::wstring& label, const std::function<void(DX9GF::ITrigger*)>& action) {
		auto btn = std::make_shared<Demo::TextIconButton>(
			transformManager, 0.0f, 0.0f, 32, 32, uiSheetTex, myFont.get(), label, 3);
		btn->SetSpriteRects({ { 0, 0, 16, 16 }, { 0, 16, 16, 32 }, { 0, 32, 16, 48 } });
		btn->SetSpriteScale(2.0f, 2.0f);
		btn->SetTextColor(0xFF111111);
		btn->SetOnReleaseLeft(action);
		btn->Init(&uiCamera);
		return btn;
		};

	btnPrevPage = makePageButton(L"<", [this](DX9GF::ITrigger*) {
		if (currentPage > 0) {
			currentPage--;
			RefreshPage();
		}
		});
	btnNextPage = makePageButton(L">", [this](DX9GF::ITrigger*) {
		if (currentPage < maxPage) {
			currentPage++;
			RefreshPage();
		}
		});

	RefreshPage();
}

void Demo::IShopScene::RefreshPage()
{
	const float sw = static_cast<float>(game->GetVirtualWidth());
	const float sh = static_cast<float>(game->GetVirtualHeight());
	const ShopLayout l = ComputeLayout(sw, sh);

	rowsPerPage = ComputeRowsPerPage(l);
	maxPage = itemsForSale.empty() ? 0 : static_cast<int>((itemsForSale.size() - 1) / rowsPerPage);
	currentPage = std::clamp(currentPage, 0, maxPage);

	const int startIndex = currentPage * rowsPerPage;
	const int endIndex = (std::min)(static_cast<int>(itemsForSale.size()), startIndex + rowsPerPage);

	// Clear any HOVER/CLICKED left over from the page we just navigated away from;
	// off-page buttons stop being updated, so they would otherwise stay highlighted.
	for (auto& btn : buyButtons) {
		btn->SetState(IButton::ButtonState::IDLE);
	}

	activeButtons.clear();
	activeButtons.push_back(leaveButton);

	const float buyX = l.rowX + l.rowW - ROW_INNER_PAD - BUY_BTN_SIZE;
	for (int i = startIndex; i < endIndex; ++i) {
		const float rowY = l.listTop + (i - startIndex) * (ROW_H + ROW_GAP);
		buyButtons[i]->SetLocalPosition(buyX, rowY + (ROW_H - BUY_BTN_SIZE) / 2.0f);
		activeButtons.push_back(buyButtons[i]);
	}

	if (maxPage > 0) {
		const float arrowY = l.footerTop + PAGE_BAR_OFFSET;
		btnPrevPage->SetLocalPosition(l.rowX + ROW_INNER_PAD, arrowY);
		btnNextPage->SetLocalPosition(l.rowX + l.rowW - ROW_INNER_PAD - PAGE_BTN_SIZE, arrowY);
		activeButtons.push_back(btnPrevPage);
		activeButtons.push_back(btnNextPage);
	}

	RefreshAffordability();

	transformManager->RebuildHierarchy();
	transformManager->UpdateAll();
}

void Demo::IShopScene::RefreshAffordability()
{
	const int startIndex = currentPage * rowsPerPage;
	const int endIndex = (std::min)(static_cast<int>(itemsForSale.size()), startIndex + rowsPerPage);
	const int gold = player->GetGold();

	for (int i = startIndex; i < endIndex; ++i) {
		auto& btn = buyButtons[i];
		const bool affordable = gold >= itemsForSale[i].cost;

		btn->SetSpriteColor(affordable ? 0xFFFFFFFF : COLOR_BTN_DISABLED);

		if (!affordable) {
			btn->SetState(IButton::ButtonState::DISABLED);
		}
		else if (btn->GetState() == IButton::ButtonState::DISABLED) {
			btn->SetState(IButton::ButtonState::IDLE);
		}
	}
}

void Demo::IShopScene::ShowMessage(std::wstring msg) {
	statusMessage = std::move(msg);
	messageTimer = 2.0f;
}

std::vector<Demo::KeyboardNavigator::Candidate> Demo::IShopScene::CollectKeyboardCandidates()
{
	std::vector<KeyboardNavigator::Candidate> candidates;
	for (auto& button : activeButtons) {
		if (!button || button->GetState() == IButton::ButtonState::DISABLED) {
			continue;
		}
		candidates.push_back({
			button,
			button->GetWorldX(),
			button->GetWorldY(),
			(float)button->GetWidth(),
			(float)button->GetHeight(),
			[button]() { button->Activate(); }
			});
	}
	return candidates;
}

void Demo::IShopScene::Update(unsigned long long deltaTime)
{
	auto inpMan = DX9GF::InputManager::GetInstance();
	inpMan->ReadMouse(deltaTime);
	inpMan->ReadKeyboard(deltaTime);
	uiCamera.Update();

	if (messageTimer > 0) {
		messageTimer -= deltaTime / 1000.0f;
		if (messageTimer <= 0) statusMessage.clear();
	}

	for (auto& btn : activeButtons) {
		btn->Update(deltaTime);
	}

	// Gold changes on purchase, so re-evaluate before the navigator collects candidates
	// (it must not be able to target a button that just became unaffordable).
	RefreshAffordability();

	keyboardNavigator.Update(deltaTime, CollectKeyboardCandidates());

	if (shouldLeave) {
		auto sceMan = this->game->GetSceneManager();

		sceMan->RemoveScene(sceMan->GetIndex());
		sceMan->GoToPrevious();
	}
}

void Demo::IShopScene::DrawWorld(unsigned long long deltaTime)
{
}

void Demo::IShopScene::DrawUI(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	const float sw = static_cast<float>(game->GetVirtualWidth());
	const float sh = static_cast<float>(game->GetVirtualHeight());
	const float leftEdge = -sw / 2.0f;
	const float topEdge = -sh / 2.0f;
	const ShopLayout l = ComputeLayout(sw, sh);

	const int startIndex = currentPage * rowsPerPage;
	const int endIndex = (std::min)(static_cast<int>(itemsForSale.size()), startIndex + rowsPerPage);

	// Which row the player is inspecting: the keyboard target's row in keyboard mode,
	// otherwise whichever row card the mouse is over. Hit-testing the whole card (rather
	// than reading the buy button's state) means unaffordable rows still show their
	// description even though their button is disabled.
	int hoveredIndex = -1;
	if (keyboardNavigator.IsInKeyboardMode()) {
		auto target = keyboardNavigator.GetTarget();
		for (int i = startIndex; i < endIndex && target; ++i) {
			if (buyButtons[i] == target) {
				hoveredIndex = i;
				break;
			}
		}
	}
	else {
		auto [mouseX, mouseY] = DX9GF::InputManager::GetInstance()->GetVirtualAbsoluteMousePos(&uiCamera);
		auto [worldMouseX, worldMouseY] = DX9GF::Utils::WindowToWorldCoords(uiCamera, mouseX, mouseY);
		for (int i = startIndex; i < endIndex; ++i) {
			const float rowY = l.listTop + (i - startIndex) * (ROW_H + ROW_GAP);
			if (worldMouseX >= l.rowX && worldMouseX <= l.rowX + l.rowW &&
				worldMouseY >= rowY && worldMouseY <= rowY + ROW_H) {
				hoveredIndex = i;
				break;
			}
		}
	}

	if (SUCCEEDED(gd->BeginDraw())) {
		gd->SetAlphaBlending(true);

		// Dim the world scene underneath.
		gd->DrawRectangle(uiCamera, leftEdge, topEdge, sw, sh, D3DCOLOR_ARGB(190, 0, 0, 0), true);

		gd->SetAlphaBlending(false);

		// --- Panel: gold trim, then the dark fill inset inside it -----------------
		panelFrameSprite->Begin();
		panelFrameSprite->Draw(uiCamera, deltaTime);
		panelFrameSprite->End();

		panelSprite->Begin();
		panelSprite->Draw(uiCamera, deltaTime);
		panelSprite->End();

		// --- Row cards -----------------------------------------------------------
		rowSprite->Begin();
		for (int i = startIndex; i < endIndex; ++i) {
			if (i == hoveredIndex) continue;
			rowSprite->SetPosition(l.rowX, l.listTop + (i - startIndex) * (ROW_H + ROW_GAP));
			rowSprite->Draw(uiCamera, deltaTime);
		}
		rowSprite->End();

		if (hoveredIndex >= 0) {
			rowHoverSprite->Begin();
			rowHoverSprite->SetPosition(l.rowX, l.listTop + (hoveredIndex - startIndex) * (ROW_H + ROW_GAP));
			rowHoverSprite->Draw(uiCamera, deltaTime);
			rowHoverSprite->End();
		}

		// --- Description box -----------------------------------------------------
		descSprite->Begin();
		descSprite->SetPosition(l.rowX, l.footerTop);
		descSprite->Draw(uiCamera, deltaTime);
		descSprite->End();

		// --- Row artwork ---------------------------------------------------------
		itemIconSprite->Begin();
		for (int i = startIndex; i < endIndex; ++i) {
			const auto& item = itemsForSale[i];
			if (item.iconSheet != ShopIconSheet::Items) continue;
			const float iconH = static_cast<float>(item.iconRect.bottom - item.iconRect.top);
			if (iconH <= 0.0f) continue;

			const float rowY = l.listTop + (i - startIndex) * (ROW_H + ROW_GAP);
			itemIconSprite->SetSrcRect(item.iconRect);
			itemIconSprite->SetPosition(
				l.rowX + ROW_INNER_PAD,
				rowY + (ROW_H - iconH * ICON_SCALE) / 2.0f);
			itemIconSprite->Draw(uiCamera, deltaTime);
		}
		itemIconSprite->End();

		cardFaceSprite->Begin();
		for (int i = startIndex; i < endIndex; ++i) {
			const auto& item = itemsForSale[i];
			if (item.iconSheet != ShopIconSheet::CardFaces) continue;
			const float faceH = static_cast<float>(item.iconRect.bottom - item.iconRect.top);
			if (faceH <= 0.0f) continue;

			const float rowY = l.listTop + (i - startIndex) * (ROW_H + ROW_GAP);
			// Unaffordable cards get the same grey wash as their buy button.
			cardFaceSprite->SetColor(player->GetGold() >= item.cost ? 0xFFFFFFFF : COLOR_FACE_DISABLED);
			cardFaceSprite->SetSrcRect(item.iconRect);
			cardFaceSprite->SetPosition(
				l.rowX + ROW_INNER_PAD,
				rowY + (ROW_H - faceH * CARD_FACE_SCALE) / 2.0f);
			cardFaceSprite->Draw(uiCamera, deltaTime);
		}
		cardFaceSprite->End();

		// --- Gold pouch icon in the header --------------------------------------
		myFontSprite->SetScale(GOLD_SCALE, GOLD_SCALE);
		myFontSprite->SetText(std::to_wstring(player->GetGold()) + L"G");
		const float goldW = static_cast<float>(myFontSprite->GetWidth()) * GOLD_SCALE;
		const float goldH = static_cast<float>(myFontSprite->GetHeight()) * GOLD_SCALE;
		const float goldX = l.panelX + l.panelW - PANEL_PAD - goldW;

		goldIconSprite->Begin();
		goldIconSprite->SetPosition(goldX - 40.0f, l.panelY + (HEADER_H - 32.0f) / 2.0f);
		goldIconSprite->Draw(uiCamera, deltaTime);
		goldIconSprite->End();

		// --- Buttons -------------------------------------------------------------
		for (auto& btn : activeButtons) {
			btn->Draw(gd, deltaTime);
		}

		// --- Text ----------------------------------------------------------------
		if (myFontSprite) {
			myFontSprite->Begin();

			// Gold amount (measured above, before the icon was placed).
			myFontSprite->SetScale(GOLD_SCALE, GOLD_SCALE);
			myFontSprite->SetColor(COLOR_GOLD);
			myFontSprite->SetPosition(goldX, l.panelY + (HEADER_H - goldH) / 2.0f);
			myFontSprite->SetText(std::to_wstring(player->GetGold()) + L"G");
			myFontSprite->Draw(uiCamera, deltaTime);

			// Title, centred on the panel.
			myFontSprite->SetScale(TITLE_SCALE, TITLE_SCALE);
			myFontSprite->SetColor(COLOR_TEXT);
			myFontSprite->SetText(std::wstring(shopTitle.begin(), shopTitle.end()));
			const float titleW = static_cast<float>(myFontSprite->GetWidth()) * TITLE_SCALE;
			const float titleH = static_cast<float>(myFontSprite->GetHeight()) * TITLE_SCALE;
			myFontSprite->SetPosition(-titleW * 0.5f, l.panelY + (HEADER_H - titleH) / 2.0f);
			myFontSprite->Draw(uiCamera, deltaTime);

			myFontSprite->SetScale(1.0f, 1.0f);

			// Header divider.
			gd->SetAlphaBlending(true);
			gd->DrawRectangle(uiCamera, l.rowX, l.listTop - 6.0f, l.rowW, 2.0f, COLOR_DIVIDER, true);
			gd->SetAlphaBlending(false);

			// Rows: name on the left (after the icon, when there is one), price
			// right-aligned just before the buy button.
			const int gold = player->GetGold();
			for (int i = startIndex; i < endIndex; ++i) {
				const auto& item = itemsForSale[i];
				const bool affordable = gold >= item.cost;
				const float rowY = l.listTop + (i - startIndex) * (ROW_H + ROW_GAP);

				// A card face already spells out the card's name and energy cost, so drawing
				// the name next to it would just repeat it; item icons are wordless and still
				// need the label.
				if (item.iconSheet != ShopIconSheet::CardFaces) {
					const float iconW = static_cast<float>(item.iconRect.right - item.iconRect.left);
					const float textX = l.rowX + ROW_INNER_PAD + (iconW > 0.0f ? iconW * ICON_SCALE + 12.0f : 0.0f);

					myFontSprite->SetColor(affordable ? COLOR_TEXT : COLOR_TEXT_DIM);
					myFontSprite->SetText(std::wstring(item.name));
					const float nameH = static_cast<float>(myFontSprite->GetHeight());
					myFontSprite->SetPosition(textX, rowY + (ROW_H - nameH) / 2.0f);
					myFontSprite->Draw(uiCamera, deltaTime);
				}

				myFontSprite->SetColor(affordable ? COLOR_GOLD : COLOR_PRICE_BAD);
				myFontSprite->SetText(std::to_wstring(item.cost) + L"G");
				const float priceW = static_cast<float>(myFontSprite->GetWidth());
				const float priceH = static_cast<float>(myFontSprite->GetHeight());
				myFontSprite->SetPosition(
					l.rowX + l.rowW - ROW_INNER_PAD - BUY_BTN_SIZE - 16.0f - priceW,
					rowY + (ROW_H - priceH) / 2.0f);
				myFontSprite->Draw(uiCamera, deltaTime);
			}

			// The footer box shows the purchase feedback while it is up, otherwise the
			// inspected row's description. Sharing the slot keeps either from having to
			// squeeze in next to the other.
			if (!statusMessage.empty()) {
				myFontSprite->SetColor(COLOR_STATUS);
				myFontSprite->SetText(std::wstring(statusMessage));
				const float msgH = static_cast<float>(myFontSprite->GetHeight());
				myFontSprite->SetPosition(l.rowX + ROW_INNER_PAD, l.footerTop + (DESC_BOX_H - msgH) / 2.0f);
				myFontSprite->Draw(uiCamera, deltaTime);
			}
			else if (hoveredIndex >= 0) {
				const auto lines = WrapText(myFontSprite.get(), itemsForSale[hoveredIndex].description,
					l.rowW - ROW_INNER_PAD * 2.0f, 2);
				myFontSprite->SetColor(COLOR_TEXT);
				float lineY = l.footerTop + 6.0f;
				for (const auto& line : lines) {
					myFontSprite->SetText(std::wstring(line));
					myFontSprite->SetPosition(l.rowX + ROW_INNER_PAD, lineY);
					myFontSprite->Draw(uiCamera, deltaTime);
					lineY += static_cast<float>(myFontSprite->GetHeight()) + 2.0f;
				}
			}

			// Page indicator, centred between the arrows.
			if (maxPage > 0) {
				myFontSprite->SetColor(COLOR_TEXT);
				myFontSprite->SetText(L"Page " + std::to_wstring(currentPage + 1) + L"/" + std::to_wstring(maxPage + 1));
				const float pageW = static_cast<float>(myFontSprite->GetWidth());
				const float pageH = static_cast<float>(myFontSprite->GetHeight());
				myFontSprite->SetPosition(-pageW * 0.5f, l.footerTop + PAGE_BAR_OFFSET + (PAGE_BTN_SIZE - pageH) / 2.0f);
				myFontSprite->Draw(uiCamera, deltaTime);
			}

			myFontSprite->End();
		}

		keyboardNavigator.Draw(gd, uiCamera, CollectKeyboardCandidates());
		if (!keyboardNavigator.IsInKeyboardMode()) {
			DX9GF::InputManager::GetInstance()->DrawCursor(&uiCamera, deltaTime);
		}
		gd->EndDraw();
	}
}
