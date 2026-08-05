#include "pch.h"
#include "CardShop.h"
#include "StrikeCard.h"
#include "EnergyCard.h"
#include "AdvancedCards.h"
#include "UtilityCards.h"
#include "FinisherCards.h"

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
	const int StrikeCardCost = 30;
	const int HeavyStrikeCardCost = 70;
	const int PoisonCardCost = 50;
	const int TwinStrikeCardCost = 40;
	const int CleaveCardCost = 40;
	const int VulnerableCardCost = 65;
	const int WeaknessCardCost = 85;
	const int ChainLightningCardCost = 100;
	const int StunCardCost = 300;
	const int EnergyCardCost = 45;
	const int JabCardCost = 20;
	const int MarkCardCost = 30;
	const int BraceCardCost = 25;
	const int PrefetchCardCost = 35;
	const int OverclockCardCost = 35;
	const int JumpstartCardCost = 55;
	const int ForesightCardCost = 70;
	const int TerminateCardCost = 150;
	const int InfernoCardCost = 140;
	const int SystemPurgeCardCost = 200;
	const int OverdriveCardCost = 180;
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
       AddShopCard("Energy Card", EnergyCardCost, EnergyCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<EnergyCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
	   AddShopCard("Jab Card", JabCardCost, JabCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<JabCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
	   AddShopCard("Brace Card", BraceCardCost, BraceCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<BraceCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
	   AddShopCard("Mark Card", MarkCardCost, MarkCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<MarkCard>(this->transformManager);
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
	   AddShopCard("Prefetch Card", PrefetchCardCost, PrefetchCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<PrefetchCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
	   AddShopCard("Overclock Card", OverclockCardCost, OverclockCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<OverclockCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
	   AddShopCard("Jumpstart Card", JumpstartCardCost, JumpstartCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<JumpstartCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
	   AddShopCard("Foresight Card", ForesightCardCost, ForesightCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<ForesightCard>(this->transformManager);
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
	   AddShopCard("Inferno Card", InfernoCardCost, InfernoCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<InfernoCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
	   AddShopCard("Terminate Card", TerminateCardCost, TerminateCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<TerminateCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
	   AddShopCard("Overdrive Card", OverdriveCardCost, OverdriveCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<OverdriveCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
	   AddShopCard("System Purge Card", SystemPurgeCardCost, SystemPurgeCard(this->transformManager).GetDescription(), [this]() {
			auto newCard = std::make_shared<SystemPurgeCard>(this->transformManager);
			this->player->AddCardToDeck(newCard->GetSaveID());
			});
   //    AddShopCard("Stun Card", StunCardCost, StunCard(this->transformManager).GetDescription(), [this]() {
			//auto newCard = std::make_shared<StunCard>(this->transformManager);
			//this->player->AddCardToDeck(newCard->GetSaveID());
			//});
		break;
	}
}