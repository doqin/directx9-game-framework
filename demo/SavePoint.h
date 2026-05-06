#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "DX9GFInputManager.h"
#include "Player.h"
#include "TextButton.h"

namespace Demo {
    class SavePoint : public DX9GF::IGameObject {
    private:
        DX9GF::GraphicsDevice* gd;
        std::weak_ptr<DX9GF::TransformManager> transformManager;
		std::shared_ptr<DX9GF::Texture> spritesheet;
        std::shared_ptr<DX9GF::AnimatedSprite> sprite;
        std::weak_ptr<Player> player;
        std::weak_ptr<DX9GF::SaveManager> saveManager;
        std::shared_ptr<DX9GF::FontSprite> fontSprite;
     std::shared_ptr<DX9GF::RectangleCollider> collider;
        std::weak_ptr<DX9GF::CommandBuffer> drawBuffer;

        bool isPlayerNear = false;
        bool isSaveMenuOpen = false;
        const float INTERACTION_DISTANCE = 50.0f;
        bool isVisible = false;

        std::shared_ptr<Demo::TextButton> btnYes;
        std::shared_ptr<Demo::TextButton> btnNo;

    public:
        SavePoint(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0);

        void Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::SaveManager> sm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer);

        void Update(unsigned long long deltaTime);
        void Draw(const DX9GF::Camera& camera, unsigned long long deltaTime);
        bool IsMenuOpen() const { return isSaveMenuOpen; }
        void SetVisible(bool visible) { isVisible = visible; }
        bool IsVisible() const { return isVisible; }
    };
}