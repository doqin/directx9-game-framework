#include "pch.h"
#include "AdvancedCards.h"
#include "IBlockCard.h"
#include "IBattleScene.h"
#include "VirtualBattleState.h"
bool Demo::HeavyStrikeCard::Execute() {
	if (isDone) return true;
	if (!targets.empty()) {
		if (auto enemy = targets[0].lock()) {
			if (auto e = enemy->GetValue())
				if (owner) owner->DealDamage(e.get(), 16.f);
		}
	}
	isDone = true;
	return true;
}

void Demo::HeavyStrikeCard::Draw(unsigned long long deltaTime) {
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!strikeTexture) {
		strikeTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		strikeTexture->LoadTexture(L"assets/ui.png");
		strikeSprite = std::make_shared<DX9GF::StaticSprite>(strikeTexture.get());
		strikeSprite->SetSrcRect(GetFaceRect());
	}
	if (strikeSprite) {
		strikeSprite->Begin();
		strikeSprite->SetPosition(GetWorldX(), GetWorldY());
		strikeSprite->SetScale(2.f, 2.f);
		strikeSprite->Draw(*camera, deltaTime);
		strikeSprite->End();
	}
	if (isCropped) {
		graphicsDevice->SetScissorTest(false);
	}
	MultiTargetCard::Draw(deltaTime);
}

bool Demo::TwinStrikeCard::Execute() {
	if (isDone) return true;
	if (!targets.empty()) {
		if (auto enemy = targets[0].lock()) {
			if (auto e = enemy->GetValue())
				if (owner) owner->DealDamage(e.get(), 3.f);
			hits++;
		}
	}
	if (hits >= 2 || targets.empty()) {
		isDone = true;
		return true;
	}
	return false;
}

void Demo::TwinStrikeCard::Draw(unsigned long long deltaTime) {
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!strikeTexture) {
		strikeTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		strikeTexture->LoadTexture(L"assets/ui.png");
		strikeSprite = std::make_shared<DX9GF::StaticSprite>(strikeTexture.get());
		strikeSprite->SetSrcRect(GetFaceRect());
	}
	if (strikeSprite) {
		strikeSprite->Begin();
		strikeSprite->SetPosition(GetWorldX(), GetWorldY());
		strikeSprite->SetScale(2.f, 2.f);
		strikeSprite->Draw(*camera, deltaTime);
		strikeSprite->End();
	}
	if (isCropped) {
		graphicsDevice->SetScissorTest(false);
	}
	MultiTargetCard::Draw(deltaTime);
}

void Demo::TwinStrikeCard::ResetExecution() {
	MultiTargetCard::ResetExecution();
	hits = 0;
}

bool Demo::CleaveCard::Execute() {
	if (isDone) return true;
	for (auto& wp : targets) {
		if (auto enemy = wp.lock()) {
			if (auto e = enemy->GetValue())
				if (owner) owner->DealDamage(e.get(), 7.f);
		}
	}
	isDone = true;
	return true;
}

void Demo::CleaveCard::Draw(unsigned long long deltaTime) {
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!strikeTexture) {
		strikeTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		strikeTexture->LoadTexture(L"assets/ui.png");
		strikeSprite = std::make_shared<DX9GF::StaticSprite>(strikeTexture.get());
		strikeSprite->SetSrcRect(GetFaceRect());
	}
	if (strikeSprite) {
		strikeSprite->Begin();
		strikeSprite->SetPosition(GetWorldX(), GetWorldY());
		strikeSprite->SetScale(2.f, 2.f);
		strikeSprite->Draw(*camera, deltaTime);
		strikeSprite->End();
	}
	if (isCropped) {
		graphicsDevice->SetScissorTest(false);
	}
	MultiTargetCard::Draw(deltaTime);
}

bool Demo::ChainLightningCard::Execute() {
	if (isDone) return true;
	float damages[] = { 10.f, 10.f, 10.f };
	for (size_t i = 0; i < targets.size() && i < 3; ++i) {
		if (auto enemy = targets[i].lock()) {
			if (auto e = enemy->GetValue())
				if (owner) owner->DealDamage(e.get(), damages[i]);
		}
	}
	isDone = true;
	return true;
}

void Demo::ChainLightningCard::Draw(unsigned long long deltaTime) {
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!strikeTexture) {
		strikeTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		strikeTexture->LoadTexture(L"assets/ui.png");
		strikeSprite = std::make_shared<DX9GF::StaticSprite>(strikeTexture.get());
		strikeSprite->SetSrcRect(GetFaceRect());
	}
	if (strikeSprite) {
		strikeSprite->Begin();
		strikeSprite->SetPosition(GetWorldX(), GetWorldY());
		strikeSprite->SetScale(2.f, 2.f);
		strikeSprite->Draw(*camera, deltaTime);
		strikeSprite->End();
	}
	if (isCropped) {
		graphicsDevice->SetScissorTest(false);
	}
	MultiTargetCard::Draw(deltaTime);
}

bool Demo::PoisonCard::Execute() {
	if (isDone) return true;
	if (!targets.empty()) {
		if (auto enemy = targets[0].lock()) {
          if (auto e = enemy->GetValue()) {
				const int poisonTurns = 3;
				e->AddModifier(ModifierType::Poison, poisonTurns, 0.f, false);
		  }
		}
	}
	isDone = true;
	return true;
}

void Demo::PoisonCard::Draw(unsigned long long deltaTime) {
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!strikeTexture) {
		strikeTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		strikeTexture->LoadTexture(L"assets/ui.png");
		strikeSprite = std::make_shared<DX9GF::StaticSprite>(strikeTexture.get());
		strikeSprite->SetSrcRect(GetFaceRect());
	}
	if (strikeSprite) {
		strikeSprite->Begin();
		strikeSprite->SetPosition(GetWorldX(), GetWorldY());
		strikeSprite->SetScale(2.f, 2.f);
		strikeSprite->Draw(*camera, deltaTime);
		strikeSprite->End();
	}
	if (isCropped) {
		graphicsDevice->SetScissorTest(false);
	}
	MultiTargetCard::Draw(deltaTime);
}

bool Demo::VulnerableCard::Execute() {
	if (isDone) return true;
	if (!targets.empty()) {
		if (auto enemy = targets[0].lock()) {
			if (auto e = enemy->GetValue()) e->AddModifier(ModifierType::Vulnerable, 1, 0.f, false);
		}
	}
	isDone = true;
	return true;
}

void Demo::VulnerableCard::DrawCardFace(unsigned long long deltaTime)
{
	DrawSheetFace(deltaTime, GetFaceRect());
}

bool Demo::WeaknessCard::Execute() {
	if (isDone) return true;
	for (auto& wp : targets) {
		if (auto enemy = wp.lock()) {
			if (auto e = enemy->GetValue()) e->AddModifier(ModifierType::Weak, 2, 0.f, false);
		}
	}
	isDone = true;
	return true;
}

void Demo::WeaknessCard::DrawCardFace(unsigned long long deltaTime) {
	DrawSheetFace(deltaTime, GetFaceRect());
}

bool Demo::StunCard::Execute() {
	if (isDone) return true;
	if (!targets.empty()) {
		if (auto enemy = targets[0].lock()) {
			if (auto e = enemy->GetValue()) e->AddModifier(ModifierType::Stun, 1, 0.f, false);
		}
	}
	isDone = true;
	return true;
}

void Demo::StunCard::Draw(unsigned long long deltaTime) {
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!strikeTexture) {
		strikeTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		strikeTexture->LoadTexture(L"assets/ui.png");
		strikeSprite = std::make_shared<DX9GF::StaticSprite>(strikeTexture.get());
		strikeSprite->SetSrcRect(GetFaceRect());
	}
	if (strikeSprite) {
		strikeSprite->Begin();
		strikeSprite->SetPosition(GetWorldX(), GetWorldY());
		strikeSprite->SetScale(2.f, 2.f);
		strikeSprite->Draw(*camera, deltaTime);
		strikeSprite->End();
	}
	if (isCropped) {
		graphicsDevice->SetScissorTest(false);
	}
	MultiTargetCard::Draw(deltaTime);
}

bool Demo::IgniteCard::Execute() {
	if (isDone) return true;
	if (targets.empty()) { isDone = true; return true; }
	auto target = targets[0].lock();
	if (!target || !target->GetValue() || target->GetValue()->IsDead()) { isDone = true; return true; }

	target->GetValue()->AddStackingModifier(ModifierType::Spark, 3, 2.f, false);
	isDone = true;
	return true;
}

bool Demo::FireDetonationCard::Execute() {
	if (isDone) return true;
	if (targets.empty()) { isDone = true; return true; }
	auto target = targets[0].lock();
	if (!target || !target->GetValue() || target->GetValue()->IsDead()) { isDone = true; return true; }

	auto enemy = target->GetValue();
	float sparkStacks = enemy->ConsumeModifier(ModifierType::Spark);

	float finalDamage = 3.f + (sparkStacks * 5.f);

	if (owner) {
		owner->DealDamage(enemy.get(), finalDamage);
	}
	isDone = true;
	return true;
}
void Demo::FireDetonationCard::CollectProjectedSteps(VirtualBattleState& state) {
	if (targets.empty()) return;
	auto target = targets[0].lock();
	if (!target || !target->GetValue()) return;

	auto enemy = target->GetValue().get();
	auto it = state.enemies.find(enemy);
	if (it != state.enemies.end()) {
		float currentSpark = it->second.spark;
		float finalDamage = 3.f + (currentSpark * 5.f);
		it->second.spark = 0.f;
		state.SimulateDamage(enemy, finalDamage);
	}
}

bool Demo::RagingStrikeCard::Execute() {
	if (isDone) return true;
	if (targets.empty()) { isDone = true; return true; }
	auto target = targets[0].lock();
	if (!target || !target->GetValue() || target->GetValue()->IsDead()) { isDone = true; return true; }

	if (owner) {
		owner->DealDamage(target->GetValue().get(), static_cast<float>(currentDamage));
	}

	currentDamage += 3;
	hasExecutedThisCycle = true;

	isDone = true;
	return true;
}
bool Demo::OverloadCard::Execute() {
	if (isDone) return true;
	if (targets.empty()) { isDone = true; return true; }
	auto target = targets[0].lock();
	if (!target || !target->GetValue() || target->GetValue()->IsDead()) { isDone = true; return true; }

	float bonusDamage = 0.f;
	auto parentBlock = std::dynamic_pointer_cast<IBlockCard>(GetParent().has_value() ? GetParent().value().lock() : nullptr);

	if (parentBlock) {
		for (const auto& wpCard : parentBlock->GetStatementCards()) {
			if (auto card = wpCard.lock()) {
				if (card->IsPersistent()) {
					bonusDamage += 4.f;
				}
			}
		}
	}

	float finalDamage = 5.f + bonusDamage;
	if (owner) {
		owner->DealDamage(target->GetValue().get(), finalDamage);
	}
	isDone = true;
	return true;
}

void Demo::OverloadCard::CollectProjectedSteps(VirtualBattleState& state) {
	float bonusDamage = 0.f;
	auto parentBlock = std::dynamic_pointer_cast<IBlockCard>(GetParent().has_value() ? GetParent().value().lock() : nullptr);
	if (parentBlock) {
		for (const auto& wpCard : parentBlock->GetStatementCards()) {
			if (auto card = wpCard.lock()) {
				if (card->IsPersistent()) bonusDamage += 4.f;
			}
		}
	}
	CollectHitsOnTargets(state, 5.f + bonusDamage, 1);
}

void Demo::OverloadCard::DrawCardFace(unsigned long long deltaTime) {
	DrawSheetFace(deltaTime, GetFaceRect());
}

bool Demo::ChainReactionCard::Execute() {
	if (isDone) return true;
	if (targets.empty()) { isDone = true; return true; }
	auto originalTarget = targets[0].lock();
	if (!originalTarget || !originalTarget->GetValue()) { isDone = true; return true; }

	float finalDamage = 4.f;
	IEnemy* finalTargetEnemy = originalTarget->GetValue().get();

	auto parentBlock = std::dynamic_pointer_cast<IBlockCard>(GetParent().has_value() ? GetParent().value().lock() : nullptr);
	if (parentBlock) {
		const auto& cardsInBlock = parentBlock->GetStatementCards();
		size_t myIndex = -1;

		for (size_t i = 0; i < cardsInBlock.size(); ++i) {
			if (cardsInBlock[i].lock().get() == this) {
				myIndex = i;
				break;
			}
		}

		if (myIndex > 0 && myIndex < cardsInBlock.size()) {
			auto prevCard = std::dynamic_pointer_cast<MultiTargetCard>(cardsInBlock[myIndex - 1].lock());
			if (prevCard) {
				bool previousKilledTarget = false;

				for (auto& prevWpTarget : prevCard->GetTargets()) {
					if (auto prevTarget = prevWpTarget.lock()) {
						if (auto prevEnemy = prevTarget->GetValue()) {
							if (prevEnemy->IsDead()) {
								previousKilledTarget = true;
								break;
							}
						}
					}
				}

				if (previousKilledTarget) {
					finalDamage = 8.f;
					if (battleScene) {
						float lowestHP = 999999.f;
						IEnemy* lowestEnemy = nullptr;

						for (auto& enemy : battleScene->GetEnemies()) {
							if (!enemy->IsDead() && enemy->GetHealth() < lowestHP) {
								lowestHP = enemy->GetHealth();
								lowestEnemy = enemy.get();
							}
						}
						if (lowestEnemy) {
							finalTargetEnemy = lowestEnemy;
						}
					}
				}
			}
		}
	}

	if (finalTargetEnemy && !finalTargetEnemy->IsDead() && owner) {
		owner->DealDamage(finalTargetEnemy, finalDamage);
	}
	isDone = true;
	return true;
}

void Demo::ChainReactionCard::CollectProjectedSteps(VirtualBattleState& state) {
	if (targets.empty()) return;
	auto originalTarget = targets[0].lock();
	if (!originalTarget || !originalTarget->GetValue()) return;

	float finalDamage = 4.f;
	IEnemy* finalTargetEnemy = originalTarget->GetValue().get();

	auto parentBlock = std::dynamic_pointer_cast<IBlockCard>(GetParent().has_value() ? GetParent().value().lock() : nullptr);
	if (parentBlock) {
		const auto& cardsInBlock = parentBlock->GetStatementCards();
		size_t myIndex = -1;
		for (size_t i = 0; i < cardsInBlock.size(); ++i) {
			if (cardsInBlock[i].lock().get() == this) {
				myIndex = i;
				break;
			}
		}

		if (myIndex > 0 && myIndex < cardsInBlock.size()) {
			auto prevCard = std::dynamic_pointer_cast<MultiTargetCard>(cardsInBlock[myIndex - 1].lock());
			if (prevCard) {
				bool previousKilledTarget = false;
				for (auto& prevWpTarget : prevCard->GetTargets()) {
					if (auto prevTarget = prevWpTarget.lock()) {
						if (auto prevEnemy = prevTarget->GetValue()) {
							if (state.enemies[prevEnemy.get()].IsDead()) {
								previousKilledTarget = true;
								break;
							}
						}
					}
				}

				if (previousKilledTarget) {
					finalDamage = 8.f;
					IEnemy* lowestEnemy = state.GetLowestHPEnemyAlive();
					if (lowestEnemy) {
						finalTargetEnemy = lowestEnemy;
					}
				}
			}
		}
	}

	state.SimulateDamage(finalTargetEnemy, finalDamage);
}
void Demo::ChainReactionCard::DrawCardFace(unsigned long long deltaTime) {
	DrawSheetFace(deltaTime, GetFaceRect());
}

bool Demo::LethalHarvestCard::Execute() {
	if (isDone) return true;
	if (targets.empty()) { isDone = true; return true; }
	auto target = targets[0].lock();
	if (!target || !target->GetValue() || target->GetValue()->IsDead()) { isDone = true; return true; }

	auto enemy = target->GetValue().get();
	if (owner) {
		owner->DealDamage(enemy, 6.f);
		if (enemy->IsDead()) {
			owner->Heal(8.f);
		}
	}
	isDone = true;
	return true;
}

bool Demo::ExecuteCard::Execute() {
	if (isDone) return true;
	if (targets.empty()) { isDone = true; return true; }
	auto target = targets[0].lock();
	if (!target || !target->GetValue() || target->GetValue()->IsDead()) { isDone = true; return true; }

	auto enemy = target->GetValue().get();
	if (owner) {
		owner->DealDamage(enemy, 15.f);
		if (enemy->IsDead() && battleScene) {
			battleScene->QueueBonusEnergy(1);
		}
	}
	isDone = true;
	return true;
}

bool Demo::ArmorPiercerCard::Execute() {
	if (isDone) return true;
	if (targets.empty()) { isDone = true; return true; }
	auto target = targets[0].lock();
	if (!target || !target->GetValue() || target->GetValue()->IsDead()) { isDone = true; return true; }

	auto enemy = target->GetValue().get();
	float finalDamage = 5.f;

	if (enemy->GetModifierValue(ModifierType::BuffDefense) > 0.f) {
		finalDamage *= 2.f;
	}

	if (owner) {
		owner->DealDamage(enemy, finalDamage);
	}
	isDone = true;
	return true;
}

void Demo::ArmorPiercerCard::CollectProjectedSteps(VirtualBattleState& state) {
	float dmg = 5.f;
	if (!targets.empty()) {
		if (auto target = targets[0].lock()) {
			if (auto enemy = target->GetValue()) {
				if (state.enemies[enemy.get()].block > 0.f) dmg *= 2.f;
			}
		}
	}
	CollectHitsOnTargets(state, dmg, 1);
}
bool Demo::CruelStrikeCard::Execute() {
	if (isDone) return true;
	if (targets.empty()) { isDone = true; return true; }
	auto target = targets[0].lock();
	if (!target || !target->GetValue() || target->GetValue()->IsDead()) { isDone = true; return true; }

	auto enemy = target->GetValue().get();
	float finalDamage = 8.f;

	if (enemy->HasModifier(ModifierType::Weak)) {
		finalDamage *= 2.f;
	}

	if (owner) {
		owner->DealDamage(enemy, finalDamage);
	}
	isDone = true;
	return true;
}

void Demo::CruelStrikeCard::CollectProjectedSteps(VirtualBattleState& state) {
	float dmg = 8.f;
	if (!targets.empty()) {
		if (auto target = targets[0].lock()) {
			if (auto enemy = target->GetValue()) {
				if (state.enemies[enemy.get()].weak) dmg *= 2.f;
			}
		}
	}
	CollectHitsOnTargets(state, dmg, 1);
}

bool Demo::ShieldBashCard::Execute() {
	if (isDone) return true;
	if (targets.empty()) { isDone = true; return true; }
	auto target = targets[0].lock();
	if (!target || !target->GetValue() || target->GetValue()->IsDead()) { isDone = true; return true; }

	auto enemy = target->GetValue().get();

	if (owner) {
		float consumedArmor = owner->ConsumeModifier(ModifierType::BuffDefense);

		float finalDamage = consumedArmor * 1.5f;

		owner->DealDamage(enemy, finalDamage);
	}
	isDone = true;
	return true;
}

void Demo::ShieldBashCard::CollectProjectedSteps(VirtualBattleState& state) {
	float expectedDmg = state.player.block * 1.5f;
	state.player.block = 0.f;

	CollectHitsOnTargets(state, expectedDmg, 1);
}