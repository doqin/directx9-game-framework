#pragma once
#include "IShopScene.h"

namespace Demo {
	class CardShop : public IShopScene {
 private:
		ShopTier currentTier;
     void AddShopCard(const std::string& name, int price, const std::wstring& description, const std::function<void()>& onBuyAction);
	public:
        CardShop(Game* game, Player* player, int screenWidth, int screenHeight, ShopTier tier);


		void LoadItems() override;
	};
}