#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "GameItems.h"
#include <vector>
#include <algorithm>

namespace Demo {
	//LOL enemy's ability makes me rebuilt combatant's turn and battle round
	enum class DamageType {
		Physical,
		Poison,
		Burn,
		Bleed
	};

	enum class TickPhase {
		StartOfTurn,
		EndOfTurn,
		EndOfRound
	};

	class ICombatant : public DX9GF::IGameObject {
	protected:
		float maxHealth;
		float health;
		float temporaryDefense = 0.f;
		std::vector<CombatModifier> modifiers;

		float CalculateActualDamage(float baseDamage);
		float CalculateOutgoingDamage(float baseDamage) const;

	public:
		ICombatant(std::weak_ptr<DX9GF::TransformManager> tm, float maxHealth, float x = 0, float y = 0, float rotation = 0, float scaleX = 1, float scaleY = 1)
			: IGameObject(tm, x, y, rotation, scaleX, scaleY), maxHealth(maxHealth), health(maxHealth) {
		}

		ICombatant(std::weak_ptr<DX9GF::TransformManager> tm, std::weak_ptr<DX9GF::IGameObject> parent, float maxHealth, float x = 0, float y = 0, float rotation = 0, float scaleX = 1, float scaleY = 1)
			: IGameObject(tm, parent, x, y, rotation, scaleX, scaleY), maxHealth(maxHealth), health(maxHealth) {
		}

		virtual ~ICombatant() = default;

		float GetMaxHealth() const { return maxHealth; }
		float GetHealth() const { return health; }
		void SetHealth(float hp) { health = (std::min)(maxHealth, (std::max)(0.f, hp)); }
		float GetTemporaryDefense() const { return temporaryDefense; }
		bool IsDead() const { return health <= 0.f; }

		virtual void Heal(float value);

		//split take damage into 2 type: physical, indirect
		virtual bool TakeDamage(float damage) = 0;
		virtual bool TakeIndirectDamage(float damage, DamageType type);

		virtual void TriggerEffects(TickPhase phase);
		virtual void TickDurations(TickPhase phase);

		void AddModifier(ModifierType type, int duration, float value, bool isBuff);
		bool HasModifier(ModifierType type) const;
		float GetModifierValue(ModifierType type) const;
		const std::vector<CombatModifier>& GetModifiers() const { return modifiers; }
	};
}