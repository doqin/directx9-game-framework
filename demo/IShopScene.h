#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Game.h"
#include "Player.h"
#include "TextButton.h"
#include "DX9GFFont.h"
#include <string>
#include <vector>
#include <functional>
#include "IconButton.h"
#include "TextIconButton.h"
#include "KeyboardNavigator.h"

namespace Demo {
	enum class ShopTier { BASIC, HYBRID, PREMIUM, RK_HYBRID };

	// Which sheet a row's artwork comes from. Items are small upright icons drawn beside
	// their name; card faces are wide strips that already read as the card's name and
	// energy cost, so they stand in for the name text entirely.
	enum class ShopIconSheet { None, Items, CardFaces };

	struct ShopItem {
		std::wstring name;
		int cost;
		std::wstring description;
		std::function<void()> onBuyAction;
		RECT iconRect{ 0, 0, 0, 0 };
		ShopIconSheet iconSheet = ShopIconSheet::None;
		// Mirrors ICard::IsPersistent/HasLimitedUses/GetMaxUses on the card this row sells, so the
		// shop can draw the same status cover (non-persistent badge, use pips) the card shows in
		// battle. Unused for ShopIconSheet::Items rows.
		bool isPersistent = true;
		bool hasLimitedUses = false;
		int maxUses = 0;
	};

	class IShopScene : public DX9GF::IScene {
	protected:
		Game* game;
		Player* player;

		std::shared_ptr<DX9GF::TransformManager> transformManager;

		std::shared_ptr<DX9GF::Font> myFont;
		std::shared_ptr<DX9GF::FontSprite> myFontSprite;

		std::vector<ShopItem> itemsForSale;
		// One buy button per item, across every page. Only the slice belonging to the
		// current page is updated and drawn, so off-page triggers can never fire.
		std::vector<std::shared_ptr<Demo::IconButton>> buyButtons;
		// The buttons that are live this frame: LEAVE, the visible rows, and the page
		// arrows when there is more than one page.
		std::vector<std::shared_ptr<Demo::IButton>> activeButtons;

		std::shared_ptr<Demo::IconButton> leaveButton;
		std::shared_ptr<Demo::TextIconButton> btnPrevPage;
		std::shared_ptr<Demo::TextIconButton> btnNextPage;

		int currentPage = 0;
		int maxPage = 0;
		int rowsPerPage = 1;

		std::wstring statusMessage;
		float messageTimer = 0.0f;
		std::string shopTitle;
		bool shouldLeave = false;

		std::shared_ptr<DX9GF::Texture> uiSheetTex;
		std::shared_ptr<DX9GF::Texture> borderTex;
		std::shared_ptr<DX9GF::Texture> itemsTex;

		std::shared_ptr<DX9GF::NineSliceSprite> panelFrameSprite;
		std::shared_ptr<DX9GF::NineSliceSprite> panelSprite;
		std::shared_ptr<DX9GF::NineSliceSprite> rowSprite;
		std::shared_ptr<DX9GF::NineSliceSprite> rowHoverSprite;
		std::shared_ptr<DX9GF::NineSliceSprite> descSprite;
		std::shared_ptr<DX9GF::StaticSprite> itemIconSprite;
		std::shared_ptr<DX9GF::StaticSprite> cardFaceSprite;
		std::shared_ptr<DX9GF::StaticSprite> goldIconSprite;

		// The status cover a card shows in battle (uses cover panel, use pips, non-persistent
		// badge), reproduced beside its face here. Same sheet (assets/ui.png) and geometry as
		// IStatementCard::DrawStatusCover; shop cards are always freshly minted, so only the
		// "full" pip variant is needed.
		std::shared_ptr<DX9GF::StaticSprite> coverBodySprite;
		std::shared_ptr<DX9GF::StaticSprite> coverCapSprite;
		std::shared_ptr<DX9GF::StaticSprite> coverPipSprite;
		std::shared_ptr<DX9GF::StaticSprite> coverBadgeSprite;
		// Width, in sheet pixels, of the status cover panel for the given row - 0 when the card
		// is persistent and unlimited, so it has nothing to show.
		int GetCardCoverPanelWidth(const ShopItem& item) const;
		// Draws the status cover immediately to the right of a card face already drawn at
		// [faceX, faceTopY] with the given on-screen width.
		void DrawCardCover(const ShopItem& item, float faceX, float faceTopY, float faceWidth,
			bool affordable, unsigned long long deltaTime);

		KeyboardNavigator keyboardNavigator;
		std::vector<KeyboardNavigator::Candidate> CollectKeyboardCandidates();

		void BuildUI();
		// Recomputes the page window, repositions the visible rows and rebuilds activeButtons.
		void RefreshPage();
		// Greys out and disables the buy buttons the player cannot currently afford.
		void RefreshAffordability();

	public:
		IShopScene(Game* game, Player* player, int screenWidth, int screenHeight, std::string title);
		virtual ~IShopScene() = default;

		virtual void LoadItems() = 0;

		bool IsOverlay() const override { return true; }
		void Init() override;
		void Update(unsigned long long deltaTime) override;
		void DrawWorld(unsigned long long deltaTime) override;
		void DrawUI(unsigned long long deltaTime) override;
		void ShowMessage(std::wstring msg);
	};
}
