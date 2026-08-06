#include "pch.h"
#include "AdvancedCards.h"

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
