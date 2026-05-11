#include "pch.h"
#include "StrikeCard.h"

bool Demo::StrikeCard::OnDrop(std::shared_ptr<IDraggable> other)
{
	auto incomingEnemyCard = std::dynamic_pointer_cast<EnemyCard>(other);
	if (!incomingEnemyCard || enemyCard.lock()) {
		return false;
	}
	auto [thisX, thisY] = this->GetWorldPosition();
	auto [otherX, otherY] = other->GetWorldPosition();
	otherY += other->GetTrigger().lock()->GetHeight() / 2.0f;
	if (otherX > thisX
		&& otherX < thisX + dragAreaWidth
		&& otherY > thisY
		&& otherY < thisY + dragAreaHeight) {
		other->SetParent(shared_from_this());
		other->SetLocalPosition(dragAreaWidth, 0);
		enemyCard = incomingEnemyCard;
		return true;
	}
	return false;
}

bool Demo::StrikeCard::Execute()
{
	if (isDone) {
		return true;
	}
	if (auto lock = enemyCard.lock()) {
		if (auto enemy = lock->GetValue()) {
			if (owner) owner->DealDamage(enemy.get(), 5.f);
		}
	}
	isDone = true;
	return true;
}

void Demo::StrikeCard::ResetExecution()
{
	isDone = false;
}

void Demo::StrikeCard::Update(unsigned long long deltaTime)
{
	IDraggable::Update(deltaTime);
	if (auto lock = enemyCard.lock()) {
		if (auto parent = lock->GetParent(); parent.has_value()) {
			if (auto parentLock = parent.value().lock()) {
				if (parentLock.get() == this) {
					return;
				}
			}
		}
	}
	enemyCard.reset();
}

void Demo::StrikeCard::Draw(unsigned long long deltaTime)
{
	IStatementCard::Draw(deltaTime);
	if (!strikeTexture) {
		strikeTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		strikeTexture->LoadTexture(L"ui.png");
		strikeSprite = std::make_shared<DX9GF::StaticSprite>(strikeTexture.get());
		strikeSprite->SetSrcRect({ .left = 0, .top = 288, .right = 80, .bottom = 304 });
	}
	if (strikeSprite) {
		strikeSprite->Begin();
		strikeSprite->SetPosition(GetWorldX(), GetWorldY());
		strikeSprite->SetScale(2.f, 2.f);
		strikeSprite->Draw(*camera, deltaTime);
		strikeSprite->End();
	}
	//if (!nameFont) {
	//	nameFont = std::make_shared<DX9GF::Font>(graphicsDevice, L"StatusPlz", 16);
	//	nameFontSprite = std::make_shared<DX9GF::FontSprite>(nameFont.get());
	//	nameFontSprite->SetColor(0xFF000000);
	//}
	//nameFontSprite->Begin();
	//nameFontSprite->SetPosition(GetWorldX() + 8.f, GetWorldY() + 8.f);
	//nameFontSprite->SetText(L"StrikeCard");
	//nameFontSprite->Draw(*camera, deltaTime);
	//nameFontSprite->End();
}

size_t Demo::StrikeCard::GetCost() const
{
	return 1;
}

std::wstring Demo::StrikeCard::GetDescription() const
{
	return L"Deal 5 damage to an enemy.";
}

std::wstring Demo::StrikeCard::GetInputsDescription() const
{
	if (enemyCard.lock()) {
		return L"1/1 Enemy";
	}
	return L"0/1 Enemy";
}

size_t Demo::StrikeCard::GetWidth() const
{
	if (auto lock = enemyCard.lock()) {
		return IDraggable::GetWidth() + lock->GetWidth();
	}
	else {
		return IDraggable::GetWidth();
	}
}