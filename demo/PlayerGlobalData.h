#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "GameItems.h"
#include <string>
#include <vector>

namespace Demo {
	// Single source of truth for save-scoped player state (health, gold, deck, inventory).
	// Every scene constructs its own Player instance, and battles construct a further transient
	// battlePlayer on top of that - routing this state through one global instance means all of
	// them automatically share it, instead of needing to be manually copied around.
	class PlayerGlobalData : public DX9GF::ISaveable {
	private:
		float maxHealth = 50.f;
		float health = 50.f;
		int gold = 100;
		std::vector<std::string> deck;
		std::vector<std::string> inventoryCards;
		ItemInventory inventoryItems;

		PlayerGlobalData() { Reset(); }
	public:
		static PlayerGlobalData* GetInstance() {
			static PlayerGlobalData instance;
			return &instance;
		}

		void Reset();

		float GetHealth() const { return health; }
		float GetMaxHealth() const { return maxHealth; }
		void SetHealth(float hp) { health = (std::min)(maxHealth, (std::max)(0.f, hp)); }
		bool IsDead() const { return health <= 0.f; }
		float Heal(float value);

		int GetGold() const { return gold; }
		void SetGold(int amount) { gold = amount; }
		void AddGold(int amount) { gold += amount; }

		const std::vector<std::string>& GetDeck() const { return deck; }
		void AddCardToDeck(const std::string& card) { deck.push_back(card); }
		void ClearDeck() { deck.clear(); }

		const std::vector<std::string>& GetInventoryCards() const { return inventoryCards; }
		void AddCardToInventory(const std::string& card) { inventoryCards.push_back(card); }
		void ClearInventory() { inventoryCards.clear(); }

		ItemInventory& GetInventoryItems() { return inventoryItems; }

		std::string GetSaveID() const override;
		void GenerateSaveData(nlohmann::json& outData) override;
		void RestoreSaveData(const nlohmann::json& inData) override;
	};
}
