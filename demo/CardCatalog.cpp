#include "pch.h"
#include "CardCatalog.h"
#include "RNG.h"
#include <algorithm>
#include <array>
#include <unordered_map>

namespace Demo::CardCatalog {

	namespace {
		// Prices mirror CardShop::LoadItems. Only cards CardShop actually sells appear here
		// (StunCard's price is defined there but its row is commented out, so it is left out).
		const std::unordered_map<std::string, int>& PriceMap() {
			static const std::unordered_map<std::string, int> prices = {
				{ "StrikeCard",         30 },
				{ "TwinStrikeCard",     40 },
				{ "CleaveCard",         40 },
				{ "JabCard",            20 },
				{ "MarkCard",           30 },
				{ "BraceCard",          25 },
				{ "PrefetchCard",       35 },
				{ "OverclockCard",      35 },
				{ "HeavyStrikeCard",    70 },
				{ "PoisonCard",         50 },
				{ "VulnerableCard",     65 },
				{ "EnergyCard",         45 },
				{ "JumpstartCard",      55 },
				{ "ForesightCard",      70 },
				{ "RagingStrikeCard",   45 },
				{ "LethalHarvestCard",  55 },
				{ "ArmorPiercerCard",   50 },
				{ "IgniteCard",         60 },
				{ "ShieldBashCard",     70 },
				{ "WeaknessCard",       85 },
				{ "ChainLightningCard", 100 },
				{ "CruelStrikeCard",    75 },
				{ "FireDetonationCard", 90 },
				{ "ChainReactionCard",  120 },
				{ "TerminateCard",      150 },
				{ "InfernoCard",        140 },
				{ "SystemPurgeCard",    200 },
				{ "OverdriveCard",      180 },
				{ "ExecuteCard",        160 },
				{ "OverloadCard",       170 },
			};
			return prices;
		}

		// Relative odds per pull, indexed by Rarity. Sum is arbitrary.
		constexpr std::array<int, 4> kRarityWeights = { 55, 30, 12, 3 };
	}

	int GetPrice(const std::string& cardSaveID) {
		const auto& prices = PriceMap();
		auto it = prices.find(cardSaveID);
		return it != prices.end() ? it->second : 0;
	}

	Rarity RarityForPrice(int price) {
		if (price <= 40) return Rarity::Common;
		if (price <= 70) return Rarity::Uncommon;
		if (price <= 120) return Rarity::Rare;
		return Rarity::Legendary;
	}

	Rarity GetRarity(const std::string& cardSaveID) {
		return RarityForPrice(GetPrice(cardSaveID));
	}

	D3DCOLOR RarityColor(Rarity rarity) {
		switch (rarity) {
		case Rarity::Common:    return 0xFFB8B8B8; // grey
		case Rarity::Uncommon:  return 0xFF5CD65C; // green
		case Rarity::Rare:      return 0xFF4DA6FF; // blue
		case Rarity::Legendary: return 0xFFFFC64D; // gold
		}
		return 0xFFFFFFFF;
	}

	const wchar_t* RarityName(Rarity rarity) {
		switch (rarity) {
		case Rarity::Common:    return L"COMMON";
		case Rarity::Uncommon:  return L"UNCOMMON";
		case Rarity::Rare:      return L"RARE";
		case Rarity::Legendary: return L"LEGENDARY";
		}
		return L"";
	}

	const std::vector<std::string>& AllCards() {
		static const std::vector<std::string> all = []() {
			std::vector<std::string> v;
			for (const auto& [id, price] : PriceMap()) v.push_back(id);
			std::sort(v.begin(), v.end());
			return v;
		}();
		return all;
	}

	namespace {
		const std::vector<std::string>& CardsOfRarity(Rarity rarity) {
			static const std::array<std::vector<std::string>, 4> buckets = []() {
				std::array<std::vector<std::string>, 4> b;
				for (const auto& id : AllCards()) {
					b[static_cast<int>(GetRarity(id))].push_back(id);
				}
				return b;
			}();
			return buckets[static_cast<int>(rarity)];
		}

		Rarity RollRarity() {
			int total = 0;
			for (int w : kRarityWeights) total += w;
			int roll = RNG::Range(0, total - 1);
			for (int i = 0; i < 4; ++i) {
				if (roll < kRarityWeights[i]) return static_cast<Rarity>(i);
				roll -= kRarityWeights[i];
			}
			return Rarity::Common;
		}

		// Falls back to the next lower non-empty tier if a tier is somehow empty.
		const std::string& PickFromTier(Rarity rarity) {
			for (int r = static_cast<int>(rarity); r >= 0; --r) {
				const auto& pool = CardsOfRarity(static_cast<Rarity>(r));
				if (!pool.empty()) return pool[RNG::Range(0, static_cast<int>(pool.size()) - 1)];
			}
			// Every tier empty would mean the price map is empty; AllCards() guards against that.
			static const std::string fallback = "StrikeCard";
			return fallback;
		}
	}

	std::vector<std::string> RollPack(int count) {
		std::vector<std::string> pack;
		pack.reserve(count);
		for (int i = 0; i < count; ++i) {
			Rarity rarity = RollRarity();
			if (i == count - 1) {
				while (rarity == Rarity::Common) rarity = RollRarity();
			}
			pack.push_back(PickFromTier(rarity));
		}
		return pack;
	}
}
