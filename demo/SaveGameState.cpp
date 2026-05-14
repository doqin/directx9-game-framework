#include "pch.h"
#include "SaveGameState.h"
#include "Game.h"
#include "Player.h"
#include "TutorialWorldScene.h"
#include "ThreadAlleyScene.h"

namespace Demo {
   SaveGameState::SaveGameState(Game* game, std::shared_ptr<DX9GF::SaveManager> saveManager)
        : game(game), saveManager(std::move(saveManager)) {
    }

    void SaveGameState::BuildScenes() {
        auto app = DX9GF::Application::GetInstance();
        auto sceneManager = game->GetSceneManager();
        sceneManager->PushScene(new TutorialWorldScene(game, saveManager, app->GetScreenWidth(), app->GetScreenHeight()));
        sceneMap["TutorialWorldScene"] = sceneManager->GetSceneCount() - 1;
        sceneManager->PushScene(new ThreadAlleyScene(game, saveManager, app->GetScreenWidth(), app->GetScreenHeight()));
        sceneMap["ThreadAlleyScene"] = sceneManager->GetSceneCount() - 1;
    }

    void SaveGameState::ClearScenes()
    {
        while(game->GetSceneManager()->GetSceneCount() > 1) {
            game->GetSceneManager()->PopScene();
		}
    }

    std::shared_ptr<Player> SaveGameState::GetPlayerFromScene(DX9GF::IScene* scene) const {
        if (auto tutorialScene = dynamic_cast<Demo::TutorialWorldScene*>(scene)) {
            return tutorialScene->GetPlayer();
        }
        if (auto threadScene = dynamic_cast<Demo::ThreadAlleyScene*>(scene)) {
            return threadScene->GetPlayer();
        }
        return nullptr;
    }

    std::string SaveGameState::GetSaveID() const {
        return "Game";
    }

    void SaveGameState::GenerateSaveData(nlohmann::json& outData) {
        auto sceneManager = game->GetSceneManager();
        auto currentScene = dynamic_cast<DX9GF::ISaveable*>(sceneManager->GetCurrentScene());
        if (currentScene) {
            outData["current_scene"] = currentScene->GetSaveID();
        }
        if (auto player = GetPlayerFromScene(sceneManager->GetCurrentScene())) {
            player->GenerateSaveGlobalData(outData["player"]);
        }
        for (size_t i = 0; i < sceneManager->GetSceneCount(); i++) {
            nlohmann::json data;
            auto scene = dynamic_cast<DX9GF::ISaveable*>(sceneManager->GetScene(i));
            if (scene) {
                scene->GenerateSaveData(data);
                outData["scene_data"][scene->GetSaveID()] = data;
            }
        }
    }

    void SaveGameState::RestoreSaveData(const nlohmann::json& inData) {
        auto sceneManager = game->GetSceneManager();
        if (inData.contains("current_scene")) {
            const std::string sceneId = inData["current_scene"].get<std::string>();
            auto sceneIt = sceneMap.find(sceneId);
            if (sceneIt != sceneMap.end()) {
                sceneManager->GoToScene(sceneIt->second);
            }
        }
        if (inData.contains("player")) {
            if (auto player = GetPlayerFromScene(sceneManager->GetCurrentScene())) {
                player->RestoreSaveGlobalData(inData["player"]);
            }
        }
        if (inData.contains("scene_data")) {
            for (auto& [key, value] : inData["scene_data"].items()) {
                auto sceneIt = sceneMap.find(key);
                if (sceneIt != sceneMap.end()) {
                    auto scene = dynamic_cast<DX9GF::ISaveable*>(sceneManager->GetScene(sceneIt->second));
                    if (scene) {
                        scene->RestoreSaveData(value);
                    }
                }
            }
        }
    }

  std::shared_ptr<SaveGameState> SaveGameState::StartNewGame(Game* game, const std::shared_ptr<DX9GF::SaveManager>& saveManager) {
        saveManager->Clear();
        auto saveState = std::make_shared<SaveGameState>(game, saveManager);
        saveManager->Register(saveState.get());
        saveState->ClearScenes();
        saveState->BuildScenes();
        auto sceneIt = saveState->sceneMap.find("ThreadAlleyScene");
        if (sceneIt != saveState->sceneMap.end()) {
            game->GetSceneManager()->GoToScene(sceneIt->second);
        }
       return saveState;
    }

 std::shared_ptr<SaveGameState> SaveGameState::LoadSavedGame(Game* game, const std::shared_ptr<DX9GF::SaveManager>& saveManager) {
        saveManager->Clear();
        auto saveState = std::make_shared<SaveGameState>(game, saveManager);
        saveManager->Register(saveState.get());
		saveState->ClearScenes();
        saveState->BuildScenes();
        saveManager->Load("savegame.json");
       return saveState;
    }
}
