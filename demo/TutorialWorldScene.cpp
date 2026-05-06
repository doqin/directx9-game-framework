#include "pch.h"
#include "TutorialWorldScene.h"
#include "RandomEncounter.h"

void Demo::TutorialWorldScene::Init()
{
	camera.SetZoom(2.0f);
	transformManager = std::make_shared<DX9GF::TransformManager>();
	colliderManager = std::make_shared<DX9GF::ColliderManager>();
	player = std::make_shared<Player>(transformManager, 248, 184);
	camera.SetPosition(248, 184);
	player->Init(game->GetGraphicsDevice(), colliderManager.get(), &camera);
	drawBuffer = std::make_shared<DX9GF::CommandBuffer>();
	commandBuffer = std::make_shared<DX9GF::CommandBuffer>();
	map = std::make_shared<DX9GF::Map>(game->GetGraphicsDevice());
	map->Create(transformManager, colliderManager, "./tutorial.tmx");
	map->SetAreaUpdateHandler("triggers", GetRandomEncounterFunc(game, player, {
		{"DemonEyeEnemy", 40},
		{"VampireBatEnemy", 30},
		{"MimicEnemy", 20},
		{"WarlockEnemy", 10},
		{"CupidEnemy", 5}
		}, drawBuffer, commandBuffer, &isGamePaused));
	font = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 16);
	
	npc1 = std::make_shared<DauDauNPC>(transformManager, 167.0f, -18.0f);
	npc1->Init(game->GetGraphicsDevice(), player, colliderManager, font, drawBuffer);
	npc1->AddLine(L"Dau Dau", L"Hello! Welcome.");
	npc1->AddLine(L"Dau Dau", L"How to escape this world? I don't know.");

	savePoint = std::make_shared<SavePoint>(transformManager, 200.0f, 150.0f);
    savePoint->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoint->SetVisible(true);

	shopPoint_Card = std::make_shared<ShopPoint>(transformManager, 300.0f, 150.0f);
	shopPoint_Card->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) {
			return new CardShop(g, p, w, h);
		}
	);
	shopPoint_Card->SetVisible(true);

	shopPoint_BSItem = std::make_shared<ShopPoint>(transformManager, 360.0f, 150.0f);
	shopPoint_BSItem->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) {
			return new ItemShop(g, p, w, h, ShopTier::BASIC);
		}
	);
	shopPoint_BSItem->SetVisible(true);

	shopPoint_HBItem = std::make_shared<ShopPoint>(transformManager, 430.0f, 150.0f);
	shopPoint_HBItem->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) {
			return new ItemShop(g, p, w, h, ShopTier::HYBRID);
		}
	);
	shopPoint_HBItem->SetVisible(true);

	shopPoint_PMItem = std::make_shared<ShopPoint>(transformManager, 500.0f, 150.0f);
	shopPoint_PMItem->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) {
			return new ItemShop(g, p, w, h, ShopTier::PREMIUM);
		}
	);
	shopPoint_PMItem->SetVisible(true);

	healingPoint = std::make_shared<HealingPoint>(transformManager, 250.0f, 220.0f);
	healingPoint->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoint->SetVisible(true);

	draggableManager = std::make_shared<Demo::DraggableManager>();
	inventoryMenu = std::make_shared<InventoryMenu>(game, player, transformManager, draggableManager, &uiCamera, font.get());
	inventoryMenu->Init();

	ItemData::GetInstance()->LoadData();
	this->GiveTestItems();

	transformManager->RebuildHierarchy();
}

void Demo::TutorialWorldScene::Update(unsigned long long deltaTime)
{
	auto [currentWidth, currentHeight] = camera.GetScreenResolution();
	auto [lastWidth, lastHeight] = uiCamera.GetScreenResolution();
	if (currentWidth != lastWidth || currentHeight != lastHeight) {
		uiCamera.SetScreenResolution(currentWidth, currentHeight);
	}

	auto inpMan = DX9GF::InputManager::GetInstance();
	inpMan->ReadMouse(deltaTime);
	inpMan->ReadKeyboard(deltaTime);

	static float escCooldown = 0.0f;
	if (escCooldown > 0) escCooldown -= deltaTime;

	if (inpMan->KeyPress(DIK_ESCAPE) && escCooldown <= 0) {
		if (inventoryMenu) inventoryMenu->Toggle();
		escCooldown = 300.0f;
	}

	bool isGamePaused = this->isGamePaused;

	if (!isGamePaused) map->UpdateAreas(player->GetWorldX(), player->GetWorldY());
	if (npc1) {
		npc1->Update(deltaTime);
		if (!currentConversation && npc1->CanInteract() && inpMan->KeyPress(DIK_E)) {
			auto [sw, sh] = camera.GetScreenResolution();
			currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
			for (auto& line : npc1->GetDialogueLines()) {
				currentConversation->AddLine(line);
			}
		}
	}

	if (currentConversation) {
		isGamePaused = true;
		currentConversation->Execute(deltaTime);
		if (currentConversation->IsFinished()) currentConversation = nullptr;
	}

	if (savePoint) {
		savePoint->Update(deltaTime);
		if (savePoint->IsMenuOpen()) isGamePaused = true;
	}

	if (shopPoint_Card) shopPoint_Card->Update(deltaTime);
	if (shopPoint_BSItem) shopPoint_BSItem->Update(deltaTime);
	if (shopPoint_HBItem) shopPoint_HBItem->Update(deltaTime);
	if (shopPoint_PMItem) shopPoint_PMItem->Update(deltaTime);

	if (healingPoint) healingPoint->Update(deltaTime);

	if (inventoryMenu && inventoryMenu->IsOpen()) {
		isGamePaused = true;
		inventoryMenu->Update(deltaTime);
	}

	if (!isGamePaused) {
		player->Update(deltaTime);
		camera.Update();
	}

	transformManager->UpdateAll();


	if (draggableManager && inventoryMenu && inventoryMenu->IsOpen() && inventoryMenu->GetCurrentTab() == Demo::InventoryMenu::Tab::DECK) {
		draggableManager->Update(deltaTime);
	}

	if (inventoryMenu && inventoryMenu->IsPendingLeave()) {
		auto sceMan = game->GetSceneManager();
		sceMan->PopScene();
		sceMan->GoToPrevious();
		return;
	}
	commandBuffer->Update(deltaTime);
}

void Demo::TutorialWorldScene::Draw(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	gd->Clear(0xFFFFFFFF);
	if (SUCCEEDED(gd->BeginDraw())) {
		map->Draw(camera);
		if (npc1) npc1->Draw(camera, deltaTime);
		if (savePoint) {
			savePoint->Draw(camera, deltaTime);
		}
		if (shopPoint_Card) shopPoint_Card->Draw(camera, deltaTime);
		if (shopPoint_BSItem) shopPoint_BSItem->Draw(camera, deltaTime);
		if (shopPoint_HBItem) shopPoint_HBItem->Draw(camera, deltaTime);
		if (shopPoint_PMItem) shopPoint_PMItem->Draw(camera, deltaTime);
		if (healingPoint) healingPoint->Draw(camera, deltaTime);
		player->Draw(deltaTime);
		if (drawBuffer) {
			drawBuffer->Update(deltaTime);
		}
		if (inventoryMenu) inventoryMenu->Draw(gd, deltaTime);
		if (draggableManager && inventoryMenu && inventoryMenu->IsOpen() && inventoryMenu->GetCurrentTab() == Demo::InventoryMenu::Tab::DECK) {
			draggableManager->Draw(deltaTime);
		}
		if (currentConversation) currentConversation->Draw(gd, deltaTime);
		DX9GF::InputManager::GetInstance()->DrawCursor(&this->uiCamera, deltaTime);
		gd->EndDraw();
	}
	gd->Present();
}

std::string Demo::TutorialWorldScene::GetSaveID() const
{
	return "TutorialWorldScene";
}

void Demo::TutorialWorldScene::GenerateSaveData(nlohmann::json& outData)
{
	player->GenerateSaveData(outData["player"]);
	auto pos = camera.GetPosition();
	outData["camera"] = {
		{"x", pos.x},
		{"y", pos.y},
		{"zoom", camera.GetZoom()}
	};
}

void Demo::TutorialWorldScene::RestoreSaveData(const nlohmann::json& inData)
{
	player->RestoreSaveData(inData["player"]);
	camera.SetPosition(inData["camera"]["x"], inData["camera"]["y"]);
	camera.SetZoom(inData["camera"]["zoom"]);
}

void Demo::TutorialWorldScene::GiveTestItems()
{
	ItemInventory& testItems = this->player->GetInventoryItems();
	testItems.InitFixedInventory(10);

	testItems.AddItem(0, 5);
	testItems.AddItem(1, 3);
	testItems.AddItem(2, 2);
	testItems.AddItem(3, 1);
	testItems.AddItem(4, 1);
	testItems.AddItem(5, 1);
	testItems.AddItem(6, 1);
	testItems.AddItem(7, 1);
	testItems.AddItem(8, 1);
	testItems.AddItem(9, 1);
}