#include "pch.h"
#include "CardShop.h"
#include "StrikeCard.h"
#include "AdvancedCards.h"

void Demo::CardShop::AddShopCard(const std::string& name, int price, const std::wstring& description, const std::function<void()>& onBuyAction)
{
	itemsForSale.push_back({
		name,
		price,
        description,
		onBuyAction
		});
}

Demo::CardShop::CardShop(Game* game, Player* player, int sw, int sh, ShopTier tier)
	: IShopScene(game, player, sw, sh,
		tier == ShopTier::BASIC ? "--- BASIC CARD SHOP ---" :
		tier == ShopTier::HYBRID ? "--- HYBRID CARD SHOP ---" : "--- PREMIUM CARD SHOP ---"),
	currentTier(tier)
{
}

void Demo::CardShop::LoadItems()
{
	const int StrikeCardCost = 50;
	const int HeavyStrikeCardCost = 100;
	const int PoisonCardCost = 125;
	const int TwinStrikeCardCost = 90;
	const int CleaveCardCost = 100;
	const int VulnerableCardCost = 200;
	const int WeaknessCardCost = 150;
	const int ChainLightningCardCost = 250;
	const int StunCardCost = 500;
    switch (currentTier) {
	case ShopTier::BASIC:
       AddShopCard("Strike Card", StrikeCardCost, StrikeCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<StrikeCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
        AddShopCard("Heavy Strike Card", HeavyStrikeCardCost, HeavyStrikeCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<HeavyStrikeCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
       AddShopCard("Poison Card", PoisonCardCost, PoisonCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<PoisonCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
		break;

	case ShopTier::HYBRID:
      AddShopCard("Twin Strike Card", TwinStrikeCardCost, TwinStrikeCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<TwinStrikeCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
       AddShopCard("Cleave Card", CleaveCardCost, CleaveCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<CleaveCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
       AddShopCard("Vulnerable Card", VulnerableCardCost, VulnerableCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<VulnerableCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
       AddShopCard("Weakness Card", WeaknessCardCost, WeaknessCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<WeaknessCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
		});
		break;

	case ShopTier::PREMIUM:
       AddShopCard("Cleave Card", CleaveCardCost, CleaveCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<CleaveCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
      AddShopCard("Chain Lightning Card", ChainLightningCardCost, ChainLightningCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<ChainLightningCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
       AddShopCard("Stun Card", StunCardCost, StunCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<StunCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
		break;
	}
}