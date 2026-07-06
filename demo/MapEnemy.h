// MapEnemy.h
#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Player.h"
#include "BattleEncounter.h"

namespace Demo {
    class MapEnemy : public DX9GF::IGameObject {
    private:
        enum class State { Idle, Patrol, Chase, Return };

        State currentState = State::Idle;
        BattleEncounter encounterData;

        float startX, startY;
        float speed = 50.f;
        float aggroRadius = 150.f;
        float returnRadius = 300.f;

        std::shared_ptr<DX9GF::Texture> texture;
        std::shared_ptr<DX9GF::AnimatedSprite> sprite;
        std::shared_ptr<DX9GF::RectangleCollider> collider;

        std::weak_ptr<Player> targetPlayer;
        DX9GF::ColliderManager* colliderManager = nullptr;
        Game* game = nullptr;

        bool isDefeated = false;
        float respawnTimer = 0.f;
        float postBattleCooldown = 0.f;

    public:
        MapEnemy(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y, const BattleEncounter& data);
        ~MapEnemy();

        void Init(Game* game, DX9GF::GraphicsDevice* gd, DX9GF::ColliderManager* colMan, std::shared_ptr<Player> player);
        void Update(unsigned long long deltaTime);
        void Draw(DX9GF::Camera* camera, unsigned long long deltaTime);
    };
}