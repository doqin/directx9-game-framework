#pragma once
#include "WorldSceneBase.h"
#include "HackTerminal.h"
#include "KeyeproNPC.h"
#include "RustyChestNPC.h"

namespace Demo {
	class BossWorldScene : public WorldSceneBase {
		std::shared_ptr<DX9GF::Texture> gateTexture;
		std::shared_ptr<DX9GF::StaticSprite> gateSprite;

		std::vector<std::shared_ptr<HackTerminal>> hackMachines;
		std::shared_ptr<HackTerminal> mainTerminal;

		int currentHackStep = 0;
		bool isBossDoorUnlocked = false;
		bool hasGottenUselessItem = false;
		bool isFinalBossDead = false;
		int currentIslandID = 1;

		std::shared_ptr<KeyeproNPC> keyeproNPC;
		std::shared_ptr<DX9GF::RectangleCollider> bossGateCollider;
		std::shared_ptr<RustyChestNPC> rustyChest;

		void StartKeyeproConversation();
		void StartKeyeproBattle();

	public:
		BossWorldScene(Game* game, std::shared_ptr<DX9GF::SaveManager> sm, UINT sw, UINT sh)
			: WorldSceneBase(game, sm, sw, sh) {
		}
		std::string GetSaveID() const override;
		void OnTerminalHacked(int terminalID);
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
