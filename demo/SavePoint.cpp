#include "pch.h"
#include "SettingsManager.h"
#include "SavePoint.h"
#include "PopupManager.h"
#include <cmath>

namespace Demo {
    SavePoint::SavePoint(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y)
        : IGameObject(tm, x, y), transformManager(tm) {
    }

    void SavePoint::Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* worldCamera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::SaveManager> sm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) {
        this->player = p;
        this->worldCamera = worldCamera;
        this->saveManager = sm;
        this->fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
        this->drawBuffer = drawBuffer;
        this->gd = gd;

        collider = std::make_shared<DX9GF::RectangleCollider>(transformManager, 32.f, 8.f, GetWorldX(), GetWorldY() + 12.f);
        collider->SetOriginCenter();
        cm->Add(collider);

        spritesheet = std::make_shared<DX9GF::Texture>(gd);
        spritesheet->LoadTexture(L"assets/savepoint-Sheet.png");
        sprite = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 32, 32, 10), 12, true);
        sprite->SetOrigin(16.f, 16.f);
        sprite->SetPosition(GetWorldX(), GetWorldY());
    }

    void SavePoint::Update(unsigned long long deltaTime) {
        if (!isVisible) return;

        if (PopupManager::GetInstance()->IsActive()) return;

        auto pLock = player.lock();
        if (!pLock) return;

        auto [px, py] = pLock->GetWorldPosition();
        auto [sx, sy] = GetWorldPosition();

        float distance = std::sqrt((px - sx) * (px - sx) + (py - sy) * (py - sy));
        isPlayerNear = (distance <= INTERACTION_DISTANCE);

        auto inpMan = DX9GF::InputManager::GetInstance();
        if (isPlayerNear && inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
            std::vector<std::pair<std::wstring, std::function<void()>>> buttons = {
                { L"Yes(Y)", [this]() {
                    if (auto smLock = this->saveManager.lock()) {
                        smLock->Save("savegame.json");
                        OutputDebugStringA("Successfully saved!\n");
                        DX9GF::AudioManager::GetInstance()->Play("checkpoint", false, 0.7f);
                    }
                }},
                { L"No(N)", nullptr }
            };

            PopupManager::GetInstance()->Show("basic_blackwhite", L"SAVE GAME", L"Do you want to save the game?", buttons);
        }
    }

    void SavePoint::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) {
        if (!isVisible) return;
        sprite->Begin();
        sprite->Draw(camera, deltaTime);
        sprite->End();
		DrawPosition(deltaTime, gd, camera);
    }

    void SavePoint::DrawUI(DX9GF::Camera* uiCamera, unsigned long long deltaTime) {
        if (!isVisible || !uiCamera || !worldCamera) return;

        if (isPlayerNear && !PopupManager::GetInstance()->IsActive()) {
            auto [worldX, worldY] = GetWorldPosition();
            float zoom = worldCamera->GetZoom();
            float uiX = (worldX - worldCamera->GetPosition().x) * zoom;
            float uiY = (worldY - worldCamera->GetPosition().y) * zoom;

            float scale = 1.0f * zoom;
            fontSprite->Begin();
            fontSprite->SetText(SettingsManager::GetInstance()->GetKeybindDisplayName("INTERACT"));
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