#include "pch.h"
#include "SettingsManager.h"
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
#include "EncounterGenerator.h"
#include "EnemyFactory.h"
#include "QuestManager.h"
#include "MapBattleScene.h"
#include "CustomBattleScene.h"
#include "SketchyGuyGlobalData.h"
#include "CardCatalog.h"
#include "Debug.h"
#include "imgui.h"
#include "backends/imgui_impl_dx9.h"

void Demo::ThreadAlleyScene::OnInit()
{
	InitCore(-544.5f, 128.5f, L"./assets/ThreadAlley.tmx");

	map->SetAreaUpdateHandler("trigger_p", [this](const DX9GF::Map::ObjectArea& area) {
		CreatePortalTransition(1, 330.f, 263.f, "bgm_boss", 0.3f);
	});
	map->SetAreaUpdateHandler("trigger_tutorial", [this](const DX9GF::Map::ObjectArea& area) {
		CreatePortalTransition(-2, 681.f, -584.f);
	});

	shopPoints.push_back(std::make_shared<ShopPoint>(transformManager, -606, -548));
	shopPoints.back()->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) { return new CardShop(g, p, w, h, ShopTier::HYBRID); }
	);
	shopPoints.back()->SetVisible(true);

	shopPoints.push_back(std::make_shared<ShopPoint>(transformManager, -449, -1293));
	shopPoints.back()->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) { return new ItemShop(g, p, w, h, ShopTier::HYBRID); }
	);
	shopPoints.back()->SetVisible(true);

	shopPoints.push_back(std::make_shared<ShopPoint>(transformManager, 35, -1413));
	shopPoints.back()->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) { return new CardShop(g, p, w, h, ShopTier::BASIC); }
	);
	shopPoints.back()->SetVisible(true);

	shopPoints.push_back(std::make_shared<ShopPoint>(transformManager, 400, -594));
	shopPoints.back()->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) { return new ItemShop(g, p, w, h, ShopTier::BASIC); }
	);
	shopPoints.back()->SetVisible(true);

	NPCConfig hkhangConfig = { L"assets/daudau-Sheet.png", 32, 32, 5, 12, 24.f, 8.f, 12.f };

	auto hKhang = std::make_shared<NPC>(transformManager, -580.f, 100.f, hkhangConfig);
	hKhang->AttachQuestMarker("Quest_ThreadAlley_Start", Demo::QuestMarkerRole::Giver);
	hKhang->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	hKhang->RegisterVoice(L"Huu Khang", "bleep12");
	hKhang->SetInteractLogic([](NPC* self) -> std::function<void()> {
		auto qState = QuestManager::GetInstance()->GetQuestState("Quest_ThreadAlley_Start");
		if (qState == Demo::QuestState::Locked) {
			self->AddLine(L"Huu Khang", L"Hey, keep your firewall up if you're heading down Thread Alley.");
			self->AddLine(L"Huu Khang", L"I wrote the routing logic for this sector. It was supposed to be a clean shortcut\nfor background processes, but lately... things go in and don't come out.");
			self->AddLine(L"Player", L"Malware?");
			self->AddLine(L"Huu Khang", L"Worse. The sneaky kind. It mimics friendly data to bypass the very antivirus protocols\nI implemented.");
			self->AddLine(L"Huu Khang", L"My admin privileges are locked out until the zone is cleared. If you're heading that way,\nmaybe you can flush the corruption out for me?");
			return []() {
				std::vector<std::pair<std::wstring, std::function<void()>>> buttons = {
					{ L"Yes(Y)", []() { QuestManager::GetInstance()->AcceptQuest("Quest_ThreadAlley_Start"); } },
					{ L"No(N)", []() {} }
				};
				PopupManager::GetInstance()->Show("stepped_blue", L"New Quest", L"Investigate Thread Alley?", buttons);
			};
		}
		else if (qState == Demo::QuestState::Active) {
			self->AddLine(L"Huu Khang", L"Watch your back in there. Malware down here doesn't always look like a monster...");
			self->AddLine(L"Huu Khang", L"Sometimes it uses the exact same asset ID as a friend in need. Trust no one's metadata.");
		}
		else {
			self->AddLine(L"Huu Khang", L"You actually deleted it? And your registry is still intact?");
			self->AddLine(L"Huu Khang", L"Not bad! My admin dashboard just lit back up. Thread Alley is finally safe to route\ndata through again. I owe you a code review sometime.");
		}
		return nullptr;
	});
	mapNPCs.push_back(hKhang);

	{
		char codeBuf[8];
		sprintf_s(codeBuf, "%04d", RNG::Range(0, 9999));
		authPassword = codeBuf;
	}

	authTerminal = std::make_shared<AuthTerminal>(transformManager, 1128.f, -280.f);
	authTerminal->Init(game, game->GetGraphicsDevice(), &camera, &this->uiCamera, player, colliderManager, font);
	authTerminal->SetPassword(std::wstring(authPassword.begin(), authPassword.end()));
	authTerminal->SetVisible(true);
	authTerminal->SetOnSolved([this]() {
		authTerminalSolved = true;
		player->GetInventoryItems().AddItem(ITEM_AUTH_TOKEN, 1);
		QuestManager::GetInstance()->NotifyEvent("TROJAN_HAS_TOKEN", "", player.get());
		if (trojanNPC && trojanNPC->GetQuestMarker()) {
			trojanNPC->GetQuestMarker()->SetConditionMet(true);
		}
		auto [sw, sh] = camera.GetScreenResolution();
		currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
		currentConversation->AddLine({ .name = L"Terminal", .content = L"ACCESS GRANTED.\nYou obtained the Authentication Token." });
	});

	trojanNPC = std::make_shared<TrojanNPC>(transformManager, 224.f, -832.f);
	trojanNPC->AttachQuestMarker("Quest_ThreadAlley_Start", Demo::QuestMarkerRole::Receiver);
	trojanNPC->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);

	trojanNPC->AddFriendlyLine(L"???", L"Oh thank the kernel, a live process!\nI was starting to think nobody came down this alley any more.");
	trojanNPC->AddFriendlyLine(L"???", L"I'm a maintenance job. Was, anyway.\nMy session expired halfway through a patch and now the gate won't read me.");
	trojanNPC->AddFriendlyLine(L"???", L"There's a terminal further down the alley that still hands out\nauthentication tokens. Four digits, old-style.");
	trojanNPC->AddFriendlyLine(L"???", L"Bring one back to me and I'll be out of your way.\nYou'd be doing the whole sector a favour, honestly.");
	trojanNPC->AddFriendlyLine(L"Player", L"...Alright. I'll see what I can find.");

	trojanNPC->AddWaitingLine(L"???", L"Still looking for that token?\nTake your time. It's not like I'm going anywhere.");
	trojanNPC->AddWaitingLine(L"???", L"Four digits. The terminal tells you which ones you got right.\nYou'll get there.");

	trojanNPC->AddRevealLine(L"???", L"You actually brought it.");
	trojanNPC->AddRevealLine(L"???", L"...Honestly? I didn't think that would work.");
	trojanNPC->AddRevealLine(L"Trojan", L"Let me walk you through what just happened.\nThat token was never mine. It was yours.");
	trojanNPC->AddRevealLine(L"Trojan", L"I didn't break a single lock. I didn't have to.\nI asked nicely, and you handed me the keys to your own system.");
	trojanNPC->AddRevealLine(L"Trojan", L"That's all I am. A friendly face wrapped around something\nyou would never have let through the door.");
	trojanNPC->AddRevealLine(L"Player", L"...No.");
	trojanNPC->AddRevealLine(L"Player", L"You didn't trick me into anything.\nI'm taking it back - and I'm deleting you with it.");
	trojanNPC->AddRevealLine(L"Trojan", L"Ha! Then stop talking and try.");

	sketchyGuy = std::make_shared<SketchyGuyNPC>(transformManager, 1024.f, -416.f);
	sketchyGuy->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, -488, -140));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);
	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 134, -1129));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);
	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 230, -392));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, -328, -537));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);
	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, -230, -1148));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	struct ChestDef {
		std::pair<float, float> pos;
		std::vector<ChestReward> rewards;
		bool randomPick = false;
	};
	const std::vector<ChestDef> chestDefs = {
		{{-474.f, -921.f}, { ChestReward::Item(2,1), ChestReward::Item(3,1) },                           true},
		{{1016.f, -246.f}, { ChestReward::Item(3,1), ChestReward::Card("CleaveCard") },                  true},
		{{ 624.f, -442.f}, { ChestReward::Item(4,1), ChestReward::Card("TwinStrikeCard") },              true},
		{{1096.f,  -90.f}, { ChestReward::Item(4,1), ChestReward::Item(5,1) },                           true},
		{{1152.f, -906.f}, { ChestReward::Item(3,1), ChestReward::Card("PoisonCard") },                  true},
		{{ 983.f, -841.f}, { ChestReward::Item(2,1), ChestReward::Item(3,1), ChestReward::Card("CleaveCard") }, true},
		{{ 992.f,-1060.f}, { ChestReward::Item(4,1), ChestReward::Card("VulnerableCard") },              true},
		{{ -69.f, -978.f}, { ChestReward::Item(5,1), ChestReward::Card("PoisonCard") },                  true},
		{{ 418.f,-1208.f}, { ChestReward::Item(3,1), ChestReward::Card("CleaveCard") },                  true},
	};
	for (auto& def : chestDefs) {
		treasureChests.push_back(std::make_shared<TreasureChestNPC>(
			transformManager, def.pos.first, def.pos.second,
			def.rewards, def.randomPick));
		treasureChests.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	}

	auto bgDraw = [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) {
		DrawCheckerBackground(gd, deltaTime);
	};

	auto spawnThreadAlley = [&](float x, float y, std::string id, std::vector<std::string> types, bool isRand, bool isGlobal) {
		std::string bgm = (id == "sec_miniboss_01") ? "bgm_boss" : "battle_loop";

		auto enemy = EnemyFactory::CreateMapEnemy(
			x, y, id, types, isRand, isGlobal, bgm, bgDraw,
			transformManager, game, colliderManager.get(), player
		);

		Demo::EventType generatedEvent = Demo::EventType::None;
		if (Demo::RNG::Range(1, 100) <= 38) {
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

	spawnThreadAlley(-735.f, -160.f, "th_intro_02", { "VampireBatEnemy" }, false, false);
	spawnThreadAlley(-210.f, -180.f, "th_intro_03", { "DemonEyeEnemy" }, false, false);
	spawnThreadAlley(-57.f, -219.f, "th_intro_04", { "KeyeEnemy", "KernelEnemy" }, true, false);
	spawnThreadAlley(-572.f, -316.f, "th_mid_01", { "VampireBatEnemy", "VampireBatEnemy" }, false, false);
	spawnThreadAlley(-330.f, -400.f, "th_mid_02", { "MimicEnemy" }, false, false);
	spawnThreadAlley(-18.f, -536.f, "th_mid_03", { "KernelEnemy", "VampireBatEnemy" }, true, false);
	spawnThreadAlley(-230.f, -550.f, "th_mid_04", { "DemonEyeEnemy", "DemonEyeEnemy" }, false, false);
	spawnThreadAlley(200.f, -560.f, "th_mid_05", {}, false, true);
	spawnThreadAlley(-610.f, -726.f, "th_dark_01", { "WarlockEnemy" }, false, false);
	spawnThreadAlley(925.f, -605.f, "th_dark_02", { "WarlockEnemy", "KernelEnemy" }, false, false);
	spawnThreadAlley(-370.f, -910.f, "th_dark_03", { "DemonEyeEnemy", "WarlockEnemy" }, true, false);
	spawnThreadAlley(580.f, -910.f, "th_dark_04", { "KernelEnemy", "VampireBatEnemy" }, false, false);
	spawnThreadAlley(920.f, -950.f, "th_dark_05", { "WarlockEnemy", "WarlockEnemy" }, false, false);
	spawnThreadAlley(-552.f, -1024.f, "th_dark_06", {}, false, true);
	spawnThreadAlley(-10.f, -1055.f, "th_deep_01", { "KeyeEnemy", "KernelEnemy" }, true, false);
	spawnThreadAlley(280.f, -1080.f, "th_deep_02", { "DemonEyeEnemy", "MimicEnemy" }, true, false);
	spawnThreadAlley(-480.f, -1110.f, "th_deep_03", { "VampireBatEnemy" }, false, false);
	spawnThreadAlley(-10.f, -1200.f, "th_deep_04", { "MimicEnemy" }, false, false);
	spawnThreadAlley(590.f, -1250.f, "th_deep_05", { "WarlockEnemy", "MimicEnemy" }, true, false);
	spawnThreadAlley(1150.f, -1280.f, "th_deep_06", { "KeyeEnemy", "DemonEyeEnemy", "KernelEnemy" }, true, false);
	spawnThreadAlley(-183.f, -1465.f, "th_end_01", { "WarlockEnemy", "DemonEyeEnemy" }, false, false);
	spawnThreadAlley(920.f, -1380.f, "th_end_02", {}, false, true);
	spawnThreadAlley(-700.f, -1430.f, "th_end_03", { "VampireBatEnemy", "VampireBatEnemy", "VampireBatEnemy" }, false, false);
	spawnThreadAlley(-410.f, -1500.f, "th_end_04", { "WarlockEnemy", "KernelEnemy", "DemonEyeEnemy" }, false, false);
	spawnThreadAlley(620.f, -165.f, "th_extra_01", { "KeyeEnemy" }, false, false);
	spawnThreadAlley(1024.f, -120.f, "th_extra_03", { "KernelEnemy" }, false, false);
	spawnThreadAlley(1135.f, -525.f, "th_extra_04", {}, false, true);

	player->SetBaseSurface("concrete");

	transformManager->RebuildHierarchy();
	this->GiveTestItems();
	drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
}

void Demo::ThreadAlleyScene::Update(unsigned long long deltaTime)
{
	PopupManager::GetInstance()->SetUICamera(&this->uiCamera);
	QuestManager::GetInstance()->SetUICamera(&this->uiCamera);
	QuestManager::GetInstance()->SetVirtualResolution(game->GetVirtualWidth(), game->GetVirtualHeight());
	QuestManager::GetInstance()->SetVisible(!(inventoryMenu && inventoryMenu->IsOpen())
		&& !(authTerminal && authTerminal->IsMenuOpen()));
	QuestManager::GetInstance()->Update(deltaTime);

	if (!hasSeenChapterIntro && !isTransitioning) {
		hasSeenChapterIntro = true;
		if (chapterTitleUI) {
			chapterTitleUI->Show(L"CHAPTER II: THREAD ALLEY", L"< Data Transit Zone >", 4.0f, 0xFFFFFFFF, 0xFF00FFFF, 0xFF000000);
		}
	}
	if (chapterTitleUI) {
		chapterTitleUI->Update(deltaTime);
	}

	auto inpMan = DX9GF::InputManager::GetInstance();
	inpMan->ReadMouse(deltaTime);
	inpMan->ReadKeyboard(deltaTime);

	bool wasTerminalMenuOpen = authTerminal && authTerminal->IsMenuOpen();
	if (authTerminal && (wasTerminalMenuOpen
		|| (!currentConversation && !PopupManager::GetInstance()->IsActive()))) {
		authTerminal->Update(deltaTime);
	}
	bool isTerminalMenuOpen = authTerminal && authTerminal->IsMenuOpen();

	static float escCooldown = 0.0f;
	if (escCooldown > 0) escCooldown -= deltaTime;
	if (wasTerminalMenuOpen && !isTerminalMenuOpen) escCooldown = 300.0f;

	if (!isTerminalMenuOpen && inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("OPEN_INVENTORY")) && escCooldown <= 0) {
		if (inventoryMenu) inventoryMenu->Toggle();
		escCooldown = 300.0f;
	}

	bool isGamePaused = this->isGamePaused || isTerminalMenuOpen;

	if (PopupManager::GetInstance()->IsActive()) {
		PopupManager::GetInstance()->Update(deltaTime, &this->uiCamera);
		isGamePaused = true;
	}

	if (popUpMessage) {
		popUpMessage->Update(deltaTime);
	}

	if (currentConversation) {
		isGamePaused = true;
		currentConversation->Execute(deltaTime);
		if (currentConversation->IsFinished()) {
			currentConversation = nullptr;
			if (activeNPC && activeNPC->GetOnDialogueEnd()) {
				activeNPC->GetOnDialogueEnd()();
			}
			activeNPC = nullptr;

			if (onConversationEnd) {
				auto callback = std::move(onConversationEnd);
				onConversationEnd = nullptr;
				callback();
			}
		}
	}

	for (auto& npc : mapNPCs) {
		npc->Update(deltaTime);
		if (!currentConversation && !isTerminalMenuOpen && npc->CanInteract() && inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
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

	if (trojanNPC) {
		trojanNPC->Update(deltaTime);
		if (!currentConversation && !isTerminalMenuOpen && trojanNPC->CanInteract()
			&& inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
			StartTrojanConversation();
		}
	}

	if (sketchyGuy) {
		sketchyGuy->Update(deltaTime);
		if (!currentConversation && !isTerminalMenuOpen
			&& !PopupManager::GetInstance()->IsActive()
			&& !(inventoryMenu && inventoryMenu->IsOpen())
			&& game->GetSceneManager()->GetCurrentScene() == this
			&& sketchyGuy->CanInteract()
			&& inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
			StartSketchyGuyInteraction();
		}
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
		if (!currentConversation && chest->CanInteract() && inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
			OpenChestWithDialog(chest);
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

void Demo::ThreadAlleyScene::OnUpdate(unsigned long long deltaTime)
{
}

void Demo::ThreadAlleyScene::OnDrawWorld(std::vector<DepthNode>& depthNodes, unsigned long long deltaTime)
{
	if (authTerminal) AddDepthNode(depthNodes, authTerminal->GetWorldY(), [&]() { authTerminal->Draw(camera, deltaTime); });
	if (trojanNPC) AddDepthNode(depthNodes, trojanNPC->GetWorldY(), [&]() { trojanNPC->Draw(camera, deltaTime); });
	if (sketchyGuy) AddDepthNode(depthNodes, sketchyGuy->GetWorldY(), [&]() { sketchyGuy->Draw(camera, deltaTime); });
}

void Demo::ThreadAlleyScene::OnDrawUI(unsigned long long deltaTime)
{
	if (trojanNPC && trojanNPC->GetPhase() != Demo::TrojanNPC::Phase::Defeated) trojanNPC->DrawUI(&this->uiCamera, deltaTime);
	if (sketchyGuy) sketchyGuy->DrawUI(&this->uiCamera, deltaTime);
	if (authTerminal) authTerminal->DrawUI(&this->uiCamera, deltaTime);
}

std::string Demo::ThreadAlleyScene::GetSaveID() const
{
	return "ThreadAlleyScene";
}

void Demo::ThreadAlleyScene::OnGenerateSaveData(nlohmann::json& outData)
{
	outData["authTerminal"] = {
		{"password", authPassword},
		{"solved", authTerminalSolved}
	};
	if (trojanNPC) outData["trojanNPC"] = { {"phase", static_cast<int>(trojanNPC->GetPhase())} };
}

void Demo::ThreadAlleyScene::OnRestoreSaveData(const nlohmann::json& inData)
{
	if (inData.contains("authTerminal")) {
		authPassword = inData["authTerminal"].value("password", authPassword);
		authTerminalSolved = inData["authTerminal"].value("solved", false);
		if (authTerminal) {
			authTerminal->SetPassword(std::wstring(authPassword.begin(), authPassword.end()));
			authTerminal->SetSolved(authTerminalSolved);
		}
	}
	if (inData.contains("trojanNPC") && trojanNPC) {
		auto savedPhase = static_cast<TrojanNPC::Phase>(inData["trojanNPC"].value("phase", 0));
		trojanNPC->SetPhase(savedPhase);

		if (savedPhase == TrojanNPC::Phase::AwaitingToken) {
			if (player->GetInventoryItems().HasItem(ITEM_AUTH_TOKEN)) {
				QuestManager::GetInstance()->NotifyEvent("TROJAN_HAS_TOKEN", "", player.get());
				if (trojanNPC && trojanNPC->GetQuestMarker()) trojanNPC->GetQuestMarker()->SetConditionMet(true);
			}
			else {
				QuestManager::GetInstance()->NotifyEvent("TROJAN_TALKED", "", player.get());
			}
		}
		else if (savedPhase == TrojanNPC::Phase::Revealed) {
			QuestManager::GetInstance()->NotifyEvent("TROJAN_REVEALED", "", player.get());
		}
	}
}

void Demo::ThreadAlleyScene::GiveTestItems()
{
}

void Demo::ThreadAlleyScene::StartTrojanConversation()
{
	auto phase = trojanNPC->GetPhase();

	if (phase == TrojanNPC::Phase::AwaitingToken && player->GetInventoryItems().HasItem(ITEM_AUTH_TOKEN)) {
		player->GetInventoryItems().ConsumeItem(ITEM_AUTH_TOKEN);
		trojanNPC->SetPhase(TrojanNPC::Phase::Revealed);
		phase = TrojanNPC::Phase::Revealed;
	}

	auto [sw, sh] = camera.GetScreenResolution();
	currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
	for (auto& line : trojanNPC->GetDialogueLines()) {
		currentConversation->AddLine(line);
	}

	switch (phase) {
	case TrojanNPC::Phase::Friendly:
		onConversationEnd = [this]() { trojanNPC->SetPhase(TrojanNPC::Phase::AwaitingToken); };
		QuestManager::GetInstance()->NotifyEvent("TROJAN_TALKED", "", player.get());
		break;
	case TrojanNPC::Phase::Revealed:
		onConversationEnd = [this]() {
			StartTrojanBattle();
			QuestManager::GetInstance()->NotifyEvent("TROJAN_REVEALED", "", player.get());
		};
		break;
	default:
		onConversationEnd = nullptr;
		break;
	}
}

void Demo::ThreadAlleyScene::StartTrojanBattle()
{
	if (isTransitioning) return;
	isTransitioning = true;

	auto transitionIn = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, true);
	drawBuffer->PushCommand(transitionIn);

	commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this, transitionIn](std::function<void(void)> markFinished) {
		if (!transitionIn->IsFinished()) return;

		auto app = DX9GF::Application::GetInstance();
		auto sceMan = game->GetSceneManager();
		auto battleScene = new CustomBattleScene(game, player, app->GetScreenWidth(), app->GetScreenHeight(),
			std::map<std::string, int>{ { "TrojanEnemy", 100 } });
		battleScene->SetCustomBGM("battle_boss");
		battleScene->SetCustomBackgroundDraw([this](DX9GF::GraphicsDevice* gd, unsigned long long dt) {
			DrawCheckerBackground(gd, dt);
		});
		battleScene->SetOnVictoryCallback([this]() {
			this->commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void()> markFinished) {
				this->trojanNPC->SetPhase(TrojanNPC::Phase::Defeated);
				auto result = QuestManager::GetInstance()->NotifyEvent("TROJAN_DEFEATED", "", this->player.get());
				if (result.hasReward && this->popUpMessage) {
					this->popUpMessage->ShowMessage(L"(+) " + result.rewardMessage, 5.0f);
				}
				markFinished();
			}));
		});

		sceMan->InsertScene(sceMan->GetIndex() + 1, battleScene);
		sceMan->GoToNext();

		isTransitioning = false;
		markFinished();
	}));

	drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
}

void Demo::ThreadAlleyScene::StartSketchyGuyInteraction()
{
	auto* npcData = SketchyGuyGlobalData::GetInstance();

	if (!npcData->HasMet()) {
		auto [sw, sh] = camera.GetScreenResolution();
		currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
		for (auto& line : sketchyGuy->GetDialogueLines()) {
			currentConversation->AddLine(line);
		}
		onConversationEnd = [npcData]() { npcData->SetMet(true); };
		return;
	}

	const int cost = npcData->PackCost();
	std::vector<std::pair<std::wstring, std::function<void()>>> buttons = {
		{ sketchyGuy->GetBuyConfirmLabel(cost), [this, cost]() {
			if (player->GetGold() < cost) {
				DX9GF::AudioManager::GetInstance()->Play("error", false, 0.3f);
				return;
			}
			player->AddGold(-cost);
			SketchyGuyGlobalData::GetInstance()->SetBoughtPack(true);

			auto pulled = CardCatalog::RollPack(5);
			for (auto& id : pulled) {
				player->AddCardToInventory(id);
			}
			DX9GF::AudioManager::GetInstance()->Play("shop_buy", false, 0.8f);

			auto app = DX9GF::Application::GetInstance();
			auto sceMan = game->GetSceneManager();
			auto* packScene = new PackOpeningScene(game, app->GetScreenWidth(), app->GetScreenHeight(), pulled);
			sceMan->InsertScene(sceMan->GetIndex() + 1, packScene);
			sceMan->GoToNext();
		}},
		{ L"No(N)", nullptr }
	};
	PopupManager::GetInstance()->Show("stepped_gold",
		sketchyGuy->GetBuyPromptTitle(), sketchyGuy->GetBuyPromptText(cost), buttons);
}

void Demo::ThreadAlleyScene::DrawCheckerBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
{
	auto [screenWidth, screenHeight] = uiCamera.GetScreenResolution();
	gd->DrawRectangle(0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0xFF403353, true);

	const float SQUARE_SIZE = 128.0f;
	const float BASE_SCROLL_SPEED = 30.0f;
	const float BLINK_PERIOD = 2000.0f;
	const float ANIMATION_DURATION = 800.0f;
	const int PADDING = 6;

	bgBaseScrollX += (BASE_SCROLL_SPEED * deltaTime) / 1000.0f;
	bgBaseScrollY += (BASE_SCROLL_SPEED * deltaTime) / 1000.0f;

	if (bgBaseScrollX >= 2.0f * SQUARE_SIZE) bgBaseScrollX -= 2.0f * SQUARE_SIZE;
	if (bgBaseScrollY >= 2.0f * SQUARE_SIZE) bgBaseScrollY -= 2.0f * SQUARE_SIZE;

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
	}
	else {
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
			}
			else {
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

void Demo::ThreadAlleyScene::DrawBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
{
	DrawCheckerBackground(gd, deltaTime);
}
