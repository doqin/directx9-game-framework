#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Game.h"
#include "Player.h"
#include "SavePoint.h"
#include "InventoryMenu.h"
#include "StrikeCard.h"
#include "ShopPoint.h"
#include "HealingPoint.h"
#include "DauDauNPC.h"
#include "IConversation.h"

#include "NPC1.h"
#include "CardShop.h"
#include "ItemShop.h"
#include "TreasureChestNPC.h"
#include "PlayerHUD.h"


namespace Demo {
	class SecretPuzzleScene : public DX9GF::IScene, public DX9GF::ISaveable {
		bool isGamePaused = false;
		bool isTransitioning = false;
		bool isBossDead = false;
		bool hasSetInitialQuest = false;
		bool questRestoredFromSave = false;
		bool questGiven = false;

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
		std::shared_ptr<IConversation> currentConversation;

		std::shared_ptr<DauDauNPC> dauDau;
		std::shared_ptr<DauDauNPC> dauDauSpawn;

		std::vector<std::shared_ptr<TreasureChestNPC>> treasureChests;

	public:
		SecretPuzzleScene(Game* game, std::shared_ptr<DX9GF::SaveManager> sm, UINT sw, UINT sh) : IScene(sw, sh), game(game), saveManager(sm) {}
		void Init() override;
		void Update(unsigned long long deltaTime) override;
		void DrawWorld(unsigned long long deltaTime) override;
		void DrawUI(unsigned long long deltaTime) override;
		void DrawBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime);

		// Inherited via ISaveable
		std::string GetSaveID() const override;
		void GenerateSaveData(nlohmann::json& outData) override;
		void RestoreSaveData(const nlohmann::json& inData) override;

		void GiveTestItems();
		std::shared_ptr<Player> GetPlayer() const { return player; }
	};
}