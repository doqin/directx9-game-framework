#include "pch.h"
#include "RandomEncounter.h"
#include <random>
#include <chrono>
#include "CustomBattleScene.h"
#include "Game.h"
#include "TransitionCommand.h"

std::function<void(const DX9GF::Map::ObjectArea&)> Demo::GetRandomEncounterFunc(
	DX9GF::IGame* game,
	std::shared_ptr<Player> player,
	std::vector<std::pair<std::string, int>> possibleEnemies,
	std::shared_ptr<DX9GF::CommandBuffer> drawBuffer,
	std::shared_ptr<DX9GF::CommandBuffer> commandBuffer,
	bool* isGamePaused,
	std::function<void(DX9GF::GraphicsDevice*, unsigned long long)> customBackgroundDraw
)
{
	auto lastEncounterTime = std::chrono::steady_clock::now();
	const int COOLDOWN_SECONDS = 5;

	return [game, player, possibleEnemies, drawBuffer, commandBuffer, lastEncounterTime, COOLDOWN_SECONDS, isGamePaused, customBackgroundDraw](const DX9GF::Map::ObjectArea& area) mutable {
		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(now - lastEncounterTime).count() < COOLDOWN_SECONDS) {
			return;
		}

		if (player->IsWalking()) {
			const int CHANCE = 1; // in percentage
			const int MAX = 100;
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> dis(1, MAX);

			if (dis(gen) <= MAX * CHANCE / 100) {
				lastEncounterTime = now;
				std::map<std::string, int> possibleEnemiesMap;
				for (const auto& [name, chance] : possibleEnemies) {
					possibleEnemiesMap[name] = chance;
				}                
				auto demoGame = dynamic_cast<Demo::Game*>(game);
				auto app = DX9GF::Application::GetInstance();
				auto battleScene = new CustomBattleScene(demoGame, player, app->GetScreenWidth(), app->GetScreenHeight(), possibleEnemiesMap);
				battleScene->SetCustomBackgroundDraw(customBackgroundDraw);
				auto sceMan = game->GetSceneManager();
				sceMan->InsertScene(sceMan->GetIndex() + 1, battleScene);

				commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([isGamePaused](std::function<void()> markFinished) {
					*isGamePaused = true;
					markFinished();
				}));
				auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), 1.f, true);
				drawBuffer->StackCommand(transitionInCommand);
				commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([sceMan, transitionInCommand, drawBuffer, game, commandBuffer, isGamePaused](std::function<void()> markFinished) {
					if (!transitionInCommand->IsFinished()) {
						return;
					}
					sceMan->GoToNext();
					markFinished();
				}));
				drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), 1.f, false));
				drawBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([isGamePaused](std::function<void()> markFinished) {
					*isGamePaused = false;
					markFinished();
				}));
			}
		}
	};
}
