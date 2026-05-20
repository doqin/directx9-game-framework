#include "pch.h"
#include "BossWorldScene.h"
#include "RandomEncounter.h"
#include "CardShop.h"
#include "ItemShop.h"
#include "GameItems.h"
#include "CustomBattleScene.h"
#include "TransitionCommand.h"
#include "Game.h"
#include <vector>
#include <cmath>

void Demo::BossWorldScene::Init() {
	camera.SetZoom(2.0f);
	transformManager = std::make_shared<DX9GF::TransformManager>();
	colliderManager = std::make_shared<DX9GF::ColliderManager>();

	player = std::make_shared<Player>(transformManager, 360.f, 190.f);
	camera.SetPosition(360.f, 190.f);

	player->Init(game->GetGraphicsDevice(), colliderManager.get(), &camera);

	drawBuffer = std::make_shared<DX9GF::CommandBuffer>();
	commandBuffer = std::make_shared<DX9GF::CommandBuffer>();
	font = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 16);

	map = std::make_shared<DX9GF::Map>(game->GetGraphicsDevice());
	map->Create(transformManager, colliderManager, "./BossMatrix.tmx");

	map->SetAreaUpdateHandler("trigger_encounters", GetRandomEncounterFunc(game, player, {
		{"VampireBatEnemy", 40},
		{"WarlockEnemy", 30},
		{"CupidEnemy", 20}
		//i removed the mimic here
		}, drawBuffer, commandBuffer, &isGamePaused, [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) { DrawBackground(gd, deltaTime, currentIslandID); }));

	//dialogue with NPC, rambles about lore regarding the optional battle to get the key, and hints at the correct color sequence for hacking the terminal
	npcHint = std::make_shared<DauDauNPC>(transformManager, -950.0f, -220.0f);
	npcHint->Init(game->GetGraphicsDevice(), player, colliderManager, font, drawBuffer);

	npcHint->AddLine(L"Veteran Debugger", L"Halt, traveler. You shouldn't be here.");
	npcHint->AddLine(L"Veteran Debugger", L"This sector is deeply corrupted-a graveyard of unresolved bugs and dead code.");
	npcHint->AddLine(L"Player", L"What exactly happened here?");
	npcHint->AddLine(L"Veteran Debugger", L"What happened? Ambition met reality.");
	npcHint->AddLine(L"Veteran Debugger", L"Teams of debuggers rushed in, thinking they could fix the core.");
	npcHint->AddLine(L"Veteran Debugger", L"They couldn't agree on a protocol. The system panicked, spawned massive anomalies, \nand wiped them out.");

	npcHint->AddLine(L"Veteran Debugger", L"I keep hearing the old admin's logs echoing in my head...");
	npcHint->AddLine(L"Veteran Debugger", L"...'The red sun sets over the blue ocean, giving life to the green earth, until it fades into\norange autumn'...");
	npcHint->AddLine(L"Veteran Debugger", L"Bah, probably just corrupted junk data. Don't mind my rambling.");

	npcHint->AddLine(L"Veteran Debugger", L"Listen carefully. There is a heavily glitched battlefield up ahead.");
	npcHint->AddLine(L"Veteran Debugger", L"You don't need to clear it to proceed. Bypassing that mess won't affect your journey at all.");
	npcHint->AddLine(L"Veteran Debugger", L"I strongly advise you to keep your head down and walk away. It's not worth it.");


	//hack machines
	auto hackCallback = std::bind(&BossWorldScene::OnTerminalHacked, this, std::placeholders::_1);

	auto machine1 = std::make_shared<HackTerminal>(transformManager, 1500, -620, 1, "M1");
	machine1->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer, hackCallback);
	hackMachines.push_back(machine1);

	auto machine2 = std::make_shared<HackTerminal>(transformManager, -480, 60, 2, "M2");
	machine2->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer, hackCallback);
	hackMachines.push_back(machine2);

	auto machine3 = std::make_shared<HackTerminal>(transformManager, -590, -200, 3, "M3");
	machine3->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer, hackCallback);
	hackMachines.push_back(machine3);

	auto machine4 = std::make_shared<HackTerminal>(transformManager, 2440, 395, 4, "M4");
	machine4->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer, hackCallback);
	hackMachines.push_back(machine4);

	mainTerminal = std::make_shared<HackTerminal>(transformManager, 780, -200, 99, "Main");
	mainTerminal->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer, hackCallback);

	for (auto& m : hackMachines) m->SetVisible(true);
	mainTerminal->SetVisible(true);

	//gate collider to block player
	bossGateCollider = std::make_shared<DX9GF::RectangleCollider>(transformManager, 32.f, 48.f, 736.f, -256.f);
	colliderManager->Add(bossGateCollider);
	//create a obj class later and pass this collider to draw the gate sprite over it. It is currently invisible but blocks players.

	//chest
	rustyChest = std::make_shared<RustyChestNPC>(transformManager, 690.f, -208.f);
	rustyChest->Init(game->GetGraphicsDevice(), player, colliderManager, font, drawBuffer);

	//heal
	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 144.f, 320.f));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, -352.f, -160.f));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);

	//save
	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 192.f, 320.f));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, -304.f, -160.f));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 704.f, -96.f));
	savePoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
	savePoints.back()->SetVisible(true);

	//shop
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

	// link with portal triggers on map
	map->SetAreaUpdateHandler("trigger_p_1_2", [this](const DX9GF::Map::ObjectArea& area) {
		player->SetLocalPosition(-1155, 0);
		//camera.SetPosition(-1155, 0);
		this->currentIslandID = 2;
		});
	map->SetAreaUpdateHandler("trigger_p_2_3", [this](const DX9GF::Map::ObjectArea& area) {
		player->SetLocalPosition(1500, 530);
		//camera.SetPosition(1500, 530);
		this->currentIslandID = 3;
		});
	map->SetAreaUpdateHandler("trigger_p_2_fake", [this](const DX9GF::Map::ObjectArea& area) {
		player->SetLocalPosition(-1155, 0);
		//camera.SetPosition(-1155, 0);
		});
	map->SetAreaUpdateHandler("trigger_p_3_4", [this](const DX9GF::Map::ObjectArea& area) {
		player->SetLocalPosition(740, -50);
		//camera.SetPosition(740, -50);
		this->currentIslandID = 4;
		});
	map->SetAreaUpdateHandler("trigger_p_3_2", [this](const DX9GF::Map::ObjectArea& area) {
		player->SetLocalPosition(-1155, 0);
		//camera.SetPosition(-1155, 0);
		this->currentIslandID = 2;
		});
	map->SetAreaUpdateHandler("trigger_p_4_3", [this](const DX9GF::Map::ObjectArea& area) {
		player->SetLocalPosition(1500, 530);
		//camera.SetPosition(1500, 530);
		this->currentIslandID = 3;
		});

	//optional battle to get key
	map->SetAreaUpdateHandler("trigger_battle_rusty_key", [this](const DX9GF::Map::ObjectArea& area) {
		if (!player->IsWalking()) return;

		bool hasRustyKey = player->GetInventoryItems().HasItem(10);
		if (!hasRustyKey && !this->isMimicDead) {

			std::map<std::string, int> forcedEnemyMap = { {"MimicEnemy", 100} };

			auto demoGame = dynamic_cast<Demo::Game*>(this->game);
			auto app = DX9GF::Application::GetInstance();

			auto battleScene = new CustomBattleScene(demoGame, player, app->GetScreenWidth(), app->GetScreenHeight(), forcedEnemyMap);

			battleScene->SetOnVictoryCallback([this]() {
				this->isMimicDead = true;
				});

			battleScene->SetCustomBackgroundDraw([](DX9GF::GraphicsDevice*, unsigned long long) {});

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

				if (this->isMimicDead) {
					this->player->GetInventoryItems().AddItem(10, 1);
				}

				markFinished();
				}));
		}
		});

	map->SetAreaUpdateHandler("trigger_battle_boss", [this](const DX9GF::Map::ObjectArea& area) {
		if (!player->IsWalking()) return;

		if (this->isBossDoorUnlocked && !this->isFinalBossDead) {

			std::map<std::string, int> forcedEnemyMap = { {"KeyeproEnemy", 100}, {"KeyeEnemy", 100} };

			auto demoGame = dynamic_cast<Demo::Game*>(this->game);
			auto app = DX9GF::Application::GetInstance();
			auto battleScene = new CustomBattleScene(demoGame, player, app->GetScreenWidth(), app->GetScreenHeight(), forcedEnemyMap);

			battleScene->SetOnVictoryCallback([this]() {
				this->isFinalBossDead = true;
			});

			battleScene->SetCustomBackgroundDraw([this](DX9GF::GraphicsDevice* gd, unsigned long long delta) { DrawBackground(gd, delta, currentIslandID); });
			auto sceMan = this->game->GetSceneManager();
			sceMan->InsertScene(sceMan->GetIndex() + 1, battleScene);

			commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void()> markFinished) {
				this->isGamePaused = true; markFinished();
				}));

			auto transitionInCommand = std::make_shared<TransitionCommand>(this->game->GetGraphicsDevice(), 1.f, true);
			drawBuffer->StackCommand(transitionInCommand);

			commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([sceMan, transitionInCommand, this](std::function<void()> markFinished) {
				if (!transitionInCommand->IsFinished()) return;
				sceMan->GoToNext(); markFinished();
				}));

			drawBuffer->PushCommand(std::make_shared<TransitionCommand>(this->game->GetGraphicsDevice(), 1.f, false));

			drawBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void()> markFinished) {
				this->isGamePaused = false;

				if (this->isFinalBossDead) {
					//cutscene, scene EndCredits, .etc.
				}
				markFinished();
				}));
		}
		});


	map->SetAreaUpdateHandler("trigger_spec_item", [this](const DX9GF::Map::ObjectArea& area) {
		if (!this->hasGottenUselessItem) {
			player->GetInventoryItems().AddItem(11, 1);
			this->hasGottenUselessItem = true;
		}
		});

	draggableManager = std::make_shared<Demo::DraggableManager>();
	inventoryMenu = std::make_shared<InventoryMenu>(game, player, transformManager, draggableManager, &uiCamera, font.get());
	inventoryMenu->Init();

	transformManager->RebuildHierarchy();
	drawBuffer->PushCommand(std::make_shared<Demo::TransitionCommand>(game->GetGraphicsDevice(), 1.f, false));
}

void Demo::BossWorldScene::OnTerminalHacked(int terminalID) {

	//srand(static_cast<unsigned int>(time(NULL))); ->> help me add this somewhere so that the random results are different every time the game runs
	if (isBossDoorUnlocked) return;

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
		}
		return;
	}


	if (terminalID == currentHackStep + 1) {
		hackMachines[currentHackStep]->SetHackedStatus(true);
		std::string msg = successMsgs[rand() % successMsgs.size()];
		hackMachines[currentHackStep]->ShowStatusMessage(msg, 3.0f);
		currentHackStep++;
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
	}
}

void Demo::BossWorldScene::Update(unsigned long long deltaTime) {
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

	if (npcHint) {
		npcHint->Update(deltaTime);
		if (!currentConversation && npcHint->CanInteract() && inpMan->KeyPress(DIK_E)) {
			auto [sw, sh] = camera.GetScreenResolution();
			currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
			for (auto& line : npcHint->GetDialogueLines()) {
				currentConversation->AddLine(line);
			}
		}
	}
	if (currentConversation) {
		isGamePaused = true;
		currentConversation->Execute(deltaTime);
		if (currentConversation->IsFinished()) currentConversation = nullptr;
	}
	if (rustyChest && !rustyChest->GetIsOpened()) {
		rustyChest->Update(deltaTime);

		if (!currentConversation && rustyChest->CanInteract() && inpMan->KeyPress(DIK_E)) {
			auto [sw, sh] = camera.GetScreenResolution();
			currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);

			auto dialogBuilder = std::make_shared<DauDauNPC>(transformManager, 0, 0);

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

	for (auto& savePoint : savePoints) {
		savePoint->Update(deltaTime);
		if (savePoint->IsMenuOpen()) isGamePaused = true;
	}
	for (auto& shopPoint : shopPoints) shopPoint->Update(deltaTime);
	for (auto& healPoint : healingPoints) healPoint->Update(deltaTime);

	if (inventoryMenu && inventoryMenu->IsOpen()) {
		isGamePaused = true;
		inventoryMenu->Update(deltaTime);
	}

	if (!isGamePaused) {
		for (auto& m : hackMachines) m->Update(deltaTime);
		mainTerminal->Update(deltaTime);

		player->Update(deltaTime);
		camera.Update();
	}

	transformManager->UpdateAll();

	if (draggableManager && inventoryMenu && inventoryMenu->IsOpen() && inventoryMenu->GetCurrentTab() == Demo::InventoryMenu::Tab::DECK) {
		draggableManager->Update(deltaTime);
	}

	if (inventoryMenu && inventoryMenu->IsPendingLeave()) {
		auto sceMan = game->GetSceneManager();
		sceMan->GoToScene(0);
		return;
	}
	commandBuffer->Update(deltaTime);
}

void Demo::BossWorldScene::Draw(unsigned long long deltaTime) {
	auto gd = game->GetGraphicsDevice();

	D3DCOLOR bgColor = 0xFF05101a;
	if (currentIslandID == 2) bgColor = 0xFF051105;
	else if (currentIslandID == 3) bgColor = 0xFF1c0d02;
	else if (currentIslandID == 4) bgColor = 0xFF1a0505;

	gd->Clear(bgColor);
	if (SUCCEEDED(gd->BeginDraw())) {
		DrawBackground(gd, deltaTime, currentIslandID);
		map->Draw(camera);

		for (auto& m : hackMachines) m->Draw(camera, deltaTime);
		mainTerminal->Draw(camera, deltaTime);

		if (npcHint) npcHint->Draw(camera, deltaTime);
		if (rustyChest) rustyChest->Draw(camera, deltaTime);

		for (auto& savePoint : savePoints) savePoint->Draw(camera, deltaTime);
		for (auto& shopPoint : shopPoints) shopPoint->Draw(camera, deltaTime);
		for (auto& healPoint : healingPoints) healPoint->Draw(camera, deltaTime);

		player->Draw(deltaTime);

		if (drawBuffer) drawBuffer->Update(deltaTime);
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

std::string Demo::BossWorldScene::GetSaveID() const {
	return "BossWorldScene";
}

void Demo::BossWorldScene::GenerateSaveData(nlohmann::json& outData) {
	player->GenerateSaveData(outData["player"]);
	auto pos = camera.GetPosition();
	outData["camera"] = { {"x", pos.x}, {"y", pos.y}, {"zoom", camera.GetZoom()} };
	outData["currentIslandID"] = currentIslandID;
	outData["puzzle"] = {
		{"currentHackStep", currentHackStep},
		{"isBossDoorUnlocked", isBossDoorUnlocked},
		{"hasGottenUselessItem", hasGottenUselessItem},
		{"isMimicDead", isMimicDead},
		{"isFinalBossDead", isFinalBossDead},
		{"isChestOpened", rustyChest ? rustyChest->GetIsOpened() : false}
	};
}

void Demo::BossWorldScene::RestoreSaveData(const nlohmann::json& inData) {
	player->RestoreSaveData(inData["player"]);
	camera.SetPosition(inData["camera"]["x"], inData["camera"]["y"]);
	camera.SetZoom(inData["camera"]["zoom"]);
	currentIslandID = inData.value("currentIslandID", 1);

	if (inData.contains("puzzle")) {
		currentHackStep = inData["puzzle"]["currentHackStep"];
		isBossDoorUnlocked = inData["puzzle"]["isBossDoorUnlocked"];
		hasGottenUselessItem = inData["puzzle"]["hasGottenUselessItem"];
		isMimicDead = inData.value("puzzle", nlohmann::json::object()).value("isMimicDead", false);
		isFinalBossDead = inData.value("puzzle", nlohmann::json::object()).value("isFinalBossDead", false);
		if (rustyChest) rustyChest->SetOpened(inData.value("puzzle", nlohmann::json::object()).value("isChestOpened", false));
	}

	if (isBossDoorUnlocked) {
		mainTerminal->SetHackedStatus(true);
		if (bossGateCollider) {
			colliderManager->Remove(bossGateCollider);
			bossGateCollider.reset();
		}
	}

	for (int i = 0; i < currentHackStep; ++i) {
		if (i < hackMachines.size()) {
			hackMachines[i]->SetHackedStatus(true);
		}
	}
}

void Demo::BossWorldScene::DrawBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime, int islandID)
{
	auto [screenWidth, screenHeight] = camera.GetScreenResolution();

	//shared static timer variable for background processes
	static float timeAcc = 0.0f;
	timeAcc += static_cast<float>(deltaTime) * 0.001f;

	switch (islandID) {
		//quantum lattice
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
		  //light rain
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
		  //chaos grid
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
		  //float polygons
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