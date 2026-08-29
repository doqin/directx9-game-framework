#pragma once

// Single source of truth for what a consumable item is "worth". The value is the price the
// item shops sell it at; the sell-back price is a fixed fraction of that. Used by ItemShop
// (to price its buy rows) and by the item shop's sell mode.
//
// Keyed by the item's registry key (the id used by ItemInventory / ItemData::GetItemBlueprint),
// which for a couple of entries differs from ConsumableItem::GetID().
namespace Demo::ItemCatalog {

	// Fraction of an item's value returned when selling it back. Items have no gamble attached
	// (unlike card packs), so a straight half-price buyback is fine.
	constexpr float SELL_RATE = 0.5f;

	// Shop value for an item registry id. 0 if the item is not sold anywhere / has no value.
	int GetValue(int itemID);

	// True for items the player is allowed to sell back: the battle consumables only. Key items,
	// trophies and quest tokens are excluded so they can never be sold into a soft-lock.
	bool IsSellable(int itemID);

	// Gold returned for selling one copy of the item, or 0 if it cannot be sold.
	int GetSellValue(int itemID);
}
