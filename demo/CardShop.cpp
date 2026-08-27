#include "pch.h"
#include "CardShop.h"
#include "StrikeCard.h"
#include "EnergyCard.h"
#include "AdvancedCards.h"
#include "UtilityCards.h"
#include "FinisherCards.h"

Demo::CardShop::CardShop(Game* game, Player* player, int sw, int sh, ShopTier tier)
	: IShopScene(game, player, sw, sh,
		tier == ShopTier::BASIC ? "BASIC CARD SHOP" :
		tier == ShopTier::HYBRID ? "HYBRID CARD SHOP" : "PREMIUM CARD SHOP"),
	currentTier(tier)
{
}

void Demo::CardShop::LoadItems()
{
	// Prices live in CardCatalog now; each row just names the card class and its label.
	switch (currentTier) {
	case ShopTier::BASIC:
		AddShopCard<StrikeCard>("Strike Card");
		AddShopCard<HeavyStrikeCard>("Heavy Strike Card");
		AddShopCard<PoisonCard>("Poison Card");
		AddShopCard<EnergyCard>("Energy Card");
		AddShopCard<JabCard>("Jab Card");
		AddShopCard<BraceCard>("Brace Card");
		AddShopCard<MarkCard>("Mark Card");
		AddShopCard<RagingStrikeCard>("Raging Strike Card");
		AddShopCard<ArmorPiercerCard>("Armor Piercer Card");
		AddShopCard<LethalHarvestCard>("Lethal Harvest Card");
		break;

	case ShopTier::HYBRID:
		AddShopCard<TwinStrikeCard>("Twin Strike Card");
		AddShopCard<CleaveCard>("Cleave Card");
		AddShopCard<VulnerableCard>("Vulnerable Card");
		AddShopCard<WeaknessCard>("Weakness Card");
		AddShopCard<PrefetchCard>("Prefetch Card");
		AddShopCard<OverclockCard>("Overclock Card");
		AddShopCard<JumpstartCard>("Jumpstart Card");
		AddShopCard<ForesightCard>("Foresight Card");
		// Ignite is the only source of Spark, so it and its detonator have to share a tier -
		// selling the payoff without the setup would put a dead card in the player's deck.
		AddShopCard<IgniteCard>("Ignite Card");
		AddShopCard<FireDetonationCard>("Fire Detonation Card");
		// Cruel Strike doubles off Weak, which this tier already sells.
		AddShopCard<CruelStrikeCard>("Cruel Strike Card");
		AddShopCard<ShieldBashCard>("Shield Bash Card");
		break;

	case ShopTier::PREMIUM:
		AddShopCard<CleaveCard>("Cleave Card");
		AddShopCard<ChainLightningCard>("Chain Lightning Card");
		AddShopCard<InfernoCard>("Inferno Card");
		AddShopCard<TerminateCard>("Terminate Card");
		AddShopCard<OverdriveCard>("Overdrive Card");
		AddShopCard<SystemPurgeCard>("System Purge Card");
		// These three only pay off against a built-out block - Overload scales with the
		// persistent cards installed in it, Chain Reaction with what resolved just before it,
		// and Execute wants the energy to spend in the first place.
		AddShopCard<ChainReactionCard>("Chain Reaction Card");
		AddShopCard<ExecuteCard>("Execute Card");
		AddShopCard<OverloadCard>("Overload Card");
		// AddShopCard<StunCard>("Stun Card");
		break;
	}
}
