#include "pch.h"
#include "SettingsManager.h"
#include "ShopPoint.h"
#include "CardShop.h"
#include <cmath>

namespace Demo {
	ShopPoint::ShopPoint(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y)
		: IGameObject(tm, x, y), transformManager(tm) {
	}

    void ShopPoint::Init(Game* game, DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer, std::function<DX9GF::IScene* (Game*, Player*, int, int)> factory) {
        this->game = game;
        this->worldCamera = camera;
        player = p;
        fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
        this->drawBuffer = drawBuffer;
        this->gd = gd;
        collider = std::make_shared<DX9GF::RectangleCollider>(transformManager, 80.f, 80.f, GetWorldX(), GetWorldY());
        collider->SetOriginCenter();
        cm->Add(collider);

        spritesheet = std::make_shared<DX9GF::Texture>(gd);
        spritesheet->LoadTexture(L"assets/Shop-Sheet.png");

        sprite = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 80, 80, 8), 8);
        sprite->SetOrigin(40.f, 40.f);
        sprite->SetPosition(GetWorldX(), GetWorldY());

        this->sceneFactory = factory;
    }

    void Demo::ShopPoint::Update(unsigned long long deltaTime) {
        if (!isVisible) return;
        auto pLock = player.lock();
        if (!pLock) return;

        auto [px, py] = pLock->GetWorldPosition();
        auto [sx, sy] = GetWorldPosition();
        float distance = std::sqrt((px - sx) * (px - sx) + (py - sy) * (py - sy));
        isPlayerNear = (distance <= INTERACTION_DISTANCE);

        auto inpMan = DX9GF::InputManager::GetInstance();

        if (isPlayerNear && inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
            auto [sw, sh] = worldCamera->GetScreenResolution();

            if (sceneFactory) {
                auto shopScene = sceneFactory(game, pLock.get(), sw, sh);
                auto sceMan = game->GetSceneManager();
                sceMan->InsertScene(sceMan->GetIndex() + 1, shopScene);
                sceMan->GoToNext();
            }
        }
    }

    void Demo::ShopPoint::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) {
        if (!isVisible) return;
        sprite->Begin();
        sprite->Draw(camera, deltaTime);
        sprite->End();
    }

    void Demo::ShopPoint::DrawUI(DX9GF::Camera* uiCamera, unsigned long long deltaTime) {
        if (!isVisible || !uiCamera || !worldCamera) return;

        if (fontSprite && isPlayerNear) {
            auto [x, y] = GetWorldPosition();
            float uiX = (x - worldCamera->GetPosition().x) * worldCamera->GetZoom();
            float uiY = (y - worldCamera->GetPosition().y) * worldCamera->GetZoom();

            float zoom = worldCamera->GetZoom();
            float scale = 1.0f * zoom;

            fontSprite->Begin();
            fontSprite->SetText(L"E");
            fontSprite->SetScale(scale);
            fontSprite->SetColor(0xFFFFFFFF);

            float textW = fontSprite->GetWidth() * scale;
            float textH = fontSprite->GetHeight() * scale;
            fontSprite->SetPosition(uiX - textW / 2.f, uiY - 30.f * zoom - textH / 2.f);
            fontSprite->SetOutline(true, 0xFF000000);
            fontSprite->Draw(*uiCamera, deltaTime);
            fontSprite->End();
        }
    }
}