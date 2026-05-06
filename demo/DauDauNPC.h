#pragma once
#include "pch.h"
#include "INPC.h"

namespace Demo {
    class DauDauNPC : public INPC {
    public:
        DauDauNPC(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y);

        void Init(DX9GF::GraphicsDevice* gd, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) override;

        void Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) override;

        bool CanInteract() const override { return isPlayerNear; }
    };
}