#include "pch.h"
#include "MapEnemy.h"
#include "MapBattleScene.h"

namespace Demo {
    MapEnemy::MapEnemy(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y, const BattleEncounter& data)
        : IGameObject(tm, x, y), startX(x), startY(y), encounterData(data) {
    }

    MapEnemy::~MapEnemy() {
        if (colliderManager && collider) {
            colliderManager->Remove(collider);
        }
    }

    void MapEnemy::Init(Game* game, DX9GF::GraphicsDevice* gd, DX9GF::ColliderManager* colMan, std::shared_ptr<Player> player) {
        this->game = game;
        this->colliderManager = colMan;
        this->targetPlayer = player;

        texture = std::make_shared<DX9GF::Texture>(gd);
        texture->LoadTexture(L"assets/notresponding-Sheet.png");

        sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 12);
        sprite->SetOrigin(32, 32);

        // CHỈ ĐỊNH NGHĨA SCALE Ở ĐÂY MỘT LẦN DUY NHẤT
        this->SetLocalScale(0.7f, 0.7f);
        sprite->SetScale(0.7f, 0.7f);

        collider = std::make_shared<DX9GF::RectangleCollider>(transformManager, shared_from_this(), 24, 24, 0, 0);
        collider->SetOriginCenter();
        colliderManager->Add(collider);
    }

    void MapEnemy::Update(unsigned long long deltaTime) {
        // 1. Logic hồi sinh
        if (isDefeated) {
            respawnTimer -= deltaTime / 1000.f;
            if (respawnTimer <= 0) {
                isDefeated = false;
                SetLocalPosition(startX, startY);
                currentState = State::Idle;

                // Hồi sinh xong thì bật lại hộp va chạm vật lý
                colliderManager->Add(collider);
            }
            return;
        }

        if (auto player = targetPlayer.lock()) {
            // 2. Logic Cooldown (Ghost Mode - Xuyên thấu)
            if (postBattleCooldown > 0) {
                postBattleCooldown -= deltaTime / 1000.f;
                if (postBattleCooldown <= 0) {
                    // Hết tàng hình -> Bật lại hộp chặn đường
                    colliderManager->Add(collider);
                }
                else {
                    return; // Đang cooldown thì quái bị đóng băng
                }
            }

            auto [px, py] = player->GetWorldPosition();
            auto [ex, ey] = GetWorldPosition();

            float dx = px - ex;
            float dy = py - ey;
            float distanceSq = dx * dx + dy * dy;

            // 3. Kích hoạt trận đánh
            if (distanceSq < 900.f) {
                auto app = DX9GF::Application::GetInstance();
                auto sceMan = game->GetSceneManager();

                currentState = State::Idle;
                postBattleCooldown = 2.0f;

                // TẮT COLLISION: Rút hộp va chạm ra để Player có thể đi xuyên qua lúc cooldown
                colliderManager->Remove(collider);

                auto battleScene = new MapBattleScene(game, player, app->GetScreenWidth(), app->GetScreenHeight(), encounterData);

                battleScene->SetOnVictoryCallback([this]() {
                    this->isDefeated = true;
                    this->respawnTimer = 180.f;
                    this->postBattleCooldown = 0.f;
                    });

                sceMan->InsertScene(sceMan->GetIndex() + 1, battleScene);
                sceMan->GoToNext();
                return;
            }

            // Xử lý AI State
            if (currentState == State::Idle || currentState == State::Patrol) {
                if (distanceSq < aggroRadius * aggroRadius) {
                    currentState = State::Chase;
                }
            }
            else if (currentState == State::Chase) {
                if (distanceSq > returnRadius * returnRadius) {
                    currentState = State::Return;
                }
            }
            else if (currentState == State::Return) {
                float homeDx = startX - ex;
                float homeDy = startY - ey;

                if (homeDx * homeDx + homeDy * homeDy < 400.f) {
                    currentState = State::Idle;
                }
                else if (distanceSq < aggroRadius * aggroRadius) {
                    currentState = State::Chase;
                }
            }

            // Tính toán hướng di chuyển
            D3DXVECTOR2 dir{ 0, 0 };
            if (currentState == State::Chase) {
                D3DXVECTOR2 targetDir(dx, dy);
                D3DXVec2Normalize(&dir, &targetDir);
            }
            else if (currentState == State::Return) {
                D3DXVECTOR2 targetDir(startX - ex, startY - ey);
                D3DXVec2Normalize(&dir, &targetDir);
            }

            // Trượt tường và lật hình
            if (dir.x != 0 || dir.y != 0) {
                float moveX = dir.x * speed * (deltaTime / 1000.f);
                float moveY = dir.y * speed * (deltaTime / 1000.f);
                auto [finalDX, finalDY] = colliderManager->GetSlidingDeltas(collider, moveX, moveY);
                SetLocalPosition(ex + finalDX, ey + finalDY);

                // KHÔNG DÙNG HARDCODE: Lấy scale tĩnh từ IGameObject ra để lật
                float baseScaleX = std::abs(this->GetLocalScaleX());
                float baseScaleY = std::abs(this->GetLocalScaleY());

                // Lật ngang (đảo dấu trục X), bắt buộc giữ nguyên trục Y (baseScaleY) để không bị lộn ngược
                sprite->SetScale(dir.x > 0 ? -baseScaleX : baseScaleX, baseScaleY);
            }
        }
    }

    void MapEnemy::Draw(DX9GF::Camera* camera, unsigned long long deltaTime) {
        if (isDefeated) return;

        // Hiệu ứng nhấp nháy nếu đang bị "choáng" sau khi Flee
        if (postBattleCooldown > 0) {
            if (static_cast<int>(postBattleCooldown * 10) % 2 == 0) return;
        }

        sprite->Begin();
        auto [x, y] = GetWorldPosition();
        sprite->SetPosition(x, y);
        sprite->Draw(*camera, deltaTime);
        sprite->End();
    }
}