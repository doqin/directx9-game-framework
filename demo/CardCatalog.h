#pragma once
#include <d3d9.h>
#include <string>
#include <vector>

// Single source of truth for what a card is "worth". Prices are the ones CardShop sells each
// card at; rarity is derived purely from that price. Used by CardShop (to price its rows) and by
// the card-pack peddler (to weight a random pull and label the reveal).
namespace Demo::CardCatalog {
	enum class Rarity { Common, Uncommon, Rare, Legendary };

	// Fraction of a card's shop price returned when selling it back. Kept low on purpose: a card
	// pack costs 75G for 5 cards whose combined shop value averages far more than that, so a
	// generous buyback would let the player farm gold by buying packs and dumping the contents.
	// At this rate a whole pack's cards sell back for well under what the pack cost.
	constexpr float SELL_RATE = 0.20f;

	// Shop price for a card save-ID (e.g. "StrikeCard"). 0 if the card is not sold anywhere.
	int GetPrice(const std::string& cardSaveID);

	// Gold returned for selling one copy of the card. 0 if the card has no shop price (e.g.
	// starter cards), in which case the shop should not offer to buy it at all.
	int GetSellValue(const std::string& cardSaveID);

	Rarity RarityForPrice(int price);
	Rarity GetRarity(const std::string& cardSaveID);

	D3DCOLOR RarityColor(Rarity rarity);
	const wchar_t* RarityName(Rarity rarity);

	// Every card that has a price, sorted, deduplicated.
	const std::vector<std::string>& AllCards();

	// Draws `count` card save-IDs weighted by rarity (duplicates allowed, like a real pack).
	// The final card is guaranteed to be Uncommon or better.
	std::vector<std::string> RollPack(int count);
}
