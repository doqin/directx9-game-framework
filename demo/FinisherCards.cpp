#include "pch.h"
#include "FinisherCards.h"
#include "IBattleScene.h"

// Faces live in the rows added below y = 336 in assets/ui.png, as in UtilityCards.cpp.

bool Demo::TerminateCard::Execute() {
	if (isDone) return true;
	if (!targets.empty()) {
		if (auto enemyCard = targets[0].lock()) {
			if (auto enemy = enemyCard->GetValue()) {
				const float maxHealth = enemy->GetMaxHealth();
				const bool finishable = maxHealth > 0.f && (enemy->GetHealth() / maxHealth) < 0.4f;
				if (owner) owner->DealDamage(enemy.get(), finishable ? 60.f : 30.f);
			}
		}
	}
	isDone = true;
	return true;
}

void Demo::TerminateCard::CollectProjectedSteps(std::vector<ProjectedStep>& out) {
	if (targets.empty()) return;
	if (auto enemyCard = targets[0].lock()) {
		if (auto enemy = enemyCard->GetValue()) {
			// Read off the enemy's health as it stands now. If something earlier in the program
			// drops it under the threshold this will read low, but resolving that would mean
			// simulating health across the whole queue.
			const float maxHealth = enemy->GetMaxHealth();
			const bool finishable = maxHealth > 0.f && (enemy->GetHealth() / maxHealth) < 0.4f;
			out.push_back(ProjectedStep::Hit(enemy.get(), finishable ? 60.f : 30.f));
		}
	}
}

void Demo::TerminateCard::DrawCardFace(unsigned long long deltaTime) {
	DrawSheetFace(deltaTime, GetFaceRect());
}

bool Demo::InfernoCard::Execute() {
	if (isDone) return true;
	if (battleScene && owner) {
		for (auto& enemy : battleScene->GetEnemies()) {
			if (!enemy || enemy->IsDead()) continue;
			owner->DealDamage(enemy.get(), 8.f);
			// Skip the burn if that killed it - a dead enemy lingers in the list until
			// CollectDeadEnemies runs, and there is no point ticking damage on it.
			if (!enemy->IsDead()) {
				enemy->AddStackingModifier(ModifierType::Burn, 3, 5.f, false);
			}
		}
	}
	isDone = true;
	return true;
}

void Demo::InfernoCard::CollectProjectedSteps(std::vector<ProjectedStep>& out) {
	if (!battleScene) return;
	for (auto& enemy : battleScene->GetEnemies()) {
		if (!enemy || enemy->IsDead()) continue;
		out.push_back(ProjectedStep::Hit(enemy.get(), 8.f));
		// Execute skips the burn when the hit already killed the enemy. The readout does not model
		// deaths, so it counts the burn either way - it can read high on a lethal hit.
		out.push_back(ProjectedStep::EnemyEffect(enemy.get(), ModifierType::Burn, 5.f, 3));
	}
}

void Demo::InfernoCard::ResetExecution() {
	isDone = false;
}

void Demo::InfernoCard::DrawCardFace(unsigned long long deltaTime) {
	DrawSheetFace(deltaTime, GetFaceRect());
}

bool Demo::SystemPurgeCard::Execute() {
	if (isDone) return true;
	if (battleScene && owner) {
		for (auto& enemy : battleScene->GetEnemies()) {
			if (!enemy || enemy->IsDead()) continue;
			owner->DealDamage(enemy.get(), 16.f);
			if (!enemy->IsDead()) {
				enemy->AddModifier(ModifierType::Stun, 1, 0.f, false);
			}
		}
		battleScene->QueuePopUpMessage(L"System purged");
	}
	isDone = true;
	return true;
}

void Demo::SystemPurgeCard::CollectProjectedSteps(std::vector<ProjectedStep>& out) {
	if (!battleScene) return;
	for (auto& enemy : battleScene->GetEnemies()) {
		if (!enemy || enemy->IsDead()) continue;
		out.push_back(ProjectedStep::Hit(enemy.get(), 16.f));
	}
}

void Demo::SystemPurgeCard::ResetExecution() {
	isDone = false;
}

void Demo::SystemPurgeCard::DrawCardFace(unsigned long long deltaTime) {
	DrawSheetFace(deltaTime, GetFaceRect());
}

bool Demo::OverdriveCard::Execute() {
	if (isDone) return true;
	if (owner) {
		owner->AddStackingModifier(ModifierType::BuffDamage, 3, 4.f, true);
		owner->AddStackingModifier(ModifierType::Regen, 3, 3.f, true);
		if (battleScene) battleScene->QueuePopUpMessage(L"Overdrive!");
	}
	isDone = true;
	return true;
}

void Demo::OverdriveCard::ResetExecution() {
	isDone = false;
}

void Demo::OverdriveCard::DrawCardFace(unsigned long long deltaTime) {
	DrawSheetFace(deltaTime, GetFaceRect());
}
