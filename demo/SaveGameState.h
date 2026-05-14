#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include <string>
#include <unordered_map>

namespace Demo {
    class Game;
    class Player;

    class SaveGameState : public DX9GF::ISaveable {
        Game* game;
        std::shared_ptr<DX9GF::SaveManager> saveManager;
        std::unordered_map<std::string, size_t> sceneMap;
        void BuildScenes();
		void ClearScenes();
        std::shared_ptr<Player> GetPlayerFromScene(DX9GF::IScene* scene) const;
    public:
        SaveGameState(Game* game, std::shared_ptr<DX9GF::SaveManager> saveManager);
        std::string GetSaveID() const override;
        void GenerateSaveData(nlohmann::json& outData) override;
        void RestoreSaveData(const nlohmann::json& inData) override;
        static std::shared_ptr<SaveGameState> StartNewGame(Game* game, const std::shared_ptr<DX9GF::SaveManager>& saveManager);
        static std::shared_ptr<SaveGameState> LoadSavedGame(Game* game, const std::shared_ptr<DX9GF::SaveManager>& saveManager);
    };
}
