#pragma once
#include "DX9GFIScene.h"
#include "DX9GFTexture.h"
#include "DX9GFSprites.h"
#include "Game.h"

namespace Demo {
    class SplashScene : public DX9GF::IScene {
    private:
        Game* game;
        std::shared_ptr<DX9GF::Texture> logo1Tex, logo2Tex;
        std::shared_ptr<DX9GF::StaticSprite> logo1Sprite, logo2Sprite;

        float timer = 0.0f;
        float alpha = 0.0f;
    public:
        SplashScene(Game* game, int screenW, int screenH) : IScene(screenW, screenH), game(game) {}
        void Init() override;
        void Update(unsigned long long deltaTime) override;
        void DrawWorld(unsigned long long deltaTime) override;
        void DrawUI(unsigned long long deltaTime) override {}; //nah it's useless here
    };
}