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
#include "CardShop.h"
#include "ItemShop.h"
#include "imgui.h"
#include "backends/imgui_impl_dx9.h"

void Demo::SecretPuzzleScene::OnInit()
{
	InitCore(-84 * 16, -39 * 16, L"./assets/SecretPuzzle.tmx");

	SetChapterTitle(L"ANOMALY DETECTED: THE ROOT", L"< Encrypted Database >");
	map->SetAreaUpdateHandler("trigger_p_back", [this](const DX9GF::Map::ObjectArea& area) {
		CreatePortalTransition(-2, -263.f, -295.f, "bgm_tutorial", 0.5f);
	});

	map->SetAreaUpdateHandler("trigger_p_next_world", [this](const DX9GF::Map::ObjectArea& area) {
		CreatePortalTransition(1, -417.f, 144.f, "bgm_arcade", 0.2f);
	});

	map->SetAreaUpdateHandler("trigger_p_next", [this](const DX9GF::Map::ObjectArea& area) {
		player->SetLocalPosition(31 * 16, 36 * 16);
	});

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

	auto addChest = [&](float tx, float ty, std::vector<ChestReward> rewards, bool randomPick = false) {
		auto c = std::make_shared<TreasureChestNPC>(transformManager, tx * 16, ty * 16, rewards, randomPick);
		c->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
		treasureChests.push_back(c);
	};
	addChest(-32, 26, { ChestReward::Item(0,1), ChestReward::Item(1,1), ChestReward::Card("PoisonCard") }, true);
	addChest(-11, -31, { ChestReward::Item(2,1), ChestReward::Item(3,1), ChestReward::Card("CleaveCard") }, true);
	addChest(80, 48, { ChestReward::Item(4,1), ChestReward::Item(5,1), ChestReward::Card("TwinStrikeCard") }, true);

	auto bgDraw = [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) {
		DrawBackground(gd, deltaTime);
	};

	auto spawnSecret = [&](float x, float y, std::string id, std::vector<std::string> types, bool isRand, bool isGlobal) {
		std::string bgm = (id == "sec_miniboss_01") ? "bgm_boss" : "battle_loop";

		auto enemy = EnemyFactory::CreateMapEnemy(
			x, y, id, types, isRand, isGlobal, bgm, bgDraw,
			transformManager, game, colliderManager.get(), player
		);

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

	spawnSecret(-1140.f, -400.f, "sec_bat_01", { "VampireBatEnemy" }, false, false);
	spawnSecret(-910.f, 80.f, "sec_eye_01", { "DemonEyeEnemy" }, false, false);
	spawnSecret(-780.f, 470.f, "sec_rand_bat_eye_01", { "VampireBatEnemy", "DemonEyeEnemy" }, true, false);
	spawnSecret(-480.f, 450.f, "sec_rand_keye_bat_01", { "KeyeEnemy", "VampireBatEnemy" }, true, false);
	spawnSecret(-330.f, 160.f, "sec_duo_bat_01", { "VampireBatEnemy", "KernelEnemy" }, false, false);
	spawnSecret(-440.f, -80.f, "sec_mimic_trap_01", { "MimicEnemy" }, false, false);
	spawnSecret(-300.f, -450.f, "sec_rand_3types_01", { "DemonEyeEnemy", "KernelEnemy", "MimicEnemy" }, true, false);
	spawnSecret(-60.f, -460.f, "sec_bat_02", { "VampireBatEnemy" }, false, false);
	spawnSecret(90.f, -340.f, "sec_rand_eye_bat_01", { "DemonEyeEnemy", "VampireBatEnemy" }, true, false);
	spawnSecret(765.f, 680.f, "sec_keye_01", { "KernelEnemy" }, false, false);
	spawnSecret(880.f, 730.f, "sec_rand_bat_mimic_01", { "VampireBatEnemy", "MimicEnemy" }, true, false);
	spawnSecret(738.f, 839.f, "sec_duo_eye_01", { "DemonEyeEnemy", "DemonEyeEnemy" }, false, false);
	spawnSecret(1135.f, 930.f, "sec_rand_keye_mimic_01", { "KeyeEnemy", "MimicEnemy" }, true, false);
	spawnSecret(1195.f, 780.f, "sec_bat_03", { "VampireBatEnemy" }, false, false);
	spawnSecret(1350.f, 785.f, "sec_rand_eye_bat_02", { "DemonEyeEnemy", "VampireBatEnemy" }, true, false);

	player->SetBaseSurface("dirt");

	transformManager->RebuildHierarchy();
	this->GiveTestItems();
	drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));

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

void Demo::SecretPuzzleScene::OnUpdate(unsigned long long deltaTime)
{
	if (cupidNPC) {
		cupidNPC->Update(deltaTime);
		if (!currentConversation && cupidNPC->CanInteract() && DX9GF::InputManager::GetInstance()->KeyPress(SettingsManager::GetInstance()->GetKeybind("INTERACT"))) {
			StartCupidConversation();
		}
	}
}

void Demo::SecretPuzzleScene::OnDrawWorld(std::vector<DepthNode>& depthNodes, unsigned long long deltaTime)
{
	if (cupidNPC) AddDepthNode(depthNodes, cupidNPC->GetWorldY(), [&]() { cupidNPC->Draw(camera, deltaTime); });
}

void Demo::SecretPuzzleScene::OnDrawUI(unsigned long long deltaTime)
{
	if (cupidNPC && cupidNPC->GetPhase() != Demo::CupidNPC::Phase::Defeated) cupidNPC->DrawUI(&this->uiCamera, deltaTime);
}

std::string Demo::SecretPuzzleScene::GetSaveID() const
{
	return "SecretPuzzleScene";
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
		this->commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void()> markFinished) {
			this->isBossDead = true;
			if (this->cupidNPC) this->cupidNPC->SetPhase(CupidNPC::Phase::Defeated);

			auto result = QuestManager::GetInstance()->NotifyEvent("ENTITY_DEAD", "SecretBoss_Pacman", this->player.get());

			if (result.hasReward && this->popUpMessage) {
				this->popUpMessage->ShowMessage(L"(+) " + result.rewardMessage, 5.0f);
			}

			markFinished();
		}));
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
		this->isTransitioning = false;
		markFinished();
	}));
}

void Demo::SecretPuzzleScene::OnGenerateSaveData(nlohmann::json& outData)
{
	outData["puzzle"] = {
		{"isBossDead", isBossDead},
		{"questGiven", questGiven }
	};
	if (cupidNPC) outData["cupidNPC"] = { {"phase", static_cast<int>(cupidNPC->GetPhase())} };
}

void Demo::SecretPuzzleScene::OnRestoreSaveData(const nlohmann::json& inData)
{
	if (inData.contains("puzzle") && inData["puzzle"].contains("isBossDead")) {
		isBossDead = inData["puzzle"]["isBossDead"];
		questGiven = inData["puzzle"].value("questGiven", false);
		questRestoredFromSave = true;
	}

	if (inData.contains("cupidNPC") && cupidNPC) {
		cupidNPC->SetPhase(static_cast<CupidNPC::Phase>(inData["cupidNPC"].value("phase", 0)));
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

void Demo::SecretPuzzleScene::GiveTestItems()
{
	//ItemInventory& testItems = this->player->GetInventoryItems();
	//testItems.InitFixedInventory(10);
}
