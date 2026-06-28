#pragma once
#include "pch.h"
#include "DX9GF.h"      
#include "DX9GFExtras.h"  
#include "Player.h"
#include "DialogueLine.h"
#include "IConversation.h"
#include "INPC.h"

namespace Demo {
    class NPC1 : public INPC {
    public:
        NPC1(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y);

        void Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) override;

        void Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) override;

        bool CanInteract() const override { return isPlayerNear; }
    };
}