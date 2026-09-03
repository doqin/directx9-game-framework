#include "pch.h"
#include "SettingsManager.h"
#include "BossWorldScene.h"
#include "RandomEncounter.h"
#include "CardShop.h"
#include "ItemShop.h"
#include "GameItems.h"
#include "CustomBattleScene.h"
#include "TransitionCommand.h"
#include "OutroScene.h"
#include "Game.h"
#include <vector>
#include <cmath>
#include "resource.h"
#include "PopupManager.h"
#include "EncounterGenerator.h"
#include "EnemyFactory.h"
#include "QuestManager.h"
#include "MapBattleScene.h"
#include "Debug.h"
#include "imgui.h"
#include "backends/imgui_impl_dx9.h"
#include "MainMenu.h"
#include "SaveGameState.h"

void Demo::BossWorldScene::OnInit()
{
	InitCore(360.f, 190.f, L"./assets/BossMatrix.tmx");

	NPCConfig daudauConfig = { L"assets/daudau-Sheet.png", 32, 32, 5, 12, 24.f, 8.f, 12.f };

	auto npcHint = std::make_shared<NPC>(transformManager, -950.0f, -220.0f, daudauConfig);
	npcHint->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	npcHint->RegisterVoice(L"Veteran Debugger", "bleep12");
	npcHint->SetInteractLogic([](NPC* self) -> std::function<void()> {
		self->AddLine(L"Veteran Debugger", L"Halt, traveler. You shouldn't be here.");
		self->AddLine(L"Veteran Debugger", L"This sector is deeply corrupted-a graveyard of unresolved bugs and dead code.");
		self->AddLine(L"Player", L"What exactly happened here?");
		self->AddLine(L"Veteran Debugger", L"What happened? Ambition met reality.");
		self->AddLine(L"Veteran Debugger", L"Teams of debuggers rushed in, thinking they could fix the core.");
		self->AddLine(L"Veteran Debugger", L"They couldn't agree on a protocol. The system panicked, spawned massive anomalies, \nand wiped them out.");
		self->AddLine(L"Veteran Debugger", L"I keep hearing the old admin's logs echoing in my head...");
		self->AddLine(L"Veteran Debugger", L"...'The red sun sets over the blue ocean, giving life to the green earth, until it fades into\norange autumn'...");
		self->AddLine(L"Veteran Debugger", L"Bah, probably just corrupted junk data. Don't mind my rambling.");
		return nullptr;
	});
	mapNPCs.push_back(npcHint);

	NPCConfig hngocConfig = { L"assets/daudau-Sheet.png", 32, 32, 5, 12, 24.f, 8.f, 12.f };
	auto hNgoc = std::make_shared<NPC>(transformManager, 300.f, 230.f, hngocConfig);
	hNgoc->AttachQuestMarker("Quest_BossWorld", Demo::QuestMarkerRole::Giver);
	hNgoc->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	hNgoc->RegisterVoice(L"A Hai`", "bleep12");
	hNgoc->SetInteractLogic([](NPC* self) -> std::function<void()> {
		auto qState = QuestManager::GetInstance()->GetQuestState("Quest_BossWorld");
		if (qState == Demo::QuestState::Locked) {
			self->AddLine(L"A Hai`", L"You made it to the Core Sector! Wait... you're not a compiled asset. Are you a player?");
			self->AddLine(L"Player", L"Who are you?");
			self->AddLine(L"A Hai`", L"Just a dev who flew too close to the sun. I coded the entity behind that gate to manage root data,\nbut my logic was flawed.");
			self->AddLine(L"A Hai`", L"It gained sentience, went rogue, and trapped me in my own system.");
			self->AddLine(L"Player", L"So I just need to go in and delete it to get us out?");
			self->AddLine(L"A Hai`", L"If only it were that simple! It went completely paranoid and sealed itself inside a heavy quarantine zone.");
			self->AddLine(L"A Hai`", L"To force the gate open, you must hack the four security terminals scattered around this area.\nPlease, clean up my mess!");
			return []() {
				std::vector<std::pair<std::wstring, std::function<void()>>> buttons = {
					{ L"Yes(Y)", []() { QuestManager::GetInstance()->AcceptQuest("Quest_BossWorld"); } },
					{ L"No(N)", []() {} }
				};
				PopupManager::GetInstance()->Show("stepped_blue", L"New Quest", L"Hack the 4 Terminals?", buttons);
			};
		}
		else if (qState == Demo::QuestState::Active) {
			self->AddLine(L"A Hai`", L"The firewall is still active. Hack all four terminals to breach the quarantine zone.\nDon't let my buggy code beat you!");
		}
		else {
			self->AddLine(L"A Hai`", L"The gate is open! The root data is right ahead.\nEnd that rogue process and get us out of here!");
		}
		return nullptr;
	});
	mapNPCs.push_back(hNgoc);

	keyeproNPC = std::make_shared<KeyeproNPC>(transformManager, 752.f, -336.f);
	keyeproNPC->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	keyeproNPC->AddLine(L"Anonymous", L"So. You made it through the terminals.");
	keyeproNPC->AddLine(L"Anonymous", L"You still think this is a sector. A place with walls, a gate, a way out.");
	keyeproNPC->AddLine(L"Anonymous", L"It isn't. It's a process. Every corridor you walked, every corrupted debugger\nyou talked to - all of it running because I keep it running.");
	keyeproNPC->AddLine(L"Anonymous", L"I hold the keys. Every one. Nothing in this system closes, resets, or lets go\nwithout my say-so.");
	keyeproNPC->AddLine(L"Anonymous", L"Which makes me the last thing standing between you and whatever you think\nfreedom looks like on the other side of this gate.");
	keyeproNPC->AddLine(L"Player", L"Then I guess I'm taking your keys.");
	keyeproNPC->AddLine(L"Anonymous", L"Try.");

	auto hackCallback = std::bind(&BossWorldScene::OnTerminalHacked, this, std::placeholders::_1);

	auto machine1 = std::make_shared<HackTerminal>(transformManager, 1500, -620, 1, "M1");
	machine1->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, hackCallback);
	hackMachines.push_back(machine1);
	auto machine2 = std::make_shared<HackTerminal>(transformManager, -480, 60, 2, "M2");
	machine2->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, hackCallback);
	hackMachines.push_back(machine2);
	auto machine3 = std::make_shared<HackTerminal>(transformManager, -590, -200, 3, "M3");
	machine3->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, hackCallback);
	hackMachines.push_back(machine3);
	auto machine4 = std::make_shared<HackTerminal>(transformManager, 2440, 395, 4, "M4");
	machine4->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, hackCallback);
	hackMachines.push_back(machine4);
	mainTerminal = std::make_shared<HackTerminal>(transformManager, 780, -200, 99, "Main");
	mainTerminal->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, hackCallback);
	for (auto& m : hackMachines) m->SetVisible(true);
	mainTerminal->SetVisible(true);

	bossGateCollider = std::make_shared<DX9GF::RectangleCollider>(transformManager, 32.f, 48.f, 736.f, -256.f);
	colliderManager->Add(bossGateCollider);

	rustyChest = std::make_shared<RustyChestNPC>(transformManager, 690.f, -208.f);
	rustyChest->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);

	auto addChest = [&](float tx, float ty, std::vector<ChestReward> rewards, bool randomPick = false) {
		auto c = std::make_shared<TreasureChestNPC>(transformManager, tx * 16, ty * 16, rewards, randomPick);
		c->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
		treasureChests.push_back(c);
	};
	addChest(-38, 0, { ChestReward::Item(5,1), ChestReward::Card("ChainLightningCard") }, true);
	addChest(-24, -18, { ChestReward::Item(6,1), ChestReward::Card("WeaknessCard") }, true);
	addChest(166, 8, { ChestReward::Item(7,1), ChestReward::Card("VulnerableCard") }, true);
	addChest(94, -8, { ChestReward::Item(8,1), ChestReward::Item(9,1), ChestReward::Card("VulnerableCard") }, true);
	addChest(94, -32, { ChestReward::Item(6,1), ChestReward::Item(7,1), ChestReward::Card("WeaknessCard") }, true);

	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, -352.f, -160.f));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 192.f, 320.f));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);
	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 704.f, -96.f));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	shopPoints.push_back(std::make_shared<ShopPoint>(transformManager, 176.f, 192.f));
	shopPoints.back()->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) { return new ItemShop(g, p, w, h, ShopTier::RK_HYBRID); }
	);
	shopPoints.back()->SetVisible(true);
	shopPoints.push_back(std::make_shared<ShopPoint>(transformManager, -80, -256));
	shopPoints.back()->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) { return new CardShop(g, p, w, h, ShopTier::PREMIUM); }
	);
	shopPoints.back()->SetVisible(true);
	shopPoints.push_back(std::make_shared<ShopPoint>(transformManager, 630.f, -192.f));
	shopPoints.back()->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) { return new ItemShop(g, p, w, h, ShopTier::PREMIUM); }
	);
	shopPoints.back()->SetVisible(true);

	auto bgDraw = [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) {
		DrawBackground(gd, deltaTime);
	};

	SpawnMapEnemy(1730.f, 10.f, "bw_c_01", { "VampireBatEnemy", "WarlockEnemy" }, false, false, bgDraw, 50);
	SpawnMapEnemy(1520.f, 145.f, "bw_c_02", {}, false, true, bgDraw, 50);
	SpawnMapEnemy(1635.f, 325.f, "bw_c_03", { "DemonEyeEnemy" }, false, false, bgDraw, 50);
	SpawnMapEnemy(1500.f, -360.f, "bw_c_05", { "WarlockEnemy", "KeyeEnemy" }, true, false, bgDraw, 50);
	SpawnMapEnemy(1620.f, -625.f, "bw_c_06", { "WarlockEnemy", "KernelEnemy" }, false, false, bgDraw, 50);
	SpawnMapEnemy(1890.f, -445.f, "bw_c_07", { "MimicEnemy" }, false, false, bgDraw, 50);
	SpawnMapEnemy(1850.f, -625.f, "bw_c_08", { "VampireBatEnemy", "DemonEyeEnemy", "KeyeEnemy" }, true, false, bgDraw, 50);
	SpawnMapEnemy(2085.f, 400.f, "bw_f_01", { "KernelEnemy", "DemonEyeEnemy" }, false, false, bgDraw, 50);
	SpawnMapEnemy(2080.f, 520.f, "bw_f_02", { "KeyeEnemy", "MimicEnemy" }, true, false, bgDraw, 50);
	SpawnMapEnemy(2130.f, -285.f, "bw_f_03", {}, false, true, bgDraw, 50);
	SpawnMapEnemy(2270.f, -120.f, "bw_f_04", { "WarlockEnemy", "WarlockEnemy", "VampireBatEnemy" }, false, false, bgDraw, 50);
	SpawnMapEnemy(2350.f, -630.f, "bw_f_05", { "DemonEyeEnemy", "DemonEyeEnemy" }, false, false, bgDraw, 50);
	SpawnMapEnemy(2460.f, 390.f, "bw_f_06", { "WarlockEnemy", "KeyeEnemy", "DemonEyeEnemy" }, true, false, bgDraw, 50);
	SpawnMapEnemy(2500.f, 20.f, "bw_f_07", { "VampireBatEnemy", "KernelEnemy" }, false, false, bgDraw, 50);
	SpawnMapEnemy(2540.f, -490.f, "bw_f_08", {}, false, true, bgDraw, 50);

	map->SetAreaUpdateHandler("trigger_alley", [this](const DX9GF::Map::ObjectArea& area) {
		CreatePortalTransition(-1, 1239.f, -920.f);
	});

	map->SetAreaUpdateHandler("trigger_p_1_2", [this](const DX9GF::Map::ObjectArea& area) {
		player->SetLocalPosition(-1155, 0);
		this->currentIslandID = 2;
	});
	map->SetAreaUpdateHandler("trigger_p_2_1", [this](const DX9GF::Map::ObjectArea& area) {
		player->SetLocalPosition(48, 230);
		this->currentIslandID = 1;
	});
	map->SetAreaUpdateHandler("trigger_p_2_3", [this](const DX9GF::Map::ObjectArea& area) {
		player->SetLocalPosition(1500, 530);
		this->currentIslandID = 3;
	});
	map->SetAreaUpdateHandler("trigger_p_2_fake", [this](const DX9GF::Map::ObjectArea& area) {
		player->SetLocalPosition(-1155, 0);
	});
	map->SetAreaUpdateHandler("trigger_p_3_4", [this](const DX9GF::Map::ObjectArea& area) {
		player->SetLocalPosition(740, -50);
		this->currentIslandID = 4;
	});
	map->SetAreaUpdateHandler("trigger_p_3_2", [this](const DX9GF::Map::ObjectArea& area) {
		player->SetLocalPosition(-1155, 0);
		this->currentIslandID = 2;
	});
	map->SetAreaUpdateHandler("trigger_p_4_3", [this](const DX9GF::Map::ObjectArea& area) {
		player->SetLocalPosition(2640, -620);
		this->currentIslandID = 3;
	});

	map->SetAreaUpdateHandler("trigger_battle_boss", [this](const DX9GF::Map::ObjectArea& area) {
		if (!player->IsWalking()) return;
		if (this->isBossDoorUnlocked && !this->isFinalBossDead) {
			std::map<std::string, int> forcedEnemyMap = { {"KeyeproEnemy", 100} };
			auto demoGame = dynamic_cast<Demo::Game*>(this->game);
			auto app = DX9GF::Application::GetInstance();
			auto battleScene = new CustomBattleScene(demoGame, player, app->GetScreenWidth(), app->GetScreenHeight(), forcedEnemyMap);
			battleScene->SetCustomBGM("battle_boss");
			battleScene->SetOnVictoryCallback([this]() {
				this->isFinalBossDead = true;
			});
			battleScene->SetCustomBackgroundDraw([this](DX9GF::GraphicsDevice* gd, unsigned long long delta) { DrawBackground(gd, delta); });
			auto sceMan = this->game->GetSceneManager();
			sceMan->InsertScene(sceMan->GetIndex() + 1, battleScene);

			commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void()> markFinished) {
				this->isGamePaused = true; markFinished();
			}));

			auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, true);
			drawBuffer->StackCommand(transitionInCommand);

			commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([sceMan, transitionInCommand, this](std::function<void()> markFinished) {
				if (!transitionInCommand->IsFinished()) return;
				sceMan->GoToNext(); markFinished();
			}));

			drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));

			drawBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void()> markFinished) {
				this->isGamePaused = false;
				markFinished();
			}));
		}
	});

	map->SetAreaUpdateHandler("trigger_outro", [this](const DX9GF::Map::ObjectArea& area) {
		if (!this->isFinalBossDead) return;
		if (!player->IsWalking()) return;

		auto sceMan = this->game->GetSceneManager();
		auto app = DX9GF::Application::GetInstance();
		sceMan->InsertScene(sceMan->GetIndex() + 1,
			new OutroScene(this->game, app->GetScreenWidth(), app->GetScreenHeight())
		);
		isTransitioning = true;
		auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, true);
		drawBuffer->PushCommand(transitionInCommand);
		commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this, transitionInCommand, sceMan](std::function<void(void)> markFinished) {
			if (!transitionInCommand->IsFinished()) {
				return;
			}
			this->isGamePaused = true;
			auto audio = DX9GF::AudioManager::GetInstance();
			audio->StopAll();
			sceMan->GoToNext();
			markFinished();
		}));
		drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
	});

	map->SetAreaUpdateHandler("trigger_spec_item", [this](const DX9GF::Map::ObjectArea& area) {
		if (!this->hasGottenUselessItem) {
			player->GetInventoryItems().AddItem(11, 1);
			this->hasGottenUselessItem = true;
		}
	});

	auto audio = DX9GF::AudioManager::GetInstance();
	audio->Load("hack_fail", IDR_TERMINAL_DENIED);
	audio->Load("hack_success", IDR_TERMINAL_GRANTED);

	gateTexture = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	gateTexture->LoadTexture(L"assets/gate.png");
	gateSprite = std::make_shared<DX9GF::StaticSprite>(gateTexture.get());
	gateSprite->SetPosition(46 * 16, -15 * 16);

	player->SetBaseSurface("vinyl");

	transformManager->RebuildHierarchy();
	this->GiveTestItems();
	drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
}

void Demo::BossWorldScene::OnTerminalHacked(int terminalID) {
	if (isBossDoorUnlocked) return;

	auto audio = DX9GF::AudioManager::GetInstance();

	std::vector<std::string> successMsgs = {
		"Node override successful...",
		"Firewall breached. Proceeding...",
		"Data stream aligned.",
		"Connection established. Keep going.",
		"System bypass at " + std::to_string((currentHackStep + 1) * 25) + "%"
	};
	std::vector<std::string> failMsgs = {
		"Sequence mismatch. Resetting...",
		"Protocol error. Connection dropped.",
		"Security alert. Locks reset.",
		"Corruption detected. Rebooting sequence...",
		"Fatal exception: Wrong node."
	};

	if (terminalID == 99) {
		if (currentHackStep >= 4) {
			isBossDoorUnlocked = true;
			mainTerminal->SetHackedStatus(true);
			mainTerminal->ShowStatusMessage("Access granted. Core unlocked.", 3.0f);

			auto result = QuestManager::GetInstance()->NotifyEvent("TERMINAL_HACKED", "4", this->player.get());
			if (result.hasReward && this->popUpMessage) {
				this->popUpMessage->ShowMessage(L"(+) " + result.rewardMessage, 5.0f);
			}

			audio->Play("hack_success");
			if (bossGateCollider) {
				colliderManager->Remove(bossGateCollider);
				bossGateCollider.reset();
			}
		}
		else {
			int remaining = 4 - currentHackStep;
			std::vector<std::string> lockedMsgs = {
				"Access denied. " + std::to_string(remaining) + " locks active.",
				"Sys.err: " + std::to_string(remaining) + " nodes offline.",
				"Core locked. Requires " + std::to_string(remaining) + " more overrides.",
				"WARNING: incomplete sequence (" + std::to_string(remaining) + " left)"
			};
			mainTerminal->ShowStatusMessage(lockedMsgs[rand() % lockedMsgs.size()], 3.0f);
			audio->Play("hack_fail");
		}
		return;
	}

	if (terminalID == currentHackStep + 1) {
		hackMachines[currentHackStep]->SetHackedStatus(true);
		std::string msg = successMsgs[rand() % successMsgs.size()];
		hackMachines[currentHackStep]->ShowStatusMessage(msg, 3.0f);
		currentHackStep++;
		audio->Play("hack_success");
		auto result = QuestManager::GetInstance()->NotifyEvent("TERMINAL_HACKED", std::to_string(currentHackStep), this->player.get());
		if (result.hasReward && this->popUpMessage) {
			this->popUpMessage->ShowMessage(L"(+) " + result.rewardMessage, 5.0f);
		}
	}
	else {
		currentHackStep = 0;
		for (auto& m : hackMachines) {
			m->SetHackedStatus(false);
		}
		std::string failMsg = failMsgs[rand() % failMsgs.size()];
		for (auto& m : hackMachines) {
			if (m->GetColorID() == terminalID) {
				m->ShowStatusMessage(failMsg, 3.0f);
			}
		}
		audio->Play("hack_fail");
		QuestManager::GetInstance()->NotifyEvent("TERMINAL_HACKED", "0", this->player.get());
	}
}

void Demo::BossWorldScene::OnUpdate(unsigned long long deltaTime)
{
	if (keyeproNPC) {
		keyeproNPC->Update(deltaTime);
		if (!currentConversation && keyeproNPC->CanInteract() && DX9GF::InputManager::GetInstance()->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
			StartKeyeproConversation();
		}
	}

	if (rustyChest && !rustyChest->GetIsOpened()) {
		rustyChest->Update(deltaTime);
		if (!currentConversation && rustyChest->CanInteract() && DX9GF::InputManager::GetInstance()->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
			auto [sw, sh] = camera.GetScreenResolution();
			currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);

			auto dialogBuilder = std::make_shared<NPC>(transformManager, 0, 0, NPCConfig{});

			if (player->GetInventoryItems().HasItem(10)) {
				dialogBuilder->AddLine(L"Rusty Chest", L"Wait, is that the key? NOOO! YOU ROB ME!!!");
				player->GetInventoryItems().ConsumeItem(10);
				player->GetInventoryItems().AddItem(6, 1);
				player->GetInventoryItems().AddItem(7, 1);
				player->GetInventoryItems().AddItem(8, 1);
				rustyChest->SetOpened(true);
			}
			else {
				dialogBuilder->AddLine(L"Rusty Chest", L"Don't touch me. You ain't got the key!");
			}

			for (auto& line : dialogBuilder->GetDialogueLines()) {
				currentConversation->AddLine(line);
			}
		}
	}

	if (!isGamePaused) {
		for (auto& m : hackMachines) m->Update(deltaTime);
		mainTerminal->Update(deltaTime);
	}
}

void Demo::BossWorldScene::OnDrawWorld(std::vector<DepthNode>& depthNodes, unsigned long long deltaTime)
{
	if (!isBossDoorUnlocked) {
		AddDepthNode(depthNodes, 0, [this, deltaTime]() {
			gateSprite->Begin();
			gateSprite->Draw(camera, deltaTime);
			gateSprite->End();
		});
	}

	for (auto& m : hackMachines) {
		AddDepthNode(depthNodes, m->GetWorldY(), [&, m]() { m->Draw(camera, deltaTime); });
	}
	if (mainTerminal) AddDepthNode(depthNodes, mainTerminal->GetWorldY(), [&]() { mainTerminal->Draw(camera, deltaTime); });
	if (keyeproNPC) AddDepthNode(depthNodes, keyeproNPC->GetWorldY(), [&]() { keyeproNPC->Draw(camera, deltaTime); });
	if (rustyChest) AddDepthNode(depthNodes, rustyChest->GetWorldY(), [&]() { rustyChest->Draw(camera, deltaTime); });
}

void Demo::BossWorldScene::OnDrawUI(unsigned long long deltaTime)
{
	if (rustyChest) rustyChest->DrawUI(&this->uiCamera, deltaTime);
	for (auto& m : hackMachines) m->DrawUI(&this->uiCamera, deltaTime);
	mainTerminal->DrawUI(&this->uiCamera, deltaTime);
	if (keyeproNPC && keyeproNPC->GetPhase() != Demo::KeyeproNPC::Phase::Defeated) keyeproNPC->DrawUI(&this->uiCamera, deltaTime);
}

std::string Demo::BossWorldScene::GetSaveID() const {
	return "BossWorldScene";
}

void Demo::BossWorldScene::StartKeyeproConversation()
{
	auto [sw, sh] = camera.GetScreenResolution();
	currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
	for (auto& line : keyeproNPC->GetDialogueLines()) {
		currentConversation->AddLine(line);
	}

	if (keyeproNPC->GetPhase() == KeyeproNPC::Phase::Waiting) {
		onConversationEnd = [this]() { StartKeyeproBattle(); };
	}
}

void Demo::BossWorldScene::StartKeyeproBattle()
{
	if (isTransitioning || isFinalBossDead) return;
	isTransitioning = true;

	std::map<std::string, int> forcedEnemyMap = { {"KeyeproEnemy", 100} };

	auto demoGame = dynamic_cast<Demo::Game*>(this->game);
	auto app = DX9GF::Application::GetInstance();
	auto battleScene = new CustomBattleScene(demoGame, player, app->GetScreenWidth(), app->GetScreenHeight(), forcedEnemyMap);

	battleScene->SetCustomBGM("battle_boss");
	battleScene->SetOnVictoryCallback([this]() {
		this->isFinalBossDead = true;
		if (keyeproNPC) keyeproNPC->SetPhase(KeyeproNPC::Phase::Defeated);
	});
	battleScene->SetCustomBackgroundDraw([this](DX9GF::GraphicsDevice* gd, unsigned long long delta) { DrawBackground(gd, delta); });

	auto sceMan = this->game->GetSceneManager();
	sceMan->InsertScene(sceMan->GetIndex() + 1, battleScene);

	commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void()> markFinished) {
		this->isGamePaused = true; markFinished();
	}));

	auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, true);
	drawBuffer->StackCommand(transitionInCommand);

	commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([sceMan, transitionInCommand, this](std::function<void()> markFinished) {
		if (!transitionInCommand->IsFinished()) return;
		sceMan->GoToNext(); markFinished();
	}));

	drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));

	drawBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void()> markFinished) {
		this->isGamePaused = false;
		this->isTransitioning = false;
		markFinished();
	}));
}

void Demo::BossWorldScene::OnGenerateSaveData(nlohmann::json& outData) {
	outData["currentIslandID"] = currentIslandID;
	outData["puzzle"] = {
		{"currentHackStep", currentHackStep},
		{"isBossDoorUnlocked", isBossDoorUnlocked},
		{"hasGottenUselessItem", hasGottenUselessItem},
		{"isFinalBossDead", isFinalBossDead},
		{"isChestOpened", rustyChest ? rustyChest->GetIsOpened() : false}
	};

	if (keyeproNPC) outData["keyeproNPC"] = { {"phase", static_cast<int>(keyeproNPC->GetPhase())} };
}

void Demo::BossWorldScene::OnRestoreSaveData(const nlohmann::json& inData) {
	currentIslandID = inData.value("currentIslandID", 1);

	if (inData.contains("puzzle")) {
		currentHackStep = inData["puzzle"]["currentHackStep"];
		isBossDoorUnlocked = inData["puzzle"]["isBossDoorUnlocked"];
		hasGottenUselessItem = inData["puzzle"]["hasGottenUselessItem"];
		isFinalBossDead = inData.value("puzzle", nlohmann::json::object()).value("isFinalBossDead", false);
		if (rustyChest) rustyChest->SetOpened(inData.value("puzzle", nlohmann::json::object()).value("isChestOpened", false));
	}

	QuestManager::GetInstance()->NotifyEvent("TERMINAL_HACKED", std::to_string(currentHackStep), player.get());
	if (isBossDoorUnlocked) {
		mainTerminal->SetHackedStatus(true);
		if (bossGateCollider) {
			colliderManager->Remove(bossGateCollider);
			bossGateCollider.reset();
		}
	}

	if (inData.contains("keyeproNPC") && keyeproNPC) {
		keyeproNPC->SetPhase(static_cast<KeyeproNPC::Phase>(inData["keyeproNPC"].value("phase", 0)));
	}

	for (int i = 0; i < currentHackStep; ++i) {
		if (i < static_cast<int>(hackMachines.size())) {
			hackMachines[i]->SetHackedStatus(true);
		}
	}
}

void Demo::BossWorldScene::DrawBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
{
	auto [screenWidth, screenHeight] = camera.GetScreenResolution();
	D3DCOLOR bgColor = 0xFF05101a;
	if (currentIslandID == 2) bgColor = 0xFF051105;
	else if (currentIslandID == 3) bgColor = 0xFF1c0d02;
	else if (currentIslandID == 4) bgColor = 0xFF1a0505;

	gd->DrawRectangle(0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), bgColor, true);
	static float timeAcc = 0.0f;
	timeAcc += static_cast<float>(deltaTime) * 0.001f;

	switch (currentIslandID) {
	case 2: {
		const D3DCOLOR waveColor = 0x3300E5FF;
		const D3DCOLOR nodeColor = 0xAA00E5FF;
		float centerY = screenHeight / 2.0f;
		float prevX = 0.0f;
		float prevY1 = centerY;
		float prevY2 = centerY;
		for (int x = 0; x <= screenWidth; x += 20) {
			float angle1 = (x * 0.005f) + timeAcc * 2.0f;
			float y1 = centerY + std::sinf(angle1) * (screenHeight * 0.25f);
			float angle2 = (x * 0.005f) - timeAcc * 1.5f + 3.14159f / 2.0f;
			float y2 = centerY + std::cosf(angle2) * (screenHeight * 0.20f);
			if (x > 0) {
				gd->DrawLine(prevX, prevY1, static_cast<float>(x), y1, waveColor);
				gd->DrawLine(prevX, prevY2, static_cast<float>(x), y2, waveColor);
			}
			if (x % 60 == 0) {
				gd->DrawLine(static_cast<float>(x), y1, static_cast<float>(x), y2, 0x1100E5FF);
				float pulse = (std::sinf(timeAcc * 4.0f + x) + 1.0f) * 0.5f;
				D3DCOLOR dynamicNodeColor = (static_cast<DWORD>(pulse * 155 + 100) << 24) | 0x0000E5FF;
				gd->DrawLine(static_cast<float>(x) - 3, y1, static_cast<float>(x) + 3, y1, dynamicNodeColor);
				gd->DrawLine(static_cast<float>(x), y1 - 3, static_cast<float>(x), y1 + 3, dynamicNodeColor);
				gd->DrawLine(static_cast<float>(x) - 3, y2, static_cast<float>(x) + 3, y2, dynamicNodeColor);
				gd->DrawLine(static_cast<float>(x), y2 - 3, static_cast<float>(x), y2 + 3, dynamicNodeColor);
			}
			prevX = static_cast<float>(x);
			prevY1 = y1;
			prevY2 = y2;
		}
		break;
	}
	case 1: {
		const D3DCOLOR rainColor = 0xFF00FF41;
		const D3DCOLOR tailColor = 0x4400FF41;
		for (int x = 0; x < screenWidth; x += 48) {
			float speed = 80.f + (x % 5) * 40.f;
			float yOffset = std::fmod(timeAcc * speed + (x * 17.f), static_cast<float>(screenHeight + 100));
			gd->DrawLine(static_cast<float>(x), yOffset, static_cast<float>(x), yOffset + 30.f, rainColor);
			gd->DrawLine(static_cast<float>(x), yOffset - 50.f, static_cast<float>(x), yOffset, tailColor);
		}
		break;
	}
	case 3: {
		const D3DCOLOR gridColor = 0x11FF8C00;
		const D3DCOLOR streakColor = 0xAAFF8C00;
		const D3DCOLOR brightColor = 0xFFFFD700;
		for (int i = 0; i < screenHeight; i += 40) gd->DrawLine(0, static_cast<float>(i), static_cast<float>(screenWidth), static_cast<float>(i), gridColor);
		for (int i = 0; i < screenWidth; i += 40) gd->DrawLine(static_cast<float>(i), 0, static_cast<float>(i), static_cast<float>(screenHeight), gridColor);
		int numStreaks = 40;
		for (int i = 0; i < numStreaks; ++i) {
			float timeStep = std::floor(timeAcc * 15.0f) + i;
			float randX = std::fmod(std::abs(std::sinf(timeStep * 12.9898f) * 43758.5453f), 1.0f);
			float randY = std::fmod(std::abs(std::sinf(timeStep * 78.233f) * 43758.5453f), 1.0f);
			float randLen = std::fmod(std::abs(std::sinf(timeStep * 34.123f) * 43758.5453f), 1.0f);
			if (std::fmod(std::abs(std::sinf(timeStep * i)), 1.0f) > 0.4f) continue;
			bool isHorizontal = (i % 2 == 0);
			float length = 40.0f + randLen * 200.0f;
			if (isHorizontal) {
				float y = std::floor(randY * screenHeight / 40.0f) * 40.0f;
				float x = randX * screenWidth;
				gd->DrawLine(x, y, x + length, y, brightColor);
				gd->DrawLine(x, y - 1, x + length, y - 1, streakColor);
			}
			else {
				float x = std::floor(randX * screenWidth / 40.0f) * 40.0f;
				float y = randY * screenHeight;
				gd->DrawLine(x, y, x, y + length, brightColor);
				gd->DrawLine(x - 1, y, x - 1, y + length, streakColor);
			}
		}
		break;
	}
	case 4: {
		const D3DCOLOR polyColor = 0x22DC143C;
		const D3DCOLOR crackColor = 0x66DC143C;
		for (int i = 0; i < 15; ++i) {
			float size = 100.0f + (i % 3) * 50.0f;
			float bx = std::fmod((i * 123.0f) + timeAcc * 10.0f, static_cast<float>(screenWidth + size)) - size / 2.0f;
			float by = std::fmod((i * 456.0f) + timeAcc * 5.0f, static_cast<float>(screenHeight + size)) - size / 2.0f;
			float angle = timeAcc * 0.1f + i;
			float glitchSize = size + std::sinf(timeAcc * 2.0f + i) * 5.0f;
			float prevX = bx + std::cosf(angle) * glitchSize;
			float prevY = by + std::sinf(angle) * glitchSize;
			for (int v = 1; v <= 5; ++v) {
				float vAngle = angle + (v * 72.0f * 3.14159f / 180.0f);
				float vx = bx + std::cosf(vAngle) * glitchSize;
				float vy = by + std::sinf(vAngle) * glitchSize;
				gd->DrawLine(prevX, prevY, vx, vy, polyColor);
				prevX = vx;
				prevY = vy;
			}
			if (static_cast<int>(timeAcc * 2.0f + i) % 7 == 0) {
				float crackX = bx + (rand() % static_cast<int>(size)) - size / 2.0f;
				float crackY = by + (rand() % static_cast<int>(size)) - size / 2.0f;
				gd->DrawLine(crackX - 20, crackY - 20, crackX + 20, crackY + 20, crackColor);
			}
		}
		break;
	}
	default: break;
	}
}

void Demo::BossWorldScene::GiveTestItems()
{
}
