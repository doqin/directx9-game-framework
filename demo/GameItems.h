#pragma once
#include "DX9GF.h"
namespace Demo
{

	enum class ModifierType {
		HealHP,
		BuffDamage,
		BuffDefense,
		Poison,
		Vulnerable,
		Weak,
		Stun,
		// Flat damage per turn that ignores block. Unlike Poison, its damage does not decay with
		// its remaining duration, and re-applying it accumulates value instead of replacing it.
		Burn,
		// Flat bonus to every point of incoming damage, applied before Vulnerable's multiplier.
		// The debuff mirror of BuffDamage.
		Marked,
		// Heals value at end of turn.
		Regen,
		//For ignite and destonate combo
		Spark,
		Freeze
		//...Add more if you have ideas
	};

	struct CombatModifier {
		ModifierType type;
		int duration;
		float value;
		bool isBuff;
		int delayTurns = 0;  //got trouble with turns, use item cost 1 turn so I use this flag to prevent "Turn Eater"
	};

	class ConsumableItem
	{
	private:
		int id;
		std::wstring name;
		std::wstring description;
		std::vector<CombatModifier> modifiers; 
		RECT itemRect;
	public:
		ConsumableItem(int _id = -1, std::wstring _name = L"NULL", std::wstring _desc = L"", std::vector<CombatModifier> _modifiers = {}, RECT _itemRect = RECT{ 0,0,0,0 })
			: id(_id), name(_name), description(_desc), modifiers(_modifiers), itemRect(_itemRect) {
		}

		int GetID() const { return id; }
		const std::wstring& GetName() const { return name; }
		const std::wstring& GetDescription() const { return description; }
		const std::vector<CombatModifier>& GetModifiers() const { return modifiers; }
		RECT GetItemRect() const { return itemRect; }
	};

	class ItemData
	{
	private:
		std::unordered_map<int, ConsumableItem> itemRegistry;
		ItemData() {}
	public:
		static ItemData* GetInstance()
		{
			static ItemData instance;
			return &instance;
		}

		void LoadData();
		const ConsumableItem* GetItemBlueprint(int id);
	};

	struct ItemInventorySlot
	{
		int itemID;
		int quantity;
	};

	class ItemInventory
	{
	private:
		std::vector <ItemInventorySlot> slots;
	public:
		const std::vector<ItemInventorySlot>& GetSlots() const { return slots; }
		void InitFixedInventory(int maxItemTypes);
		void EnsureCapacity(int requiredID);
		bool HasItem(int id) const;
		void AddItem(int id, int amount);
		bool ConsumeItem(int id);
		void Clear() { for (auto& slot : slots) slot.quantity = 0; }
	};
}

