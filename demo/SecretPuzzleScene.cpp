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
#include "Debug.h"
#include "imgui.h"
#include "backends/imgui_impl_dx9.h"

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

	popUpMessage = std::make_shared<Demo::PopUpMessage>(transformManager, game);
	popUpMessage->SetLocalPosition(0.0f, 0.0f);
	popUpMessage->Init(game->GetGraphicsDevice(), &this->uiCamera);

	map = std::make_shared<DX9GF::Map>(game->GetGraphicsDevice());
	map->Create(transformManager, colliderManager, "./assets/SecretPuzzle.tmx");

	map->SetAreaUpdateHandler("trigger_p_back", [this](const DX9GF::Map::ObjectArea& area) {
		if (isTransitioning) return;
		isTransitioning = true;
		auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, true);
		drawBuffer->PushCommand(transitionInCommand);
		commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this, transitionInCommand](std::function<void(void)> markFinished) {
			if (!transitionInCommand->IsFinished()) {
				return;
			}
			auto sceMan = game->GetSceneManager();
			auto targetScene = sceMan->GetScene(static_cast<size_t>(sceMan->GetIndex()) - 1);
			auto targetPlayer = MainMenu::gameSaveState->GetPlayerFromScene(targetScene);
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
			auto sceMan = game->GetSceneManager();
			auto targetScene = sceMan->GetScene(static_cast<size_t>(sceMan->GetIndex()) + 1);
			auto targetPlayer = MainMenu::gameSaveState->GetPlayerFromScene(targetScene);
			targetPlayer->SetLocalPosition(-417.f, 144.f);
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

	font = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 16);

	auto borderTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	borderTex->LoadTexture(L"assets/popup-borders.png");

	auto uiTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	uiTex->LoadTexture(L"assets/ui.png");

	PopupManager::GetInstance()->Init(game->GetGraphicsDevice(), borderTex, uiTex, font);
	QuestManager::GetInstance()->SetVirtualResolution(game->GetVirtualWidth(), game->GetVirtualHeight());
	QuestManager::GetInstance()->Init(game->GetGraphicsDevice(), transformManager, &this->uiCamera, font);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, -47.0f * 16, -43.0f * 16));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);
	savePoints.push_back(std::make_shared<SavePoint>(transformManager, -42.0f * 16, 23.0f * 16));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);
	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 31.0f * 16, 47.0f * 16));
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
	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 1700.0f, 860.0f));
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

		//token spawning area
		Demo::EventType generatedEvent = Demo::EventType::None;

		if (Demo::RNG::Range(1, 100) <= 45) {
			if (enemy->GetEncounterData().enemyTypes.size() >= 2) {
				generatedEvent = (Demo::RNG::Range(1, 100) <= 50) ? Demo::EventType::Gold : Demo::EventType::Energy;
			}
			else {
				generatedEvent = Demo::EventType::Gold;
			}
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

	spawn(-1140.f, -400.f, "sec_bat_01", { "VampireBatEnemy" }, false, false);
	spawn(-910.f, 80.f, "sec_eye_01", { "DemonEyeEnemy" }, false, false);
	spawn(-780.f, 470.f, "sec_rand_bat_eye_01", { "VampireBatEnemy", "DemonEyeEnemy" }, true, false);
	spawn(-480.f, 450.f, "sec_rand_keye_bat_01", { "KeyeEnemy", "VampireBatEnemy" }, true, false);
	spawn(-330.f, 160.f, "sec_duo_bat_01", { "VampireBatEnemy", "KernelEnemy" }, false, false);
	spawn(-440.f, -80.f, "sec_mimic_trap_01", { "MimicEnemy" }, false, false);
	spawn(-300.f, -450.f, "sec_rand_3types_01", { "DemonEyeEnemy", "KernelEnemy", "MimicEnemy" }, true, false);
	spawn(-60.f, -460.f, "sec_bat_02", { "VampireBatEnemy" }, false, false);
	spawn(90.f, -340.f, "sec_rand_eye_bat_01", { "DemonEyeEnemy", "VampireBatEnemy" }, true, false);
	spawn(765.f, 680.f, "sec_keye_01", { "KernelEnemy" }, false, false);
	spawn(880.f, 730.f, "sec_rand_bat_mimic_01", { "VampireBatEnemy", "MimicEnemy" }, true, false);
	spawn(738.f, 839.f, "sec_duo_eye_01", { "DemonEyeEnemy", "DemonEyeEnemy" }, false, false);
	spawn(1135.f, 930.f, "sec_rand_keye_mimic_01", { "KeyeEnemy", "MimicEnemy" }, true, false);
	spawn(1195.f, 780.f, "sec_bat_03", { "VampireBatEnemy" }, false, false);
	spawn(1350.f, 785.f, "sec_rand_eye_bat_02", { "DemonEyeEnemy", "VampireBatEnemy" }, true, false);

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
	chapterTitleUI = std::make_shared<ChapterTitleUI>(font);

	//npcs init
	NPCConfig daudauConfig = { L"assets/daudau-Sheet.png", 32, 32, 5, 12, 24.f, 8.f, 12.f };

	auto dauDau = std::make_shared<NPC>(transformManager, 1 * 16, -31.0f * 16, daudauConfig);
	dauDau->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	dauDau->RegisterVoice(L"Dau Dau", "bleep12");
	dauDau->SetInteractLogic([](NPC* self) -> std::function<void()> {
		self->AddLine(L"Dau Dau", L"Watch out! This portal is a one-way trip to the invisible maze! Enter if you dare!");
		return nullptr;
		});
	mapNPCs.push_back(dauDau);

	auto dauDauSpawn = std::make_shared<NPC>(transformManager, -80 * 16, -37 * 16, daudauConfig);
	dauDauSpawn->AttachQuestMarker("SecretBoss_Pacman", Demo::QuestMarkerRole::Giver);
	dauDauSpawn->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	dauDauSpawn->RegisterVoice(L"Dau Dau", "bleep12");
	dauDauSpawn->SetInteractLogic([](NPC* self) -> std::function<void()> {
		auto qState = QuestManager::GetInstance()->GetQuestState("SecretBoss_Pacman");

		if (qState == Demo::QuestState::Locked) {
			self->AddLine(L"Dau Dau", L"There's a hidden boss somewhere in this maze.");
			self->AddLine(L"Dau Dau", L"Defeat it for a secret reward!");

			return []() {
				std::vector<std::pair<std::wstring, std::function<void()>>> buttons = {
					{ L"Yes(Y)", []() { QuestManager::GetInstance()->AcceptQuest("SecretBoss_Pacman"); } },
					{ L"No(N)", []() {} }
				};
				PopupManager::GetInstance()->Show("stepped_blue", L"Secret Boss", L"Accept this challenge?", buttons);
				};
		}
		else if (qState == Demo::QuestState::Active) {
			self->AddLine(L"Dau Dau", L"The boss is still lurking somewhere... Find it!");
		}
		else {
			self->AddLine(L"Dau Dau", L"Incredible! You actually defeated the secret boss!");
		}
		return nullptr;
		});
	mapNPCs.push_back(dauDauSpawn);

	//the hidden boss dauDauSpawn was hinting at; talks once, then the fight starts when he's done talking
	cupidNPC = std::make_shared<CupidNPC>(transformManager, 1671.f, 792.f);
	cupidNPC->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	cupidNPC->AddLine(L"???", L"Oh? A visitor, all the way out here?");
	cupidNPC->AddLine(L"???", L"Nobody finds this dead end unless they're looking for something they shouldn't.");
	cupidNPC->AddLine(L"Pacman", L"Fine, fine. Pacman. Not that the name means much anymore -\nthis process got repurposed long before you wandered in.");
	cupidNPC->AddLine(L"Pacman", L"I'm what's left guarding a rusty key to a precious treasure.\nYou must know what I'm talking about, right?");
	cupidNPC->AddLine(L"Pacman", L"So here's the deal: beat me, take the key.\nLose, and you get to enjoy the scenic route back to the start.");
	cupidNPC->AddLine(L"Player", L"...I've come this far. Might as well.");
	cupidNPC->AddLine(L"Pacman", L"That's the spirit. Don't say I didn't warn you.");
}

void Demo::SecretPuzzleScene::Update(unsigned long long deltaTime)
{
	PopupManager::GetInstance()->SetUICamera(&this->uiCamera);
	QuestManager::GetInstance()->SetUICamera(&this->uiCamera);
	QuestManager::GetInstance()->SetVirtualResolution(game->GetVirtualWidth(), game->GetVirtualHeight());
	QuestManager::GetInstance()->SetVisible(!(inventoryMenu && inventoryMenu->IsOpen()));
	QuestManager::GetInstance()->Update(deltaTime);
	if (!hasSeenChapterIntro && !isTransitioning) {
		hasSeenChapterIntro = true;
		if (chapterTitleUI) {
			chapterTitleUI->Show(L"ANOMALY DETECTED: THE ROOT", L"< Encrypted Database >", 4.0f, 0xFFFF3333, 0xFF33FF99, 0xFF000000);
		}
	}
	if (chapterTitleUI) {
		chapterTitleUI->Update(deltaTime);
	}

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
		currentConversation->AddLine({ .name = L"Treasure Chest", .content = msg, .voiceClip = std::optional<std::string>("bleep20") });
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

	// Update PopUpMessage
	if (popUpMessage) {
		popUpMessage->Update(deltaTime);
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

	//npcs init
	for (auto& npc : mapNPCs) {
		npc->Update(deltaTime);

		if (!currentConversation && npc->CanInteract() && inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
			npc->ClearLines();
			auto onEndCallback = npc->TriggerInteract();

			activeNPC = npc;
			activeNPC->SetOnDialogueEnd(onEndCallback);

			auto [sw, sh] = camera.GetScreenResolution();
			currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
			for (auto& line : npc->GetDialogueLines()) {
				currentConversation->AddLine(line);
			}
			break;
		}
	}

	if (currentConversation) {
		isGamePaused = true;
		currentConversation->Execute(deltaTime);

		if (currentConversation->IsFinished()) {
			//call back active quest
			if (activeNPC && activeNPC->GetOnDialogueEnd()) {
				activeNPC->GetOnDialogueEnd()();
			}

			if (onConversationEnd) {
				auto callback = std::move(onConversationEnd);
				onConversationEnd = nullptr;
				callback();
			}

			currentConversation = nullptr;
			activeNPC = nullptr;
		}
	}

	if (cupidNPC) {
		cupidNPC->Update(deltaTime);
		if (!currentConversation && cupidNPC->CanInteract() && inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
			StartCupidConversation();
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
				currentConversation->AddLine({ .name = L"Treasure Chest", .content = msg, .voiceClip = std::optional<std::string>("bleep20") });
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

		struct DepthNode {
			float y;
			std::function<void()> drawCall;
			bool operator<(const DepthNode& other) const { return y < other.y; }
		};
		std::vector<DepthNode> depthNodes;

		for (auto& savePoint : savePoints) depthNodes.push_back({ savePoint->GetWorldY(), [&, savePoint]() { savePoint->Draw(camera, deltaTime); } });
		for (auto& shopPoint : shopPoints) depthNodes.push_back({ shopPoint->GetWorldY(), [&, shopPoint]() { shopPoint->Draw(camera, deltaTime); } });
		for (auto& healingPoint : healingPoints) depthNodes.push_back({ healingPoint->GetWorldY(), [&, healingPoint]() { healingPoint->Draw(camera, deltaTime); } });
		for (auto& chest : treasureChests) depthNodes.push_back({ chest->GetWorldY(), [&, chest]() { chest->Draw(camera, deltaTime); } });
		for (auto& enemy : mapEnemies) depthNodes.push_back({ enemy->GetWorldY(), [&, enemy]() { enemy->Draw(&camera, deltaTime); } });

		for (auto& npc : mapNPCs) {
			depthNodes.push_back({ npc->GetWorldY(), [&, npc]() { npc->Draw(camera, deltaTime); } });
		}

		if (cupidNPC) depthNodes.push_back({ cupidNPC->GetWorldY(), [&]() { cupidNPC->Draw(camera, deltaTime); } });
		if (player) depthNodes.push_back({ player->GetWorldY(), [&]() { player->Draw(deltaTime); } });

		std::sort(depthNodes.begin(), depthNodes.end());
		for (auto& node : depthNodes) {
			node.drawCall();
		}

		gd->EndDraw();
	}
}

void Demo::SecretPuzzleScene::DrawUI(unsigned long long deltaTime)
{
	CreateImGuiDebugFrame(player, game);
	auto gd = game->GetGraphicsDevice();
	if (SUCCEEDED(gd->BeginDraw())) {

		for (auto& savePoint : savePoints) savePoint->DrawUI(&this->uiCamera, deltaTime);
		for (auto& shopPoint : shopPoints) shopPoint->DrawUI(&this->uiCamera, deltaTime);
		for (auto& healingPoint : healingPoints) healingPoint->DrawUI(&this->uiCamera, deltaTime);
		for (auto& chest : treasureChests) chest->DrawUI(&this->uiCamera, deltaTime);

		for (auto& npc : mapNPCs) {
			npc->DrawUI(&this->uiCamera, deltaTime);
		}

		if (playerHUD) playerHUD->Draw(gd, deltaTime);
		if (inventoryMenu) inventoryMenu->Draw(gd, deltaTime);
		if (draggableManager && inventoryMenu && inventoryMenu->IsOpen() && inventoryMenu->GetCurrentTab() == Demo::InventoryMenu::Tab::DECK) {
			draggableManager->Draw(deltaTime);
		}
		if (inventoryMenu) inventoryMenu->DrawKeyboardReticle(gd, deltaTime);
		if (cupidNPC && cupidNPC->GetPhase() != Demo::CupidNPC::Phase::Defeated) cupidNPC->DrawUI(&this->uiCamera, deltaTime);

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

		if (popUpMessage) {
			popUpMessage->Draw(deltaTime);
		}

		if (chapterTitleUI) {
			chapterTitleUI->Draw(&this->uiCamera, deltaTime);
		}
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
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

void Demo::SecretPuzzleScene::StartCupidConversation()
{
	auto [sw, sh] = camera.GetScreenResolution();
	currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
	for (auto& line : cupidNPC->GetDialogueLines()) {
		currentConversation->AddLine(line);
	}

	if (cupidNPC->GetPhase() == CupidNPC::Phase::Waiting) {
		onConversationEnd = [this]() { StartCupidBattle(); };
	}
}

void Demo::SecretPuzzleScene::StartCupidBattle()
{
	if (isTransitioning || isBossDead) return;
	isTransitioning = true;

	std::map<std::string, int> forcedEnemyMap = { {"CupidEnemy", 100} };

	auto demoGame = dynamic_cast<Demo::Game*>(this->game);
	auto app = DX9GF::Application::GetInstance();
	auto battleScene = new CustomBattleScene(demoGame, player, app->GetScreenWidth(), app->GetScreenHeight(), forcedEnemyMap);

	battleScene->SetOnVictoryCallback([this]() {
		this->isBossDead = true;
		if (cupidNPC) cupidNPC->SetPhase(CupidNPC::Phase::Defeated);

		auto result = QuestManager::GetInstance()->NotifyEvent("ENTITY_DEAD", "SecretBoss_Pacman", this->player.get());

		if (result.hasReward) {
			if (this->popUpMessage) {
				this->popUpMessage->ShowMessage(L"(+) " + result.rewardMessage, 5.0f);
			}
		}
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
		// Cleared here (not right after the fight) so it only frees up once we're back in this scene.
		// Without this, trigger_p_next_world stays blocked until a save/reload resets the flag.
		this->isTransitioning = false;
		markFinished();
		}));
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

	if (cupidNPC) outData["cupidNPC"] = { {"phase", static_cast<int>(cupidNPC->GetPhase())} };

	outData["hasSeenChapterIntro"] = hasSeenChapterIntro;
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

	if (inData.contains("cupidNPC") && cupidNPC) {
		// SetPhase(Defeated) also drops the collider, which is what re-opens the path.
		cupidNPC->SetPhase(static_cast<CupidNPC::Phase>(inData["cupidNPC"].value("phase", 0)));
	}

	hasSeenChapterIntro = inData.value("hasSeenChapterIntro", false);
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

void Demo::SecretPuzzleScene::GiveTestItems()
{
	//ItemInventory& testItems = this->player->GetInventoryItems();
	//testItems.InitFixedInventory(10);
}