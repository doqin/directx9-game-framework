#include "pch.h"
#include "MainBlockCard.h"

void Demo::MainBlockCard::Draw(unsigned long long deltaTime)
{
    IBlockCard::Draw(deltaTime);
	if (!blockTexture) {
        blockTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		blockTexture->LoadTexture(L"ui.png");
        blockSprite = std::make_shared<DX9GF::StaticSprite>(blockTexture.get());
		blockSprite->SetSrcRect({ .left = 0, .top = 272, .right = 80, .bottom = 288 });
		pointerSprite = std::make_shared<DX9GF::StaticSprite>(blockTexture.get());
		pointerSprite->SetSrcRect({ .left = 128, .top = 0, .right = 144, .bottom = 16 });
        pointerSprite->SetScale(2.f, 2.f);
		pointerSprite->SetOrigin(8.f, 8.f);
    }
	blockSprite->Begin();
	blockSprite->SetPosition(GetWorldX(), GetWorldY());
	blockSprite->SetScale(2.f, 2.f);
	blockSprite->Draw(*camera, deltaTime);
	blockSprite->End();
    if (!nameFont) {
        nameFont = std::make_shared<DX9GF::Font>(graphicsDevice, L"StatusPlz", 16);
        nameFontSprite = std::make_shared<DX9GF::FontSprite>(nameFont.get());
        nameFontSprite->SetColor(0xFF000000);
    }
    //nameFontSprite->Begin();
    //nameFontSprite->SetPosition(GetWorldX() + 8.f, GetWorldY() + 8.f);
    //nameFontSprite->SetText(L"Main block");
    //nameFontSprite->Draw(*camera, deltaTime);
    if (auto currentCard = GetCurrentExecutingCard()) {
        pointerSprite->Begin();
        pointerSprite->SetPosition(currentCard->GetWorldX() - 32.f, currentCard->GetWorldY() + 16.f);
        pointerSprite->Draw(*camera, deltaTime);
        pointerSprite->End();
    }
    //nameFontSprite->End();
}
