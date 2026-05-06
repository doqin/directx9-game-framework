#include "pch.h"
#include "NPC1.h"

namespace Demo {
    NPC1::NPC1(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y)
        : INPC(tm, x, y) {
    }

    void NPC1::Init(DX9GF::GraphicsDevice* gd, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) {
        INPC::Init(gd, p, cm, font, drawBuffer);

        collider = std::make_shared<DX9GF::RectangleCollider>(transformManager, 32.f, 32.f, this->GetWorldX(), this->GetWorldY());
        collider->SetOriginCenter();
        cm->Add(collider);

        spritesheet = std::make_shared<DX9GF::Texture>(gd);
        spritesheet->LoadTexture(L"NPC1.png");
        sprite = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 32, 32, 4), 8, true);
        sprite->SetOrigin(16.f, 16.f);
        sprite->SetPosition(this->GetWorldX(), this->GetWorldY());
    }

    void NPC1::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) {
        sprite->Begin();
        sprite->Draw(camera, deltaTime);
        sprite->End();

        INPC::Draw(camera, deltaTime);
    }
}