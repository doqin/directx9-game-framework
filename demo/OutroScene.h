#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "DX9GFIScene.h"
#include "Game.h"
#include "IConversation.h"

namespace Demo {
    class OutroScene : public DX9GF::IScene {
    private:
        Game* game;
        std::shared_ptr<DX9GF::Font> font;
        std::shared_ptr<DX9GF::FontSprite> fontSprite;
        std::shared_ptr<IConversation> conversation;
        bool hasTransitioned = false;
    public:
        OutroScene(Game* game, int screenWidth, int screenHeight)
            : IScene(screenWidth, screenHeight), game(game){}
        void Init() override;
        void Update(unsigned long long deltaTime) override;
        void DrawWorld(unsigned long long deltaTime) override;
        void DrawUI(unsigned long long deltaTime) override;
    };
}
