#include "pch.h"
#include "ItemShop.h"
#include "GameItems.h"

void Demo::ItemShop::AddShopItem(int itemID, int price)
{
	const auto* blueprint = ItemData::GetInstance()->GetItemBlueprint(itemID);

	if (blueprint) {
		itemsForSale.push_back({
			blueprint->GetName(),
			price,
			blueprint->GetDescription(),
			[this, itemID]() {
				this->player->GetInventoryItems().AddItem(itemID, 1);
			},
			blueprint->GetItemRect(),
			ShopIconSheet::Items
			});
	}
}

Demo::ItemShop::ItemShop(Game* game, Player* player, int sw, int sh, ShopTier tier)
	: IShopScene(game, player, sw, sh,
		tier == ShopTier::BASIC ? "BASIC SHOP" :
		tier == ShopTier::HYBRID || tier == ShopTier::RK_HYBRID ? "HYBRID SHOP" : "PREMIUM SHOP"),
	currentTier(tier)
{
}

void Demo::ItemShop::LoadItems()
{
	//i set 100G to test, remember to balance them later
	switch (currentTier) {
	case ShopTier::BASIC:
		AddShopItem(0, 30);
		AddShopItem(1, 30);
		AddShopItem(2, 40);
		break;

	case ShopTier::HYBRID:
		AddShopItem(3, 50);
		AddShopItem(4, 50);
		AddShopItem(5, 50);
		break;

	case ShopTier::PREMIUM:
		AddShopItem(6, 100);
		AddShopItem(7, 100);
		AddShopItem(8, 200);
		AddShopItem(9, 150);
		break;
	case ShopTier::RK_HYBRID:
		//load hybrid items just like HYBRID shop
		AddShopItem(3, 50);
		AddShopItem(4, 50);
		AddShopItem(5, 50);
		//add that rusty key with high cost - balance this later
		AddShopItem(10, 100);
		break;
	}
}