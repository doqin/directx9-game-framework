#include "pch.h"
#include "ICombatant.h"

namespace Demo {
	void ICombatant::Heal(float value) {
		if (IsDead()) return;
		health += value;
		if (health > maxHealth) health = maxHealth;
	}

	float ICombatant::CalculateActualDamage(float baseDamage) {
		float finalDamage = baseDamage;
		if (HasModifier(ModifierType::Vulnerable)) {
			finalDamage *= 1.5f;
		}

		if (temporaryDefense > 0.f) {
			float blockedDamage = (std::min)(temporaryDefense, finalDamage);
			temporaryDefense -= blockedDamage;
			finalDamage -= blockedDamage;

			float dmgToDeduct = blockedDamage;
			for (auto& mod : modifiers) {
				//check delay
				if (mod.type == ModifierType::BuffDefense && mod.value > 0.f && mod.delayTurns <= 0) {
					float deduct = (std::min)(mod.value, dmgToDeduct);
					mod.value -= deduct;
					dmgToDeduct -= deduct;
					if (dmgToDeduct <= 0.f) break;
				}
			}
		}

		return finalDamage;
	}

	float ICombatant::CalculateOutgoingDamage(float baseDamage) const {
		float finalDamage = baseDamage + GetModifierValue(ModifierType::BuffDamage);
		if (HasModifier(ModifierType::Weak)) {
			finalDamage *= 0.75f;
		}
		return finalDamage;
	}

	void ICombatant::AddModifier(ModifierType type, int duration, float value, bool isBuff, int delayTurns) {
		for (auto& mod : modifiers) {
			if (mod.type == type) {
				mod.duration += duration;
				mod.value = (std::max)(mod.value, value);
				if (type == ModifierType::BuffDefense) {
					temporaryDefense = (std::max)(0.f, GetModifierValue(ModifierType::BuffDefense));
				}
				return;
			}
		}
		modifiers.push_back({ type, duration, value, isBuff, delayTurns });
		if (type == ModifierType::BuffDefense) {
			temporaryDefense = (std::max)(0.f, GetModifierValue(ModifierType::BuffDefense));
		}
	}

	bool ICombatant::HasModifier(ModifierType type) const {
		for (const auto& mod : modifiers) {
			if (mod.type == type && mod.duration > 0 && mod.delayTurns <= 0) return true;
		}
		return false;
	}

	float ICombatant::GetModifierValue(ModifierType type) const {
		float val = 0.f;
		for (const auto& mod : modifiers) {
			if (mod.type == type && mod.delayTurns <= 0) {
				val += mod.value;
			}
		}
		return val;
	}

	bool ICombatant::TakeIndirectDamage(float damage, DamageType type) {
		health -= damage;
		if (health < 0) health = 0;
		return IsDead();
	}

	void ICombatant::TriggerEffects(TickPhase phase) {
		for (auto& it : modifiers) {
			if (it.duration > 0 && it.delayTurns <= 0) {
				if (it.type == ModifierType::Poison && phase == TickPhase::EndOfTurn) {
					const float poisonDamage = (it.value > 0.f) ? it.value : static_cast<float>(it.duration);

					float tempDef = temporaryDefense;
					temporaryDefense = 0.f;

					int vulnDuration = 0;
					for (auto& m : modifiers) {
						if (m.type == ModifierType::Vulnerable && m.delayTurns <= 0) {
							vulnDuration = m.duration;
							m.duration = 0;
						}
					}

					this->TakeIndirectDamage(poisonDamage, DamageType::Poison);

					temporaryDefense = tempDef;
					for (auto& m : modifiers) {
						if (m.type == ModifierType::Vulnerable && vulnDuration > 0) {
							m.duration = vulnDuration;
						}
					}
				}
			}
		}
	}

	void ICombatant::TickDurations(TickPhase phase) {
		if (phase != TickPhase::EndOfRound) return;

		for (auto it = modifiers.begin(); it != modifiers.end(); ) {
			if (it->delayTurns > 0) {
				it->delayTurns--;
				++it;
				continue;
			}

			it->duration--;

			if (it->duration <= 0 || (it->type == ModifierType::BuffDefense && it->value <= 0.f)) {
				it = modifiers.erase(it);
			}
			else {
				++it;
			}
		}
		temporaryDefense = (std::max)(0.f, GetModifierValue(ModifierType::BuffDefense));
	}
}