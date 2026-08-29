#include "pch.h"
#include "InventoryMenu.h"
#include "SettingsScene.h"
#include "DX9GFAudioManager.h"
#include <algorithm>
#include <unordered_map>

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
		float sw = static_cast<float>(game->GetVirtualWidth());
		float sh = static_cast<float>(game->GetVirtualHeight());

		float centerX = 0.0f;
		float topY = -sh / 2.0f;
		float bottomY = sh / 2.0f;
		float tabY = topY + 40.0f;
		float tabGap = 24.0f;
		float bottomGap = 20.0f;
		float containerGap = 40.0f;
		float containerW = (sw - 120.0f - containerGap) / 2.0f;
		float containerY = topY + 190.0f;
		float leftContainerX = -containerGap / 2.0f - containerW;
		float rightContainerX = containerGap / 2.0f;
		float buttonW = 48.0f * 2;
		float totalTabW = 3 * buttonW + 2 * tabGap;
		float startTabX = centerX - totalTabW / 2.0f;
		float resumeX = -(buttonW + bottomGap + buttonW + bottomGap + buttonW) / 2.0f;
		float optionsX = resumeX + buttonW + bottomGap;
		float leaveX = optionsX + buttonW + bottomGap;

		itemSheetTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		itemSheetTex->LoadTexture(L"assets/items.png");

		uiTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
		uiTex->LoadTexture(L"assets/ui.png");

		btnTabItems = std::make_shared<IconButton>(transformManager, startTabX, tabY, 48.0f * 2, 32.0f * 2, uiTex);
		btnTabItems->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(0, 144, 48, 32, 3));
		btnTabItems->SetOnReleaseLeft([this](DX9GF::ITrigger* t) { this->SetTab(Tab::ITEMS); });
		btnTabItems->SetSpriteScale(2.f, 2.f);
		btnTabItems->Init(uiCamera);

		btnTabDeck = std::make_shared<IconButton>(transformManager, startTabX + buttonW + tabGap, tabY, 48.0f * 2, 32.0f * 2, uiTex);
		btnTabDeck->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(0, 176, 48, 32, 3));
		btnTabDeck->SetOnReleaseLeft([this](DX9GF::ITrigger* t) { this->SetTab(Tab::DECK); });
		btnTabDeck->SetSpriteScale(2.f, 2.f);
		btnTabDeck->Init(uiCamera);

		btnTabQuest = std::make_shared<IconButton>(transformManager, startTabX + 2 * (buttonW + tabGap), tabY, 48.0f * 2, 32.0f * 2, uiTex);
		btnTabQuest->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(0, 464, 48, 32, 3));
		btnTabQuest->SetOnReleaseLeft([this](DX9GF::ITrigger* t) { this->SetTab(Tab::QUEST); });
		btnTabQuest->SetSpriteScale(2.f, 2.f);
		btnTabQuest->Init(uiCamera);

		btnResume = std::make_shared<IconButton>(transformManager, resumeX, bottomY - 50.0f, 48.0f * 2, 32.0f * 2, uiTex);
		btnResume->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(144, 176, 48, 32, 3));
		btnResume->SetOnReleaseLeft([this](DX9GF::ITrigger* t) { this->Toggle(); });
		btnResume->SetSpriteScale(2.f, 2.f);
		btnResume->Init(uiCamera);

		btnOptions = std::make_shared<IconButton>(transformManager, optionsX, bottomY - 50.0f, 48.0f * 2, 32.0f * 2, uiTex);
		btnOptions->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(0, 208, 48, 32, 3));
		btnOptions->SetOnReleaseLeft([this, sw, sh](DX9GF::ITrigger* t) {
			auto sceMan = this->game->GetSceneManager();
			sceMan->InsertScene(sceMan->GetIndex() + 1, new SettingsScene(this->game, sw, sh));
			sceMan->GoToNext();
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

		float deckContainerY = containerY - 35.0f;

		deckContainer = std::make_shared<CardContainer>(transformManager, containerW, 40.0f, leftContainerX, deckContainerY);
		deckContainer->Init(draggableManager, game->GetGraphicsDevice(), uiCamera);
		deckContainer->SetMaxHeight(sh * 0.5f);
		deckContainer->SetCulling(true);

		inventoryContainer = std::make_shared<CardContainer>(transformManager, containerW, 40.0f, rightContainerX, deckContainerY);
		inventoryContainer->Init(draggableManager, game->GetGraphicsDevice(), uiCamera);
		inventoryContainer->SetMaxHeight(sh * 0.5f);
		inventoryContainer->SetCulling(true);
	}

	void InventoryMenu::Toggle()
	{
		isOpen = !isOpen;
		if (isOpen) {
			RefreshItemsUI();
			RefreshQuestUI();
			DX9GF::AudioManager::GetInstance()->Play("open_inv");

			// Items rebuild lazily in Update's ITEMS branch; only mark dirty if they changed.
			if (ItemSignatureChanged()) {
				isItemsDirty = true;
			}
			SyncCards();
		}
		else {
			DX9GF::AudioManager::GetInstance()->Play("close_inv");
			CommitCards();
			SetCardsHidden(true);
		}

		if (transformManager) {
			transformManager->RebuildHierarchy();
		}
	}

	// Re-materialises the containers from the player's current deck / inventory, reusing the card
	// objects already built. The common case - open, look, close with no change - does no
	// structural work at all.
	void InventoryMenu::SyncCards()
	{
		if (!player) return;

		const std::vector<std::string>& deck = player->GetDeck();
		const std::vector<std::string>& inventory = player->GetInventoryCards();

		auto sameMultiset = [](std::vector<std::string> a, std::vector<std::string> b) {
			if (a.size() != b.size()) return false;
			std::sort(a.begin(), a.end());
			std::sort(b.begin(), b.end());
			return a == b;
			};

		if (cardsSynced && sameMultiset(deck, syncedDeck) && sameMultiset(inventory, syncedInventory)) {
			SetCardsHidden(false);
			return;
		}

		draggableManager->SetDeferRebuild(true);
		ReconcileContainer(deckContainer, deck);
		ReconcileContainer(inventoryContainer, inventory);
		draggableManager->SetDeferRebuild(false); // one DraggableManager rebuild for the whole batch

		syncedDeck.assign(deck.begin(), deck.end());
		syncedInventory.assign(inventory.begin(), inventory.end());
		cardsSynced = true;
	}

	void InventoryMenu::ReconcileContainer(const std::shared_ptr<CardContainer>& container, const std::vector<std::string>& targetIds)
	{
		std::unordered_map<std::string, int> need;
		for (auto& id : targetIds) {
			need[id]++;
		}

		std::vector<std::shared_ptr<IDraggable>> result;
		result.reserve(targetIds.size());

		// Keep the children still called for; return the rest to the pool.
		for (auto& weakChild : container->GetChildren()) {
			auto child = weakChild.lock();
			if (!child) continue;
			auto card = std::dynamic_pointer_cast<ICard>(child);
			const std::string id = card ? card->GetSaveID() : std::string();
			auto it = need.find(id);
			if (card && it != need.end() && it->second > 0) {
				it->second--;
				child->SetHidden(false);
				result.push_back(child);
			}
			else {
				child->DetachParent(); // cheap: SetDeferRebuild is on
				child->SetHidden(true);
				// Park it far away so its trigger can't be hovered/grabbed while pooled.
				child->SetLocalPosition(-100000.f, -100000.f);
				if (card) cardPool[id].push_back(card);
			}
		}

		// Fill the shortfall from the pool, or build a new card only if the pool is empty.
		for (auto& [id, count] : need) {
			for (int k = 0; k < count; ++k) {
				auto card = AcquireCard(id);
				auto dragCard = std::dynamic_pointer_cast<IDraggable>(card);
				if (!dragCard) continue;
				dragCard->SetHidden(false);
				container->AdoptChild(dragCard);
				result.push_back(dragCard);
			}
		}

		container->SetChildList(result);
	}

	std::shared_ptr<ICard> InventoryMenu::AcquireCard(const std::string& cardId)
	{
		auto& pool = cardPool[cardId];
		if (!pool.empty()) {
			auto card = pool.back();
			pool.pop_back();
			return card;
		}
		return ICard::CreateCard(cardId, transformManager, draggableManager, game->GetGraphicsDevice(), uiCamera);
	}

	void InventoryMenu::CommitCards()
	{
		if (!player) return;

		player->ClearDeck();
		for (auto& weakChild : deckContainer->GetChildren()) {
			if (auto child = weakChild.lock()) {
				if (auto card = std::dynamic_pointer_cast<ICard>(child)) {
					player->AddCardToDeck(card->GetSaveID());
				}
			}
		}

		player->ClearInventory();
		for (auto& weakChild : inventoryContainer->GetChildren()) {
			if (auto child = weakChild.lock()) {
				if (auto card = std::dynamic_pointer_cast<ICard>(child)) {
					player->AddCardToInventory(card->GetSaveID());
				}
			}
		}

		const std::vector<std::string>& deck = player->GetDeck();
		const std::vector<std::string>& inventory = player->GetInventoryCards();
		syncedDeck.assign(deck.begin(), deck.end());
		syncedInventory.assign(inventory.begin(), inventory.end());
	}

	void InventoryMenu::SetCardsHidden(bool hidden)
	{
		for (auto& weakChild : deckContainer->GetChildren()) {
			if (auto child = weakChild.lock()) child->SetHidden(hidden);
		}
		for (auto& weakChild : inventoryContainer->GetChildren()) {
			if (auto child = weakChild.lock()) child->SetHidden(hidden);
		}
	}

	bool InventoryMenu::ItemSignatureChanged()
	{
		std::vector<std::pair<int, int>> sig;
		if (player) {
			for (auto& slot : player->GetInventoryItems().GetSlots()) {
				if (slot.quantity > 0) sig.push_back({ slot.itemID, slot.quantity });
			}
		}
		if (sig != syncedItemSig) {
			syncedItemSig = std::move(sig);
			return true;
		}
		return false;
	}

	std::vector<KeyboardNavigator::Candidate> InventoryMenu::CollectKeyboardCandidates()
	{
		std::vector<KeyboardNavigator::Candidate> candidates;

		auto addButton = [&](std::shared_ptr<IconButton> button) {
			if (!button || button->GetState() == IButton::ButtonState::DISABLED) return;
			candidates.push_back({
				button,
				button->GetWorldX(),
				button->GetWorldY(),
				(float)button->GetWidth(),
				(float)button->GetHeight(),
				[button]() { button->Activate(); }
				});
			};

		addButton(btnTabItems);
		addButton(btnTabDeck);
		if (btnTabQuest) addButton(btnTabQuest);
		addButton(btnResume);
		addButton(btnOptions);
		addButton(btnLeaveGame);

		if (currentTab == Tab::DECK) {
			// Cards in either container; activating one moves it to the other container
			// (deck order doesn't matter - it's shuffled at battle start).
			auto addCards = [&](std::shared_ptr<CardContainer> from, std::shared_ptr<CardContainer> to) {
				for (auto& weakChild : from->GetChildren()) {
					auto card = weakChild.lock();
					if (!card || card->IsDragging()) continue;
					candidates.push_back({
						card,
						card->GetWorldX(),
						card->GetWorldY(),
						(float)card->GetWidth(),
						(float)card->GetHeight(),
						[this, to, card]() {
							// Keep the cursor at the card's old slot (the next card shifts up
							// into it) so several cards can be moved in a row without re-navigating.
							const float oldX = card->GetWorldX();
							const float oldY = card->GetWorldY();
							to->AddChildProgrammatically(card);
							DX9GF::AudioManager::GetInstance()->PlayRandom("card_snap", 0.2f);
							keyboardNavigator.RetargetNearest(oldX, oldY);
						}
						});
				}
				};
			addCards(deckContainer, inventoryContainer);
			addCards(inventoryContainer, deckContainer);
		}
		else if (currentTab == Tab::QUEST) {
			float sh = static_cast<float>(game->GetVirtualHeight());
			float containerY = -sh / 2.0f + 190.0f;
			float containerH = sh * 0.5f;

			for (auto& qb : questButtons) {
				if (!qb.btn) continue;
				if (qb.btn->GetState() == IButton::ButtonState::DISABLED) continue;

				float btnY = qb.btn->GetWorldY();
				if (btnY + qb.btn->GetHeight() > containerY && btnY < containerY + containerH) {
					candidates.push_back({
						qb.btn,
						qb.btn->GetWorldX(),
						qb.btn->GetWorldY(),
						(float)qb.btn->GetWidth(),
						(float)qb.btn->GetHeight(),
						[this, qb]() {
							this->selectedQuestId = qb.questId;
							qb.btn->Activate();
						}
						});
				}
			}
		}

		return candidates;
	}

	void InventoryMenu::Update(unsigned long long deltaTime)
	{
		if (!isOpen) return;

		float sw = static_cast<float>(game->GetVirtualWidth());
		float sh = static_cast<float>(game->GetVirtualHeight());

		float centerX = 0.0f;
		float topY = -sh / 2.0f;
		float bottomY = sh / 2.0f;
		float tabY = topY + 40.0f;
		float tabGap = 24.0f;
		float bottomGap = 20.0f;
		float containerGap = 40.0f;
		float containerW = (sw - 120.0f - containerGap) / 2.0f;
		float containerY = topY + 190.0f;
		float containerH = sh * 0.5f;
		float leftContainerX = -containerGap / 2.0f - containerW;
		float rightContainerX = containerGap / 2.0f;
		float buttonW = 48.0f * 2;
		float totalTabW = 3 * buttonW + 2 * tabGap;
		float startTabX = centerX - totalTabW / 2.0f;
		float resumeX = -(buttonW + bottomGap + buttonW + bottomGap + buttonW) / 2.0f;
		float optionsX = resumeX + buttonW + bottomGap;
		float leaveX = optionsX + buttonW + bottomGap;

		btnTabItems->SetLocalPosition(startTabX, tabY);
		btnTabDeck->SetLocalPosition(startTabX + buttonW + tabGap, tabY);
		if (btnTabQuest) btnTabQuest->SetLocalPosition(startTabX + 2 * (buttonW + tabGap), tabY);

		btnResume->SetLocalPosition(resumeX, bottomY - 80.0f);
		btnOptions->SetLocalPosition(optionsX, bottomY - 80.0f);
		btnLeaveGame->SetLocalPosition(leaveX, bottomY - 80.0f);

		float deckContainerY = containerY - 35.0f;
		deckContainer->SetLocalPosition(leftContainerX, deckContainerY);
		inventoryContainer->SetLocalPosition(rightContainerX, deckContainerY);

		btnTabItems->Update(deltaTime);
		btnTabDeck->Update(deltaTime);
		if (btnTabQuest) btnTabQuest->Update(deltaTime);

		btnResume->Update(deltaTime);
		btnOptions->Update(deltaTime);
		btnLeaveGame->Update(deltaTime);

		// Navigate first so the containers can scroll to follow the keyboard target this frame.
		keyboardNavigator.Update(deltaTime, CollectKeyboardCandidates());

		if (currentTab == Tab::DECK) {
			btnTabDeck->SetState(Demo::IButton::ButtonState::CLICKED);

			if (keyboardNavigator.IsInKeyboardMode()) {
				if (auto target = std::dynamic_pointer_cast<IDraggable>(keyboardNavigator.GetTarget())) {
					deckContainer->ScrollChildIntoView(target);
					inventoryContainer->ScrollChildIntoView(target);
				}
			}

			deckContainer->Update(deltaTime);
			inventoryContainer->Update(deltaTime);
		}
		else if (currentTab == Tab::ITEMS) {
			btnTabItems->SetState(Demo::IButton::ButtonState::CLICKED);
			if (isItemsDirty) RefreshItemsUI();

			for (auto& btn : buffItems) {
				btn->Update(deltaTime);
			}
		}
		else if (currentTab == Tab::QUEST) {
			if (btnTabQuest) btnTabQuest->SetState(Demo::IButton::ButtonState::CLICKED);
			if (isQuestsDirty) RefreshQuestUI();

			long scrollDelta = DX9GF::InputManager::GetInstance()->GetMouseScroll();
			if (scrollDelta != 0) {
				questScrollY -= scrollDelta * 0.2f;
			}
			float maxScroll = (std::max)(0.0f, questListTotalHeight - containerH);
			questScrollY = std::clamp(questScrollY, 0.0f, maxScroll);

			for (auto& qb : questButtons) {
				if (qb.btn) {
					//move right 2 pixels because the button is pressing against the edge of the frame
					qb.btn->SetLocalPosition(leftContainerX + 2.0f, containerY + qb.originalY - questScrollY);
					qb.btn->Update(deltaTime);

					if (qb.btn->GetState() == IButton::ButtonState::HOVER && qb.btn->GetState() != IButton::ButtonState::DISABLED) {
						selectedQuestId = qb.questId;
					}
				}
			}
		}
	}

	void InventoryMenu::Draw(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
	{
		if (!isOpen) return;

		float sw = static_cast<float>(game->GetVirtualWidth());
		float sh = static_cast<float>(game->GetVirtualHeight());

		float leftEdge = -sw / 2.0f;
		float topEdge = -sh / 2.0f;
		float bottomEdge = sh / 2.0f;

		gd->SetAlphaBlending(true);
		// Use D3DCOLOR_ARGB instead of D3DXCOLOR for the best compatibility with DrawRectangle
		gd->DrawRectangle(*uiCamera, leftEdge, topEdge, sw, sh, D3DCOLOR_ARGB(165, 0, 0, 0), true);
		gd->SetAlphaBlending(false);

		btnTabItems->Draw(gd, deltaTime);
		btnTabDeck->Draw(gd, deltaTime);
		if (btnTabQuest) btnTabQuest->Draw(gd, deltaTime);
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
			float leftContainerX = -containerGap / 2.0f - containerW;
			float rightContainerX = containerGap / 2.0f;

			fontSprite->Begin();
			fontSprite->SetScale(1.0f, 1.0f);
			fontSprite->SetOutline(true, 0xFF000000, 3.f);
			fontSprite->SetColor(0xFFFFFFFF);

			fontSprite->SetText(L"Current Deck");
			float centerLeft = leftContainerX + containerW / 2.f;
			fontSprite->SetPosition(centerLeft - fontSprite->GetWidth() / 2.f, topEdge + 150.0f);
			fontSprite->Draw(*uiCamera, deltaTime);

			fontSprite->SetText(L"Available Cards");
			float centerRight = rightContainerX + containerW / 2.f;
			fontSprite->SetPosition(centerRight - fontSprite->GetWidth() / 2.f, topEdge + 150.0f);
			fontSprite->Draw(*uiCamera, deltaTime);
			fontSprite->End();

			deckContainer->Draw(deltaTime);
			inventoryContainer->Draw(deltaTime);
		}
		else if (currentTab == Tab::QUEST) {
			float containerGap = 40.0f;
			float containerW = (sw - 120.0f - containerGap) / 2.0f;
			float leftContainerX = -containerGap / 2.0f - containerW;
			float rightContainerX = containerGap / 2.0f;
			float containerY = topEdge + 190.0f;
			float containerH = sh * 0.5f;

			fontSprite->Begin();
			fontSprite->SetScale(1.0f, 1.0f);
			fontSprite->SetOutline(true, 0xFF000000, 3.f);
			fontSprite->SetColor(0xFFFFFFFF);

			fontSprite->SetText(L"Quest Log");
			float centerLeft = leftContainerX + containerW / 2.f;
			fontSprite->SetPosition(centerLeft - fontSprite->GetWidth() / 2.f, topEdge + 150.0f);
			fontSprite->Draw(*uiCamera, deltaTime);

			fontSprite->SetText(L"Quest Details");
			float centerRight = rightContainerX + containerW / 2.f;
			fontSprite->SetPosition(centerRight - fontSprite->GetWidth() / 2.f, topEdge + 150.0f);
			fontSprite->Draw(*uiCamera, deltaTime);
			fontSprite->End();

			const D3DCOLOR PANEL_BG = D3DCOLOR_ARGB(230, 15, 15, 20);
			const D3DCOLOR PANEL_BORDER = D3DCOLOR_ARGB(255, 100, 100, 120);

			gd->SetAlphaBlending(true);
			gd->DrawRectangle(*uiCamera, leftContainerX, containerY, containerW, containerH, PANEL_BG, true);
			gd->DrawRectangle(*uiCamera, rightContainerX, containerY, containerW, containerH, PANEL_BG, true);
			gd->SetAlphaBlending(false);

			gd->DrawRectangle(*uiCamera, leftContainerX, containerY, containerW, containerH, PANEL_BORDER, false);
			gd->DrawRectangle(*uiCamera, rightContainerX, containerY, containerW, containerH, PANEL_BORDER, false);


			//Quest list
			D3DXMATRIX matCamera = uiCamera->GetTransformMatrix();
			D3DXVECTOR4 topLeft(leftContainerX, containerY, 0.0f, 1.0f);
			D3DXVECTOR4 bottomRight(leftContainerX + containerW, containerY + containerH, 0.0f, 1.0f);
			D3DXVec4Transform(&topLeft, &topLeft, &matCamera);
			D3DXVec4Transform(&bottomRight, &bottomRight, &matCamera);

			RECT scissorRect;
			scissorRect.left = static_cast<LONG>(topLeft.x / topLeft.w);
			scissorRect.top = static_cast<LONG>(topLeft.y / topLeft.w);
			scissorRect.right = static_cast<LONG>(bottomRight.x / bottomRight.w);
			scissorRect.bottom = static_cast<LONG>(bottomRight.y / bottomRight.w);

			gd->SetScissorTest(true);
			gd->SetScissorRect(scissorRect);

			for (auto& qb : questButtons) {
				if (qb.questId.find("HEADER|") == 0) {
					//Headers
					size_t p1 = qb.questId.find('|');
					size_t p2 = qb.questId.find('|', p1 + 1);
					if (p1 != std::string::npos && p2 != std::string::npos) {
						DWORD color = std::stoul(qb.questId.substr(p1 + 1, p2 - p1 - 1));
						std::string text = qb.questId.substr(p2 + 1);

						float drawY = containerY + qb.originalY - questScrollY;

						fontSprite->Begin();
						fontSprite->SetScale(1.2f, 1.2f);
						fontSprite->SetOutline(true, 0xFF000000, 3.0f);
						fontSprite->SetColor(color);

						std::wstring wText(text.begin(), text.end());
						fontSprite->SetText(wText);

						float padX = (containerW - fontSprite->GetWidth() * 1.2f) / 2.0f;
						fontSprite->SetPosition(leftContainerX + padX, drawY);

						fontSprite->Draw(*uiCamera, deltaTime);
						fontSprite->End();
						fontSprite->SetOutline(false);
					}
				}
				else if (qb.btn) {
					if (!qb.questId.empty() && qb.questId == selectedQuestId) {
						float btnDrawY = containerY + qb.originalY - questScrollY;
						float fullBtnH = qb.btn->GetHeight() + 11.0f;

						gd->SetAlphaBlending(true);
						gd->DrawRectangle(*uiCamera, leftContainerX + 2.0f, btnDrawY, containerW - 4.0f, fullBtnH, D3DCOLOR_ARGB(50, 255, 170, 0), true);
						gd->SetAlphaBlending(false);

						gd->DrawRectangle(*uiCamera, leftContainerX + 2.0f, btnDrawY, 4.0f, fullBtnH, 0xFFFFAA00, true);
					}
					qb.btn->Draw(gd, deltaTime);
				}
			}
			gd->SetScissorTest(false);

			//Quest's detail
			if (selectedQuestId.empty()) {
				fontSprite->Begin();
				fontSprite->SetOutline(false);
				fontSprite->SetColor(0xFF888888);

				fontSprite->SetText(L"No quest data available.");
				float centerNoDataX = rightContainerX + containerW / 2.0f - fontSprite->GetWidth() / 2.0f;
				fontSprite->SetPosition(centerNoDataX, containerY + containerH / 2.0f - 10.0f);

				fontSprite->Draw(*uiCamera, deltaTime);
				fontSprite->End();
			}
			else {
				auto* info = QuestManager::GetInstance()->GetQuestInfo(selectedQuestId);
				auto state = QuestManager::GetInstance()->GetQuestState(selectedQuestId);

				if (info) {
					float drawY = containerY + 15.0f;
					float padX = 20.0f;
					float maxTextWidth = containerW - padX * 2;

					fontSprite->Begin();
					fontSprite->SetOutline(false);

					fontSprite->SetScale(1.3f, 1.3f);
					fontSprite->SetColor(0xFFFFD700);
					fontSprite->SetPosition(rightContainerX + padX, drawY);
					fontSprite->SetText(L"(+) " + info->title);
					fontSprite->Draw(*uiCamera, deltaTime);
					drawY += 30.0f;

					fontSprite->SetScale(1.0f, 1.0f);
					fontSprite->SetColor(state == QuestState::Completed ? 0xFF00FF00 : 0xFFFFFFFF);
					fontSprite->SetPosition(rightContainerX + padX, drawY);
					fontSprite->SetText(state == QuestState::Completed ? L"Status: Completed" : L"Status: Active");
					fontSprite->Draw(*uiCamera, deltaTime);
					drawY += 25.0f;

					gd->DrawLine(*uiCamera, rightContainerX + padX, drawY, rightContainerX + containerW - padX, drawY, 0xFF555566);
					drawY += 15.0f;

					fontSprite->SetColor(0xFFCCCCCC);
					auto descLines = WrapText(info->description, maxTextWidth, 1.0f);
					for (const auto& line : descLines) {
						fontSprite->SetPosition(rightContainerX + padX, drawY);
						fontSprite->SetText(line);
						fontSprite->Draw(*uiCamera, deltaTime);
						drawY += 20.0f;
					}
					drawY += 15.0f;

					fontSprite->SetColor(0xFFFFFFFF);
					fontSprite->SetPosition(rightContainerX + padX, drawY);
					fontSprite->SetText(L"Objective:");
					fontSprite->Draw(*uiCamera, deltaTime);
					drawY += 20.0f;

					fontSprite->SetColor(0xFFFFAA00);
					fontSprite->SetPosition(rightContainerX + padX + 10.0f, drawY);
					fontSprite->SetText(L"- " + info->currentObjective);
					fontSprite->Draw(*uiCamera, deltaTime);

					float bottomY = containerY + containerH - 35.0f;
					fontSprite->SetColor(0xFFFFFFFF);
					fontSprite->SetPosition(rightContainerX + padX, bottomY - 20.0f);
					fontSprite->SetText(L"Reward:");
					fontSprite->Draw(*uiCamera, deltaTime);

					fontSprite->SetColor(0xFF00FFFF);
					fontSprite->SetPosition(rightContainerX + padX + 10.0f, bottomY);
					fontSprite->SetText(L"(+) " + info->rewardText);
					fontSprite->Draw(*uiCamera, deltaTime);

					fontSprite->End();
				}
			}
		}
	}

	void InventoryMenu::DrawKeyboardReticle(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
	{
		if (!isOpen) return;
		keyboardNavigator.Draw(gd, *uiCamera, CollectKeyboardCandidates());
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

	void InventoryMenu::RefreshQuestUI() {
		for (auto& qb : questButtons) {
			if (qb.btn) {
				qb.btn->SetLocalPosition(-9999.0f, -9999.0f);
				qb.btn->SetOnReleaseLeft([](DX9GF::ITrigger*) {});
			}
		}
		questButtons.clear();

		questListTotalHeight = 0.0f;
		questScrollY = 0.0f;

		float sw = static_cast<float>(game->GetVirtualWidth());
		float containerGap = 40.0f;
		float containerW = (sw - 120.0f - containerGap) / 2.0f;

		float currentY = 5.0f;
		float buttonH = 28.0f;
		float paddingY = 6.0f;

		auto activeQuests = QuestManager::GetInstance()->GetActiveQuests();
		auto completedQuests = QuestManager::GetInstance()->GetCompletedQuests();

		auto addHeader = [&](const std::string& text, DWORD color) {
			auto btn = std::make_shared<TextButton>(transformManager, 0, 0, containerW - 4.0f, buttonH, "", font, [](DX9GF::ITrigger*) {});
			btn->Init(uiCamera);
			btn->SetState(Demo::IButton::ButtonState::DISABLED);
			btn->SetBackgroundColors(D3DXCOLOR(0, 0, 0, 0), D3DXCOLOR(0, 0, 0, 0), D3DXCOLOR(0, 0, 0, 0), D3DXCOLOR(0, 0, 0, 0));

			std::string encoded = "HEADER|" + std::to_string(color) + "|" + text;
			questButtons.push_back({ btn, currentY, encoded });
			currentY += 35.0f;
			};

		auto addQuest = [&](const Demo::QuestInfo& q, bool isActive) {
			std::string prefix = isActive ? "(+) " : "(-) ";
			std::string titleStr = std::string(q.title.begin(), q.title.end());

			float questPadX = 15.0f;
			float btnInnerW = containerW - 4.0f - (questPadX * 2);

			auto btn = std::make_shared<TextButton>(transformManager, 0, 0, btnInnerW, buttonH, prefix + titleStr, font,
				[this, qId = q.id](DX9GF::ITrigger*) {
					this->selectedQuestId = qId;
				});
			btn->Init(uiCamera);
			btn->SetPadding(questPadX, 6.0f);

			if (isActive) btn->SetTextColors(0xFFFFFFFF, 0xFFFFD700, 0xFFFFAA00, 0xFF888888);
			else btn->SetTextColors(0xFFAAAAAA, 0xFFCCCCCC, 0xFFFFFFFF, 0xFF888888);

			btn->SetBackgroundColors(D3DXCOLOR(0, 0, 0, 0), D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.1f), D3DXCOLOR(1.0f, 0.84f, 0.0f, 0.2f), D3DXCOLOR(0, 0, 0, 0));

			questButtons.push_back({ btn, currentY, q.id });
			currentY += buttonH + paddingY;

			if (selectedQuestId.empty()) selectedQuestId = q.id;
			};

		if (!activeQuests.empty()) {
			addHeader("ACTIVE", 0xFFFFD700);
			for (auto& q : activeQuests) addQuest(q, true);
		}

		if (!completedQuests.empty()) {
			if (!activeQuests.empty()) currentY += 15.0f;
			addHeader("COMPLETED", 0xFF00FF66);
			for (auto& q : completedQuests) addQuest(q, false);
		}

		questListTotalHeight = currentY;
		isQuestsDirty = false;
	}

	std::vector<std::wstring> InventoryMenu::WrapText(const std::wstring& text, float maxWidth, float scale) {
		std::vector<std::wstring> lines;
		std::wstring currentLine = L"";
		std::wstring currentWord = L"";

		fontSprite->SetScale(scale, scale);

		auto flushWord = [&]() {
			if (currentWord.empty()) return;
			std::wstring testLine = currentLine.empty() ? currentWord : currentLine + L" " + currentWord;

			fontSprite->SetText(testLine);
			if (fontSprite->GetWidth() * scale > maxWidth && !currentLine.empty()) {
				lines.push_back(currentLine);
				currentLine = currentWord;
			}
			else {
				currentLine = testLine;
			}
			currentWord = L"";
			};

		for (wchar_t c : text) {
			if (c == L' ' || c == L'\n') {
				flushWord();
				if (c == L'\n') {
					lines.push_back(currentLine);
					currentLine = L"";
				}
			}
			else {
				currentWord += c;
			}
		}
		flushWord();
		if (!currentLine.empty()) lines.push_back(currentLine);

		return lines;
	}

}