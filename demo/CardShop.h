#pragma once
#include "IShopScene.h"
#include "CardCatalog.h"

namespace Demo {
	class CardShop : public IShopScene {
 private:
		ShopTier currentTier;
		// Builds the listing from the card type itself, so its description, face artwork and
		// the card the player actually receives can never drift apart. The price comes from
		// CardCatalog, the single source of truth shared with the card-pack peddler.
		template <typename TCard>
		void AddShopCard(const std::string& name) {
			TCard prototype(this->transformManager);
			itemsForSale.push_back({
				std::wstring(name.begin(), name.end()),
				CardCatalog::GetPrice(prototype.GetSaveID()),
				prototype.GetDescription(),
				[this]() {
					auto newCard = std::make_shared<TCard>(this->transformManager);
					this->player->AddCardToInventory(newCard->GetSaveID());
				},
				prototype.GetFaceRect(),
				ShopIconSheet::CardFaces,
				prototype.IsPersistent(),
				prototype.HasLimitedUses(),
				prototype.HasLimitedUses() ? prototype.GetMaxUses() : 0
				});
		}
	public:
        CardShop(Game* game, Player* player, int screenWidth, int screenHeight, ShopTier tier);


		void LoadItems() override;
		void LoadSellItems() override;
	};
}