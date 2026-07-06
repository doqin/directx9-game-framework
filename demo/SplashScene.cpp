#include "pch.h"
#include "SplashScene.h"
#include "MainMenu.h"
#include <algorithm>

namespace Demo {
    void SplashScene::Init() {
        auto gd = game->GetGraphicsDevice();

        logo1Tex = std::make_shared<DX9GF::Texture>(gd);
        logo1Tex->LoadTexture(L"assets/school_logo.png");

        logo2Tex = std::make_shared<DX9GF::Texture>(gd);
        logo2Tex->LoadTexture(L"assets/department_logo.png");

        logo1Sprite = std::make_shared<DX9GF::StaticSprite>(logo1Tex.get());
        logo2Sprite = std::make_shared<DX9GF::StaticSprite>(logo2Tex.get());

        //logo's location
        logo1Sprite->SetOrigin(logo1Tex->GetWidth() / 2.0f, logo1Tex->GetHeight() / 2.0f);
        logo1Sprite->SetPosition(0.0f, 0.0f);

        logo2Sprite->SetOrigin(logo2Tex->GetWidth() / 2.0f, logo2Tex->GetHeight() / 2.0f);
        logo2Sprite->SetPosition(0.0f, 0.0f);

        //scale
        logo1Sprite->SetScale(1.f);
        logo2Sprite->SetScale(1.f);

        logo1Sprite->SetColor(D3DCOLOR_ARGB(0, 255, 255, 255));
        logo2Sprite->SetColor(D3DCOLOR_ARGB(0, 255, 255, 255));
    }

    void SplashScene::Update(unsigned long long deltaTime) {
        float dt = deltaTime / 1000.0f;
        timer += dt;

        float alpha1 = 0.0f;
        float alpha2 = 0.0f;

        //fade sequence
        // 0-3
        if (timer < 1.0f) {
            alpha1 = (timer / 1.0f) * 255.0f; // Fade in
        }
        else if (timer < 2.0f) {
            alpha1 = 255.0f; // Hold
        }
        else if (timer < 3.0f) {
            alpha1 = (1.0f - (timer - 2.0f) / 1.0f) * 255.0f; // Fade out
        }
        // 3-6
        else if (timer < 4.0f) {
            alpha2 = ((timer - 3.0f) / 1.0f) * 255.0f; // Fade in
        }
        else if (timer < 5.0f) {
            alpha2 = 255.0f; // Hold
        }
        else if (timer < 6.0f) {
            alpha2 = (1.0f - (timer - 5.0f) / 1.0f) * 255.0f; // Fade out
        }
        else {
            auto sceMan = game->GetSceneManager();
            sceMan->InsertScene(sceMan->GetIndex() + 1, new MainMenu(game, game->GetVirtualWidth(), game->GetVirtualHeight()));
            sceMan->GoToNext();
            return;
        }

        int a1 = std::clamp(static_cast<int>(alpha1), 0, 255);
        int a2 = std::clamp(static_cast<int>(alpha2), 0, 255);

        logo1Sprite->SetColor(D3DCOLOR_ARGB(a1, 255, 255, 255));
        logo2Sprite->SetColor(D3DCOLOR_ARGB(a2, 255, 255, 255));

        camera.Update();
    }

    void SplashScene::DrawWorld(unsigned long long deltaTime) {
        auto gd = game->GetGraphicsDevice();
        if (SUCCEEDED(gd->BeginDraw())) {
            
            //draw bg
            auto [sw, sh] = camera.GetScreenResolution();
            gd->DrawRectangle(0.0f, 0.0f, static_cast<float>(sw), static_cast<float>(sh), 0xFFFFFFFF, true);

            //draw logo w alpha blending
            gd->SetAlphaBlending(true);
            logo1Sprite->Begin(); logo1Sprite->Draw(camera, deltaTime); logo1Sprite->End();
            logo2Sprite->Begin(); logo2Sprite->Draw(camera, deltaTime); logo2Sprite->End();
            gd->SetAlphaBlending(false);

            gd->EndDraw();
        }
    }
}