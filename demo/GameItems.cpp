#include "pch.h"
#include "GameItems.h"
namespace Demo
{
	//register an item here
	void ItemData::LoadData()
	{
		itemRegistry[0] = ConsumableItem(0, L"Heal 20HP",
			L"Instant Heal 20HP",
			{ CombatModifier{ ModifierType::HealHP, 0, 20.0f, true, 0 } },
			{ 0, 0, 23, 35 });

		itemRegistry[1] = ConsumableItem(1, L"Buff 10 Defense",
			L"Buff 10 Defense for 2 turns",
			{ CombatModifier{ ModifierType::BuffDefense, 2, 10.0f, true, 1 } },
			{ 24, 0, 47, 35 });

		itemRegistry[2] = ConsumableItem(2, L"Buff 5 Damage",
			L"Buff 5 Damage for 1 turns",
			{ CombatModifier{ ModifierType::BuffDamage, 1, 5.0f, true, 1 } },
			{ 48, 0, 71, 35 });

		itemRegistry[3] = ConsumableItem(3, L"Buff 8 Damage & Heal 15HP",
			L"Instant Heal 15HP & Buff 8 Damage for 3 turns",
			{
				CombatModifier{ ModifierType::BuffDamage, 3, 8.0f, true, 1 },
				CombatModifier{ ModifierType::HealHP, 0, 15.0f, true, 0 }
			},
			{ 72, 0, 95, 35 });

		itemRegistry[4] = ConsumableItem(4, L"Buff 10 Defense & Heal 12HP",
			L"Instant Heal 12HP & Buff 10 Defense for 4 turns",
			{
				CombatModifier{ ModifierType::BuffDefense, 4, 10.0f, true, 1 },
				CombatModifier{ ModifierType::HealHP, 0, 12.0f, true, 0 }
			},
			{ 96, 0, 119, 35 });

		itemRegistry[5] = ConsumableItem(5, L"Mega Heal Elixir",
			L"Instant Heal 45HP",
			{ CombatModifier{ ModifierType::HealHP, 0, 45.0f, true, 0 } },
			{ 0, 36, 23, 71 });

		itemRegistry[6] = ConsumableItem(6, L"Iron Wall Shield",
			L"Buff 30.0 Defense for 2 turns",
			{ CombatModifier{ ModifierType::BuffDefense, 2, 30.0f, true, 1 } },
			{ 24, 36, 47, 71 });

		itemRegistry[7] = ConsumableItem(7, L"Berserker's Wrath",
			L"Buff 10 Damage for 1 turn",
			{ CombatModifier{ ModifierType::BuffDamage, 1, 10.0f, true, 1 } },
			{ 48, 36, 71, 71 });

		itemRegistry[8] = ConsumableItem(8, L"Paladin's Blessing",
			L"Instant Heal 10HP & Buff 12 Defense and Damage for 2 turns",
			{
				CombatModifier{ ModifierType::HealHP, 0, 10.0f, true, 0 },
				CombatModifier{ ModifierType::BuffDefense, 2, 12.0f, true, 1 },
				CombatModifier{ ModifierType::BuffDamage, 2, 12.0f, true, 1 }
			},
			{ 72, 36, 95, 71 });

		itemRegistry[9] = ConsumableItem(9, L"Titan's Resolve",
			L"Buff 20 Defense and 5 Damage for 3 turns",
			{
				CombatModifier{ ModifierType::BuffDefense, 3, 20.0f, true, 1 },
				CombatModifier{ ModifierType::BuffDamage, 3, 5.0f, true, 1 }
			},
			{ 96, 36, 119, 71 });

		itemRegistry[10] = ConsumableItem(99, L"Rusty Key", L"...", {}, { 120, 0, 143, 35 });
		itemRegistry[11] = ConsumableItem(99, L"G(r)ayStone", L"Has no practical use. Just a trophy for our winner.", {}, { 120, 36, 143, 71 });

		itemRegistry[12] = ConsumableItem(12, L"Authentication Token",
			L"A one-time access credential, pulled from a terminal in the alley.",
			{}, { 144, 0, 167, 35 });


		//TODO: Update gear rects here. Default size is 12x12. Assets can be 9x9 max, but please import 12x12 rects
		
		//gearRegistry[0] = { 0, L"Titan Core", L"Grants 20 Max HP.",
		//	{ {7,7,19,19},
		//	{40,7,52,19},
		//	{73,7,85,19},
		//	{107,7,119,19},
		//	{139,7,151,19},
		//	{172,7,184,19},
		//	{206,7,218,19},
		//	{239,7,251,19} },
		//	GearType::Passive, GearEffect::AddMaxHP, 20 };

		gearRegistry[0] = { 0, L"Titan Core", L"Grants 20 Max HP.", //red core
			{ {7,91,19,103} }, GearType::Passive, GearEffect::AddMaxHP, 20 };

		gearRegistry[1] = { 1, L"Energy Cell", L"Active: Restore 1 Energy.", //energy cell
			{ {7, 77, 19, 89} }, GearType::Active, GearEffect::AddEnergy, 1 };

		gearRegistry[2] = { 2, L"Data Extractor", L"Active: Draw 1 card.", //.rar
			{ {7, 21, 19, 33} }, GearType::Active, GearEffect::DrawCard, 1 };

		gearRegistry[3] = { 3, L"Memory Locker", L"Active: Retain 1 hand card.", //lock
			{ {7, 35, 19, 47} }, GearType::Active, GearEffect::RetainCard, 1 };

		gearRegistry[4] = { 4, L"Midas Chip", L"Gain extra Gold on win.", //cpu chip
			{ {7, 49, 19, 61} }, GearType::Passive, GearEffect::GoldBoost, 20 };

		gearRegistry[5] = { 5, L"Aegis Plating", L"Start combat with 15 Armor in 3 turns.", //actully a white plate
			{ {7, 63, 19, 75} }, GearType::Passive, GearEffect::StartArmor, 15 };
	}

	const ConsumableItem* ItemData::GetItemBlueprint(int id)
	{
		auto it = itemRegistry.find(id);
		if (it != itemRegistry.end())
		{
			return &(it->second);
		}
		return nullptr;
	}

	const GearBlueprint* ItemData::GetGearBlueprint(int id)
	{
		auto it = gearRegistry.find(id);
		if (it != gearRegistry.end())
		{
			return &(it->second);
		}
		return nullptr;
	}

	void ItemInventory::InitFixedInventory(int maxItemTypes)
	{
		slots.clear();
		slots.reserve(maxItemTypes);
		for (int i = 0; i < maxItemTypes; i++)
		{
			slots.push_back({ i, 0 });
		}
	}

	void ItemInventory::AddItem(int id, int amount)
	{
		if (id >= 0 && id < slots.size())
		{
			EnsureCapacity(id);
			slots[id].quantity += amount;
		}
	}

	bool ItemInventory::ConsumeItem(int id)
	{
		if (id >= 0 && id < slots.size() && slots[id].quantity > 0 && !IsItemLocked(id))
		{
			slots[id].quantity--;
			DX9GF::AudioManager::GetInstance()->PlayRandom("power_up", 0.2f);
			return true;
		}
		return false;
	}
	bool ItemInventory::RemoveItem(int id)
	{
		if (id >= 0 && id < slots.size() && slots[id].quantity > 0)
		{
			slots[id].quantity--;
			return true;
		}
		return false;
	}

	void ItemInventory::EnsureCapacity(int requiredID) {
		if (requiredID >= slots.size()) {
			int oldSize = slots.size();
			slots.resize(requiredID + 1);
			for (int i = oldSize; i <= requiredID; ++i) {
				slots[i] = { i, 0 };
			}
		}
	}
	bool ItemInventory::HasItem(int id) const {
		if (id >= 0 && id < slots.size()) {
			return slots[id].quantity > 0;
		}
		return false;
	}

	void ItemInventory::LockItem(int id, int turns)
	{
		if (turns <= 0) return;
		auto it = lockedItems.find(id);
		if (it == lockedItems.end() || it->second < turns) {
			lockedItems[id] = turns;
		}
	}

	bool ItemInventory::IsItemLocked(int id) const
	{
		return GetItemLockedTurns(id) > 0;
	}

	int ItemInventory::GetItemLockedTurns(int id) const
	{
		auto it = lockedItems.find(id);
		return (it != lockedItems.end()) ? it->second : 0;
	}

	void ItemInventory::TickLocks()
	{
		for (auto it = lockedItems.begin(); it != lockedItems.end(); ) {
			if (--(it->second) <= 0) it = lockedItems.erase(it);
			else ++it;
		}
	}
}
