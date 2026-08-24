#pragma once
#include "IShopScene.h"

namespace Demo {
	class CardShop : public IShopScene {
 private:
		ShopTier currentTier;
		// Builds the listing from the card type itself, so its description, face artwork and
		// the card the player actually receives can never drift apart.
		template <typename TCard>
		void AddShopCard(const std::string& name, int price) {
			TCard prototype(this->transformManager);
			itemsForSale.push_back({
				std::wstring(name.begin(), name.end()),
				price,
				prototype.GetDescription(),
				[this]() {
					auto newCard = std::make_shared<TCard>(this->transformManager);
					this->player->AddCardToInventory(newCard->GetSaveID());
				},
				prototype.GetFaceRect(),
				ShopIconSheet::CardFaces
				});
		}
	public:
        CardShop(Game* game, Player* player, int screenWidth, int screenHeight, ShopTier tier);


		void LoadItems() override;
	};
}