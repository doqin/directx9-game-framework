#include "pch.h"
#include "SavePoint.h"
#include <cmath>

namespace Demo {
    SavePoint::SavePoint(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y)
        : IGameObject(tm, x, y), transformManager(tm) {
    }

    void SavePoint::Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::SaveManager> sm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) {
        player = p;
        saveManager = sm;
        fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
        this->drawBuffer = drawBuffer;
        this->gd = gd;
		collider = std::make_shared<DX9GF::RectangleCollider>(transformManager, 32.f, 32.f, GetWorldX(), GetWorldY());
        collider->SetOriginCenter();
        cm->Add(collider);
        spritesheet = std::make_shared<DX9GF::Texture>(gd);
		spritesheet->LoadTexture(L"savepoint-Sheet.png");
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
            }
            this->isSaveMenuOpen = false;
            });
        btnYes->SetBackgroundColors(D3DXCOLOR(0xFFE0E0E0), D3DXCOLOR(0, 1, 0, 0.3f), D3DXCOLOR(0, 1, 0, 0.6f))
            ->SetTextColors(0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF)
            ->SetOutline(1.f, D3DXCOLOR(0xFF000000), D3DXCOLOR(0xFF000000), D3DXCOLOR(0xFF000000))
            ->SetPadding(4.f, 4.f);
        btnYes->Init(camera);

        btnNo = std::make_shared<Demo::TextButton>(
            transformManager.lock(), x + 20.f, y - 10.f, 30.f, 20.f, "No", font.get(),
            [](DX9GF::ITrigger* t) {}
        );
		btnNo->SetLocalScale(0.5f, 0.5f);
        btnNo->SetOnReleaseLeft([this](DX9GF::ITrigger* t) {
            this->isSaveMenuOpen = false;
            });
        btnNo->SetBackgroundColors(D3DXCOLOR(0xFFE0E0E0), D3DXCOLOR(1, 0, 0, 0.3f), D3DXCOLOR(1, 0, 0, 0.6f))
            ->SetTextColors(0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF)
            ->SetOutline(1.f, D3DXCOLOR(0xFF000000), D3DXCOLOR(0xFF000000), D3DXCOLOR(0xFF000000))
            ->SetPadding(4.f, 4.f);
        btnNo->Init(camera);
    }

    void SavePoint::Update(unsigned long long deltaTime) {
        if (!isVisible) return;

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

        auto [x, y] = GetWorldPosition();

        sprite->Begin();
		sprite->Draw(camera, deltaTime);
        sprite->End();

        if (fontSprite) {
            fontSprite->Begin();
            if (isSaveMenuOpen) {
                const auto padding = 10.f;
                const auto promptY = y - 30.f;
                fontSprite->SetText(L"Do you want to save the game?");
                fontSprite->SetPosition(x - fontSprite->GetWidth() / 4.f, promptY - fontSprite->GetHeight() / 4.f);
                if (gd) {
                    gd->DrawRectangle(
                        camera,
                        x - fontSprite->GetWidth() / 4.f - padding,
                        promptY - fontSprite->GetHeight() / 4.f - padding,
                        fontSprite->GetWidth() / 2.f + 2 * padding,
                        fontSprite->GetHeight() / 2.f + 2 * padding,
                        0xFFE0E0E0, true
                    );
                    gd->DrawRectangle(
                        camera,
                        x - fontSprite->GetWidth() / 4.f - padding,
                        promptY - fontSprite->GetHeight() / 4.f - padding,
                        fontSprite->GetWidth() / 2.f + 2 * padding,
                        fontSprite->GetHeight() / 2.f + 2 * padding,
                        0xFF000000, false
                    );
                }
                fontSprite->SetColor(0xFF000000);
                fontSprite->SetOutline(false);
                fontSprite->SetScale(0.5f);
                fontSprite->Draw(camera, deltaTime);
                fontSprite->End();

                btnYes->Draw(gd, deltaTime);
                btnNo->Draw(gd, deltaTime);
            }
            else if (isPlayerNear) {
                fontSprite->End();
                if (auto bufferLock = drawBuffer.lock()) {
                    bufferLock->StackCommand(std::make_shared<DX9GF::CustomCommand>([this, x, y, &camera, deltaTime](std::function<void(void)> markFinished) {
                        fontSprite->Begin();
                        fontSprite->SetText(L"E");
                        fontSprite->SetScale(1.f);
                        fontSprite->SetColor(0xFFFFFFFF);
                        fontSprite->SetPosition(x - fontSprite->GetWidth() / 2.f, y - 30.f - fontSprite->GetHeight() / 2.f);
                        fontSprite->SetOutline(true, 0xFF000000);
                        fontSprite->Draw(camera, deltaTime);
                        fontSprite->End();
                        markFinished();
                    }));
                }
            }
            else {
                fontSprite->End();
            }
        }
    }
}