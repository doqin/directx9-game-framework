#pragma once
#include "ICombatant.h"
#include "Player.h"
#include "IEnemy.h"
#include <unordered_map>
#include <memory>
#include <vector>

namespace Demo {

	struct VirtualCombatant {
		float health = 0.f;
		float maxHealth = 0.f;
		float block = 0.f;

		//modifiers
		float buffDamage = 0.f;
		float marked = 0.f;
		float burn = 0.f;
		float spark = 0.f;
		bool vulnerable = false;
		bool weak = false;
		int poisonDuration = 0;
		float poisonValue = 0.f;

		bool IsDead() const { return health <= 0.f; }
	};

	class VirtualBattleState {
	public:
		VirtualCombatant player;
		std::unordered_map<IEnemy*, VirtualCombatant> enemies;

		void Initialize(const std::shared_ptr<Player>& realPlayer, const std::vector<std::shared_ptr<IEnemy>>& realEnemies);

		float SimulateDamage(IEnemy* target, float baseDamage);
		void SimulateEnemyModifier(IEnemy* target, ModifierType type, float value, int duration);
		void SimulatePlayerHeal(float amount);

		//query function serves conditional cards(e.g., Chain Reaction)
		IEnemy* GetLowestHPEnemyAlive() const;
	};
}