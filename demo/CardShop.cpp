#include "pch.h"
#include "CardShop.h"
#include "StrikeCard.h"
#include "EnergyCard.h"
#include "AdvancedCards.h"
#include "UtilityCards.h"
#include "FinisherCards.h"
#include "ICard.h"
#include <cctype>
#include <map>

namespace {
	// "HeavyStrikeCard" -> "Heavy Strike". Only used for the sell row's status message; the row
	// itself shows the card's face art, same as the buy rows.
	std::wstring PrettyCardName(const std::string& saveId)
	{
		std::string s = saveId;
		const std::string suffix = "Card";
		if (s.size() > suffix.size() &&
			s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0) {
			s.erase(s.size() - suffix.size());
		}
		std::wstring out;
		for (size_t i = 0; i < s.size(); ++i) {
			const unsigned char c = static_cast<unsigned char>(s[i]);
			if (i > 0 && std::isupper(c) && !std::isupper(static_cast<unsigned char>(s[i - 1]))) {
				out += L' ';
			}
			out += static_cast<wchar_t>(c);
		}
		return out;
	}
}

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

void Demo::CardShop::LoadSellItems()
{
	// Only loose cards (the inventory, not the battle deck) can be sold. Cards are grouped by
	// type so a stack of three shows as one row with an "x3" count.
	std::map<std::string, int> counts;
	for (const auto& id : player->GetInventoryCards()) {
		counts[id]++;
	}

	for (const auto& [id, count] : counts) {
		// GetSellValue is a small fraction of the card's shop price, deliberately low so a card
		// pack (75G for 5) can never be milked for gold by dumping its contents here.
		const int sellValue = CardCatalog::GetSellValue(id);
		if (sellValue <= 0) continue; // starter cards and anything with no shop price

		auto proto = ICard::CreateCard(id, transformManager);
		if (!proto) continue;

		const std::string sellId = id;
		ShopItem row;
		row.name = PrettyCardName(id);
		row.cost = sellValue;
		row.description = proto->GetDescription();
		row.iconRect = proto->GetFaceRect();
		row.iconSheet = ShopIconSheet::CardFaces;
		row.isPersistent = proto->IsPersistent();
		row.hasLimitedUses = proto->HasLimitedUses();
		row.maxUses = proto->HasLimitedUses() ? proto->GetMaxUses() : 0;
		row.stackCount = count;
		row.onBuyAction = [this, sellId]() { this->player->RemoveCardFromInventory(sellId); };
		itemsToSell.push_back(std::move(row));
	}
}
