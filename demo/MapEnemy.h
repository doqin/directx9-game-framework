#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Player.h"
#include "BattleEncounter.h"
#include <deque>

namespace Demo {
    class MapEnemy : public DX9GF::IGameObject {
    private:
        enum class State { Idle, Patrol, Chase, Return };

        State currentState = State::Idle;
        BattleEncounter encounterData;

        float startX, startY;
        float speed = 75.f;
        float aggroRadius = 80.f;
        float returnRadius = 150.f;

        // THÊM: Giới hạn "xích cổ" để quái không bị dắt đi quá xa khỏi điểm gốc
        float tetherRadius = 400.f;

        std::shared_ptr<DX9GF::Texture> texture;
        std::shared_ptr<DX9GF::AnimatedSprite> sprite;
        std::shared_ptr<DX9GF::RectangleCollider> collider;

        std::weak_ptr<Player> targetPlayer;
        DX9GF::ColliderManager* colliderManager = nullptr;
        Game* game = nullptr;

        bool isDefeated = false;
        float respawnTimer = 0.f;
        float postBattleCooldown = 0.f;

        // Hàng đợi lưu vết chân
        std::deque<D3DXVECTOR2> chasePath;
        std::deque<D3DXVECTOR2> returnPath;
        D3DXVECTOR2 lastPlayerPos{ 0, 0 };
        D3DXVECTOR2 lastEnemyPos{ 0, 0 };

        // Hàm giả lập Raycast để làm Hybrid AI
        bool CheckLineOfSight(float startX, float startY, float targetX, float targetY);

    public:
        MapEnemy(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y, const BattleEncounter& data);
        ~MapEnemy();

        void Init(Game* game, DX9GF::GraphicsDevice* gd, DX9GF::ColliderManager* colMan, std::shared_ptr<Player> player);
        void Update(unsigned long long deltaTime);
        void Draw(DX9GF::Camera* camera, unsigned long long deltaTime);

        // --- CÁC HÀM PHỤC VỤ SAVE/LOAD GAME ---
        std::string GetEnemyID() const { return encounterData.mapEnemyID; }
        bool IsDefeated() const { return isDefeated; }
        float GetRespawnTimer() const { return respawnTimer; }

        void SetDefeatedState(bool defeated, float timer) {
            isDefeated = defeated;
            respawnTimer = timer;
            // Nếu nạp game lên mà quái đang chết, lập tức gỡ hộp va chạm để Player không đụng trúng tàng hình
            if (isDefeated && colliderManager && collider) {
                colliderManager->Remove(collider);
            }
        }
    };
}