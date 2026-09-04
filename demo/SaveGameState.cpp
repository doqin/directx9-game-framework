#include "pch.h"
#include "SaveGameState.h"
#include "Game.h"
#include "Player.h"
#include "PlayerGlobalData.h"
#include "SketchyGuyGlobalData.h"
#include "IntroScene.h"
#include "TutorialWorldScene.h"
#include "LabInsideScene.h"
#include "SecretPuzzleScene.h"
#include "ThreadAlleyScene.h"
#include "BossWorldScene.h"
#include "QuestManager.h"

namespace Demo {
	SaveGameState::SaveGameState(Game* game, std::shared_ptr<DX9GF::SaveManager> saveManager)
		: game(game), saveManager(std::move(saveManager)) {
	}

	void SaveGameState::BuildScenes() {
		auto app = DX9GF::Application::GetInstance();
		auto sceneManager = game->GetSceneManager();
		sceneManager->PushScene(new IntroScene(game, app->GetScreenWidth(), app->GetScreenHeight()));
		sceneManager->PushScene(new TutorialWorldScene(game, saveManager, app->GetScreenWidth(), app->GetScreenHeight()));
		sceneMap["TutorialWorldScene"] = sceneManager->GetSceneCount() - 1;
		sceneManager->PushScene(new LabInsideScene(game, saveManager, app->GetScreenWidth(), app->GetScreenHeight()));
		sceneMap["LabInsideScene"] = sceneManager->GetSceneCount() - 1;
		sceneManager->PushScene(new SecretPuzzleScene(game, saveManager, app->GetScreenWidth(), app->GetScreenHeight()));
		sceneMap["SecretPuzzleScene"] = sceneManager->GetSceneCount() - 1;
		sceneManager->PushScene(new ThreadAlleyScene(game, saveManager, app->GetScreenWidth(), app->GetScreenHeight()));
		sceneMap["ThreadAlleyScene"] = sceneManager->GetSceneCount() - 1;
		sceneManager->PushScene(new BossWorldScene(game, saveManager, app->GetScreenWidth(), app->GetScreenHeight()));
		sceneMap["BossWorldScene"] = sceneManager->GetSceneCount() - 1;
	}

	void SaveGameState::ClearScenes()
	{
		while (game->GetSceneManager()->GetSceneCount() > 1) {
			game->GetSceneManager()->PopScene();
		}
	}

	std::shared_ptr<Player> SaveGameState::GetPlayerFromScene(DX9GF::IScene* scene) const {
		if (auto tutorialScene = dynamic_cast<Demo::TutorialWorldScene*>(scene)) {
			return tutorialScene->GetPlayer();
		}
		if (auto secretScene = dynamic_cast<Demo::SecretPuzzleScene*>(scene)) {
			return secretScene->GetPlayer();
		}
		if (auto threadScene = dynamic_cast<Demo::ThreadAlleyScene*>(scene)) {
			return threadScene->GetPlayer();
		}
		if (auto threadScene = dynamic_cast<Demo::BossWorldScene*>(scene)) {
			return threadScene->GetPlayer();
		}
		if (auto labInsideScene = dynamic_cast<Demo::LabInsideScene*>(scene)) {
			return labInsideScene->GetPlayer();
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

		QuestManager::GetInstance()->GenerateSaveData(outData["quest_manager"]);

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

				auto audio = DX9GF::AudioManager::GetInstance();

				if (sceneId == "TutorialWorldScene") {
					audio->PlayBGM_Fade("bgm_tutorial", 0.5f, 1.5f);
				}
				else if (sceneId == "SecretPuzzleScene") {
					audio->PlayBGM_Fade("bgm_secret", 0.3f, 1.5f);
				}
				else if (sceneId == "ThreadAlleyScene") {
					audio->PlayBGM_Fade("bgm_arcade", 0.2f, 1.5f);
				}
				else if (sceneId == "BossWorldScene") {
					audio->PlayBGM_Fade("bgm_boss", 0.3f, 1.5f);
				}
			}
		}
		if (inData.contains("quest_manager")) {
			QuestManager::GetInstance()->RestoreSaveData(inData["quest_manager"]);
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
		PlayerGlobalData::GetInstance()->Reset();
		SketchyGuyGlobalData::GetInstance()->Reset();
		auto saveState = std::make_shared<SaveGameState>(game, saveManager);
		saveManager->Register(saveState.get());
		saveManager->Register(PlayerGlobalData::GetInstance());
		saveManager->Register(SketchyGuyGlobalData::GetInstance());
		saveState->ClearScenes();
		saveState->BuildScenes();
		QuestManager::GetInstance()->Reset();
		DX9GF::AudioManager::GetInstance()->StopAll();
		game->GetSceneManager()->GoToScene(1);
		return saveState;
	}

	std::shared_ptr<SaveGameState> SaveGameState::LoadSavedGame(Game* game, const std::shared_ptr<DX9GF::SaveManager>& saveManager) {
		saveManager->Clear();
		auto saveState = std::make_shared<SaveGameState>(game, saveManager);
		saveManager->Register(saveState.get());
		saveManager->Register(PlayerGlobalData::GetInstance());
		saveManager->Register(SketchyGuyGlobalData::GetInstance());
		saveState->ClearScenes();
		saveState->BuildScenes();
		QuestManager::GetInstance()->Reset();
		saveManager->Load("savegame.json");
		return saveState;
	}
}
