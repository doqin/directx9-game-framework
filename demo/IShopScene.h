#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Game.h"
#include "Player.h"
#include "TextButton.h"
#include "DX9GFFont.h"
#include <string>
#include <vector>
#include <functional>

namespace Demo {
	struct ShopItem {
		std::string name;
		int cost;
		std::function<void()> onBuyAction;
	};

	class IShopScene : public DX9GF::IScene {
	protected: 
		Game* game;
		Player* player;

		DX9GF::Camera uiCamera;
		std::shared_ptr<DX9GF::TransformManager> transformManager;

		std::shared_ptr<DX9GF::Font> myFont;
		std::shared_ptr<DX9GF::FontSprite> myFontSprite;

		std::vector<ShopItem> itemsForSale;
		std::vector<std::shared_ptr<Demo::IButton>> uiButtons;

		std::shared_ptr<DX9GF::Texture> backBufferTexture;
		std::shared_ptr<DX9GF::StaticSprite> backBufferSprite;

		std::string statusMessage = "";
		float messageTimer = 0.0f;
		std::string shopTitle; 
		bool shouldLeave = false;

		void BuildUI();

	public:
		IShopScene(Game* game, Player* player, int screenWidth, int screenHeight, std::string title);
		virtual ~IShopScene() = default;

		virtual void LoadItems() = 0;

		void Init() override;
		void Update(unsigned long long deltaTime) override;
		void Draw(unsigned long long deltaTime) override;

		void ShowMessage(std::string msg);
	};
}