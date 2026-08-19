#include "pch.h"
#include "INPC.h"
#include "SettingsManager.h"

namespace Demo {
    void INPC::Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) {
        player = p;
        this->worldCamera = camera;
        fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
        this->drawBuffer = drawBuffer;
    }

    void INPC::Update(unsigned long long deltaTime) {
        auto pLock = player.lock();
        if (!pLock) return;

        auto [px, py] = pLock->GetWorldPosition();
        auto [sx, sy] = this->GetWorldPosition();

        float distance = std::sqrt((px - sx) * (px - sx) + (py - sy) * (py - sy));
        this->isPlayerNear = (distance <= this->INTERACTION_DISTANCE);
    }

    void INPC::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) {
		DrawPosition(deltaTime, gd, camera);
    }

    void INPC::DrawUI(DX9GF::Camera* uiCamera, unsigned long long deltaTime) {
        if (isPlayerNear && fontSprite && uiCamera && worldCamera) {
            auto [worldX, worldY] = GetWorldPosition();

            float zoom = worldCamera->GetZoom();

            float uiX = (worldX - worldCamera->GetPosition().x) * zoom;
            float uiY = (worldY - worldCamera->GetPosition().y) * zoom;

            float scale = 1.0f * zoom;

            fontSprite->Begin();
            fontSprite->SetText(SettingsManager::GetInstance()->GetKeybindDisplayName("INTERACT"));
            fontSprite->SetScale(scale);
            fontSprite->SetColor(0xFFFFFFFF);

            float textW = fontSprite->GetWidth() * scale;

            fontSprite->SetPosition(uiX - textW / 2.f, uiY - 40.f * zoom);

            fontSprite->SetOutline(true, 0xFF000000);
            fontSprite->Draw(*uiCamera, deltaTime);
            fontSprite->End();
        }
    }
    void INPC::AddLine(std::wstring name, std::wstring content) {
        DialogueLine line;
        line.name = name;
        line.content = content;
        dialogueLines.push_back(line);
    }
}