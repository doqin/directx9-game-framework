#include "pch.h"
#include "VirtualBattleState.h"
#include <algorithm>

namespace Demo {

	void VirtualBattleState::Initialize(const std::shared_ptr<Player>& realPlayer, const std::vector<std::shared_ptr<IEnemy>>& realEnemies) {
		enemies.clear();

		if (realPlayer) {
			player.health = realPlayer->GetHealth();
			player.maxHealth = realPlayer->GetMaxHealth();
			player.block = realPlayer->GetTemporaryDefense();
			player.buffDamage = realPlayer->GetModifierValue(ModifierType::BuffDamage);
			player.weak = realPlayer->HasModifier(ModifierType::Weak);
		}

		for (const auto& enemy : realEnemies) {
			if (!enemy || enemy->IsDead()) continue;

			VirtualCombatant vc;
			vc.health = enemy->GetHealth();
			vc.maxHealth = enemy->GetMaxHealth();
			vc.block = enemy->GetTemporaryDefense();

			vc.marked = enemy->GetModifierValue(ModifierType::Marked);
			vc.burn = enemy->GetModifierValue(ModifierType::Burn);
			vc.spark = enemy->GetModifierValue(ModifierType::Spark);
			vc.vulnerable = enemy->HasModifier(ModifierType::Vulnerable);
			vc.weak = enemy->HasModifier(ModifierType::Weak);

			for (const auto& mod : enemy->GetModifiers()) {
				if (mod.type == ModifierType::Poison && mod.duration > 0) {
					vc.poisonDuration += mod.duration;
					vc.poisonValue = (std::max)(vc.poisonValue, mod.value);
				}
			}
			enemies[enemy.get()] = vc;
		}
	}

	float VirtualBattleState::SimulateDamage(IEnemy* target, float baseDamage) {
		auto it = enemies.find(target);
		if (it == enemies.end() || it->second.IsDead()) return 0.f;

		VirtualCombatant& vc = it->second;

		float finalDamage = baseDamage + player.buffDamage;
		if (player.weak) finalDamage *= 0.75f;

		finalDamage += vc.marked;
		if (vc.vulnerable) finalDamage *= 1.5f;

		float absorbed = (std::min)(vc.block, finalDamage);
		vc.block -= absorbed;
		finalDamage -= absorbed;

		vc.health -= finalDamage;

		return finalDamage;
	}

	void VirtualBattleState::SimulateEnemyModifier(IEnemy* target, ModifierType type, float value, int duration) {
		auto it = enemies.find(target);
		if (it == enemies.end() || it->second.IsDead()) return;

		VirtualCombatant& vc = it->second;
		switch (type) {
		case ModifierType::Marked: vc.marked += value; break;
		case ModifierType::Vulnerable: vc.vulnerable = true; break;
		case ModifierType::Burn: vc.burn += value; break;
		case ModifierType::Spark: vc.spark += value; break;
		case ModifierType::BuffDefense: vc.block += value; break;
		case ModifierType::Weak: vc.weak = true; break;
		case ModifierType::Stun: break;
		case ModifierType::Poison:
			vc.poisonDuration += duration;
			vc.poisonValue = (std::max)(vc.poisonValue, value);
			break;
		default: break;
		}
	}

	void VirtualBattleState::SimulatePlayerHeal(float amount) {
		player.health += amount;
		if (player.health > player.maxHealth) {
			player.health = player.maxHealth;
		}
	}

	IEnemy* VirtualBattleState::GetLowestHPEnemyAlive() const {
		IEnemy* lowestEnemy = nullptr;
		float lowestHP = 999999.f;

		for (const auto& [enemyPtr, vc] : enemies) {
			if (!vc.IsDead() && vc.health < lowestHP) {
				lowestHP = vc.health;
				lowestEnemy = enemyPtr;
			}
		}
		return lowestEnemy;
	}
}