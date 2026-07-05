#include "pch.h"
#include "ThreadAlleyScene.h"
#include "RandomEncounter.h"
#include "CardShop.h"
#include "ItemShop.h"
#include <cmath>
#include "MainMenu.h"
#include "SaveGameState.h"
#include "TransitionCommand.h"
#include "resource.h"
#include "PopupManager.h"

void Demo::ThreadAlleyScene::Init()
{
	camera.SetZoom(2.0f);
	transformManager = std::make_shared<DX9GF::TransformManager>();
	colliderManager = std::make_shared<DX9GF::ColliderManager>();

	player = std::make_shared<Player>(transformManager, -544.5f, 128.5f);
	camera.SetPosition(-544.5f, 128.5f);

	player->Init(game->GetGraphicsDevice(), colliderManager.get(), &camera);

	drawBuffer = std::make_shared<DX9GF::CommandBuffer>();
	commandBuffer = std::make_shared<DX9GF::CommandBuffer>();

	map = std::make_shared<DX9GF::Map>(game->GetGraphicsDevice());
	map->Create(transformManager, colliderManager, "./assets/ThreadAlley.tmx");

	map->SetAreaUpdateHandler("triggers", GetRandomEncounterFunc(game, player, {
		{"VampireBatEnemy", 40},
		{"MimicEnemy", 25},
		}, drawBuffer, commandBuffer, &isGamePaused, & this->uiCamera, [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) { DrawCheckerBackground(gd, deltaTime); }));

	map->SetAreaUpdateHandler("trigger_p", [this](const DX9GF::Map::ObjectArea& area) {
		isTransitioning = true;
		auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, true);
		drawBuffer->PushCommand(transitionInCommand);
		commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this, transitionInCommand](std::function<void(void)> markFinished) {
			if (!transitionInCommand->IsFinished()) {
				return;
			}
			nlohmann::json saveData;
			player->GenerateSaveGlobalData(saveData["player"]);
			auto sceMan = game->GetSceneManager();
			MainMenu::gameSaveState->GetPlayerFromScene(sceMan->GetScene(static_cast<size_t>(sceMan->GetIndex()) + 1))->RestoreSaveGlobalData(saveData["player"]);
			DX9GF::AudioManager::GetInstance()->PlayBGM_Fade("bgm_boss", 0.3f, 1.5f);
			sceMan->GoToNext();
			isTransitioning = false;
			markFinished();
			}));
		drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
		});

	font = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 16);

	shopPoints.push_back(std::make_shared<ShopPoint>(transformManager, -710, -450));
	shopPoints.back()->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
       [](Game* g, Player* p, int w, int h) { return new CardShop(g, p, w, h, ShopTier::HYBRID); }
	);
	shopPoints.back()->SetVisible(true);

	shopPoints.push_back(std::make_shared<ShopPoint>(transformManager, -697, -1148));
	shopPoints.back()->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
       [](Game* g, Player* p, int w, int h) { return new ItemShop(g, p, w, h, ShopTier::HYBRID); }
	);
	shopPoints.back()->SetVisible(true);

	shopPoints.push_back(std::make_shared<ShopPoint>(transformManager, 35, -1413));
	shopPoints.back()->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
       [](Game* g, Player* p, int w, int h) { return new CardShop(g, p, w, h, ShopTier::BASIC); }
	);
	shopPoints.back()->SetVisible(true);

	shopPoints.push_back(std::make_shared<ShopPoint>(transformManager, 230, -456));
	shopPoints.back()->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
       [](Game* g, Player* p, int w, int h) { return new ItemShop(g, p, w, h, ShopTier::BASIC); }
	);
	shopPoints.back()->SetVisible(true);
	  
	auto borderTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	borderTex->LoadTexture(L"assets/popup-borders.png");

	auto uiTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	uiTex->LoadTexture(L"assets/ui.png");

	PopupManager::GetInstance()->Init(game->GetGraphicsDevice(), borderTex, uiTex, font);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, -710, -554));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, -474, -137));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, -727, -1201));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, -235, -1302));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 134, -1129));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 824, -969));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 582, -1036));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 230, -392));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 968, -216));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 1210, -91));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);


	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, -328, -537));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, -538, -137));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	/*healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, -456, -1016));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);*/

	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, -634, -1273));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, -59, -1356));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 407, -1337));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 582, -841));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	/*healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 984, -839));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);*/

	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 767, -1041));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	/*healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 920, -1351));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);*/

	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 1257, -1351));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	//healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 233, -297));
	//healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	//healingPoints.back()->SetVisible(true);

	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 585, -539));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	/*healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 584, -123));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);*/

	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 872, -262));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	/*healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 935, -616));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);*/

	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 1174, -1143));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 1174, -441));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	//TreasureChest
	struct ChestDef {
		std::pair<float, float> pos;
		std::vector<ChestReward> rewards;
		bool randomPick = false;
	};

	const std::vector<ChestDef> chestDefs = {
		{{-411.f, -249.f}, { ChestReward::Item(0,1), ChestReward::Card("TwinStrikeCard") },              true},
		{{-603.f, -626.f}, { ChestReward::Item(1,1), ChestReward::Card("PoisonCard") },                  true},
		{{-474.f, -921.f}, { ChestReward::Item(2,1), ChestReward::Item(3,1) },                           true},
		{{1016.f, -246.f}, { ChestReward::Item(3,1), ChestReward::Card("CleaveCard") },                  true},
		{{ 821.f, -506.f}, { ChestReward::Item(4,1), ChestReward::Card("TwinStrikeCard") },              true},
		{{1096.f,  -76.f}, { ChestReward::Item(4,1), ChestReward::Item(5,1) },                           true},
		{{1127.f, -892.f}, { ChestReward::Item(3,1), ChestReward::Card("PoisonCard") },                  true},
		{{ 983.f, -841.f}, { ChestReward::Item(2,1), ChestReward::Item(3,1), ChestReward::Card("CleaveCard") }, true},
		{{ 982.f,-1035.f}, { ChestReward::Item(4,1), ChestReward::Card("VulnerableCard") },              true},
		{{-102.f, -951.f}, { ChestReward::Item(5,1), ChestReward::Card("PoisonCard") },                  true},
		{{ 408.f,-1178.f}, { ChestReward::Item(3,1), ChestReward::Card("CleaveCard") },                  true},
	};

	for (auto& def : chestDefs) {
		treasureChests.push_back(std::make_shared<TreasureChestNPC>(
			transformManager, def.pos.first, def.pos.second,
			def.rewards, def.randomPick));
		treasureChests.back()->Init(game->GetGraphicsDevice(),&camera, player, colliderManager, font, drawBuffer);
	}

	draggableManager = std::make_shared<Demo::DraggableManager>();
	inventoryMenu = std::make_shared<InventoryMenu>(game, player, transformManager, draggableManager, &uiCamera, font.get());
	inventoryMenu->Init();

	playerHUD = std::make_shared<PlayerHUD>(game, player, transformManager, &this->uiCamera, font.get());
	playerHUD->SetOnInventoryOpen([this]() {
		if (inventoryMenu && !inventoryMenu->IsOpen()) inventoryMenu->Toggle();
		});
	playerHUD->Init();

	auto audio = DX9GF::AudioManager::GetInstance();

	audio->Load("step_c1", IDR_STEP_C1);
	audio->Load("step_c2", IDR_STEP_C2);
	audio->Load("step_c3", IDR_STEP_C3);
	audio->Load("step_c4", IDR_STEP_C4);

	audio->RegisterBank("step_concrete", { "step_c1", "step_c2", "step_c3", "step_c4"});
	player->SetBaseSurface("concrete");

	map->SetAreaUpdateHandler("audio_zone_leaves", [this](const DX9GF::Map::ObjectArea&) {
		GetPlayer()->SetSurface("leaves");
		});

	map->SetAreaUpdateHandler("audio_zone_metal", [this](const DX9GF::Map::ObjectArea&) {
		GetPlayer()->SetSurface("metal");
		});

	ItemData::GetInstance()->LoadData();
	this->GiveTestItems();

	transformManager->RebuildHierarchy();
	drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false)); 
}

void Demo::ThreadAlleyScene::Update(unsigned long long deltaTime)
{
	PopupManager::GetInstance()->SetUICamera(&this->uiCamera);

	auto OpenChestWithDialog = [&](std::shared_ptr<TreasureChestNPC>& chest) {
		auto given = chest->Open(player.get());
		if (given.empty()) return;

		std::wstring msg = L"You found: ";
		for (auto& r : given) {
			if (r.type == ChestRewardType::ITEM) {
				auto* bp = ItemData::GetInstance()->GetItemBlueprint(r.itemID);
				if (bp) {
					msg += bp->GetName();
					if (r.quantity > 1) msg += L" x" + std::to_wstring(r.quantity);
					msg += L"  ";
				}
			}
			else if (r.type == ChestRewardType::CARD) {
				std::wstring wid(r.cardSaveID.begin(), r.cardSaveID.end());
				msg += wid + L"  ";
			}
		}
		auto [sw, sh] = camera.GetScreenResolution();
		currentConversation = std::make_shared<IConversation>(
			std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
		currentConversation->AddLine({ .name = L"Treasure Chest", .content = msg });
		};

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

	if (PopupManager::GetInstance()->IsActive()) {
		PopupManager::GetInstance()->Update(deltaTime, &this->uiCamera);
		isGamePaused = true;
	}

	if (currentConversation) {
		isGamePaused = true;
		currentConversation->Execute(deltaTime);
		if (currentConversation->IsFinished()) currentConversation = nullptr;
	}

	for (auto& savePoint : savePoints) {
		savePoint->Update(deltaTime);
	}

	for (auto& shopPoint : shopPoints) {
		shopPoint->Update(deltaTime);
	}

	for (auto& healPoint : healingPoints) {
		healPoint->Update(deltaTime);
	}

	for (auto& chest : treasureChests) {
		chest->Update(deltaTime);
		if (!currentConversation && chest->CanInteract() && inpMan->KeyPress(DIK_E)) {
			auto given = chest->Open(player.get());
			if (!given.empty()) {
				std::wstring msg = L"You found: ";
				for (auto& r : given) {
					if (r.type == ChestRewardType::ITEM) {
						auto* bp = ItemData::GetInstance()->GetItemBlueprint(r.itemID);
						if (bp) {
							msg += bp->GetName();
							if (r.quantity > 1) msg += L" x" + std::to_wstring(r.quantity);
							msg += L"  ";
						}
					}
					else if (r.type == ChestRewardType::CARD) {
						std::wstring wid(r.cardSaveID.begin(), r.cardSaveID.end());
						msg += wid + L"  ";
					}
				}
				auto [sw, sh] = camera.GetScreenResolution();
				currentConversation = std::make_shared<IConversation>(
					std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
				currentConversation->AddLine({ .name = L"Treasure Chest", .content = msg });
			}
		}
	}

	if (inventoryMenu && inventoryMenu->IsOpen()) {
		isGamePaused = true;
		inventoryMenu->Update(deltaTime);
	}

	if (playerHUD && !isGamePaused) playerHUD->Update(deltaTime);

	if (!isGamePaused) {
		player->Update(deltaTime);
		camera.Update();
	}

	this->uiCamera.Update();

	transformManager->UpdateAll();
	if (!isGamePaused) map->UpdateAreas(player->GetWorldX(), player->GetWorldY());

	if (draggableManager && inventoryMenu && inventoryMenu->IsOpen() && inventoryMenu->GetCurrentTab() == Demo::InventoryMenu::Tab::DECK) {
		draggableManager->Update(deltaTime);
	}

	if (inventoryMenu && inventoryMenu->IsPendingLeave()) {
		auto sceMan = game->GetSceneManager();
		sceMan->GoToScene(0);
		auto audio = DX9GF::AudioManager::GetInstance();
		audio->PlayBGM_Fade("bgm_sky", 0.9f, 1.5f);
		return;
	}
	commandBuffer->Update(deltaTime);
}

void Demo::ThreadAlleyScene::DrawWorld(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	if (SUCCEEDED(gd->BeginDraw())) {
		DrawCheckerBackground(gd, deltaTime);

		map->Draw(camera);

		for (auto& savePoint : savePoints) savePoint->Draw(camera, deltaTime);
		for (auto& shopPoint : shopPoints) shopPoint->Draw(camera, deltaTime);
		for (auto& healPoint : healingPoints) healPoint->Draw(camera, deltaTime);
		for (auto& chest : treasureChests) chest->Draw(camera, deltaTime);

		player->Draw(deltaTime);


		gd->EndDraw();
	}
}

void Demo::ThreadAlleyScene::DrawUI(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	if (SUCCEEDED(gd->BeginDraw())) {

		for (auto& savePoint : savePoints) savePoint->DrawUI(&this->uiCamera, deltaTime);
		for (auto& chest : treasureChests) chest->DrawUI(&this->uiCamera, deltaTime);
		for (auto& healPoint : healingPoints) healPoint->DrawUI(&this->uiCamera, deltaTime);
		for (auto& shopPoint : shopPoints) shopPoint->DrawUI(&this->uiCamera, deltaTime);

		if (playerHUD) playerHUD->Draw(gd, deltaTime);
		if (inventoryMenu) inventoryMenu->Draw(gd, deltaTime);
		if (draggableManager && inventoryMenu && inventoryMenu->IsOpen() && inventoryMenu->GetCurrentTab() == Demo::InventoryMenu::Tab::DECK) {
			draggableManager->Draw(deltaTime);
		}

		if (currentConversation) {
			currentConversation->Draw(gd, &this->uiCamera, deltaTime);
		}

		if (drawBuffer) {
			drawBuffer->Update(deltaTime);
		}

		PopupManager::GetInstance()->DrawUI(deltaTime, &this->uiCamera);

		DX9GF::InputManager::GetInstance()->DrawCursor(&this->uiCamera, deltaTime);

		gd->EndDraw();
	}
}

std::string Demo::ThreadAlleyScene::GetSaveID() const
{
	return "ThreadAlleyScene";
}

void Demo::ThreadAlleyScene::GenerateSaveData(nlohmann::json& outData)
{
	player->GenerateSaveData(outData["player"]);
	auto pos = camera.GetPosition();
	outData["camera"] = {
		{"x", pos.x},
		{"y", pos.y},
		{"zoom", camera.GetZoom()}
	};

	nlohmann::json chestStates = nlohmann::json::array();
	for (auto& c : treasureChests) chestStates.push_back(c->GetIsOpened());
	outData["treasureChests"] = chestStates;
}

void Demo::ThreadAlleyScene::RestoreSaveData(const nlohmann::json& inData)
{
	player->RestoreSaveData(inData["player"]);
	camera.SetPosition(inData["camera"]["x"], inData["camera"]["y"]);
	camera.SetZoom(inData["camera"]["zoom"]);

	if (inData.contains("treasureChests")) {
		auto& arr = inData["treasureChests"];
		for (size_t i = 0; i < treasureChests.size() && i < arr.size(); ++i)
			treasureChests[i]->SetOpened(arr[i].get<bool>());
	}
}

void Demo::ThreadAlleyScene::GiveTestItems()
{

}

void Demo::ThreadAlleyScene::DrawCheckerBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
{
	auto [screenWidth, screenHeight] = uiCamera.GetScreenResolution();
	gd->DrawRectangle(0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0xFF403353, true);
	
	const float SQUARE_SIZE = 128.0f;
	const float BASE_SCROLL_SPEED = 30.0f; 
	const float BLINK_PERIOD = 2000.0f; 
	const float ANIMATION_DURATION = 800.0f; 
	const int PADDING = 6; // Adjust this value to increase or decrease the extra squares generated off-screen

	// Update base scroll
	bgBaseScrollX += (BASE_SCROLL_SPEED * deltaTime) / 1000.0f;
	bgBaseScrollY += (BASE_SCROLL_SPEED * deltaTime) / 1000.0f;

	if (bgBaseScrollX >= 2.0f * SQUARE_SIZE) bgBaseScrollX -= 2.0f * SQUARE_SIZE;
	if (bgBaseScrollY >= 2.0f * SQUARE_SIZE) bgBaseScrollY -= 2.0f * SQUARE_SIZE;

	// Update periodic blink and movement
	bgPeriodTimer += deltaTime;
	if (bgPeriodTimer >= BLINK_PERIOD) {
		bgPeriodTimer = std::fmod(bgPeriodTimer, BLINK_PERIOD);
		bgAnimPhase = (bgAnimPhase + 1) % 2; 
		bgEaseProgress = bgPeriodTimer / ANIMATION_DURATION;
	}

	float blinkFactor = 0.0f;
	if (bgPeriodTimer < 200.0f) {
		blinkFactor = 1.0f - (bgPeriodTimer / 200.0f);
	}

	// Update easing for row shifting
	if (bgEaseProgress < 1.0f) {
		bgEaseProgress += deltaTime / ANIMATION_DURATION;
		if (bgEaseProgress > 1.0f) bgEaseProgress = 1.0f;
	}

	auto easeInOut = [](float t) {
		return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
	};

	float easedValue = easeInOut(bgEaseProgress) * SQUARE_SIZE;
	
	if (bgAnimPhase == 1) {
		bgOddRowShift = -easedValue; 
		bgEvenRowShift = -easedValue; 
	} else {
		bgOddRowShift = -SQUARE_SIZE - easedValue; 
		bgEvenRowShift = -SQUARE_SIZE - easedValue; 
	}

	int cols = (int)std::ceil(screenWidth / SQUARE_SIZE) + PADDING * 2;
	int rows = (int)std::ceil(screenHeight / SQUARE_SIZE) + PADDING * 2;

	gd->SetAlphaBlending(true);
	for (int row = -PADDING; row < rows; ++row) {
		for (int col = -PADDING; col < cols; ++col) {
			float x = col * SQUARE_SIZE + bgBaseScrollX;
			float y = row * SQUARE_SIZE + bgBaseScrollY;

			if (row % 2 != 0) {
				y += bgOddRowShift;
			} else {
				x += bgEvenRowShift;
			}

			bool isColor1 = (col + row) % 2 == 0;
			D3DCOLOR baseColor = isColor1 ? bgBaseColor1 : bgBaseColor2;
			
			if (blinkFactor > 0.0f) {
				int a = (baseColor >> 24) & 0xFF;
				int r = ((baseColor >> 16) & 0xFF) + (int)((((bgBlinkColor >> 16) & 0xFF) - ((baseColor >> 16) & 0xFF)) * blinkFactor);
				int g = ((baseColor >> 8) & 0xFF) + (int)((((bgBlinkColor >> 8) & 0xFF) - ((baseColor >> 8) & 0xFF)) * blinkFactor);
				int b = (baseColor & 0xFF) + (int)((((bgBlinkColor & 0xFF) - (baseColor & 0xFF)) * blinkFactor));

				baseColor = D3DCOLOR_ARGB(a, r, g, b);
			}

			gd->DrawRectangle(uiCamera, x, y, SQUARE_SIZE, SQUARE_SIZE, baseColor, true);
		}
	}
	gd->SetAlphaBlending(false);
}