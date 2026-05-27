#include "pch.h"
#include "SecretPuzzleScene.h"
#include "CustomBattleScene.h"
#include "RandomEncounter.h"
#include "MainMenu.h"
#include "SaveGameState.h"
#include "TransitionCommand.h"
#include "resource.h"

void Demo::SecretPuzzleScene::Init()
{
	camera.SetZoom(2.0f);
	transformManager = std::make_shared<DX9GF::TransformManager>();
	colliderManager = std::make_shared<DX9GF::ColliderManager>();
	player = std::make_shared<Player>(transformManager, -84 * 16, -39 * 16);
	camera.SetPosition(248, 184);
	player->Init(game->GetGraphicsDevice(), colliderManager.get(), &camera);
	drawBuffer = std::make_shared<DX9GF::CommandBuffer>();
	commandBuffer = std::make_shared<DX9GF::CommandBuffer>();
	map = std::make_shared<DX9GF::Map>(game->GetGraphicsDevice());
	map->Create(transformManager, colliderManager, "./assets/SecretPuzzle.tmx");
	map->SetAreaUpdateHandler("trigger_encounter", GetRandomEncounterFunc(game, player, {
		{"VampireBatEnemy", 40},
		{"MimicEnemy", 35},
		}, drawBuffer, commandBuffer, &isGamePaused, [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) { DrawBackground(gd, deltaTime); }));
	map->SetAreaUpdateHandler("trigger_p_back", [this](const DX9GF::Map::ObjectArea& area) {
		if (isTransitioning) return;
		isTransitioning = true;
		auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), 1.f, true);
		drawBuffer->PushCommand(transitionInCommand);
		commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this, transitionInCommand](std::function<void(void)> markFinished) {
		if (!transitionInCommand->IsFinished()) {
			return;
		}
		nlohmann::json saveData;
		player->GenerateSaveGlobalData(saveData["player"]);
		auto sceMan = game->GetSceneManager();
		auto targetScene = sceMan->GetScene(static_cast<size_t>(sceMan->GetIndex()) - 1);
		auto targetPlayer = MainMenu::gameSaveState->GetPlayerFromScene(targetScene);
		targetPlayer->RestoreSaveGlobalData(saveData["player"]);
		targetPlayer->SetLocalPosition(-263.f, -295.f);
		DX9GF::AudioManager::GetInstance()->PlayBGM_Fade("bgm_tutorial", 0.5f, 1.5f);
		sceMan->GoToPrevious();
		isTransitioning = false;
		markFinished();
		}));
		drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), 1.f, false));
		});
	map->SetAreaUpdateHandler("trigger_p_next_world", [this](const DX9GF::Map::ObjectArea& area) {
		if (isTransitioning) return;
		isTransitioning = true;
		auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), 1.f, true);
		drawBuffer->PushCommand(transitionInCommand);
		commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this, transitionInCommand](std::function<void(void)> markFinished) {
			if (!transitionInCommand->IsFinished()) {
				return;
			}
			nlohmann::json saveData;
			player->GenerateSaveGlobalData(saveData["player"]);
			auto sceMan = game->GetSceneManager();
			auto targetScene = sceMan->GetScene(static_cast<size_t>(sceMan->GetIndex()) + 1);
			auto targetPlayer = MainMenu::gameSaveState->GetPlayerFromScene(targetScene);
			targetPlayer->RestoreSaveGlobalData(saveData["player"]);
			DX9GF::AudioManager::GetInstance()->PlayBGM_Fade("bgm_arcade", 0.2f, 1.5f);
			sceMan->GoToNext();
			isTransitioning = false;
			markFinished();
			}));
		drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), 1.f, false));
	});
	map->SetAreaUpdateHandler("trigger_p_next", [this](const DX9GF::Map::ObjectArea& area) {
		player->SetLocalPosition(31 * 16, 36 * 16);
	});
	map->SetAreaUpdateHandler("trigger_secretboss_encounter", [this](const DX9GF::Map::ObjectArea& area) {
		if (!player->IsWalking()) return;

		bool hasRustyKey = player->GetInventoryItems().HasItem(10);
		if (!hasRustyKey && !this->isBossDead) {

			std::map<std::string, int> forcedEnemyMap = { {"CupidEnemy", 100} };

			auto demoGame = dynamic_cast<Demo::Game*>(this->game);
			auto app = DX9GF::Application::GetInstance();

			auto battleScene = new CustomBattleScene(demoGame, player, app->GetScreenWidth(), app->GetScreenHeight(), forcedEnemyMap);

			battleScene->SetOnVictoryCallback([this]() {
				this->isBossDead = true;
				});

			battleScene->SetCustomBackgroundDraw([this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) { DrawBackground(gd, deltaTime); });

			auto sceMan = this->game->GetSceneManager();
			sceMan->InsertScene(sceMan->GetIndex() + 1, battleScene);

			commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void()> markFinished) {
				this->isGamePaused = true;
				markFinished();
				}));

			auto transitionInCommand = std::make_shared<TransitionCommand>(this->game->GetGraphicsDevice(), 1.f, true);
			drawBuffer->StackCommand(transitionInCommand);

			commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([sceMan, transitionInCommand, this](std::function<void()> markFinished) {
				if (!transitionInCommand->IsFinished()) {
					return;
				}
				sceMan->GoToNext();
				markFinished();
				}));

			drawBuffer->PushCommand(std::make_shared<TransitionCommand>(this->game->GetGraphicsDevice(), 1.f, false));

			//check battle result and give key
			drawBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void()> markFinished) {
				this->isGamePaused = false;

				if (this->isBossDead) {
					this->player->GetInventoryItems().AddItem(10, 1);
				}

				markFinished();
				}));
		}
	});
	font = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 16);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, -47.0f * 16, -43.0f * 16));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);
	savePoints.push_back(std::make_shared<SavePoint>(transformManager, -42.0f * 16, 23.0f * 16));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);
	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 31.0f * 16, 47.0f * 16));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);
	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 64.0f * 16, 37.0f * 16));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);
	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 86.0f * 16, 58.0f * 16));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	shopPoints.push_back(std::make_shared<ShopPoint>(transformManager, -58.0f * 16, -26.0f * 16));
	shopPoints.back()->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) {
			return new CardShop(g, p, w, h, ShopTier::HYBRID);
		}
	);
	shopPoints.back()->SetVisible(true);
	shopPoints.push_back(std::make_shared<ShopPoint>(transformManager, -40.0f * 16, -13.0f * 16));
	shopPoints.back()->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) {
			return new ItemShop(g, p, w, h, ShopTier::HYBRID);
		}
	);
	shopPoints.back()->SetVisible(true);
	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, -32.0f * 16, -38.0f * 16));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);
	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, -71.0f * 16, -28.0f * 16));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);
	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, -42.0f * 16, 28.0f * 16));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);
	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, -20.0f * 16, -16.0f * 16));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);
	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 31.0f * 16, 53.0f * 16));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);
	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 68.0f * 16, 37.0f * 16));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);
	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 88.0f * 16, 58.0f * 16));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	//TreasureChest
	auto addChest = [&](float tx, float ty, std::vector<ChestReward> rewards, bool randomPick = false) {
		auto c = std::make_shared<TreasureChestNPC>(transformManager, tx * 16, ty * 16, rewards, randomPick);
		c->Init(game->GetGraphicsDevice(), player, colliderManager, font, drawBuffer);
		treasureChests.push_back(c);
		};
	addChest(-32, 26, { ChestReward::Item(0,1), ChestReward::Item(1,1), ChestReward::Card("PoisonCard") }, true);
	addChest(-11, -31, { ChestReward::Item(2,1), ChestReward::Item(3,1), ChestReward::Card("CleaveCard") }, true);
	addChest(80, 48, { ChestReward::Item(4,1), ChestReward::Item(5,1), ChestReward::Card("TwinStrikeCard") }, true);

	draggableManager = std::make_shared<Demo::DraggableManager>();
	inventoryMenu = std::make_shared<InventoryMenu>(game, player, transformManager, draggableManager, &uiCamera, font.get());
	inventoryMenu->Init();

	auto audio = DX9GF::AudioManager::GetInstance();

	audio->Load("step_dir1", IDR_STEP_DIR1);
	audio->Load("step_dir2", IDR_STEP_DIR2);
	audio->Load("step_dir3", IDR_STEP_DIR3);
	audio->Load("step_dir4", IDR_STEP_DIR4);
	audio->Load("step_dir5", IDR_STEP_DIR5);

	audio->RegisterBank("step_dirt", { "step_dir1", "step_dir2", "step_dir3", "step_dir4" , "step_dir5" });
	player->SetBaseSurface("dirt");

	map->SetAreaUpdateHandler("audio_zone_default", [this](const DX9GF::Map::ObjectArea&) {
		GetPlayer()->SetSurface("default");
		});

	map->SetAreaUpdateHandler("audio_zone_leaves", [this](const DX9GF::Map::ObjectArea&) {
		GetPlayer()->SetSurface("leaves");
		});

	map->SetAreaUpdateHandler("audio_zone_metal", [this](const DX9GF::Map::ObjectArea&) {
		GetPlayer()->SetSurface("metal");
		});

	ItemData::GetInstance()->LoadData();
	this->GiveTestItems();

	transformManager->RebuildHierarchy();
	drawBuffer->PushCommand(std::make_shared<Demo::TransitionCommand>(game->GetGraphicsDevice(), 1.f, false));

	dauDau = std::make_shared<DauDauNPC>(transformManager, 1 * 16, -31.0f * 16);
	dauDau->Init(game->GetGraphicsDevice(), player, colliderManager, font, drawBuffer);
	dauDau->AddLine(L"Dau Dau", L"Watch out! This portal is a one-way trip to the invisible maze! Enter if you dare!"); 
}

void Demo::SecretPuzzleScene::Update(unsigned long long deltaTime)
{
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

	if (currentConversation) {
		isGamePaused = true;
		currentConversation->Execute(deltaTime);
		if (currentConversation->IsFinished()) currentConversation = nullptr;
	}

	for (auto& savePoint : savePoints) {
		savePoint->Update(deltaTime);
		if (savePoint->IsMenuOpen()) isGamePaused = true;
	}

	for (auto& shopPoint : shopPoints) {
		shopPoint->Update(deltaTime);
	}
	for (auto& healingPoint : healingPoints) {
		healingPoint->Update(deltaTime);
	}

	dauDau->Update(deltaTime);
	if (!currentConversation && dauDau->CanInteract() && inpMan->KeyPress(DIK_E)) {
		auto [sw, sh] = camera.GetScreenResolution();
		currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
		for (auto& line : dauDau->GetDialogueLines()) {
			currentConversation->AddLine(line);
		}
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

	if (!isGamePaused) {
		player->Update(deltaTime);
		camera.Update();
	}

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

void Demo::SecretPuzzleScene::Draw(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	gd->Clear(0xFF403353);
	if (SUCCEEDED(gd->BeginDraw())) {
		/* Cool wave grid effect */
		DrawBackground(gd, deltaTime);
		/* End of cool wave grid effect */

		map->Draw(camera);
		for (auto& savePoint : savePoints) {
			savePoint->Draw(camera, deltaTime);
		}
		for (auto& shopPoint : shopPoints) {
			shopPoint->Draw(camera, deltaTime);
		}
		for (auto& healingPoint : healingPoints) {
			healingPoint->Draw(camera, deltaTime);
		}

		for (auto& chest : treasureChests)
			chest->Draw(camera, deltaTime);

		dauDau->Draw(camera, deltaTime);

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

void Demo::SecretPuzzleScene::DrawBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
{

	auto [screenWidth, screenHeight] = camera.GetScreenResolution();
	const int spacingX = 32;
	const int spacingY = 32;
	const float segmentLength = 16.0f;
	const float amplitude = 12.0f;
	const float frequency = 0.05f;
	const D3DCOLOR gridColor = 0xFF9cdb43;
	static float waveTime = 0.0f;
	waveTime += static_cast<float>(deltaTime) * 0.002f;
	while (waveTime > 6.2831853f) {
		waveTime -= 6.2831853f;
	}

	for (int y = 0; y <= screenHeight; y += spacingY) {
		gd->DrawLine(0.0f, static_cast<float>(y), static_cast<float>(screenWidth), static_cast<float>(y), gridColor);
	}

	for (int x = 0; x <= screenWidth; x += spacingX) {
		float prevY = 0.0f;
		float prevX = static_cast<float>(x) + std::sinf(waveTime) * amplitude;
		for (float y = segmentLength; y <= static_cast<float>(screenHeight); y += segmentLength) {
			float offsetX = static_cast<float>(x) + std::sinf((y * frequency) + waveTime) * amplitude;
			gd->DrawLine(prevX, prevY, offsetX, y, gridColor);
			prevX = offsetX;
			prevY = y;
		}
	}
}

std::string Demo::SecretPuzzleScene::GetSaveID() const
{
	return "SecretPuzzleScene";
}

void Demo::SecretPuzzleScene::GenerateSaveData(nlohmann::json& outData)
{
	player->GenerateSaveData(outData["player"]);
	auto pos = camera.GetPosition();
	outData["camera"] = {
		{"x", pos.x},
		{"y", pos.y},
		{"zoom", camera.GetZoom()}
	};
	outData["puzzle"] = {
		{"isBossDead", isBossDead}
	};

	nlohmann::json chestStates = nlohmann::json::array();
	for (auto& c : treasureChests) chestStates.push_back(c->GetIsOpened());
	outData["treasureChests"] = chestStates;
}

void Demo::SecretPuzzleScene::RestoreSaveData(const nlohmann::json& inData)
{
	player->RestoreSaveData(inData["player"]);
	camera.SetPosition(inData["camera"]["x"], inData["camera"]["y"]);
	camera.SetZoom(inData["camera"]["zoom"]);
	if (inData.contains("puzzle") && inData["puzzle"].contains("isBossDead")) {
		isBossDead = inData["puzzle"]["isBossDead"];
	}

	if (inData.contains("treasureChests")) {
		auto& arr = inData["treasureChests"];
		for (size_t i = 0; i < treasureChests.size() && i < arr.size(); ++i)
			treasureChests[i]->SetOpened(arr[i].get<bool>());
	}
}

void Demo::SecretPuzzleScene::GiveTestItems()
{
	//ItemInventory& testItems = this->player->GetInventoryItems();
	//testItems.InitFixedInventory(10);

	//testItems.AddItem(0, 5);
	//testItems.AddItem(1, 3);
	//testItems.AddItem(2, 2);
	//testItems.AddItem(3, 1);
	//testItems.AddItem(4, 1);
	//testItems.AddItem(5, 1);
	//testItems.AddItem(6, 1);
	//testItems.AddItem(7, 1);
	//testItems.AddItem(8, 1);
	//testItems.AddItem(9, 1);
}
