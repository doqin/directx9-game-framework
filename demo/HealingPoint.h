#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "DX9GFInputManager.h"
#include "Player.h"
#include "Debug.h"

namespace Demo {
    class HealingPoint : public DX9GF::IGameObject, virtual public Pointable {
    private:
        DX9GF::GraphicsDevice* gd;
        DX9GF::Camera* worldCamera;
        std::weak_ptr<DX9GF::TransformManager> transformManager;
        std::shared_ptr<DX9GF::Texture> spritesheet;
        std::shared_ptr<DX9GF::AnimatedSprite> sprite;
        std::weak_ptr<Player> player;
        std::shared_ptr<DX9GF::FontSprite> fontSprite;
        std::shared_ptr<DX9GF::RectangleCollider> collider;
        std::weak_ptr<DX9GF::CommandBuffer> drawBuffer;

        bool isPlayerNear = false;
        const float INTERACTION_DISTANCE = 50.0f;
        bool isVisible = false;

        std::string statusMessage = "";
        float messageTimer = 0.0f;

    public:
        HealingPoint(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0);

        void Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer);

        std::pair<float, float> GetPoint() const override {
            return { GetWorldX(), GetWorldY() };
        }
        void Update(unsigned long long deltaTime);
        void Draw(const DX9GF::Camera& camera, unsigned long long deltaTime);
        void DrawUI(DX9GF::Camera* uiCamera, unsigned long long deltaTime);
        void SetVisible(bool visible) { isVisible = visible; }
        bool IsVisible() const { return isVisible; }
    };
}