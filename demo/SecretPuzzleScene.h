#pragma once
#include "WorldSceneBase.h"
#include "CupidNPC.h"

namespace Demo {
	class SecretPuzzleScene : public WorldSceneBase {
		bool isBossDead = false;
		bool hasSetInitialQuest = false;
		bool questRestoredFromSave = false;
		bool questGiven = false;

		std::shared_ptr<CupidNPC> cupidNPC;

		void StartCupidConversation();
		void StartCupidBattle();

	public:
		SecretPuzzleScene(Game* game, std::shared_ptr<DX9GF::SaveManager> sm, UINT sw, UINT sh)
			: WorldSceneBase(game, sm, sw, sh) {
		}
		std::string GetSaveID() const override;
		void GiveTestItems();

	protected:
		void OnInit() override;
		void OnUpdate(unsigned long long deltaTime) override;
		void OnDrawWorld(std::vector<DepthNode>& depthNodes, unsigned long long deltaTime) override;
		void OnDrawUI(unsigned long long deltaTime) override;
		void OnGenerateSaveData(nlohmann::json& outData) override;
		void OnRestoreSaveData(const nlohmann::json& inData) override;
		void DrawBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) override;
	};
}
