#include "pch.h"
#include "OutroScene.h"

void Demo::OutroScene::Init() {
    font = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 16);
    fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());

    float sw = game->GetVirtualWidth();
    float sh = game->GetVirtualHeight();
    conversation = std::make_shared<IConversation>(fontSprite, sw, sh);

    conversation->AddLine({ .name = L"Player", .content = L"Did I... actually make it out?" });
    conversation->AddLine({ .name = L"Player", .content = L"The digital world is collapsing behind me. I can see the code breaking apart." });
    conversation->AddLine({ .name = L"Player", .content = L"All those battles, those terminals, those glitched creatures..." });
    conversation->AddLine({ .name = L"Player", .content = L"It felt so real. But now the light is pulling me back." });
    conversation->AddLine({ .name = L"???", .content = L"Hey! Wake up! Are you okay?" });
    conversation->AddLine({ .name = L"Player", .content = L"I open my eyes. I'm back in my room. My phone is still on the suspicious webpage." });
    conversation->AddLine({ .name = L"Player", .content = L"A message on the screen reads: \"Session terminated. Welcome back to reality.\"" });
    conversation->AddLine({ .name = L"Player", .content = L"I immediately close the tab and take a deep breath. Never again." });
}

void Demo::OutroScene::Update(unsigned long long deltaTime) {
    auto inpMan = DX9GF::InputManager::GetInstance();
    inpMan->ReadMouse(deltaTime);
    inpMan->ReadKeyboard(deltaTime);

    if (conversation) {
        conversation->Execute(deltaTime);
        if (conversation->IsFinished()) {
            conversation = nullptr;
        }
    }

    if (!conversation && !hasTransitioned) {
        hasTransitioned = true;
        auto audio = DX9GF::AudioManager::GetInstance();
        audio->PlayBGM_Fade("bgm_sky", 0.9f, 1.5f);
        game->GetSceneManager()->GoToScene(0);
    }

    this->uiCamera.Update();
}

void Demo::OutroScene::DrawWorld(unsigned long long deltaTime) {
    auto gd = game->GetGraphicsDevice();
    if (SUCCEEDED(gd->BeginDraw())) {
        gd->EndDraw();
    }
}

void Demo::OutroScene::DrawUI(unsigned long long deltaTime)
{
    auto gd = game->GetGraphicsDevice();
    if (SUCCEEDED(gd->BeginDraw())) {

        if (conversation) {
            conversation->Draw(gd, &this->uiCamera, deltaTime);
        }

        DX9GF::InputManager::GetInstance()->DrawCursor(&this->uiCamera, deltaTime);

        gd->EndDraw();
    }
}