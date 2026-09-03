#pragma once
#include "WorldSceneBase.h"
#include "AuthTerminal.h"
#include "TrojanNPC.h"
#include "SketchyGuyNPC.h"
#include "PackOpeningScene.h"

namespace Demo {
	class ThreadAlleyScene : public WorldSceneBase {
		std::shared_ptr<AuthTerminal> authTerminal;
		std::shared_ptr<TrojanNPC> trojanNPC;
		std::shared_ptr<SketchyGuyNPC> sketchyGuy;
		std::string authPassword;
		bool authTerminalSolved = false;

		float bgBaseScrollX = 0;
		float bgBaseScrollY = 0;
		float bgPeriodTimer = 0;
		float bgEaseProgress = 0;
		float bgOddRowShift = 0;
		float bgEvenRowShift = 0;
		int bgAnimPhase = 0;
		D3DCOLOR bgBlinkColor = D3DCOLOR_XRGB(80, 80, 80);
		D3DCOLOR bgBaseColor1 = D3DCOLOR_ARGB(0, 20, 20, 20);
		D3DCOLOR bgBaseColor2 = 0xFF793a80;

		void DrawCheckerBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime);
		void StartTrojanConversation();
		void StartTrojanBattle();
		void StartSketchyGuyInteraction();

	public:
		ThreadAlleyScene(Game* game, std::shared_ptr<DX9GF::SaveManager> sm, UINT sw, UINT sh)
			: WorldSceneBase(game, sm, sw, sh) {
		}
		std::string GetSaveID() const override;
		void Update(unsigned long long deltaTime) override;
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
