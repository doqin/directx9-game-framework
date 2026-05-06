#include "pch.h"
#include "CardShop.h"
#include "StrikeCard.h"
#include "MainBlockCard.h"

Demo::CardShop::CardShop(Game* game, Player* player, int sw, int sh)
	: IShopScene(game, player, sw, sh, "CARD SHOP")
{
}

void Demo::CardShop::LoadItems()
{
	itemsForSale.push_back({
		"Strike Card",
		50,
		[this]() {
			auto newCard = std::make_shared<StrikeCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
		}
		});

	itemsForSale.push_back({
		"Main Block Card",
		75,
		[this]() {
			auto newCard = std::make_shared<MainBlockCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
		}
		});
}