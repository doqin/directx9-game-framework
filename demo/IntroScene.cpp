#include "pch.h"
#include "IntroScene.h"

void Demo::IntroScene::Init() {
    font = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 16);
    fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());

    auto [sw, sh] = uiCamera.GetScreenResolution();
    conversation = std::make_shared<IConversation>(fontSprite, sw, sh);

    conversation->AddLine({ .name = L"???", .content = L"Ugh... where... where am I?" });
    conversation->AddLine({ .name = L"Player", .content = L"The last thing I remember... I clicked on that suspicious link in the chat." });
    conversation->AddLine({ .name = L"Player", .content = L"\"Free VBucks!\" it said. Of course it was too good to be true." });
    conversation->AddLine({ .name = L"Player", .content = L"But before I could close the page, my screen started glitching..." });
    conversation->AddLine({ .name = L"Player", .content = L"Lines of code flashed before my eyes, and then everything went black." });
    conversation->AddLine({ .name = L"Player", .content = L"And now... I'm inside some kind of digital world?" });
    conversation->AddLine({ .name = L"Player", .content = L"Everything around me looks like a corrupted program. I need to find a way out." });
    conversation->AddLine({ .name = L"Player", .content = L"Wait... I can see something in the distance. Maybe I should head that way." });
}

void Demo::IntroScene::Update(unsigned long long deltaTime) {
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
        DX9GF::AudioManager::GetInstance()->PlayBGM_Fade("bgm_tutorial", 0.5f, 1.5f);
        game->GetSceneManager()->GoToNext();
    }
}

void Demo::IntroScene::Draw(unsigned long long deltaTime) {
    auto gd = game->GetGraphicsDevice();
    gd->Clear(0xFF000000);
    if (SUCCEEDED(gd->BeginDraw())) {
        if (conversation) {
            conversation->Draw(gd, deltaTime);
        }
        DX9GF::InputManager::GetInstance()->DrawCursor(&uiCamera, deltaTime);
        gd->EndDraw();
    }
    gd->Present();
}
