#include "pch.h"
#include "IShopScene.h"
#include "IconButton.h"

Demo::IShopScene::IShopScene(Game* game, Player* player, int sw, int sh, std::string title)
	: IScene(sw, sh), game(game), player(player), uiCamera(sw, sh), shopTitle(title)
{
	uiCamera.SetPosition(sw / 2.0f, sh / 2.0f);
}

void Demo::IShopScene::Init()
{
	transformManager = std::make_shared<DX9GF::TransformManager>();
    myFont = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 24);
	myFontSprite = std::make_shared<DX9GF::FontSprite>(myFont.get());
	backBufferTexture = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	backBufferTexture->CaptureCurrentBackBuffer();
	backBufferSprite = std::make_shared<DX9GF::StaticSprite>(backBufferTexture.get());
	uiSheetTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice()); 
    uiSheetTex->LoadTexture(L"ui.png"); // load here
	LoadItems();

	BuildUI();

	transformManager->RebuildHierarchy();
}

void Demo::IShopScene::BuildUI()
{
	auto [sw, sh] = uiCamera.GetScreenResolution();
	float leftPadding = sw * 0.12f;
	float rightPadding = sw * 0.12f;
	float listStartY = sh * 0.28f;
	float rowHeight = 70.0f;
	float buyButtonW = 64.0f; 
	float buyButtonX = sw - rightPadding - buyButtonW;

	for (size_t i = 0; i < itemsForSale.size(); ++i) {
		auto item = itemsForSale[i];

		auto buyBtn = std::make_shared<Demo::IconButton>(
			transformManager,
            buyButtonX, listStartY + (i * rowHeight), 64, 64,
			uiSheetTex, 3
		);
        buyBtn->SetSpriteCoords(0, 240, 32, 32, 0, false);
		buyBtn->SetSpriteScale(2, 2);
        buyBtn->SetOnReleaseLeft([this, item](DX9GF::ITrigger* t) {
            if (this->player->GetGold() >= item.cost) {
                this->player->AddGold(-item.cost);

                if (item.onBuyAction) {
                    item.onBuyAction();
                }

                this->ShowMessage("Bought " + item.name + "!");
            }
            else {
                this->ShowMessage("Not enough gold!");
            }
        });
		buyBtn->Init(&uiCamera);
		uiButtons.push_back(buyBtn);
	}

	auto leaveBtn = std::make_shared<Demo::IconButton>(
		transformManager,
        leftPadding, sh * 0.15f, 96, 64,
		uiSheetTex, 3
	);
    leaveBtn->SetSpriteCoords(144, 240, 48, 32, 0, false);
    leaveBtn->SetSpriteScale(2, 2);
    leaveBtn->SetOnReleaseLeft([this](DX9GF::ITrigger* t) {
        this->shouldLeave = true;
    });
	leaveBtn->Init(&uiCamera);
	uiButtons.push_back(leaveBtn);
}

void Demo::IShopScene::ShowMessage(std::string msg) {
	statusMessage = msg;
	messageTimer = 2.0f;
}

void Demo::IShopScene::Update(unsigned long long deltaTime)
{
	auto inpMan = DX9GF::InputManager::GetInstance();
	inpMan->ReadMouse(deltaTime);
	inpMan->ReadKeyboard(deltaTime);
	uiCamera.Update();

	if (messageTimer > 0) {
		messageTimer -= deltaTime / 1000.0f;
		if (messageTimer <= 0) statusMessage = "";
	}

	for (auto& btn : uiButtons) {
		btn->Update(deltaTime);
	}

	if (shouldLeave) {
		auto sceMan = this->game->GetSceneManager();

		sceMan->PopScene();
		sceMan->GoToPrevious();
	}
}

void Demo::IShopScene::Draw(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	gd->Clear();
	auto [sw, sh] = uiCamera.GetScreenResolution();
	float centerX = 0.0f;
	float leftEdge = -sw / 2.0f;
	float topEdge = -sh / 2.0f;
	float bottomEdge = sh / 2.0f;

	if (SUCCEEDED(gd->BeginDraw())) {
		if (backBufferSprite) {
			backBufferSprite->Begin();
			backBufferSprite->SetPosition(0.0f, 0.0f);
			backBufferSprite->Draw(uiCamera, deltaTime);
			backBufferSprite->End();
		}

		gd->SetAlphaBlending(true);
		gd->DrawRectangle(uiCamera, 0, 0, sw, sh, 0, 1, 1, 0, 0, D3DXCOLOR(0, 0, 0, 0.65f), true);
		gd->DrawRectangle(uiCamera, sw * 0.08f, sh * 0.14f, sw * 0.84f, sh * 0.72f, 0, 1, 1, 0, 0, D3DXCOLOR(0.12f, 0.12f, 0.16f, 0.95f), true);
		gd->SetAlphaBlending(false);

		for (auto& btn : uiButtons) {
			btn->Draw(gd, deltaTime);
		}

		if (myFontSprite) {
			myFontSprite->Begin();
			myFontSprite->SetPosition(sw - 170.0f, sh * 0.15f);
			myFontSprite->SetColor(0xFFFFD700);
			myFontSprite->SetScale(1.5f, 1.5f);
            myFontSprite->SetText(std::to_wstring(player->GetGold()) + L"G");
			myFontSprite->Draw(uiCamera, deltaTime);
			myFontSprite->SetScale(1.0f, 1.0f);

			myFontSprite->SetColor(0xFFFFFFFF);
			myFontSprite->SetText(std::wstring(shopTitle.begin(), shopTitle.end()));
			auto width = myFontSprite->GetWidth();
			myFontSprite->SetPosition(sw * 0.5f - width * 0.5f, sh * 0.15f);
			myFontSprite->Draw(uiCamera, deltaTime);

			myFontSprite->SetPosition(sw * 0.5f - 180.0f, sh * 0.82f);
			myFontSprite->SetColor(0xFF00FFFF);
			myFontSprite->SetText(std::wstring(statusMessage.begin(), statusMessage.end()));
			myFontSprite->Draw(uiCamera, deltaTime);

          float startY = sh * 0.29f;
			for (size_t i = 0; i < itemsForSale.size(); ++i) {
               myFontSprite->SetPosition(sw * 0.14f, startY + (i * 70.0f));
				myFontSprite->SetColor(0xFFFFFFFF);

               std::string itemText = itemsForSale[i].name + "  " + std::to_string(itemsForSale[i].cost) + "G";
				myFontSprite->SetText(std::wstring(itemText.begin(), itemText.end()));
				myFontSprite->Draw(uiCamera, deltaTime);
			}

			myFontSprite->End();
		}
		DX9GF::Camera cursorCam(sw, sh);
		DX9GF::InputManager::GetInstance()->DrawCursor(&cursorCam, deltaTime);
		gd->EndDraw();
	}
	gd->Present();
}