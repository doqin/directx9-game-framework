#include "pch.h"
#include "AdvancedCards.h"

bool Demo::HeavyStrikeCard::Execute() {
	if (isDone) return true;
	if (!targets.empty()) {
		if (auto enemy = targets[0].lock()) {
			if (auto e = enemy->GetValue())
				if (owner) owner->DealDamage(e.get(), 12.f);
		}
	}
	isDone = true;
	return true;
}

void Demo::HeavyStrikeCard::Draw(unsigned long long deltaTime) {
	MultiTargetCard::Draw(deltaTime);
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!strikeTexture) {
		strikeTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		strikeTexture->LoadTexture(L"ui.png");
		strikeSprite = std::make_shared<DX9GF::StaticSprite>(strikeTexture.get());
		strikeSprite->SetSrcRect({ .left = 128, .top = 272, .right = 224, .bottom = 288 });
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
	MultiTargetCard::Draw(deltaTime);
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!strikeTexture) {
		strikeTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		strikeTexture->LoadTexture(L"ui.png");
		strikeSprite = std::make_shared<DX9GF::StaticSprite>(strikeTexture.get());
		strikeSprite->SetSrcRect({ .left = 128, .top = 288, .right = 224, .bottom = 304 });
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
				if (owner) owner->DealDamage(e.get(), 4.f);
		}
	}
	isDone = true;
	return true;
}

void Demo::CleaveCard::Draw(unsigned long long deltaTime) {
	MultiTargetCard::Draw(deltaTime);
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!strikeTexture) {
		strikeTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		strikeTexture->LoadTexture(L"ui.png");
		strikeSprite = std::make_shared<DX9GF::StaticSprite>(strikeTexture.get());
		strikeSprite->SetSrcRect({ .left = 0, .top = 304, .right = 80, .bottom = 320 });
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
}

bool Demo::ChainLightningCard::Execute() {
	if (isDone) return true;
	float damages[] = { 6.f, 4.f, 2.f };
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
	MultiTargetCard::Draw(deltaTime);
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!strikeTexture) {
		strikeTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		strikeTexture->LoadTexture(L"ui.png");
		strikeSprite = std::make_shared<DX9GF::StaticSprite>(strikeTexture.get());
		strikeSprite->SetSrcRect({ .left = 80, .top = 304, .right = 192, .bottom = 320 });
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
}

bool Demo::PoisonCard::Execute() {
	if (isDone) return true;
	if (!targets.empty()) {
		if (auto enemy = targets[0].lock()) {
          if (auto e = enemy->GetValue()) {
				const int poisonTurns = 3;
				e->ApplyStatus(StatusType::POISON, poisonTurns);
			}
		}
	}
	isDone = true;
	return true;
}

void Demo::PoisonCard::Draw(unsigned long long deltaTime) {
	MultiTargetCard::Draw(deltaTime);
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!strikeTexture) {
		strikeTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		strikeTexture->LoadTexture(L"ui.png");
		strikeSprite = std::make_shared<DX9GF::StaticSprite>(strikeTexture.get());
		strikeSprite->SetSrcRect({ .left = 192, .top = 304, .right = 272, .bottom = 320 });
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
}

bool Demo::VulnerableCard::Execute() {
	if (isDone) return true;
	if (!targets.empty()) {
		if (auto enemy = targets[0].lock()) {
			if (auto e = enemy->GetValue()) e->ApplyStatus(StatusType::VULNERABLE, 1);
		}
	}
	isDone = true;
	return true;
}

void Demo::VulnerableCard::Draw(unsigned long long deltaTime) {
	MultiTargetCard::Draw(deltaTime);
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!strikeTexture) {
		strikeTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		strikeTexture->LoadTexture(L"ui.png");
		strikeSprite = std::make_shared<DX9GF::StaticSprite>(strikeTexture.get());
		strikeSprite->SetSrcRect({ .left = 0, .top = 320, .right = 96, .bottom = 336 });
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
}

bool Demo::WeaknessCard::Execute() {
	if (isDone) return true;
	for (auto& wp : targets) {
		if (auto enemy = wp.lock()) {
			if (auto e = enemy->GetValue()) e->ApplyStatus(StatusType::WEAK, 2);
		}
	}
	isDone = true;
	return true;
}

void Demo::WeaknessCard::Draw(unsigned long long deltaTime) {
	MultiTargetCard::Draw(deltaTime);
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!strikeTexture) {
		strikeTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		strikeTexture->LoadTexture(L"ui.png");
		strikeSprite = std::make_shared<DX9GF::StaticSprite>(strikeTexture.get());
		strikeSprite->SetSrcRect({ .left = 96, .top = 320, .right = 192, .bottom = 336 });
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
}

bool Demo::StunCard::Execute() {
	if (isDone) return true;
	if (!targets.empty()) {
		if (auto enemy = targets[0].lock()) {
			if (auto e = enemy->GetValue()) e->ApplyStatus(StatusType::STUN, 1);
		}
	}
	isDone = true;
	return true;
}

void Demo::StunCard::Draw(unsigned long long deltaTime) {
	MultiTargetCard::Draw(deltaTime);
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!strikeTexture) {
		strikeTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		strikeTexture->LoadTexture(L"ui.png");
		strikeSprite = std::make_shared<DX9GF::StaticSprite>(strikeTexture.get());
		strikeSprite->SetSrcRect({ .left = 192, .top = 320, .right = 272, .bottom = 352 });
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
}
