#include "pch.h"
#include "UtilityCards.h"
#include "IBattleScene.h"
#include "VirtualBattleState.h"
// Faces live in the rows added below y = 336 in assets/ui.png. Each is a 16px strip drawn at 2x,
// so a card's dragArea in the header must be exactly twice its rect here.

bool Demo::JabCard::Execute() {
	if (isDone) return true;
	if (!targets.empty()) {
		if (auto enemyCard = targets[0].lock()) {
			if (auto enemy = enemyCard->GetValue())
				if (owner) owner->DealDamage(enemy.get(), 3.f);
		}
	}
	isDone = true;
	return true;
}

void Demo::JabCard::DrawCardFace(unsigned long long deltaTime) {
	DrawSheetFace(deltaTime, GetFaceRect());
}

bool Demo::MarkCard::Execute() {
	if (isDone) return true;
	if (!targets.empty()) {
		if (auto enemyCard = targets[0].lock()) {
			if (auto enemy = enemyCard->GetValue()) {
				enemy->AddStackingModifier(ModifierType::Marked, 2, 2.f, false);
			}
		}
	}
	isDone = true;
	return true;
}

void Demo::MarkCard::DrawCardFace(unsigned long long deltaTime) {
	DrawSheetFace(deltaTime, GetFaceRect());
}

bool Demo::BraceCard::Execute() {
	if (isDone) return true;
	if (owner) {
		// Stacking, so two Brace cards in one program give 10 block rather than 5 - AddModifier
		// would take max(value) and leave it at 5.
		owner->AddStackingModifier(ModifierType::BuffDefense, 1, 5.f, true);
		if (battleScene) battleScene->QueuePopUpMessage(L"+5 block");
	}
	isDone = true;
	return true;
}

void Demo::BraceCard::ResetExecution() {
	isDone = false;
}

void Demo::BraceCard::DrawCardFace(unsigned long long deltaTime) {
	DrawSheetFace(deltaTime, GetFaceRect());
}

bool Demo::PrefetchCard::Execute() {
	if (isDone) return true;
	if (battleScene) {
		battleScene->DrawCardsNow(2);
		battleScene->QueuePopUpMessage(L"Drew 2 cards");
	}
	isDone = true;
	return true;
}

void Demo::PrefetchCard::ResetExecution() {
	isDone = false;
}

void Demo::PrefetchCard::DrawCardFace(unsigned long long deltaTime) {
	DrawSheetFace(deltaTime, GetFaceRect());
}

bool Demo::OverclockCard::Execute() {
	if (isDone) return true;
	if (battleScene) {
		battleScene->GainEnergyNow(1);
		battleScene->QueuePopUpMessage(L"+1 energy");
	}
	if (owner) {
		owner->TakeIndirectDamage(4.f, DamageType::Physical);
	}
	isDone = true;
	return true;
}

void Demo::OverclockCard::ResetExecution() {
	isDone = false;
}

void Demo::OverclockCard::DrawCardFace(unsigned long long deltaTime) {
	DrawSheetFace(deltaTime, GetFaceRect());
}

bool Demo::JumpstartCard::Execute() {
	if (isDone) return true;
	if (battleScene) {
		battleScene->GainEnergyNow(2);
		battleScene->QueuePopUpMessage(L"+2 energy");
	}
	isDone = true;
	return true;
}

void Demo::JumpstartCard::ResetExecution() {
	isDone = false;
}

void Demo::JumpstartCard::DrawCardFace(unsigned long long deltaTime) {
	DrawSheetFace(deltaTime, GetFaceRect());
}

bool Demo::ForesightCard::Execute() {
	if (isDone) return true;
	if (battleScene) {
		battleScene->QueueBonusDraw(1);
		battleScene->QueuePopUpMessage(L"+1 card next turn");
	}
	isDone = true;
	return true;
}

void Demo::ForesightCard::ResetExecution() {
	isDone = false;
}

void Demo::ForesightCard::DrawCardFace(unsigned long long deltaTime) {
	DrawSheetFace(deltaTime, GetFaceRect());
}
