#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Game.h"
#include "Player.h"
#include "SavePoint.h"
#include "InventoryMenu.h"
#include "ShopPoint.h"
#include "HealingPoint.h"
#include "NPC.h"
#include "IConversation.h"
#include "TreasureChestNPC.h"
#include "PopUpMessage.h"
#include "PlayerHUD.h"
#include "MapEnemy.h"
#include "ChapterTitleUI.h"

namespace Demo {
	class WorldSceneBase : public DX9GF::IScene, public DX9GF::ISaveable {
	protected:
		bool isGamePaused = false;
		bool isTransitioning = false;
		bool hasSeenChapterIntro = false;

		Game* game;
		std::shared_ptr<DX9GF::ColliderManager> colliderManager;
		std::shared_ptr<DX9GF::TransformManager> transformManager;
		std::shared_ptr<Demo::DraggableManager> draggableManager;
		std::shared_ptr<InventoryMenu> inventoryMenu;
		std::shared_ptr<PlayerHUD> playerHUD;
		std::shared_ptr<DX9GF::SaveManager> saveManager;

		std::vector<std::shared_ptr<SavePoint>> savePoints;
		std::vector<std::shared_ptr<ShopPoint>> shopPoints;
		std::vector<std::shared_ptr<HealingPoint>> healingPoints;
		std::shared_ptr<DX9GF::Font> font;
		std::shared_ptr<Player> player;
		std::shared_ptr<DX9GF::Map> map;
		std::shared_ptr<DX9GF::CommandBuffer> drawBuffer;
		std::shared_ptr<DX9GF::CommandBuffer> commandBuffer;
		std::shared_ptr<PopUpMessage> popUpMessage;
		std::wstring chapterTitle = L"Unnamed";
		std::wstring chapterSubtitle = L"< No Subtitle >";
		std::shared_ptr<ChapterTitleUI> chapterTitleUI;
		std::shared_ptr<IConversation> currentConversation;
		std::shared_ptr<INPC> activeNPC = nullptr;
		std::vector<std::shared_ptr<NPC>> mapNPCs;
		std::vector<std::shared_ptr<TreasureChestNPC>> treasureChests;
		std::vector<std::shared_ptr<MapEnemy>> mapEnemies;
		std::function<void()> onConversationEnd;

		WorldSceneBase(Game* game, std::shared_ptr<DX9GF::SaveManager> sm, UINT sw, UINT sh);

		void Init() override;
		void InitCore(float playerX, float playerY, const wchar_t* mapFile);

		void Update(unsigned long long deltaTime) override;
		void DrawWorld(unsigned long long deltaTime) override;
		void DrawUI(unsigned long long deltaTime) override;

		void GenerateSaveData(nlohmann::json& outData) override;
		void RestoreSaveData(const nlohmann::json& inData) override;

		void OpenChestWithDialog(std::shared_ptr<TreasureChestNPC>& chest);

		void SetChapterTitle(const std::wstring& title, const std::wstring& subtitle);

		struct DepthNode {
			float y;
			std::function<void()> drawCall;
			bool operator<(const DepthNode& other) const { return y < other.y; }
		};

		void AddDepthNode(std::vector<DepthNode>& nodes, float y, std::function<void()> drawCall);

		void CreatePortalTransition(int sceneOffset, float targetX, float targetY, const char* bgm = nullptr, float bgmVol = 0.3f);

		void SpawnMapEnemy(float x, float y, std::string id, std::vector<std::string> types,
			bool isRand, bool isGlobal, std::function<void(DX9GF::GraphicsDevice*, unsigned long long)> bgDraw,
			int tokenChance = 30, const std::string& questEvent = "", const std::string& questId = "");

		virtual void OnInit() = 0;
		virtual void OnUpdate(unsigned long long deltaTime) {}
		// Hook: a subclass-owned modal (e.g. a terminal UI) that should freeze the
		// world and suppress the shared inventory / interaction input while it is up.
		virtual bool IsSubsceneModalActive() const { return false; }
		virtual void OnDrawWorld(std::vector<DepthNode>& depthNodes, unsigned long long deltaTime) {}
		virtual void OnDrawUI(unsigned long long deltaTime) {}
		virtual void OnGenerateSaveData(nlohmann::json& outData) {}
		virtual void OnRestoreSaveData(const nlohmann::json& inData) {}
		virtual void DrawBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) = 0;

	public:
		std::shared_ptr<Player> GetPlayer() const { return player; }
	};
}
