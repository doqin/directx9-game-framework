#include "pch.h"
#include "EnergyCard.h"
#include "IBattleScene.h"

bool Demo::EnergyCard::Execute()
{
	if (isDone) {
		return true;
	}
	if (battleScene) {
		battleScene->QueueBonusEnergy(1);
		battleScene->QueuePopUpMessage(L"+1 energy next turn");
	}
	isDone = true;
	return true;
}

void Demo::EnergyCard::ResetExecution()
{
	isDone = false;
}

void Demo::EnergyCard::DrawCardFace(unsigned long long deltaTime)
{
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!cardTexture) {
		cardTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		cardTexture->LoadTexture(L"assets/ui.png");
		cardSprite = std::make_shared<DX9GF::StaticSprite>(cardTexture.get());
		cardSprite->SetSrcRect(GetFaceRect());
	}
	if (cardSprite) {
		cardSprite->Begin();
		cardSprite->SetPosition(GetWorldX(), GetWorldY());
		cardSprite->SetScale(2.f, 2.f);
		cardSprite->Draw(*camera, deltaTime);
		cardSprite->End();
	}
	if (isCropped) {
		graphicsDevice->SetScissorTest(false);
	}
}
