#include "pch.h"
#include "PlayerGlobalData.h"

void Demo::PlayerGlobalData::Reset() {
	maxHealth = 50.f;
	health = maxHealth;
	gold = 100;
	deck = { "StrikeCard", "StrikeCard", "StrikeCard", "TwinStrikeCard", "TwinStrikeCard" };
	inventoryCards.clear();
	inventoryItems = ItemInventory();
	inventoryItems.InitFixedInventory(13);
}

float Demo::PlayerGlobalData::Heal(float value) {
	if (IsDead()) return 0.f;

	float actualHeal = value;
	if (health + value > maxHealth) {
		actualHeal = maxHealth - health;
	}

	SetHealth(health + value);
	return actualHeal;
}

std::string Demo::PlayerGlobalData::GetSaveID() const {
	return "PlayerGlobalData";
}

void Demo::PlayerGlobalData::GenerateSaveData(nlohmann::json& outData) {
	outData["gold"] = gold;
	outData["health"] = health;
	outData["deck"] = nlohmann::json::array();
	for (auto& card : deck) {
		outData["deck"].push_back(card);
	}
	outData["inventoryCards"] = nlohmann::json::array();
	for (auto& card : inventoryCards) {
		outData["inventoryCards"].push_back(card);
	}
	auto inventorySlots = inventoryItems.GetSlots();
	for (size_t i = 0; i < inventorySlots.size(); i++) {
		outData["inventoryItems"][i]["id"] = inventorySlots[i].itemID;
		outData["inventoryItems"][i]["quantity"] = inventorySlots[i].quantity;
	}
}

void Demo::PlayerGlobalData::RestoreSaveData(const nlohmann::json& inData) {
	if (inData.contains("gold")) gold = inData["gold"];
	if (inData.contains("health")) health = inData["health"];
	if (inData.contains("deck")) {
		deck.clear();
		for (auto& item : inData["deck"]) {
			deck.push_back(item.get<std::string>());
		}
	}
	if (inData.contains("inventoryCards")) {
		inventoryCards.clear();
		for (auto& item : inData["inventoryCards"]) {
			inventoryCards.push_back(item.get<std::string>());
		}
	}
	if (inData.contains("inventoryItems")) {
		inventoryItems.Clear();
		for (auto& item : inData["inventoryItems"]) {
			int id = item["id"];
			int quantity = item["quantity"];
			inventoryItems.AddItem(id, quantity);
		}
	}
}
