#pragma once
#include "WorldSceneBase.h"

namespace Demo {
	class LabInsideScene : public WorldSceneBase {
		bool isBossDead = false;

	public:
		LabInsideScene(Game* game, std::shared_ptr<DX9GF::SaveManager> sm, UINT sw, UINT sh)
			: WorldSceneBase(game, sm, sw, sh) {
		}
		std::string GetSaveID() const override;

	protected:
		void OnInit() override;
		void OnGenerateSaveData(nlohmann::json& outData) override;
		void OnRestoreSaveData(const nlohmann::json& inData) override;
		void DrawBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) override;
	};
}
