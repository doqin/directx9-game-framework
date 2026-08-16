#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Player.h"
#include "BattleEncounter.h"
#include <deque>
#include "Debug.h"

namespace Demo {
    class MapEnemy : public DX9GF::IGameObject, virtual public Pointable {
    private:
        enum class State { Idle, Patrol, Chase, Return };

        State currentState = State::Idle;
        BattleEncounter encounterData;

        float startX, startY;
        float speed = 75.f;
        float aggroRadius = 80.f;
        float returnRadius = 150.f;
        float tetherRadius = 400.f;

        float losTimer = 0.f;
        bool lastLosResult = true;

        std::shared_ptr<DX9GF::Texture> texture;
        std::shared_ptr<DX9GF::AnimatedSprite> sprite;
        std::shared_ptr<DX9GF::RectangleCollider> collider;

        std::weak_ptr<Player> targetPlayer;
        DX9GF::ColliderManager* colliderManager = nullptr;
        Game* game = nullptr;

        bool isDefeated = false;
        float respawnTimer = 0.f;
        float postBattleCooldown = 0.f;

        // queue to store footprints
        std::deque<D3DXVECTOR2> chasePath;
        std::deque<D3DXVECTOR2> returnPath;
        D3DXVECTOR2 lastPlayerPos{ 0, 0 };
        D3DXVECTOR2 lastEnemyPos{ 0, 0 };

        // simulated raycast function for hybrid ai
        bool CheckLineOfSight(float startX, float startY, float targetX, float targetY);
        std::function<void(std::shared_ptr<MapEnemy>)> onEncounterTriggered;
    public:
        static bool isDisabled;
        MapEnemy(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y, const BattleEncounter& data);
        ~MapEnemy();

		std::pair<float, float> GetPoint() const override {
			return { GetWorldX(), GetWorldY() };
		}
        void Init(Game* game, DX9GF::GraphicsDevice* gd, DX9GF::ColliderManager* colMan, std::shared_ptr<Player> player);
        void Update(unsigned long long deltaTime);
        void Draw(DX9GF::Camera* camera, unsigned long long deltaTime);

        std::string GetEnemyID() const { return encounterData.mapEnemyID; }
        bool IsDefeated() const { return isDefeated; }
        float GetRespawnTimer() const { return respawnTimer; }

        void SetDefeatedState(bool defeated, float timer) {
            isDefeated = defeated;
            respawnTimer = timer;
            if (isDefeated && colliderManager && collider) {
                colliderManager->Remove(collider);
            }
        }
        void SetOnEncounterTriggered(std::function<void(std::shared_ptr<MapEnemy>)> callback) { onEncounterTriggered = callback; }
        const BattleEncounter& GetEncounterData() const { return encounterData; }
    };
}