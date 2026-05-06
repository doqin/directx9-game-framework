#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "DX9GFInputManager.h"
#include "Player.h"
#include "Game.h"

namespace Demo {
    class ShopPoint : public DX9GF::IGameObject {
    private:
        Game* game;
        DX9GF::GraphicsDevice* gd;
        std::weak_ptr<DX9GF::TransformManager> transformManager;
        std::shared_ptr<DX9GF::Texture> spritesheet;
        std::shared_ptr<DX9GF::AnimatedSprite> sprite;
        std::weak_ptr<Player> player;
        std::shared_ptr<DX9GF::FontSprite> fontSprite;
        std::shared_ptr<DX9GF::RectangleCollider> collider;
        std::weak_ptr<DX9GF::CommandBuffer> drawBuffer;

		std::function<DX9GF::IScene* (Game*, Player*, int, int)> sceneFactory;

		bool isPlayerNear = false;
		const float INTERACTION_DISTANCE = 50.0f;
		bool isVisible = false;

	public:
		ShopPoint(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0);

        void Init(Game* game, DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer, std::function<DX9GF::IScene* (Game*, Player*, int, int)> factory);

		void Update(unsigned long long deltaTime);
		void Draw(const DX9GF::Camera& camera, unsigned long long deltaTime);
		void SetVisible(bool visible) { isVisible = visible; }
		bool IsVisible() const { return isVisible; }
	};
}