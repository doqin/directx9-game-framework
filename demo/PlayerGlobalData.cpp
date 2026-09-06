#include "pch.h"
#include "PlayerGlobalData.h"

float Demo::PlayerGlobalData::GetMaxHealth() const {
	float total = baseMaxHealth;
	if (equippedGearID != -1) {
		auto* gear = ItemData::GetInstance()->GetGearBlueprint(equippedGearID);
		if (gear && gear->effect == GearEffect::AddMaxHP) {
			total += gear->effectValue;
		}
	}
	return total;
}
void Demo::PlayerGlobalData::EquipGear(int gearID) {
	auto it = std::find(inventoryGears.begin(), inventoryGears.end(), gearID);
	if (it == inventoryGears.end()) return;

	int oldGear = equippedGearID;

	inventoryGears.erase(it);
	equippedGearID = gearID;

	if (oldGear != -1) {
		inventoryGears.push_back(oldGear);
	}

	SetHealth(health);
}

void Demo::PlayerGlobalData::UnequipGear() {
	if (equippedGearID == -1) return;

	inventoryGears.push_back(equippedGearID);
	equippedGearID = -1;

	SetHealth(health);
}
void Demo::PlayerGlobalData::Reset() {
	baseMaxHealth = 50.f;
	health = baseMaxHealth;
	gold = 100;
	deck = { "StrikeCard", "StrikeCard", "StrikeCard", "TwinStrikeCard", "TwinStrikeCard" };
	inventoryCards.clear();
	inventoryItems = ItemInventory();
	inventoryItems.InitFixedInventory(13);
	inventoryGears.clear();
	equippedGearID = -1;
}

float Demo::PlayerGlobalData::Heal(float value) {
	if (IsDead()) return 0.f;

	float currentMaxHealth = GetMaxHealth();
	float actualHeal = value;
	if (health + value > currentMaxHealth) {
		actualHeal = currentMaxHealth - health;
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
	outData["equippedGearID"] = equippedGearID;

	outData["inventoryGears"] = nlohmann::json::array();
	for (int gear : inventoryGears) {
		outData["inventoryGears"].push_back(gear);
	}

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
	if (inData.contains("equippedGearID")) equippedGearID = inData["equippedGearID"];
	if (inData.contains("inventoryGears")) {
		inventoryGears.clear();
		for (auto& item : inData["inventoryGears"]) {
			inventoryGears.push_back(item.get<int>());
		}
	}
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
