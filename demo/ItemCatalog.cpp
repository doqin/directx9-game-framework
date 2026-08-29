#include "pch.h"
#include "ItemCatalog.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace Demo::ItemCatalog {

	namespace {
		// Values mirror the prices ItemShop::LoadItems sells each item at. Ids 0-9 are the battle
		// consumables; 10 is the Rusty Key (buyable in the RK_HYBRID shop but never sellable).
		const std::unordered_map<int, int>& ValueMap() {
			static const std::unordered_map<int, int> values = {
				{ 0,  30 },
				{ 1,  30 },
				{ 2,  40 },
				{ 3,  50 },
				{ 4,  50 },
				{ 5,  50 },
				{ 6,  100 },
				{ 7,  100 },
				{ 8,  200 },
				{ 9,  150 },
				{ 10, 100 },
			};
			return values;
		}
	}

	int GetValue(int itemID) {
		const auto& values = ValueMap();
		auto it = values.find(itemID);
		return it != values.end() ? it->second : 0;
	}

	bool IsSellable(int itemID) {
		return itemID >= 0 && itemID <= 9;
	}

	int GetSellValue(int itemID) {
		if (!IsSellable(itemID)) return 0;
		const int value = GetValue(itemID);
		if (value <= 0) return 0;
		return (std::max)(1, static_cast<int>(std::lround(value * SELL_RATE)));
	}
}
