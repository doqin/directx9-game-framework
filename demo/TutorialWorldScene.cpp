#include "pch.h"
#include "SettingsManager.h"
#include "TutorialWorldScene.h"
#include "RandomEncounter.h"
#include "MainMenu.h"
#include "SaveGameState.h"
#include "TransitionCommand.h"
#include "resource.h"
#include "PopupManager.h"
#include "EncounterGenerator.h"
#include "EnemyFactory.h"
#include "QuestManager.h"
#include "MapBattleScene.h"
#include "backends/imgui_impl_dx9.h"
#include "backends/imgui_impl_win32.h"
#include "Debug.h"
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
	map->Create(transformManager, colliderManager, "./assets/tutorial.tmx");
	/*map->SetAreaUpdateHandler("triggers", GetRandomEncounterFunc(game, player, {
		{"DemonEyeEnemy", 40},
		{"MimicEnemy", 20},
		}, drawBuffer, commandBuffer, &isGamePaused, & this->uiCamera, [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) { DrawBackground(gd, deltaTime); }));*/

	map->SetAreaUpdateHandler("trigger_p", [this](const DX9GF::Map::ObjectArea& area) {
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
			MainMenu::gameSaveState->GetPlayerFromScene(sceMan->GetScene(static_cast<size_t>(sceMan->GetIndex()) + 2))->RestoreSaveGlobalData(saveData["player"]);
			DX9GF::AudioManager::GetInstance()->PlayBGM_Fade("bgm_arcade", 0.2f, 1.5f);
			sceMan->GoToScene(sceMan->GetIndex() + 2);
			isTransitioning = false;
			markFinished();
			}));
		drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
		});

	map->SetAreaUpdateHandler("trigger_secret", [this](const DX9GF::Map::ObjectArea& area) {if (isTransitioning) return;
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
		targetPlayer->SetLocalPosition(-84 * 16, -39 * 16);
		DX9GF::AudioManager::GetInstance()->PlayBGM_Fade("bgm_secret", 0.3f, 1.5f);
		sceMan->GoToNext();
		isTransitioning = false;
		markFinished();
		}));
	drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
		});

	font = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 16);

	npcIntroduction = std::make_shared<DauDauNPC>(transformManager, 167.0f, -18.0f);
	npcIntroduction->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	npcIntroduction->AddLine(L"Dau Dau", L"Hello! Welcome.");
	npcIntroduction->AddLine(L"Player", L"Where am I?");
	npcIntroduction->AddLine(L"Dau Dau", L"This is a cyber world! You will encounter many challenges here. Like digital foes and cyber pirates!");
	npcIntroduction->AddLine(L"Player", L"How do I get out?");
	npcIntroduction->AddLine(L"Dau Dau", L"How to escape this world? I don't know.");
	npcIntroduction->AddLine(L"Dau Dau", L"But I can teach you how to survive here! Explore around a bit and I'll explain further.");
	npcIntroduction->AddLine(L"Dau Dau", L"By the way, use the floppy disk icon over there to save your progress.");
	npcExplainingEnemyEncounters = std::make_shared<DauDauNPC>(transformManager, 544.0f, -56.0f);
	npcExplainingEnemyEncounters->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	npcExplainingEnemyEncounters->AddLine(L"Dau Dau", L"Look out ahead! See those digital creeps roaming around?");
	npcExplainingEnemyEncounters->AddLine(L"Dau Dau", L"If they spot you, they will chase you down! Touching them will drag you into a battle.");
	npcExplainingEnemyEncounters->AddLine(L"Dau Dau", L"You can try to outrun them or hide behind walls to break their line of sight.");
	npcExplainingEnemyEncounters->AddLine(L"Dau Dau", L"Don't worry, you can run away from battles if you want.\n But you won't get any rewards if you do that!");
	npcExplainingHealingPoint = std::make_shared<DauDauNPC>(transformManager, 289.0f, -496.0f);
	npcExplainingHealingPoint->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	npcExplainingHealingPoint->AddLine(L"Dau Dau", L"Hey, you look hurt.");
	npcExplainingHealingPoint->AddLine(L"Player", L"Yeah, I feel dizzy...");
	npcExplainingHealingPoint->AddLine(L"Dau Dau", L"This is a healing point. You can use it to restore your health. Just interact with it like you do with me.");
	npcExplainingHealingPoint->AddLine(L"Dau Dau", L"If you want to heal in combat, you can use healing items! Check out my shop up ahead for some.");
	npcExplainingPortal = std::make_shared<DauDauNPC>(transformManager, 630.f, -639.f);
	npcExplainingPortal->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	npcExplainingPortal->AddLine(L"Dau Dau", L"This is a portal. It will take you to the next area.");
	npcExplainingPortal->AddLine(L"Dau Dau", L"Just step on it and you'll be teleported. It's that simple!");
	npcExplainingPortal->AddLine(L"Dau Dau", L"Beware that portals can be a one way trip!");

	auto borderTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	borderTex->LoadTexture(L"assets/popup-borders.png");

	auto uiTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	uiTex->LoadTexture(L"assets/ui.png");

	PopupManager::GetInstance()->Init(game->GetGraphicsDevice(), borderTex, uiTex, font);
	QuestManager::GetInstance()->SetVirtualResolution(game->GetVirtualWidth(), game->GetVirtualHeight());
	QuestManager::GetInstance()->Init(game->GetGraphicsDevice(), transformManager, &this->uiCamera, font);
	QuestManager::GetInstance()->SetQuest(L"Quest: ???");

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 248.0f, -70.0f));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, -64.0f, -592.0f));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	shopPoint_Card = std::make_shared<ShopPoint>(transformManager, 183.0f, -460.0f);
	shopPoint_Card->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) {
			return new CardShop(g, p, w, h, ShopTier::BASIC);
		}
	);
	shopPoint_Card->SetVisible(true);

	shopPoint_BSItem = std::make_shared<ShopPoint>(transformManager, 320.0f, -660.0f);
	shopPoint_BSItem->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) {
			return new ItemShop(g, p, w, h, ShopTier::BASIC);
		}
	);
	shopPoint_BSItem->SetVisible(true);

	healingPoint = std::make_shared<HealingPoint>(transformManager, 368.0f, -400.0f);
	healingPoint->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoint->SetVisible(true);

	//TreasureChest
	treasureChests.push_back(std::make_shared<TreasureChestNPC>(
		transformManager, 372.f, -483.f,
		std::vector<ChestReward>{
		ChestReward::Item(0, 1),
			ChestReward::Card("StrikeCard")
	}, true));
	treasureChests.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);

	treasureChests.push_back(std::make_shared<TreasureChestNPC>(
		transformManager, 164.f, -380.f,
		std::vector<ChestReward>{
		ChestReward::Item(2, 1),
			ChestReward::Card("HeavyStrikeCard")
	}, true));
	treasureChests.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);

	BattleEncounter testEncounter;
	testEncounter.mapEnemyID = "tutorial_mimic_01";
	testEncounter.enemyTypes = { "MimicEnemy" };
	testEncounter.bgmName = "battle_loop1";
	testEncounter.bgDrawFunc = [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) {
		DrawBackground(gd, deltaTime);
		};
	testEncounter.mapTexturePath = L"assets/notresponding-Sheet.png";
	testEncounter.spriteWidth = 64;
	testEncounter.spriteHeight = 64;

	//map enemies
	auto spawn = [&](float x, float y, std::string id, std::vector<std::string> types, bool isRand, bool isGlobal) {
		auto bgDraw = [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) {
			DrawBackground(gd, deltaTime);
			};

		auto enemy = EnemyFactory::CreateMapEnemy(
			x, y, id, types, isRand, isGlobal, "battle_loop", bgDraw,
			transformManager, game, colliderManager.get(), player
		);

		//token spawning area
		Demo::EventType generatedEvent = Demo::EventType::None;
		if (enemy->GetEncounterData().enemyTypes.size() >= 2 && Demo::RNG::Range(1, 100) <= 30) {
			generatedEvent = (Demo::RNG::Range(1, 100) <= 50) ? Demo::EventType::Gold : Demo::EventType::Energy;
		}
		enemy->SetEventState(generatedEvent);

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

	//test kernel
	spawn(615.f, -170.f, "tutorial_keye_01", { "KeyeEnemy"}, false, false);
	spawn(510.f, -380.f, "tutorial_demoneye_01", { "DemonEyeEnemy" }, false, false);
	spawn(500.f, -590.f, "tutorial_random_01", { "KeyeEnemy", "DemonEyeEnemy" }, true, false);
	spawn(50.f, -330.f, "tutorial_random_02", { "KeyeEnemy", "DemonEyeEnemy", "MimicEnemy" }, true, false);

	draggableManager = std::make_shared<Demo::DraggableManager>();

	inventoryMenu = std::make_shared<InventoryMenu>(game, player, transformManager, draggableManager, &this->uiCamera, font.get());
	inventoryMenu->Init();

	playerHUD = std::make_shared<PlayerHUD>(game, player, transformManager, &this->uiCamera, font.get());
	playerHUD->SetOnInventoryOpen([this]() {
		if (inventoryMenu && !inventoryMenu->IsOpen()) inventoryMenu->Toggle();
		});
	playerHUD->Init();

	player->SetBaseSurface("default");

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

void Demo::TutorialWorldScene::Update(unsigned long long deltaTime)
{
	PopupManager::GetInstance()->SetUICamera(&this->uiCamera);
	QuestManager::GetInstance()->SetUICamera(&this->uiCamera);
	QuestManager::GetInstance()->SetQuest(
		questStarted ? L"Quest: Fint a way out of this place!" : L"Quest: ???"
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
		float sw = game->GetVirtualWidth();
		float sh = game->GetVirtualHeight();
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

	if (npcIntroduction) {
		npcIntroduction->Update(deltaTime);
		if (!currentConversation && npcIntroduction->CanInteract() && inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
			float sw = game->GetVirtualWidth();
			float sh = game->GetVirtualHeight();
			currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
			for (auto& line : npcIntroduction->GetDialogueLines()) {
				currentConversation->AddLine(line);
			}
			QuestManager::GetInstance()->SetQuest(L"Quest: Fint a way out of this place!");
			questStarted = true;
		}
	}
	if (npcExplainingHealingPoint) {
		npcExplainingHealingPoint->Update(deltaTime);
		if (!currentConversation && npcExplainingHealingPoint->CanInteract() && inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
			float sw = game->GetVirtualWidth();
			float sh = game->GetVirtualHeight();
			currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
			for (auto& line : npcExplainingHealingPoint->GetDialogueLines()) {
				currentConversation->AddLine(line);
			}
		}
	}
	if (npcExplainingEnemyEncounters) {
		npcExplainingEnemyEncounters->Update(deltaTime);
		if (!currentConversation && npcExplainingEnemyEncounters->CanInteract() && inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
			float sw = game->GetVirtualWidth();
			float sh = game->GetVirtualHeight();
			currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
			for (auto& line : npcExplainingEnemyEncounters->GetDialogueLines()) {
				currentConversation->AddLine(line);
			}
		}
	}
	if (npcExplainingPortal) {
		npcExplainingPortal->Update(deltaTime);
		if (!currentConversation && npcExplainingPortal->CanInteract() && inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
			float sw = game->GetVirtualWidth();
			float sh = game->GetVirtualHeight();
			currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
			for (auto& line : npcExplainingPortal->GetDialogueLines()) {
				currentConversation->AddLine(line);
			}
		}
	}

	if (currentConversation) {
		isGamePaused = true;
		currentConversation->Execute(deltaTime);
		if (currentConversation->IsFinished()) currentConversation = nullptr;
	}

	for (auto& savePoint : savePoints) {
		savePoint->Update(deltaTime);
	}

	if (shopPoint_Card) shopPoint_Card->Update(deltaTime);
	if (shopPoint_BSItem) shopPoint_BSItem->Update(deltaTime);

	if (healingPoint) healingPoint->Update(deltaTime);

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
				float sw = game->GetVirtualWidth();
				float sh = game->GetVirtualHeight();
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

void Demo::TutorialWorldScene::DrawWorld(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	if (SUCCEEDED(gd->BeginDraw())) {

		DrawBackground(gd, deltaTime);
		map->Draw(camera);
		struct DepthNode {
			float y;
			std::function<void()> drawCall;
			bool operator<(const DepthNode& other) const { return y < other.y; }
		};
		std::vector<DepthNode> depthNodes;

		if (npcIntroduction) depthNodes.push_back({ npcIntroduction->GetWorldY(), [&]() { npcIntroduction->Draw(camera, deltaTime); } });
		if (npcExplainingHealingPoint) depthNodes.push_back({ npcExplainingHealingPoint->GetWorldY(), [&]() { npcExplainingHealingPoint->Draw(camera, deltaTime); } });
		if (npcExplainingEnemyEncounters) depthNodes.push_back({ npcExplainingEnemyEncounters->GetWorldY(), [&]() { npcExplainingEnemyEncounters->Draw(camera, deltaTime); } });
		if (npcExplainingPortal) depthNodes.push_back({ npcExplainingPortal->GetWorldY(), [&]() { npcExplainingPortal->Draw(camera, deltaTime); } });
		if (shopPoint_Card) depthNodes.push_back({ shopPoint_Card->GetWorldY(), [&]() { shopPoint_Card->Draw(camera, deltaTime); } });
		if (shopPoint_BSItem) depthNodes.push_back({ shopPoint_BSItem->GetWorldY(), [&]() { shopPoint_BSItem->Draw(camera, deltaTime); } });
		if (healingPoint) depthNodes.push_back({ healingPoint->GetWorldY(), [&]() { healingPoint->Draw(camera, deltaTime); } });

		for (auto& savePoint : savePoints) {
			depthNodes.push_back({ savePoint->GetWorldY(), [&, savePoint]() { savePoint->Draw(camera, deltaTime); } });
		}
		for (auto& chest : treasureChests) {
			depthNodes.push_back({ chest->GetWorldY(), [&, chest]() { chest->Draw(camera, deltaTime); } });
		}
		for (auto& enemy : mapEnemies) {
			depthNodes.push_back({ enemy->GetWorldY(), [&, enemy]() { enemy->Draw(&camera, deltaTime); } });
		}
		
		if (player) depthNodes.push_back({ player->GetWorldY(), [&]() { player->Draw(deltaTime); } });

		std::sort(depthNodes.begin(), depthNodes.end());
		for (auto& node : depthNodes) {
			node.drawCall();
		}

		gd->EndDraw();
	}
}

void Demo::TutorialWorldScene::DrawUI(unsigned long long deltaTime)
{
	CreateImGuiDebugFrame(player);
	auto gd = game->GetGraphicsDevice();

	if (SUCCEEDED(gd->BeginDraw())) {

		if (healingPoint) healingPoint->DrawUI(&this->uiCamera, deltaTime);
		for (auto& savePoint : savePoints) {
			savePoint->DrawUI(&this->uiCamera, deltaTime);
		}
		for (auto& chest : treasureChests) {
			chest->DrawUI(&this->uiCamera, deltaTime);
		}

		if (shopPoint_Card) shopPoint_Card->DrawUI(&this->uiCamera, deltaTime);
		if (shopPoint_BSItem) shopPoint_BSItem->DrawUI(&this->uiCamera, deltaTime);

		if (npcIntroduction) npcIntroduction->DrawUI(&this->uiCamera, deltaTime);
		if (npcExplainingHealingPoint) npcExplainingHealingPoint->DrawUI(&this->uiCamera, deltaTime);
		if (npcExplainingEnemyEncounters) npcExplainingEnemyEncounters->DrawUI(&this->uiCamera, deltaTime);
		if (npcExplainingPortal) npcExplainingPortal->DrawUI(&this->uiCamera, deltaTime);
		if (playerHUD) playerHUD->Draw(gd, deltaTime);
		if (inventoryMenu) inventoryMenu->Draw(gd, deltaTime);
		if (draggableManager && inventoryMenu && inventoryMenu->IsOpen() && inventoryMenu->GetCurrentTab() == Demo::InventoryMenu::Tab::DECK) {
			draggableManager->Draw(deltaTime);
		}
		if (inventoryMenu) inventoryMenu->DrawKeyboardReticle(gd, deltaTime);

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

		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		gd->EndDraw();
	}
}

void Demo::TutorialWorldScene::DrawBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
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

	outData["quest"] = { {"questStarted", questStarted} };

	nlohmann::json chestStates = nlohmann::json::array();
	for (auto& c : treasureChests) chestStates.push_back(c->GetIsOpened());
	outData["treasureChests"] = chestStates;

	nlohmann::json enemiesState = nlohmann::json::object();
	for (auto& enemy : mapEnemies) {
		enemiesState[enemy->GetEnemyID()] = {
			{"isDefeated", enemy->IsDefeated()},
			{"respawnTimer", enemy->GetRespawnTimer()},
			{ "eventType", static_cast<int>(enemy->GetEncounterData().eventType) }
		};
	}
	outData["mapEnemies"] = enemiesState;
}

void Demo::TutorialWorldScene::RestoreSaveData(const nlohmann::json& inData)
{
	player->RestoreSaveData(inData["player"]);
	camera.SetPosition(inData["camera"]["x"], inData["camera"]["y"]);
	camera.SetZoom(inData["camera"]["zoom"]);

	if (inData.contains("quest")) {
		questStarted = inData["quest"].value("questStarted", false);
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
				EventType savedEvent = static_cast<EventType>(enemiesState[id].value("eventType", 0));

				enemy->SetDefeatedState(def, timer);
				enemy->SetEventState(savedEvent);
			}
		}
	}
}

void Demo::TutorialWorldScene::GiveTestItems()
{
	//ItemInventory& testItems = this->player->GetInventoryItems();
	//testItems.InitFixedInventory(12);

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