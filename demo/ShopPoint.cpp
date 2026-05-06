#include "pch.h"
#include "ShopPoint.h"
#include "CardShop.h"
#include <cmath>

namespace Demo {
	ShopPoint::ShopPoint(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y)
		: IGameObject(tm, x, y), transformManager(tm) {
	}

    void ShopPoint::Init(Game* game, DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer, std::function<DX9GF::IScene* (Game*, Player*, int, int)> factory) {
        this->game = game;
        player = p;
        fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
        this->drawBuffer = drawBuffer;
        this->gd = gd;
        collider = std::make_shared<DX9GF::RectangleCollider>(transformManager, 80.f, 80.f, GetWorldX(), GetWorldY());
        collider->SetOriginCenter();
        cm->Add(collider);

        spritesheet = std::make_shared<DX9GF::Texture>(gd);
        spritesheet->LoadTexture(L"Shop-Sheet.png");

        sprite = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 80, 80, 8), 8);
        sprite->SetOrigin(40.f, 40.f);
        sprite->SetPosition(GetWorldX(), GetWorldY());

        this->sceneFactory = factory;
    }

	void ShopPoint::Update(unsigned long long deltaTime) {
		if (!isVisible) return;

		auto pLock = player.lock();
		if (!pLock) return;

		auto [px, py] = pLock->GetWorldPosition();
		auto [sx, sy] = GetWorldPosition();

		float distance = std::sqrt((px - sx) * (px - sx) + (py - sy) * (py - sy));
		isPlayerNear = (distance <= INTERACTION_DISTANCE);

		auto inpMan = DX9GF::InputManager::GetInstance();

		if (isPlayerNear && inpMan->KeyPress(DIK_E)) {
			auto app = DX9GF::Application::GetInstance();
			int sw = app->GetScreenWidth();
			int sh = app->GetScreenHeight();

			if (sceneFactory) {
				auto shopScene = sceneFactory(game, pLock.get(), sw, sh);
				game->GetSceneManager()->PushScene(shopScene);
				game->GetSceneManager()->GoToNext();
			}

		}
	}

	void ShopPoint::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) {
		if (!isVisible) return;

		auto [x, y] = GetWorldPosition();

		sprite->Begin();
		sprite->Draw(camera, deltaTime);
		sprite->End();

        if (fontSprite && isPlayerNear) {
            if (auto bufferLock = drawBuffer.lock()) {
                bufferLock->PushCommand(std::make_shared<DX9GF::CustomCommand>([this, x, y, &camera, deltaTime](std::function<void(void)> markFinished) {
                    fontSprite->Begin();
                    fontSprite->SetText(L"E");
                    fontSprite->SetScale(1.f);
                    fontSprite->SetColor(0xFFFFFFFF);
                    fontSprite->SetPosition(x - fontSprite->GetWidth() / 2.f, y - 30.f - fontSprite->GetHeight() / 2.f);
                    fontSprite->SetOutline(true, 0xFF000000);
                    fontSprite->Draw(camera, deltaTime);
                    fontSprite->End();
                    markFinished();
                }));
            }
        }
    }
}