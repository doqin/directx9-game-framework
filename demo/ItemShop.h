#pragma once
#include "IShopScene.h"

namespace Demo {
    class ItemShop : public IShopScene {
    private:
        ShopTier currentTier;
        void AddShopItem(int itemID, int price);
    public:
        ItemShop(Game* game, Player* player, int screenWidth, int screenHeight, ShopTier tier);
        void LoadItems() override;
    };
}