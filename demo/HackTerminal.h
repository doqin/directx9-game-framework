#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "DX9GFInputManager.h"
#include "Player.h"
#include <functional>

namespace Demo {
    class HackTerminal : public DX9GF::IGameObject {
    private:
        DX9GF::GraphicsDevice* gd;
        DX9GF::Camera* worldCamera;
        std::weak_ptr<DX9GF::TransformManager> transformManager;
        std::shared_ptr<DX9GF::Texture> spritesheet;
        std::shared_ptr<DX9GF::StaticSprite> sprite;
        std::weak_ptr<Player> player;
        std::shared_ptr<DX9GF::FontSprite> fontSprite;
        std::shared_ptr<DX9GF::RectangleCollider> collider;

        bool isPlayerNear = false;
        const float INTERACTION_DISTANCE = 50.0f;
        bool isVisible = false;

        bool isHacked = false;
        std::string terminalName;
        int colorID;

        std::string statusMessage = "";
        float messageTimer = 0.0f;

        std::function<void(int)> onHackAttempt;

    public:
        HackTerminal(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y, int cID, std::string name);

        void Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::function<void(int)> hackCallback);

        void Update(unsigned long long deltaTime);
        void Draw(const DX9GF::Camera& camera, unsigned long long deltaTime);
        void DrawUI(DX9GF::Camera* uiCamera, unsigned long long deltaTime);

        void SetVisible(bool visible) { isVisible = visible; }
        bool IsVisible() const { return isVisible; }

        void SetHackedStatus(bool status);
        void ShowStatusMessage(const std::string& msg, float time);
        bool IsHacked() const { return isHacked; }

        int GetColorID() { return colorID; }
    };
}