#include "pch.h"
#include "EnemyCard.h"
#include "IContainer.h"

std::shared_ptr<Demo::IEnemy> Demo::EnemyCard::GetValue()
{
	return enemy;
}

void Demo::EnemyCard::Draw(unsigned long long deltaTime)
{
	IExpressionCard::Draw(deltaTime);
	if (!uiTexture) {
		uiTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		uiTexture->LoadTexture(L"ui.png");
		untargetedSprite = std::make_shared<DX9GF::StaticSprite>(uiTexture.get());
		untargetedSprite->SetSrcRect({ .left = 80, .top = 272, .right = 128, .bottom = 288 });
		untargetedSprite->SetScale(2.f, 2.f);
		targetedSprite = std::make_shared<DX9GF::StaticSprite>(uiTexture.get());
		targetedSprite->SetSrcRect({ .left = 80, .top = 288, .right = 128, .bottom = 304 });
		targetedSprite->SetScale(2.f, 2.f);
		targetSprite = std::make_shared<DX9GF::StaticSprite>(uiTexture.get());
		targetSprite->SetSrcRect({ .left = 128, .top = 16, .right = 144, .bottom = 48 });
		targetSprite->SetScale(2.f, 2.f);
		targetSprite->SetOrigin(8.f, 16.f);
		arrowTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		arrowTexture->LoadTexture(L"arrow.png");
		arrowSprite = std::make_shared<DX9GF::StaticSprite>(arrowTexture.get());
		arrowSprite->SetScale(2.f);
		auto [w, h] = arrowTexture->GetSize();
		arrowSprite->SetOrigin(w / 2.0f, h / 2.0f);
	}
	if (!nameFont) {
		nameFont = std::make_shared<DX9GF::Font>(graphicsDevice, L"StatusPlz", 16);
		nameFontSprite = std::make_shared<DX9GF::FontSprite>(nameFont.get());
		nameFontSprite->SetColor(0xFF000000);
	}
	nameFontSprite->Begin();
	nameFontSprite->SetPosition(GetWorldX() + 8.f, GetWorldY() + 8.f);
	nameFontSprite->SetText(L"EnemyCard");
	nameFontSprite->Draw(*camera, deltaTime);
	nameFontSprite->End();

	bool isAttachedToContainer = false;
	if (auto p = GetParent()) {
		if (auto lock = p->lock()) {
			if (std::dynamic_pointer_cast<IDraggable>(lock)) {
				isAttachedToContainer = true;
			}
		}
	}

	if (isAttachedToContainer) {
		targetedSprite->Begin();
		targetedSprite->SetPosition(GetWorldX(), GetWorldY());
		targetedSprite->Draw(*camera, deltaTime);
		targetedSprite->End();
	} else {
		untargetedSprite->Begin();
		untargetedSprite->SetPosition(GetWorldX(), GetWorldY());
		untargetedSprite->Draw(*camera, deltaTime);
		untargetedSprite->End();
	}

	const float x = GetWorldX() + GetWidth() * 3.f / 4.f;
	const float y = GetWorldY() + GetHeight() / 2.f;
	const float enemyX = enemy->GetWorldX();
	const float enemyY = enemy->GetWorldY();
	float dx = enemyX - x;
	float dy = enemyY - y;
	if (trigger->IsHovering(deltaTime)) {
		//graphicsDevice->DrawLine(*camera, x, y, enemyX, enemyY, 0xFF000000, 8.0f);
		//graphicsDevice->DrawLine(*camera, x, y, enemyX, enemyY, 0xFFFFFFFF, 6.0f);
		//graphicsDevice->DrawLine(*camera, x, y, enemyX, enemyY, 0xFFB4202A, 4.0f);
		auto targetX = enemyX - 64.f;
		auto targetY = enemyY;
		targetSprite->Begin();
		targetSprite->SetPosition(targetX, targetY);
		targetSprite->Draw(*camera, deltaTime);
		targetSprite->End();
	}
	float angle = atan2(dy, dx);
	arrowSprite->SetPosition(x, y);
	arrowSprite->SetRotation(angle);

	arrowSprite->Begin();
	arrowSprite->Draw(*camera, deltaTime);
	arrowSprite->End();
}

bool Demo::EnemyCard::OnDrop(std::shared_ptr<IDraggable> draggable)
{
	return false;
}
