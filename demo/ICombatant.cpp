#include "pch.h"
#include "ICombatant.h"
#include "DamageTextManager.h"
namespace Demo {
	void Demo::ICombatant::Heal(float value) {
		if (IsDead()) return;

		float actualHeal = value;
		if (health + value > maxHealth) {
			actualHeal = maxHealth - health;
		}

		health += value;
		if (health > maxHealth) health = maxHealth;

		if (actualHeal > 0) {
			SpawnHealText(actualHeal);
		}
	}

	void Demo::ICombatant::SpawnHealText(float actualHeal) {
		DamageTextManager::GetInstance()->Spawn(actualHeal, GetWorldX(), GetWorldY() - 40.f, TextType::Heal);
	}

	float ICombatant::CalculateActualDamage(float baseDamage, bool ignoreArmor) {
		// Marked is a flat bonus per hit, so it lands before Vulnerable scales the total and
		// before block eats into it.
		float finalDamage = baseDamage + GetModifierValue(ModifierType::Marked);
		if (HasModifier(ModifierType::Vulnerable)) {
			finalDamage *= 1.5f;
		}

		if (temporaryDefense > 0.f && !ignoreArmor) {
			float blockedDamage = (std::min)(temporaryDefense, finalDamage);
			temporaryDefense -= blockedDamage;
			finalDamage -= blockedDamage;

			float dmgToDeduct = blockedDamage;
			for (auto& mod : modifiers) {
				if (mod.type == ModifierType::BuffDefense && mod.value > 0.f) {
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

	void ICombatant::AddStackingModifier(ModifierType type, int duration, float value, bool isBuff, int delayTurns) {
		for (auto& mod : modifiers) {
			if (mod.type == type) {
				mod.value += value;
				mod.duration = (std::max)(mod.duration, duration);
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
			if (mod.type == type && mod.duration > 0) return true;
		}
		return false;
	}

	float ICombatant::GetModifierValue(ModifierType type) const {
		float val = 0.f;
		for (const auto& mod : modifiers) {
			if (mod.type == type) {
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
			if (it.duration > 0) {
				if (it.type == ModifierType::Poison && phase == TickPhase::EndOfTurn) {
					const float poisonDamage = (it.value > 0.f) ? it.value : static_cast<float>(it.duration);

					float tempDef = temporaryDefense;
					temporaryDefense = 0.f;

					int vulnDuration = 0;
					for (auto& m : modifiers) {
						if (m.type == ModifierType::Vulnerable) {
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
				// Burn and Regen need none of the dance above: TakeIndirectDamage already
				// bypasses block and modifiers, and Heal is unaffected by both.
				else if (it.type == ModifierType::Burn && phase == TickPhase::EndOfTurn) {
					this->TakeIndirectDamage(it.value, DamageType::Burn);
				}
				else if (it.type == ModifierType::Regen && phase == TickPhase::EndOfTurn) {
					this->Heal(it.value);
				}
			}
		}
	}

	void ICombatant::TickDurations(TickPhase phase) {
		if (phase != TickPhase::EndOfRound) return;

		for (auto it = modifiers.begin(); it != modifiers.end(); ) {
			// Grace turns are spent instead of duration, so a buff applied this round keeps every
			// turn it advertised - it is already in effect, it just does not age yet.
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

	float ICombatant::ConsumeModifier(ModifierType type) {
		float totalVal = 0.f;
		for (auto it = modifiers.begin(); it != modifiers.end(); ) {
			if (it->type == type) {
				totalVal += it->value;
				it = modifiers.erase(it);
			}
			else {
				++it;
			}
		}
		if (type == ModifierType::BuffDefense) {
			temporaryDefense = (std::max)(0.f, GetModifierValue(ModifierType::BuffDefense));
		}
		return totalVal;
	}

	float ICombatant::ConsumeAllArmor() {
		return ConsumeModifier(ModifierType::BuffDefense);
	}

	void ICombatant::ClearBuffs() {
		modifiers.erase(
			std::remove_if(modifiers.begin(), modifiers.end(),
				[](const CombatModifier& mod) { return mod.isBuff; }),
			modifiers.end());
	}
}