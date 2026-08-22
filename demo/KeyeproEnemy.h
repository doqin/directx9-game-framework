#pragma once
#include "IEnemy.h"
#include <memory>
namespace Demo {
    class KeyeproEnemy : public EnemyBase<KeyeproEnemy> {
    private:
        std::shared_ptr<DX9GF::Texture> texture;
        std::shared_ptr<DX9GF::AnimatedSprite> sprite;
        std::shared_ptr<DX9GF::Texture> projTexture;
        std::vector<RECT> projFrames;
        std::shared_ptr<DX9GF::Texture> batProjTexture;
        std::vector<RECT> batProjFrames;
        std::weak_ptr<Player> player;

        //ability vars
        int currentCycle = -1;
        int skillTurnThisCycle = -1;

        int GetRandomPattern();
        void PatternTargetedSniping(float projDamage);
        void PatternEcholocation(float projDamage);
        void PatternSwoopBite(float projDamage);
        void PatternShatterVolley(float projDamage);
        void PatternFanning(float projDamage);
    public:
        using EnemyBase<KeyeproEnemy>::EnemyBase;
        void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera);
        void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) override;

        void OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn, std::vector<std::shared_ptr<IEnemy>>* enemies, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) override;
        void StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) override;

        float GetBodyHeight() const override { return 256.f; }
        float GetBodyWidth() const override { return 256.f; }
    };
}