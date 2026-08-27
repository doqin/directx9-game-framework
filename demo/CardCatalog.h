#pragma once
#include <d3d9.h>
#include <string>
#include <vector>

// Single source of truth for what a card is "worth". Prices are the ones CardShop sells each
// card at; rarity is derived purely from that price. Used by CardShop (to price its rows) and by
// the card-pack peddler (to weight a random pull and label the reveal).
namespace Demo::CardCatalog {
	enum class Rarity { Common, Uncommon, Rare, Legendary };

	// Shop price for a card save-ID (e.g. "StrikeCard"). 0 if the card is not sold anywhere.
	int GetPrice(const std::string& cardSaveID);

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
