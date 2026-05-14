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

namespace Demo {
	class ThreadAlleyScene : public DX9GF::IScene, public DX9GF::ISaveable {
		bool isGamePaused = false;
		Game* game;
		DX9GF::Camera uiCamera;
		std::shared_ptr<DX9GF::ColliderManager> colliderManager;
		std::shared_ptr<DX9GF::TransformManager> transformManager;
		std::shared_ptr<Demo::DraggableManager> draggableManager;
		std::shared_ptr<InventoryMenu> inventoryMenu;
		std::shared_ptr<DX9GF::SaveManager> saveManager;

		std::vector<std::shared_ptr<SavePoint>> savePoints;
		std::vector<std::shared_ptr<ShopPoint>> shopPoints;
		std::vector<std::shared_ptr<HealingPoint>> healingPoints;

		std::shared_ptr<DX9GF::Font> font;
		std::shared_ptr<Player> player;
		std::shared_ptr<DX9GF::Map> map;
		std::shared_ptr<DX9GF::CommandBuffer> drawBuffer;
		std::shared_ptr<DX9GF::CommandBuffer> commandBuffer;

		float bgBaseScrollX = 0;
		float bgBaseScrollY = 0;
		float bgPeriodTimer = 0;
		float bgEaseProgress = 0;
		float bgOddRowShift = 0;
		float bgEvenRowShift = 0;

		int bgAnimPhase = 0; // Tracks which square shift we are on
		D3DCOLOR bgBlinkColor = D3DCOLOR_XRGB(80, 80, 80);
		D3DCOLOR bgBaseColor1 = D3DCOLOR_ARGB(0, 20, 20, 20);
		D3DCOLOR bgBaseColor2 = 0xFF793a80;

		void DrawCheckerBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime);

	public:
		ThreadAlleyScene(Game* game, std::shared_ptr<DX9GF::SaveManager> sm, UINT sw, UINT sh) : IScene(sw, sh), game(game), saveManager(sm), uiCamera(sw, sh) {}
		void Init() override;
		void Update(unsigned long long deltaTime) override;
		void Draw(unsigned long long deltaTime) override;

		std::string GetSaveID() const override;
		void GenerateSaveData(nlohmann::json& outData) override;
		void RestoreSaveData(const nlohmann::json& inData) override;

		void GiveTestItems();
      std::shared_ptr<Player> GetPlayer() const { return player; }
	};
}