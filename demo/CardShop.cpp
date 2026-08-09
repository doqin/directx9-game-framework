#include "pch.h"
#include "CardShop.h"
#include "StrikeCard.h"
#include "EnergyCard.h"
#include "AdvancedCards.h"
#include "UtilityCards.h"
#include "FinisherCards.h"

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
		AddShopCard<StrikeCard>("Strike Card", StrikeCardCost);
		AddShopCard<HeavyStrikeCard>("Heavy Strike Card", HeavyStrikeCardCost);
		AddShopCard<PoisonCard>("Poison Card", PoisonCardCost);
		AddShopCard<EnergyCard>("Energy Card", EnergyCardCost);
		AddShopCard<JabCard>("Jab Card", JabCardCost);
		AddShopCard<BraceCard>("Brace Card", BraceCardCost);
		AddShopCard<MarkCard>("Mark Card", MarkCardCost);
		break;

	case ShopTier::HYBRID:
		AddShopCard<TwinStrikeCard>("Twin Strike Card", TwinStrikeCardCost);
		AddShopCard<CleaveCard>("Cleave Card", CleaveCardCost);
		AddShopCard<VulnerableCard>("Vulnerable Card", VulnerableCardCost);
		AddShopCard<WeaknessCard>("Weakness Card", WeaknessCardCost);
		AddShopCard<PrefetchCard>("Prefetch Card", PrefetchCardCost);
		AddShopCard<OverclockCard>("Overclock Card", OverclockCardCost);
		AddShopCard<JumpstartCard>("Jumpstart Card", JumpstartCardCost);
		AddShopCard<ForesightCard>("Foresight Card", ForesightCardCost);
		break;

	case ShopTier::PREMIUM:
		AddShopCard<CleaveCard>("Cleave Card", CleaveCardCost);
		AddShopCard<ChainLightningCard>("Chain Lightning Card", ChainLightningCardCost);
		AddShopCard<InfernoCard>("Inferno Card", InfernoCardCost);
		AddShopCard<TerminateCard>("Terminate Card", TerminateCardCost);
		AddShopCard<OverdriveCard>("Overdrive Card", OverdriveCardCost);
		AddShopCard<SystemPurgeCard>("System Purge Card", SystemPurgeCardCost);
		// AddShopCard<StunCard>("Stun Card", StunCardCost);
		break;
	}
}
