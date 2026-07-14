#include "pch.h"
#include "SettingsManager.h"
#include "SecretPuzzleScene.h"
#include "CustomBattleScene.h"
#include "RandomEncounter.h"
#include "MainMenu.h"
#include "SaveGameState.h"
#include "TransitionCommand.h"
#include "resource.h"
#include "PopupManager.h"
#include "QuestManager.h"
#include "MapEnemy.h"
#include "EnemyFactory.h"
#include "EncounterGenerator.h"
#include "MapBattleScene.h"

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

	/*map->SetAreaUpdateHandler("trigger_encounter", GetRandomEncounterFunc(game, player, {
		{"VampireBatEnemy", 40},
		{"DemonEyeEnemy", 35},
		}, drawBuffer, commandBuffer, &isGamePaused, &this->uiCamera, [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) { DrawBackground(gd, deltaTime); }));*/

	map->SetAreaUpdateHandler("trigger_p_back", [this](const DX9GF::Map::ObjectArea& area) {
		if (isTransitioning) return;
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
			auto targetScene = sceMan->GetScene(static_cast<size_t>(sceMan->GetIndex()) - 1);
			auto targetPlayer = MainMenu::gameSaveState->GetPlayerFromScene(targetScene);
			targetPlayer->RestoreSaveGlobalData(saveData["player"]);
			targetPlayer->SetLocalPosition(-263.f, -295.f);
			DX9GF::AudioManager::GetInstance()->PlayBGM_Fade("bgm_tutorial", 0.5f, 1.5f);
			sceMan->GoToPrevious();
			isTransitioning = false;
			markFinished();
			}));
		drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
		});

	map->SetAreaUpdateHandler("trigger_p_next_world", [this](const DX9GF::Map::ObjectArea& area) {
		if (isTransitioning) return;
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
			auto targetScene = sceMan->GetScene(static_cast<size_t>(sceMan->GetIndex()) + 1);
			auto targetPlayer = MainMenu::gameSaveState->GetPlayerFromScene(targetScene);
			targetPlayer->RestoreSaveGlobalData(saveData["player"]);
			DX9GF::AudioManager::GetInstance()->PlayBGM_Fade("bgm_arcade", 0.2f, 1.5f);
			sceMan->GoToNext();
			isTransitioning = false;
			markFinished();
			}));
		drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
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

			auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, true);
			drawBuffer->StackCommand(transitionInCommand);

			commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([sceMan, transitionInCommand, this](std::function<void()> markFinished) {
				if (!transitionInCommand->IsFinished()) {
					return;
				}
				sceMan->GoToNext();
				markFinished();
				}));

			drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));

			drawBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void()> markFinished) {
				this->isGamePaused = false;
				if (this->isBossDead) {
					this->player->GetInventoryItems().AddItem(10, 1);
					QuestManager::GetInstance()->SetQuest(L"Find secret boss, defeat it and get rewards: Boss defeated 1/1");

					auto* bp = ItemData::GetInstance()->GetItemBlueprint(10);
					std::wstring msg = L"You found: ";
					if (bp) msg += bp->GetName();

					auto [sw, sh] = camera.GetScreenResolution();
					currentConversation = std::make_shared<IConversation>(
						std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
					currentConversation->AddLine({ .name = L"Secret boss (Pacman)", .content = msg });
				}
				markFinished();
				}));
		}
		});

	font = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 16);

	auto borderTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	borderTex->LoadTexture(L"assets/popup-borders.png");

	auto uiTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	uiTex->LoadTexture(L"assets/ui.png");

	PopupManager::GetInstance()->Init(game->GetGraphicsDevice(), borderTex, uiTex, font);
	QuestManager::GetInstance()->SetVirtualResolution(game->GetVirtualWidth(), game->GetVirtualHeight());
	QuestManager::GetInstance()->Init(game->GetGraphicsDevice(), transformManager, &this->uiCamera, font);
	QuestManager::GetInstance()->SetQuest(L"Quest: ???");

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

	// TreasureChest
	auto addChest = [&](float tx, float ty, std::vector<ChestReward> rewards, bool randomPick = false) {
		auto c = std::make_shared<TreasureChestNPC>(transformManager, tx * 16, ty * 16, rewards, randomPick);
		c->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
		treasureChests.push_back(c);
		};
	addChest(-32, 26, { ChestReward::Item(0,1), ChestReward::Item(1,1), ChestReward::Card("PoisonCard") }, true);
	addChest(-11, -31, { ChestReward::Item(2,1), ChestReward::Item(3,1), ChestReward::Card("CleaveCard") }, true);
	addChest(80, 48, { ChestReward::Item(4,1), ChestReward::Item(5,1), ChestReward::Card("TwinStrikeCard") }, true);

	//map enemies
	auto spawn = [&](float x, float y, std::string id, std::vector<std::string> types, bool isRand, bool isGlobal) {
		auto bgDraw = [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) {
			DrawBackground(gd, deltaTime);
			};

		std::string bgm = (id == "sec_miniboss_01") ? "bgm_boss" : "battle_loop";

		auto enemy = EnemyFactory::CreateMapEnemy(
			x, y, id, types, isRand, isGlobal, bgm, bgDraw,
			transformManager, game, colliderManager.get(), player
		);

		enemy->SetOnEncounterTriggered([this](std::shared_ptr<MapEnemy> e) {
			if (this->isTransitioning) return;
			this->isTransitioning = true;

			auto transitionIn = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, true);
			this->drawBuffer->PushCommand(transitionIn);

			this->commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this, transitionIn, e](std::function<void(void)> markFinished) {
				if (!transitionIn->IsFinished()) return;

				auto app = DX9GF::Application::GetInstance();
				auto sceMan = this->game->GetSceneManager();
				auto battleScene = new MapBattleScene(this->game, this->player, app->GetScreenWidth(), app->GetScreenHeight(), e->GetEncounterData());

				battleScene->SetOnVictoryCallback([e]() {
					e->SetDefeatedState(true, 180.f);
					});

				sceMan->InsertScene(sceMan->GetIndex() + 1, battleScene);
				sceMan->GoToNext();

				this->isTransitioning = false;
				markFinished();
				}));

			this->drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
			});

		mapEnemies.push_back(enemy);
		};

	spawn(-1140.f, -400.f, "sec_bat_01", { "VampireBatEnemy" }, false, false);
	spawn(-910.f, 80.f, "sec_eye_01", { "DemonEyeEnemy" }, false, false);
	spawn(-780.f, 470.f, "sec_rand_bat_eye_01", { "VampireBatEnemy", "DemonEyeEnemy" }, true, false);
	spawn(-480.f, 450.f, "sec_rand_keye_bat_01", { "KeyeEnemy", "VampireBatEnemy" }, true, false);
	spawn(-330.f, 160.f, "sec_duo_bat_01", { "VampireBatEnemy", "VampireBatEnemy" }, false, false);
	spawn(-440.f, -80.f, "sec_mimic_trap_01", { "MimicEnemy" }, false, false);
	spawn(-300.f, -450.f, "sec_rand_3types_01", { "DemonEyeEnemy", "KeyeEnemy", "MimicEnemy" }, true, false);
	spawn(-60.f, -460.f, "sec_bat_02", { "VampireBatEnemy" }, false, false);
	spawn(90.f, -340.f, "sec_rand_eye_bat_01", { "DemonEyeEnemy", "VampireBatEnemy" }, true, false);
	spawn(765.f, 680.f, "sec_keye_01", { "KeyeEnemy" }, false, false);
	spawn(880.f, 730.f, "sec_rand_bat_mimic_01", { "VampireBatEnemy", "MimicEnemy" }, true, false);
	spawn(840.f, 900.f, "sec_duo_eye_01", { "DemonEyeEnemy", "DemonEyeEnemy" }, false, false);
	spawn(1070.f, 640.f, "sec_rand_3types_02", { "VampireBatEnemy", "DemonEyeEnemy", "KeyeEnemy" }, true, false);
	spawn(1135.f, 930.f, "sec_rand_keye_mimic_01", { "KeyeEnemy", "MimicEnemy" }, true, false);
	spawn(1195.f, 780.f, "sec_bat_03", { "VampireBatEnemy" }, false, false);
	spawn(1350.f, 785.f, "sec_rand_eye_bat_02", { "DemonEyeEnemy", "VampireBatEnemy" }, true, false);
	spawn(1700.f, 860.f, "sec_rand_all_01", {}, false, true);

	draggableManager = std::make_shared<Demo::DraggableManager>();
	inventoryMenu = std::make_shared<InventoryMenu>(game, player, transformManager, draggableManager, &uiCamera, font.get());
	inventoryMenu->Init();

	playerHUD = std::make_shared<PlayerHUD>(game, player, transformManager, &this->uiCamera, font.get());
	playerHUD->SetOnInventoryOpen([this]() {
		if (inventoryMenu && !inventoryMenu->IsOpen()) inventoryMenu->Toggle();
		});
	playerHUD->Init();

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
	drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));

	dauDau = std::make_shared<DauDauNPC>(transformManager, 1 * 16, -31.0f * 16);
	dauDau->Init(game->GetGraphicsDevice(),&camera, player, colliderManager, font, drawBuffer);
	dauDau->AddLine(L"Dau Dau", L"Watch out! This portal is a one-way trip to the invisible maze! Enter if you dare!"); 

	dauDauSpawn = std::make_shared<DauDauNPC>(transformManager, -80 * 16, -37 * 16);
	dauDauSpawn->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	dauDauSpawn->AddLine(L"Dau Dau", L"There's a hidden boss somewhere in this maze. Defeat it for a secret reward!");
}

void Demo::SecretPuzzleScene::Update(unsigned long long deltaTime)
{
	PopupManager::GetInstance()->SetUICamera(&this->uiCamera);
	QuestManager::GetInstance()->SetUICamera(&this->uiCamera);
	QuestManager::GetInstance()->SetQuest(
		!questGiven ? L"Quest: ???"
		: (isBossDead
			? L"Quest: Find secret boss, defeat it and get rewards: Boss defeated 1/1"
			: L"Quest: Find secret boss, defeat it and get rewards: Boss defeated 0/1")
	);
	QuestManager::GetInstance()->SetVirtualResolution(game->GetVirtualWidth(), game->GetVirtualHeight());
	QuestManager::GetInstance()->SetVisible(!(inventoryMenu && inventoryMenu->IsOpen()));
	QuestManager::GetInstance()->Update(deltaTime);

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

	if (inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("OPEN_INVENTORY")) && escCooldown <= 0) {
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
	for (auto& healingPoint : healingPoints) {
		healingPoint->Update(deltaTime);
	}

	dauDau->Update(deltaTime);
	if (!currentConversation && dauDau->CanInteract() && inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
		auto [sw, sh] = camera.GetScreenResolution();
		currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
		for (auto& line : dauDau->GetDialogueLines()) {
			currentConversation->AddLine(line);
		}
	}

	if (dauDauSpawn) {
		dauDauSpawn->Update(deltaTime);
		if (!currentConversation && dauDauSpawn->CanInteract() && inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
			auto [sw, sh] = camera.GetScreenResolution();
			currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
			for (auto& line : dauDauSpawn->GetDialogueLines()) {
				currentConversation->AddLine(line);
			}
			questGiven = true;
			QuestManager::GetInstance()->SetQuest(isBossDead
				? L"Quest: Find secret boss, defeat it and get rewards: Boss defeated 1/1"
				: L"Quest: Find secret boss, defeat it and get rewards: Boss defeated 0/1");
		}
	}


	for (auto& chest : treasureChests) {
		chest->Update(deltaTime);
		if (!currentConversation && chest->CanInteract() && inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
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
		for (auto& enemy : mapEnemies) {
			enemy->Update(deltaTime);
		}
		player->Update(deltaTime);
		camera.Update();
	}
	this->uiCamera.Update();
	transformManager->UpdateAll();
	if (!isGamePaused) map->UpdateAreas(player->GetCollider().lock()->GetWorldX(), player->GetCollider().lock()->GetWorldY());

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

void Demo::SecretPuzzleScene::DrawWorld(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	if (SUCCEEDED(gd->BeginDraw())) {
		DrawBackground(gd, deltaTime);
		map->Draw(camera);

		for (auto& savePoint : savePoints) savePoint->Draw(camera, deltaTime);
		for (auto& shopPoint : shopPoints) shopPoint->Draw(camera, deltaTime);
		for (auto& healingPoint : healingPoints) healingPoint->Draw(camera, deltaTime);
		for (auto& chest : treasureChests) chest->Draw(camera, deltaTime);
		
		if (dauDauSpawn) dauDauSpawn->Draw(camera, deltaTime);
		dauDau->Draw(camera, deltaTime);

		for (auto& enemy : mapEnemies) {
			enemy->Draw(&camera, deltaTime);
		}

		player->Draw(deltaTime);

		gd->EndDraw();
	}
}

void Demo::SecretPuzzleScene::DrawUI(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	if (SUCCEEDED(gd->BeginDraw())) {

		for (auto& savePoint : savePoints) savePoint->DrawUI(&this->uiCamera, deltaTime);
		for (auto& shopPoint : shopPoints) shopPoint->DrawUI(&this->uiCamera, deltaTime);
		for (auto& healingPoint : healingPoints) healingPoint->DrawUI(&this->uiCamera, deltaTime);
		for (auto& chest : treasureChests) chest->DrawUI(&this->uiCamera, deltaTime);
		dauDau->DrawUI(&this->uiCamera, deltaTime);

		if (playerHUD) playerHUD->Draw(gd, deltaTime);
		if (inventoryMenu) inventoryMenu->Draw(gd, deltaTime);
		if (draggableManager && inventoryMenu && inventoryMenu->IsOpen() && inventoryMenu->GetCurrentTab() == Demo::InventoryMenu::Tab::DECK) {
			draggableManager->Draw(deltaTime);
		}
		if (inventoryMenu) inventoryMenu->DrawKeyboardReticle(gd, deltaTime);
		if (dauDauSpawn) dauDauSpawn->DrawUI(&this->uiCamera, deltaTime);

		if (currentConversation) {
			currentConversation->Draw(gd, &this->uiCamera, deltaTime);
		}

		QuestManager::GetInstance()->Draw(gd, &this->uiCamera, deltaTime);

		if (drawBuffer) {
			drawBuffer->Update(deltaTime);
		}

		PopupManager::GetInstance()->DrawUI(deltaTime, &this->uiCamera);

		if (!(inventoryMenu && inventoryMenu->IsInKeyboardMode())) {
			DX9GF::InputManager::GetInstance()->DrawCursor(&this->uiCamera, deltaTime);
		}

		gd->EndDraw();
	}
}

void Demo::SecretPuzzleScene::DrawBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
{
	auto [screenWidth, screenHeight] = camera.GetScreenResolution();
	gd->DrawRectangle(0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0xFF403353, true);
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
		{"isBossDead", isBossDead},
		{ "questGiven", questGiven }
	};

	nlohmann::json chestStates = nlohmann::json::array();
	for (auto& c : treasureChests) chestStates.push_back(c->GetIsOpened());
	outData["treasureChests"] = chestStates;

	nlohmann::json enemiesState = nlohmann::json::object();
	for (auto& enemy : mapEnemies) {
		enemiesState[enemy->GetEnemyID()] = {
			{"isDefeated", enemy->IsDefeated()},
			{"respawnTimer", enemy->GetRespawnTimer()}
		};
	}
	outData["mapEnemies"] = enemiesState;
}

void Demo::SecretPuzzleScene::RestoreSaveData(const nlohmann::json& inData)
{
	player->RestoreSaveData(inData["player"]);
	camera.SetPosition(inData["camera"]["x"], inData["camera"]["y"]);
	camera.SetZoom(inData["camera"]["zoom"]);
	if (inData.contains("puzzle") && inData["puzzle"].contains("isBossDead")) {
		isBossDead = inData["puzzle"]["isBossDead"];
		questGiven = inData["puzzle"].value("questGiven", false);
		questRestoredFromSave = true;
	}

	if (inData.contains("treasureChests")) {
		auto& arr = inData["treasureChests"];
		for (size_t i = 0; i < treasureChests.size() && i < arr.size(); ++i)
			treasureChests[i]->SetOpened(arr[i].get<bool>());
	}

	if (inData.contains("mapEnemies")) {
		auto& enemiesState = inData["mapEnemies"];
		for (auto& enemy : mapEnemies) {
			std::string id = enemy->GetEnemyID();
			if (enemiesState.contains(id)) {
				bool def = enemiesState[id]["isDefeated"].get<bool>();
				float timer = enemiesState[id]["respawnTimer"].get<float>();
				enemy->SetDefeatedState(def, timer);
			}
		}
	}
}

void Demo::SecretPuzzleScene::GiveTestItems()
{
	//ItemInventory& testItems = this->player->GetInventoryItems();
	//testItems.InitFixedInventory(10);
}