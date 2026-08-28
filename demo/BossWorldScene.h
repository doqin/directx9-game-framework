#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Game.h"
#include "Player.h"
#include "HackTerminal.h"
#include "InventoryMenu.h"
#include "NPC.h"
#include "IConversation.h"
#include "SavePoint.h"
#include "ShopPoint.h"
#include "HealingPoint.h"
#include "RustyChestNPC.h"
#include "TreasureChestNPC.h"
#include "PlayerHUD.h"
#include "MapEnemy.h"
#include "PopUpMessage.h"
#include "ChapterTitleUI.h"

namespace Demo {
	class BossWorldScene : public DX9GF::IScene, public DX9GF::ISaveable {
		bool isGamePaused = false;
		bool hasSeenChapterIntro = false;

		Game* game;
		std::shared_ptr<DX9GF::ColliderManager> colliderManager;
		std::shared_ptr<DX9GF::TransformManager> transformManager;
		std::shared_ptr<Demo::DraggableManager> draggableManager;
		std::shared_ptr<InventoryMenu> inventoryMenu;
		std::shared_ptr<PlayerHUD> playerHUD;

		std::shared_ptr<DX9GF::Font> font;
		std::shared_ptr<Player> player;
		std::shared_ptr<DX9GF::Map> map;
		std::shared_ptr<DX9GF::CommandBuffer> drawBuffer;
		std::shared_ptr<DX9GF::CommandBuffer> commandBuffer;
		std::shared_ptr<DX9GF::SaveManager> saveManager;
		std::shared_ptr<ChapterTitleUI> chapterTitleUI;

		std::shared_ptr<DX9GF::Texture> gateTexture;
		std::shared_ptr<DX9GF::StaticSprite> gateSprite;

		std::vector<std::shared_ptr<HackTerminal>> hackMachines;
		std::shared_ptr<HackTerminal> mainTerminal;
		std::vector<std::shared_ptr<MapEnemy>> mapEnemies;

		int currentHackStep = 0;
		bool isBossDoorUnlocked = false;
		bool hasGottenUselessItem = false;
		bool isTransitioning = false;
	public:
		BossWorldScene(Game* game, std::shared_ptr<DX9GF::SaveManager> sm, UINT sw, UINT sh) : IScene(sw, sh), game(game), saveManager(sm) {}
		void Init() override;
		void Update(unsigned long long deltaTime) override;
		void DrawWorld(unsigned long long deltaTime) override;
		void DrawUI(unsigned long long deltaTime) override;
		void OnTerminalHacked(int terminalID);

		std::string GetSaveID() const override;
		void GenerateSaveData(nlohmann::json& outData) override;
		void RestoreSaveData(const nlohmann::json& inData) override;

		void GiveTestItems();
		std::shared_ptr<Player> GetPlayer() const { return player; }
	private:
		std::vector<std::shared_ptr<NPC>> mapNPCs;

		std::shared_ptr<IConversation> currentConversation;

		std::shared_ptr<INPC> activeNPC = nullptr;
		std::shared_ptr<PopUpMessage> popUpMessage;

		std::vector<std::shared_ptr<SavePoint>> savePoints;
		std::vector<std::shared_ptr<HealingPoint>> healingPoints;
		std::vector<std::shared_ptr<ShopPoint>> shopPoints;

		std::shared_ptr<DX9GF::RectangleCollider> bossGateCollider;

		std::shared_ptr<RustyChestNPC> rustyChest;
		std::vector<std::shared_ptr<TreasureChestNPC>> treasureChests;
		bool isFinalBossDead = false;

		int currentIslandID = 1;
		void DrawBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime, int islandID);
	};
}