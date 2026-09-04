#pragma once
#include "WorldSceneBase.h"
#include "SpamNPC.h"

namespace Demo {
	class TutorialWorldScene : public WorldSceneBase {
	public:
		TutorialWorldScene(Game* game, std::shared_ptr<DX9GF::SaveManager> sm, UINT sw, UINT sh)
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

	private:
		bool spamBossDefeated = false;
		std::shared_ptr<SpamNPC> spamNPC;
		void StartSpamConversation();
		void StartSpamBattle();
	};
}
