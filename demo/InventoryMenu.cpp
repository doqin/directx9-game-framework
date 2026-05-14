#include "pch.h"
#include "InventoryMenu.h"
#include "SettingsScene.h"
namespace {
	constexpr float RAW_ITEM_W = 23.0f;
	constexpr float RAW_ITEM_H = 35.0f;
	constexpr float RAW_BG_W = 192.0f;
	constexpr float RAW_BG_H = 128.0f;

	constexpr float MENU_SCALE = 3.0f;

	constexpr float ITEM_W = RAW_ITEM_W * MENU_SCALE;
	constexpr float ITEM_H = RAW_ITEM_H * MENU_SCALE;
	constexpr float BG_W = RAW_BG_W * MENU_SCALE;
	constexpr float BG_H = RAW_BG_H * MENU_SCALE;

	constexpr float HALF_BG_W = BG_W / 2.0f;
	constexpr float HALF_BG_H = BG_H / 2.0f;

	constexpr float PADDING_X = 20.0f;
	constexpr float PADDING_Y = 30.0f;
}
namespace Demo {

	InventoryMenu::InventoryMenu(Game* g, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::TransformManager> tm, std::shared_ptr<DraggableManager> dm, DX9GF::Camera* cam, DX9GF::Font* f)
		: game(g), player(p), transformManager(tm), draggableManager(dm), uiCamera(cam), font(f)
	{
		fontSprite = std::make_shared<DX9GF::FontSprite>(f);
	}

	void InventoryMenu::Init()
	{
		auto [sw, sh] = uiCamera->GetScreenResolution();
		float centerX = 0.0f;
		float topY = -sh / 2.0f;
		float bottomY = sh / 2.0f;
		float tabY = topY + 40.0f;
		float tabGap = 24.0f;
		float bottomGap = 20.0f;
		float containerGap = 40.0f;
		float containerW = (sw - 120.0f - containerGap) / 2.0f;
		float containerY = topY + 110.0f;
		float leftContainerX = -containerGap / 2.0f - containerW;
		float rightContainerX = containerGap / 2.0f;
		float buttonW = 48.0f * 2;
		float resumeX = -(buttonW + bottomGap + buttonW + bottomGap + buttonW) / 2.0f;
		float optionsX = resumeX + buttonW + bottomGap;
		float leaveX = optionsX + buttonW + bottomGap;

		itemSheetTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		itemSheetTex->LoadTexture(L"items.png");

		uiTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		uiTex->LoadTexture(L"ui.png");

		btnTabItems = std::make_shared<IconButton>(transformManager, centerX - tabGap / 2.0f - 80.0f, tabY, 48.0f * 2, 32.0f * 2, uiTex);
		btnTabItems->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(0, 144, 48, 32, 3));
		btnTabItems->SetOnReleaseLeft([this](DX9GF::ITrigger* t) { this->SetTab(Tab::ITEMS); });
		btnTabItems->SetSpriteScale(2.f, 2.f);
		btnTabItems->Init(uiCamera);

		btnTabDeck = std::make_shared<IconButton>(transformManager, centerX + tabGap / 2.0f, tabY, 48.0f * 2, 32.0f * 2, uiTex);
		btnTabDeck->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(0, 176, 48, 32, 3));
		btnTabDeck->SetOnReleaseLeft([this](DX9GF::ITrigger* t) { this->SetTab(Tab::DECK); });
		btnTabDeck->SetSpriteScale(2.f, 2.f);
		btnTabDeck->Init(uiCamera);

		btnResume = std::make_shared<IconButton>(transformManager, resumeX, bottomY - 50.0f, 48.0f * 2, 32.0f * 2, uiTex);
		btnResume->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(144, 176, 48, 32, 3));
		btnResume->SetOnReleaseLeft([this](DX9GF::ITrigger* t) { this->Toggle(); });
		btnResume->SetSpriteScale(2.f, 2.f);
		btnResume->Init(uiCamera);

		btnOptions = std::make_shared<IconButton>(transformManager, optionsX, bottomY - 50.0f, 48.0f * 2, 32.0f * 2, uiTex);
		btnOptions->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(0, 208, 48, 32, 3));
		btnOptions->SetOnReleaseLeft([this, sw, sh](DX9GF::ITrigger* t) {
				this->game->GetSceneManager()->PushScene(new SettingsScene(this->game, sw, sh));
				this->game->GetSceneManager()->GoToNext();
			});
		btnOptions->SetSpriteScale(2.f, 2.f);
		btnOptions->Init(uiCamera);

		btnLeaveGame = std::make_shared<IconButton>(transformManager, leaveX, bottomY - 50.0f, 48.0f * 2, 32.0f * 2, uiTex);
		btnLeaveGame->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(144, 208, 48, 32, 3));
		btnLeaveGame->SetOnReleaseLeft([this](DX9GF::ITrigger* t) {
				this->pendingLeave = true;
			});
		btnLeaveGame->SetSpriteScale(2.f, 2.f);
		btnLeaveGame->Init(uiCamera);

		deckContainer = std::make_shared<IContainer>(transformManager, containerW, 40.0f, leftContainerX, containerY);
		deckContainer->Init(draggableManager, game->GetGraphicsDevice(), uiCamera);
		deckContainer->SetMaxHeight(sh * 0.5f);

        inventoryContainer = std::make_shared<IContainer>(transformManager, containerW, 40.0f, rightContainerX, containerY);
		inventoryContainer->Init(draggableManager, game->GetGraphicsDevice(), uiCamera);
		inventoryContainer->SetMaxHeight(sh * 0.5f);
	}

	void InventoryMenu::Toggle()
	{
		isOpen = !isOpen;
		if (isOpen) {
			RefreshItemsUI();
			backBufferTexture = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
			backBufferTexture->CaptureCurrentBackBuffer();
			backBufferSprite = std::make_shared<DX9GF::StaticSprite>(backBufferTexture.get());
			for (auto& cardId : player->GetDeck()) {
				auto dragCard = std::dynamic_pointer_cast<IDraggable>(ICard::CreateCard(cardId, transformManager, draggableManager, game->GetGraphicsDevice(), uiCamera));
				if (dragCard) deckContainer->AddChildProgrammatically(dragCard);
			}

			for (auto& cardId : player->GetInventoryCards()) {
				auto dragCard = std::dynamic_pointer_cast<IDraggable>(ICard::CreateCard(cardId, transformManager, draggableManager, game->GetGraphicsDevice(), uiCamera));
				if (dragCard) inventoryContainer->AddChildProgrammatically(dragCard);
			}
		}
		else {
			player->ClearDeck();
			for (auto& weakChild : deckContainer->GetChildren()) {
				if (auto child = weakChild.lock()) {
					child->DetachParent();
					if (auto card = std::dynamic_pointer_cast<ICard>(child)) {
						player->AddCardToDeck(card->GetSaveID());
					}
					draggableManager->Remove(child);
				}
			}
			deckContainer->ClearChildren();

			player->ClearInventory();
			for (auto& weakChild : inventoryContainer->GetChildren()) {
				if (auto child = weakChild.lock()) {
					child->DetachParent();
					if (auto card = std::dynamic_pointer_cast<ICard>(child)) {
						player->AddCardToInventory(card->GetSaveID());
					}
					draggableManager->Remove(child);
				}
			}
			inventoryContainer->ClearChildren();
		}

		if (transformManager) {
			transformManager->RebuildHierarchy();
		}
	}

	void InventoryMenu::Update(unsigned long long deltaTime)
	{
		if (!isOpen) return;

		auto [sw, sh] = uiCamera->GetScreenResolution();
		float centerX = 0.0f;
		float topY = -sh / 2.0f;
		float bottomY = sh / 2.0f;
		float tabY = topY + 40.0f;
		float tabGap = 24.0f;
		float bottomGap = 20.0f;
		float containerGap = 40.0f;
		float containerW = (sw - 120.0f - containerGap) / 2.0f;
		float containerY = topY + 110.0f;
		float leftContainerX = -containerGap / 2.0f - containerW;
		float rightContainerX = containerGap / 2.0f;
		float buttonW = 48.0f * 2;
		float resumeX = -(buttonW + bottomGap + buttonW + bottomGap + buttonW) / 2.0f;
		float optionsX = resumeX + buttonW + bottomGap;
		float leaveX = optionsX + buttonW + bottomGap;

		btnTabItems->SetLocalPosition(centerX - tabGap / 2.0f - btnTabItems->GetWidth(), tabY);
		btnTabDeck->SetLocalPosition(centerX + tabGap / 2.0f, tabY);
		btnResume->SetLocalPosition(resumeX, bottomY - 80.0f);
		btnOptions->SetLocalPosition(optionsX, bottomY - 80.0f);
		btnLeaveGame->SetLocalPosition(leaveX, bottomY - 80.0f);

		deckContainer->SetLocalPosition(leftContainerX, containerY);
		inventoryContainer->SetLocalPosition(rightContainerX, containerY);

		btnTabItems->Update(deltaTime);
		btnTabDeck->Update(deltaTime);
		btnResume->Update(deltaTime);
		btnOptions->Update(deltaTime);
		btnLeaveGame->Update(deltaTime);

		if (currentTab == Tab::DECK) {
			btnTabDeck->SetState(Demo::IButton::ButtonState::CLICKED);
			deckContainer->Update(deltaTime);
			inventoryContainer->Update(deltaTime);
		}
		else if (currentTab == Tab::ITEMS) {
			btnTabItems->SetState(Demo::IButton::ButtonState::CLICKED);
			if (isItemsDirty) RefreshItemsUI(); //only refresh items when needed

			for (auto& btn : buffItems) {
				btn->Update(deltaTime);
			}
		}
	}

	void InventoryMenu::Draw(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
	{
		if (!isOpen) return;

		auto [sw, sh] = uiCamera->GetScreenResolution();
		float centerX = 0.0f;
		float leftEdge = -sw / 2.0f;
		float topEdge = -sh / 2.0f;
		float bottomEdge = sh / 2.0f;

		if (backBufferSprite) {
			backBufferSprite->Begin();
			backBufferSprite->SetPosition(leftEdge, topEdge);
			backBufferSprite->Draw(*uiCamera, deltaTime);
			backBufferSprite->End();
		}

		gd->SetAlphaBlending(true);
		gd->DrawRectangle(*uiCamera, leftEdge, topEdge, sw, sh, 0, 1, 1, 0, 0, D3DXCOLOR(0, 0, 0, 0.65f), true);
		gd->SetAlphaBlending(false);

		btnTabItems->Draw(gd, deltaTime);
		btnTabDeck->Draw(gd, deltaTime);
		btnResume->Draw(gd, deltaTime);
		btnOptions->Draw(gd, deltaTime);
		btnLeaveGame->Draw(gd, deltaTime);

		fontSprite->Begin();
		fontSprite->SetScale(1.5f, 1.5f);
		fontSprite->SetColor(0xFFFFFF00);
		fontSprite->SetOutline(true, 0xFF000000, 3.f);
		fontSprite->SetPosition(leftEdge + 30.0f, bottomEdge - 95.0f);

        fontSprite->SetText(std::to_wstring(player->GetGold()) + L"G");
		fontSprite->Draw(*uiCamera, deltaTime);

		fontSprite->SetPosition(leftEdge + 30.0f, bottomEdge - 60.0f);
		fontSprite->SetText(std::to_wstring(static_cast<int>(player->GetHealth())) + L"/" + std::to_wstring(static_cast<int>(player->GetMaxHealth())) + L"HP");
		fontSprite->Draw(*uiCamera, deltaTime);
		fontSprite->End();
		fontSprite->SetOutline(false);

		if (currentTab == Tab::ITEMS) 
		{
			auto& inventory = player->GetInventoryItems().GetSlots();
			hoverDescription = L"";
			int displayIndex = 0;

			fontSprite->Begin();
			fontSprite->SetScale(1.0f, 1.0f);
			fontSprite->SetColor(0xFFFFFFFF);

			for (int i = 0; i < inventory.size(); i++) {
				if (inventory[i].quantity <= 0) continue;

				auto blueprint = Demo::ItemData::GetInstance()->GetItemBlueprint(inventory[i].itemID);
				if (!blueprint) continue;

				if (displayIndex >= buffItems.size()) break;

				auto btn = buffItems[displayIndex];

				btn->Draw(gd, deltaTime);

				float textX = btn->GetWorldX() + (ITEM_W / 2.0f) - 10.0f;

				float textY = btn->GetWorldY() + ITEM_H + 5.0f;

				fontSprite->SetPosition(textX, textY);
				fontSprite->SetOutline(true, 0xFF000000, 3.f);
				fontSprite->SetText(L"x" + std::to_wstring(inventory[i].quantity));
				fontSprite->Draw(*uiCamera, deltaTime);

				if (btn->GetTrigger()->IsHovering(deltaTime)) {
					hoverDescription = blueprint->GetDescription();
				}

				displayIndex++;
			}
			fontSprite->End();

			//Draw description
			if (!hoverDescription.empty()) {
				fontSprite->Begin();
				fontSprite->SetScale(1.2f, 1.2f);
				fontSprite->SetColor(0xFFFFFFFF);
				fontSprite->SetOutline(true, 0xFF000000, 3.f);

				fontSprite->SetPosition(leftEdge + 50.0f, sh / 2.0f - 150.0f);

				fontSprite->SetText(std::move(hoverDescription));
				fontSprite->Draw(*uiCamera, deltaTime);
				fontSprite->End();
				fontSprite->SetOutline(false);
			}
		}
		else if (currentTab == Tab::DECK) {
			float containerGap = 40.0f;
			float containerW = (sw - 120.0f - containerGap) / 2.0f;
			fontSprite->Begin();
			fontSprite->SetScale(1.0f, 1.0f);
			fontSprite->SetOutline(true, 0xFF000000, 3.f);
			fontSprite->SetColor(0xFFFFFFFF);
			fontSprite->SetText(L"Current Deck");
			float leftContainerX = -containerGap / 2.0f - containerW / 2.f - fontSprite->GetWidth() / 2.f;
			fontSprite->SetPosition(leftContainerX + 10.0f, topEdge + 120.0f);
			fontSprite->Draw(*uiCamera, deltaTime);
			fontSprite->SetText(L"Available Cards");
			float rightContainerX = containerGap / 2.0f + containerW / 2.f - fontSprite->GetWidth() / 2.f;
            fontSprite->SetPosition(rightContainerX + 10.0f, topEdge + 120.0f);
			fontSprite->Draw(*uiCamera, deltaTime);
			fontSprite->End();

			deckContainer->Draw(deltaTime);
			inventoryContainer->Draw(deltaTime);
		}
	}

	void InventoryMenu::RefreshItemsUI()
	{
		buffItems.clear();
		if (!player) return;

		auto& inventory = player->GetInventoryItems().GetSlots();
		int displayIndex = 0;

		int columns = std::floor((BG_W - PADDING_X) / (ITEM_W + PADDING_X));
		if (columns < 1) columns = 1;

		float totalGridWidth = (columns * ITEM_W) + ((columns - 1) * PADDING_X);

		float startX = -HALF_BG_W + (BG_W - totalGridWidth) / 2.0f;

		float startY = -HALF_BG_H;

		for (int i = 0; i < inventory.size(); i++) {
			auto slot = inventory[i];

			if (slot.quantity <= 0) continue;

			const auto* blueprint = Demo::ItemData::GetInstance()->GetItemBlueprint(slot.itemID);
			if (!blueprint) continue;

			int col = displayIndex % columns;
			int row = displayIndex / columns;

			float baseX = startX + col * (ITEM_W + PADDING_X);
			float baseY = startY + row * (ITEM_H + PADDING_Y);

			auto btn = std::make_shared<IconButton>(transformManager, 0, 0, ITEM_W, ITEM_H, itemSheetTex, 1);
			btn->Init(uiCamera);
			btn->SetSpriteScale(MENU_SCALE, MENU_SCALE);
			btn->SetLocalPosition(baseX, baseY);
			btn->Update(0);

			btn->SetSpriteRects({ blueprint->GetItemRect() });

			btn->SetOnReleaseLeft([&, slot, blueprint](DX9GF::ITrigger* thisObj) {
				//Nothing happens
				});

			buffItems.push_back(btn);
			displayIndex++;
		}

		if (transformManager) {
			transformManager->RebuildHierarchy();
			transformManager->UpdateAll();
		}
		isItemsDirty = false;
	}
}