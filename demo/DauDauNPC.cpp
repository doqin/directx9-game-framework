#include "pch.h"
#include "DauDauNPC.h"

namespace Demo {
    DauDauNPC::DauDauNPC(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y)
        : INPC(tm, x, y) {
    }

    void DauDauNPC::Init(DX9GF::GraphicsDevice* gd, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) {
        INPC::Init(gd, p, cm, font, drawBuffer);

        collider = std::make_shared<DX9GF::RectangleCollider>(transformManager, 24.f, 32.f, this->GetWorldX(), this->GetWorldY());
        collider->SetOriginCenter();
        cm->Add(collider);

        spritesheet = std::make_shared<DX9GF::Texture>(gd);
        spritesheet->LoadTexture(L"daudau-Sheet.png");
        sprite = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 32, 32, 5), 12, true);
        sprite->SetOrigin(16.f, 16.f);
        sprite->SetPosition(this->GetWorldX(), this->GetWorldY());
    }

    void DauDauNPC::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) {
        sprite->Begin();
        sprite->Draw(camera, deltaTime);
        sprite->End();

        INPC::Draw(camera, deltaTime);
    }
}