#pragma once
#include "DX9GF.h"
namespace Demo
{
	enum class ItemBuffType
	{
		HealHP,
		BuffDamage,
		BuffDefense
		//...Add more if you have ideas
	};

	struct ActiveBuff
	{
		ItemBuffType type;
		float value;
		int turnsLeft;
		bool isNewlyAdded = true; //got trouble with turns, use item cost 1 turn so I use this flag to prevent "Turn Eater"
	};

	class ConsumableItem
	{
	private:
		int id;
		std::wstring name;
		std::wstring description;
		std::vector<ActiveBuff> buffs;
		RECT itemRect;
	public:
		ConsumableItem(int _id = -1, std::wstring _name = L"NULL", std::wstring _desc = L"", std::vector<ActiveBuff> _buffs = {}, RECT _itemRect = RECT{ 0,0,0,0 })
			: id(_id), name(_name), description(_desc), buffs(_buffs), itemRect(_itemRect) {
		}

		int GetID() const { return id; }
		const std::wstring& GetName() const { return name; }
		const std::wstring& GetDescription() const { return description; }
		const std::vector<ActiveBuff>& GetBuffs() const { return buffs; }
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
		void AddItem(int id, int amount);
		bool ConsumeItem(int id);
		void Clear() { for (auto& slot : slots) slot.quantity = 0; }
	};
}

