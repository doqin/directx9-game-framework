#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Game.h"
#include "Player.h"
#include "TextButton.h"
#include "CardContainer.h"
#include "IconButton.h"
#include "KeyboardNavigator.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <utility>
#include "QuestManager.h"
#include "GearSlotUI.h"

namespace Demo {

	//manage quests to scroll
	struct QuestButtonData {
		std::shared_ptr<TextButton> btn;
		float originalY;
		std::string questId;
	};

	class InventoryMenu {
	public:
		enum class Tab { ITEMS, DECK, GEAR, QUEST };

	private:
		Game* game;
		std::shared_ptr<Player> player;
		std::shared_ptr<DX9GF::TransformManager> transformManager;
		std::shared_ptr<DraggableManager> draggableManager;
		DX9GF::Camera* uiCamera;

		DX9GF::Font* font;
		bool isOpen = false;
		Tab currentTab = Tab::ITEMS;

		// UI Buttons
		std::shared_ptr<IconButton> btnTabItems;
		std::shared_ptr<IconButton> btnTabDeck;
		std::shared_ptr<IconButton> btnTabGear;
		std::shared_ptr<IconButton> btnTabQuest;
		std::shared_ptr<IconButton> btnResume;
		std::shared_ptr<IconButton> btnOptions;
		std::shared_ptr<IconButton> btnLeaveGame;

		std::shared_ptr<DX9GF::FontSprite> fontSprite;

		// Deck Builder Containers
		std::shared_ptr<CardContainer> deckContainer;
		std::shared_ptr<CardContainer> inventoryContainer;

		// Tab Items
		std::shared_ptr<DX9GF::Texture> itemSheetTex;
		std::shared_ptr<DX9GF::Texture> uiTex;
		std::vector<std::shared_ptr<IconButton>> buffItems;
		bool isItemsDirty = true;
		std::wstring hoverDescription = L"";

		// Tab Gear
		std::shared_ptr<DX9GF::Texture> uiPlayerTex;
		std::shared_ptr<DX9GF::StaticSprite> uiPlayerSprite;
		float gearAnimTimer = 0.0f;

		std::shared_ptr<DX9GF::Texture> flameTex;
		std::shared_ptr<DX9GF::Texture> gearTex;

		std::shared_ptr<GearSlotUI> coreSlot;
		std::vector<std::shared_ptr<GearSlotUI>> orbitSlots;

		std::shared_ptr<DX9GF::AnimatedSprite> gearDroneAnim;
		float orbitAngle = 0.0f;

		// Tab Quests
		std::vector<QuestButtonData> questButtons;
		std::string selectedQuestId;
		float questScrollY = 0.0f;
		float questListTotalHeight = 0.0f;
		bool isQuestsDirty = true;

		// --- Card caching across toggles ------------------------------------------------
		// Cards are built once and kept alive (hidden) between opens; the containers only
		// restructure when the player's deck / inventory actually changed.
		bool cardsSynced = false;
		std::vector<std::string> syncedDeck;
		std::vector<std::string> syncedInventory;
		std::vector<std::pair<int, int>> syncedItemSig; // (itemID, quantity) of non-empty slots
		std::unordered_map<std::string, std::vector<std::shared_ptr<ICard>>> cardPool;

		void SyncCards();
		void ReconcileContainer(const std::shared_ptr<CardContainer>& container, const std::vector<std::string>& targetIds);
		void CommitCards();
		void SetCardsHidden(bool hidden);
		std::shared_ptr<ICard> AcquireCard(const std::string& cardId);
		bool ItemSignatureChanged();

		std::shared_ptr<DX9GF::Texture> backBufferTexture;
		std::shared_ptr<DX9GF::StaticSprite> backBufferSprite;

		bool pendingLeave = false;

		KeyboardNavigator keyboardNavigator;
		std::vector<KeyboardNavigator::Candidate> CollectKeyboardCandidates();

	public:
		InventoryMenu(Game* g, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::TransformManager> tm, std::shared_ptr<DraggableManager> dm, DX9GF::Camera* cam, DX9GF::Font* font);

		void Init();
		void Update(unsigned long long deltaTime);
		void Draw(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime);
		// Host scenes call this after DraggableManager::Draw so the reticle draws above the cards.
		void DrawKeyboardReticle(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime);

		void Toggle();
		bool IsOpen() const { return isOpen; }
		bool IsInKeyboardMode() const { return isOpen && keyboardNavigator.IsInKeyboardMode(); }
		void SetTab(Tab tab) {
			currentTab = tab;
			if (tab == Tab::QUEST) isQuestsDirty = true;
		}
		bool IsPendingLeave() const { return pendingLeave; }
		Tab GetCurrentTab() const { return currentTab; }

		void RefreshItemsUI();
		void RefreshQuestUI();
		//utils wraptext
		std::vector<std::wstring> WrapText(const std::wstring& text, float maxWidth, float scale);
	};
}