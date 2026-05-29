#include "pch.h"
#include "HackTerminal.h"
#include <cmath>

namespace Demo {
    HackTerminal::HackTerminal(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y, int cID, std::string name)
        : IGameObject(tm, x, y), transformManager(tm), colorID(cID), terminalName(name) {
    }

    void HackTerminal::Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer, std::function<void(int)> hackCallback) {
        player = p;
        fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
        this->drawBuffer = drawBuffer;
        this->gd = gd;
        this->onHackAttempt = hackCallback;

        collider = std::make_shared<DX9GF::RectangleCollider>(transformManager, 32.f, 32.f, GetWorldX(), GetWorldY());
        collider->SetOriginCenter();
        cm->Add(collider);

        spritesheet = std::make_shared<DX9GF::Texture>(gd);

        spritesheet->LoadTexture(L"assets/terminals.png");


        sprite = std::make_shared<DX9GF::StaticSprite>(spritesheet.get());
        if (terminalName == "M1") {
			sprite->SetSrcRect({.left=0, .top=0, .right=16, .bottom=32});
        }
        else if (terminalName == "M2") {
			sprite->SetSrcRect({ .left = 16, .top = 0, .right = 32, .bottom = 32 });
        }
        else if (terminalName == "M3") {
            sprite->SetSrcRect({ .left = 32, .top = 0, .right = 48, .bottom = 32 });
        }
        else if (terminalName == "M4") {
			sprite->SetSrcRect({ .left = 48, .top = 0, .right = 64, .bottom = 32 });
        }
        else {
			sprite->SetSrcRect({ .left = 64, .top = 0, .right = 80, .bottom = 32 });
        }
        sprite->SetOrigin(8.f, 16.f);
        sprite->SetPosition(GetWorldX(), GetWorldY());
    }

    void HackTerminal::Update(unsigned long long deltaTime) {
        if (!isVisible) return;

        if (messageTimer > 0) {
            messageTimer -= deltaTime / 1000.0f;
            if (messageTimer <= 0) statusMessage = "";
        }

        auto pLock = player.lock();
        if (!pLock) return;

        auto [px, py] = pLock->GetWorldPosition();
        auto [sx, sy] = GetWorldPosition();

        float distance = std::sqrt((px - sx) * (px - sx) + (py - sy) * (py - sy));
        isPlayerNear = (distance <= INTERACTION_DISTANCE);

        auto inpMan = DX9GF::InputManager::GetInstance();

        if (isPlayerNear && inpMan->KeyDown(DIK_E) && !isHacked) {
            if (onHackAttempt) {
                onHackAttempt(this->colorID);
            }
        }
    }

    void HackTerminal::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) {
        if (!isVisible) return;

        auto [x, y] = GetWorldPosition();

        sprite->Begin();
        sprite->Draw(camera, deltaTime);
        sprite->End();

        if (fontSprite) {
            if (!statusMessage.empty()) {
                if (auto bufferLock = drawBuffer.lock()) {
                    const auto statusText = statusMessage;
                    bufferLock->StackCommand(std::make_shared<DX9GF::CustomCommand>([this, statusText, x, y, &camera, deltaTime](std::function<void(void)> markFinished) {
                        fontSprite->Begin();
                        fontSprite->SetText(std::wstring(statusText.begin(), statusText.end()));
                        fontSprite->SetScale(0.8f);
                        fontSprite->SetColor(0xFF55FF55);
                        fontSprite->SetPosition(x - fontSprite->GetWidth() * 0.8f / 2.f, y - 45.f - fontSprite->GetHeight() / 2.f);
                        fontSprite->SetOutline(true, 0xFF000000);
                        fontSprite->Draw(camera, deltaTime);
                        fontSprite->End();
                        markFinished();
                        }));
                }
            }
            else if (isPlayerNear && !isHacked) {
                if (auto bufferLock = drawBuffer.lock()) {
                    bufferLock->StackCommand(std::make_shared<DX9GF::CustomCommand>([this, x, y, &camera, deltaTime](std::function<void(void)> markFinished) {
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

    //set frame before/after hacked here
    void HackTerminal::SetHackedStatus(bool status) {
        isHacked = status;
        if (isHacked) {
			sprite->SetSrcRect({ .left = 80, .top = 0, .right = 96, .bottom = 32 });
        }
    }

    void HackTerminal::ShowStatusMessage(const std::string& msg, float time) {
        statusMessage = msg;
        messageTimer = time;
    }
}