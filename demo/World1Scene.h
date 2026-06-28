#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Game.h"
#include "Player.h"

namespace Demo {
	class World1Scene : public DX9GF::IScene {
	Game* game;
	std::shared_ptr<Player> player;
	DX9GF::ColliderManager colliderManager;
	std::shared_ptr<DX9GF::TransformManager> transformManager;
	public:
		World1Scene(Game* game, int screenWidth, int screenHeight) : IScene(screenWidth, screenHeight), game(game) {}
		void Init() override;
		void Update(unsigned long long deltaTime) override;
		void DrawWorld(unsigned long long deltaTime) override;
		void DrawUI(unsigned long long deltaTime) override;
	};
}