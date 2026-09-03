#pragma once
#include "WorldSceneBase.h"

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
		void DrawBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) override;
	};
}
