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
	const int RagingStrikeCardCost = 45;
	const int LethalHarvestCardCost = 55;
	const int ArmorPiercerCardCost = 50;
	const int IgniteCardCost = 60;
	const int ShieldBashCardCost = 70;
	const int CruelStrikeCardCost = 75;
	const int FireDetonationCardCost = 90;
	const int ChainReactionCardCost = 120;
	const int ExecuteCardCost = 160;
	const int OverloadCardCost = 170;
	switch (currentTier) {
	case ShopTier::BASIC:
		AddShopCard<StrikeCard>("Strike Card", StrikeCardCost);
		AddShopCard<HeavyStrikeCard>("Heavy Strike Card", HeavyStrikeCardCost);
		AddShopCard<PoisonCard>("Poison Card", PoisonCardCost);
		AddShopCard<EnergyCard>("Energy Card", EnergyCardCost);
		AddShopCard<JabCard>("Jab Card", JabCardCost);
		AddShopCard<BraceCard>("Brace Card", BraceCardCost);
		AddShopCard<MarkCard>("Mark Card", MarkCardCost);
		AddShopCard<RagingStrikeCard>("Raging Strike Card", RagingStrikeCardCost);
		AddShopCard<ArmorPiercerCard>("Armor Piercer Card", ArmorPiercerCardCost);
		AddShopCard<LethalHarvestCard>("Lethal Harvest Card", LethalHarvestCardCost);
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
		// Ignite is the only source of Spark, so it and its detonator have to share a tier -
		// selling the payoff without the setup would put a dead card in the player's deck.
		AddShopCard<IgniteCard>("Ignite Card", IgniteCardCost);
		AddShopCard<FireDetonationCard>("Fire Detonation Card", FireDetonationCardCost);
		// Cruel Strike doubles off Weak, which this tier already sells.
		AddShopCard<CruelStrikeCard>("Cruel Strike Card", CruelStrikeCardCost);
		AddShopCard<ShieldBashCard>("Shield Bash Card", ShieldBashCardCost);
		break;

	case ShopTier::PREMIUM:
		AddShopCard<CleaveCard>("Cleave Card", CleaveCardCost);
		AddShopCard<ChainLightningCard>("Chain Lightning Card", ChainLightningCardCost);
		AddShopCard<InfernoCard>("Inferno Card", InfernoCardCost);
		AddShopCard<TerminateCard>("Terminate Card", TerminateCardCost);
		AddShopCard<OverdriveCard>("Overdrive Card", OverdriveCardCost);
		AddShopCard<SystemPurgeCard>("System Purge Card", SystemPurgeCardCost);
		// These three only pay off against a built-out block - Overload scales with the
		// persistent cards installed in it, Chain Reaction with what resolved just before it,
		// and Execute wants the energy to spend in the first place.
		AddShopCard<ChainReactionCard>("Chain Reaction Card", ChainReactionCardCost);
		AddShopCard<ExecuteCard>("Execute Card", ExecuteCardCost);
		AddShopCard<OverloadCard>("Overload Card", OverloadCardCost);
		// AddShopCard<StunCard>("Stun Card", StunCardCost);
		break;
	}
}
