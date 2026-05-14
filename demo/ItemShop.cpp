#include "pch.h"
#include "ItemShop.h"
#include "GameItems.h"

void Demo::ItemShop::AddShopItem(int itemID, int price)
{
    const auto* blueprint = ItemData::GetInstance()->GetItemBlueprint(itemID);

    if (blueprint) {
        std::wstring wName = blueprint->GetName();
        std::string name(wName.begin(), wName.end());

        itemsForSale.push_back({
            name,
            price,
            blueprint->GetDescription(),
            [this, itemID]() {
                this->player->GetInventoryItems().AddItem(itemID, 1);
            }
            });
    }
}

Demo::ItemShop::ItemShop(Game* game, Player* player, int sw, int sh, ShopTier tier)
    : IShopScene(game, player, sw, sh,
        tier == ShopTier::BASIC ? "--- BASIC SHOP ---" :
        tier == ShopTier::HYBRID ? "--- HYBRID SHOP ---" : "--- PREMIUM SHOP ---"),
    currentTier(tier)
{
}

void Demo::ItemShop::LoadItems()
{
    //i set 100G to test, remember to balance them later
    switch (currentTier) {
    case ShopTier::BASIC:
        AddShopItem(0, 50);
        AddShopItem(1, 70);
        AddShopItem(2, 90);
        break;

    case ShopTier::HYBRID:
        AddShopItem(3, 100);
        AddShopItem(4, 100);
        AddShopItem(5, 100);
        break;

    case ShopTier::PREMIUM:
        AddShopItem(6, 100);
        AddShopItem(7, 100);
        AddShopItem(8, 100);
        AddShopItem(9, 100);
        break;
    }
}