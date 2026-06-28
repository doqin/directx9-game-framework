#include "pch.h"
#include "SavePoint.h"
#include <cmath>

namespace Demo {
    SavePoint::SavePoint(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y)
        : IGameObject(tm, x, y), transformManager(tm) {
    }

    void SavePoint::Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* worldCamera, DX9GF::Camera* uiCamera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::SaveManager> sm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) {
        player = p;
        this->worldCamera = worldCamera;
        saveManager = sm;
        fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
        this->drawBuffer = drawBuffer;
        this->gd = gd;
		collider = std::make_shared<DX9GF::RectangleCollider>(transformManager, 32.f, 32.f, GetWorldX(), GetWorldY());
        collider->SetOriginCenter();
        cm->Add(collider);
        spritesheet = std::make_shared<DX9GF::Texture>(gd);
		spritesheet->LoadTexture(L"assets/savepoint-Sheet.png");
        sprite = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 32, 32, 10), 12, true);
		sprite->SetOrigin(16.f, 16.f);
		sprite->SetPosition(GetWorldX(), GetWorldY());

        auto [x, y] = GetWorldPosition();

        btnYes = std::make_shared<Demo::TextButton>(
            transformManager.lock(), x - 50.f, y - 10.f, 30.f, 20.f, "Yes", font.get(),
            [](DX9GF::ITrigger* t) {}
        );
		btnYes->SetLocalScale(0.5f, 0.5f);
        btnYes->SetOnReleaseLeft([this](DX9GF::ITrigger* t) {
            if (auto smLock = this->saveManager.lock()) {
                smLock->Save("savegame.json");
                OutputDebugStringA("Successfully saved!\n");
                DX9GF::AudioManager::GetInstance()->Play("checkpoint",false, 0.7f);
            }
            this->isSaveMenuOpen = false;
            });
        btnYes->SetBackgroundColors(D3DXCOLOR(0xFFE0E0E0), 0xFF59c135, 0xFF14a02e)
            ->SetTextColors(0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF)
            ->SetOutline(1.f, D3DXCOLOR(0xFF000000), D3DXCOLOR(0xFF000000), D3DXCOLOR(0xFF000000))
            ->SetPadding(4.f, 4.f);
        btnYes->Init(uiCamera);

        btnNo = std::make_shared<Demo::TextButton>(
            transformManager.lock(), x + 20.f, y - 10.f, 30.f, 20.f, "No", font.get(),
            [](DX9GF::ITrigger* t) {}
        );
		btnNo->SetLocalScale(0.5f, 0.5f);
        btnNo->SetOnReleaseLeft([this](DX9GF::ITrigger* t) {
            this->isSaveMenuOpen = false;
            });
        btnNo->SetBackgroundColors(D3DXCOLOR(0xFFE0E0E0), 0xFFb4202a, 0xFF73172d)
            ->SetTextColors(0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF)
            ->SetOutline(1.f, D3DXCOLOR(0xFF000000), D3DXCOLOR(0xFF000000), D3DXCOLOR(0xFF000000))
            ->SetPadding(4.f, 4.f);
        btnNo->Init(uiCamera);
    }

    void SavePoint::Update(unsigned long long deltaTime) {
        if (!isVisible) return;

        if (worldCamera) {
            auto [worldX, worldY] = GetWorldPosition();
            float zoom = worldCamera->GetZoom();

            float uiX = (worldX - worldCamera->GetPosition().x) * zoom;
            float uiY = (worldY - worldCamera->GetPosition().y) * zoom;

            btnYes->SetLocalScale(0.5f * zoom, 0.5f * zoom);
            btnNo->SetLocalScale(0.5f * zoom, 0.5f * zoom);

            btnYes->SetLocalPosition(uiX - 50.f * zoom, uiY - 10.f * zoom);
            btnNo->SetLocalPosition(uiX + 20.f * zoom, uiY - 10.f * zoom);
        }
        auto inpMan = DX9GF::InputManager::GetInstance();

        if (isSaveMenuOpen) {
            btnYes->Update(deltaTime);
            btnNo->Update(deltaTime);

            if (inpMan->KeyPress(DIK_Y)) {
                if (auto smLock = saveManager.lock()) {
                    smLock->Save("savegame.json");
                }
                isSaveMenuOpen = false;
            }
            if (inpMan->KeyPress(DIK_N)) {
                isSaveMenuOpen = false;
            }
            return;
        }

        auto pLock = player.lock();
        if (!pLock) return;

        auto [px, py] = pLock->GetWorldPosition();
        auto [sx, sy] = GetWorldPosition();

        float distance = std::sqrt((px - sx) * (px - sx) + (py - sy) * (py - sy));
        isPlayerNear = (distance <= INTERACTION_DISTANCE);

        if (isPlayerNear && inpMan->KeyPress(DIK_E)) {
            isSaveMenuOpen = true;
        }
    }

    void SavePoint::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) {
        if (!isVisible) return;
        sprite->Begin();
        sprite->Draw(camera, deltaTime);
        sprite->End();
    }

    void SavePoint::DrawUI(DX9GF::Camera* uiCamera, unsigned long long deltaTime) {
        if (!isVisible || !uiCamera || !worldCamera) return;

        auto [worldX, worldY] = GetWorldPosition();
        float zoom = worldCamera->GetZoom();
        float uiX = (worldX - worldCamera->GetPosition().x) * zoom;
        float uiY = (worldY - worldCamera->GetPosition().y) * zoom;

        if (isSaveMenuOpen) {
            float padding = 10.f * zoom;
            float promptY = uiY - 30.f * zoom;
            float scale = 0.5f * zoom;

            fontSprite->Begin();
            fontSprite->SetText(L"Do you want to save the game?");
            float textW = fontSprite->GetWidth() * scale;
            float textH = fontSprite->GetHeight() * scale;

            fontSprite->SetPosition(uiX - textW / 2.f, promptY - textH / 2.f);

            if (gd) {
                gd->DrawRectangle(
                    *uiCamera,
                    uiX - textW / 2.f - padding,
                    promptY - textH / 2.f - padding,
                    textW + 2 * padding,
                    textH + 2 * padding,
                    0xFFE0E0E0, true
                );
                gd->DrawRectangle(
                    *uiCamera,
                    uiX - textW / 2.f - padding,
                    promptY - textH / 2.f - padding,
                    textW + 2 * padding,
                    textH + 2 * padding,
                    0xFF000000, false
                );
            }

            fontSprite->SetColor(0xFF000000);
            fontSprite->SetOutline(false);
            fontSprite->SetScale(scale);
            fontSprite->Draw(*uiCamera, deltaTime);
            fontSprite->End();

            btnYes->Draw(gd, deltaTime);
            btnNo->Draw(gd, deltaTime);
        }
        else if (isPlayerNear) {
            float scale = 1.0f * zoom;
            fontSprite->Begin();
            fontSprite->SetText(L"E");
            float textW = fontSprite->GetWidth() * scale;
            float textH = fontSprite->GetHeight() * scale;

            fontSprite->SetScale(scale);
            fontSprite->SetColor(0xFFFFFFFF);
            fontSprite->SetPosition(uiX - textW / 2.f, uiY - 30.f * zoom - textH / 2.f);
            fontSprite->SetOutline(true, 0xFF000000);
            fontSprite->Draw(*uiCamera, deltaTime);
            fontSprite->End();
        }
    }
}