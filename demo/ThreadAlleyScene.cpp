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
#include "Debug.h"
#include "imgui.h"
#include "backends/imgui_impl_dx9.h"

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

	popUpMessage = std::make_shared<Demo::PopUpMessage>(transformManager, game);
	popUpMessage->SetLocalPosition(0.0f, 0.0f);
	popUpMessage->Init(game->GetGraphicsDevice(), &this->uiCamera);

	map = std::make_shared<DX9GF::Map>(game->GetGraphicsDevice());
	map->Create(transformManager, colliderManager, "./assets/ThreadAlley.tmx");
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

	chapterTitleUI = std::make_shared<ChapterTitleUI>(font);

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
	QuestManager::GetInstance()->SetVirtualResolution(game->GetVirtualWidth(), game->GetVirtualHeight());
	QuestManager::GetInstance()->Init(game->GetGraphicsDevice(), transformManager, &this->uiCamera, font);


	NPCConfig hkhangConfig = { L"assets/daudau-Sheet.png", 32, 32, 5, 12, 24.f, 8.f, 12.f }; //TODO: change HKhang npc's asset here

	auto hKhang = std::make_shared<NPC>(transformManager, -580.f, 100.f, hkhangConfig);
	hKhang->AttachQuestMarker("Quest_ThreadAlley_Start", Demo::QuestMarkerRole::Giver);
	hKhang->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	hKhang->RegisterVoice(L"Huu Khang", "bleep12");
	hKhang->SetInteractLogic([](NPC* self) -> std::function<void()> {
		auto qState = QuestManager::GetInstance()->GetQuestState("Quest_ThreadAlley_Start");

		if (qState == Demo::QuestState::Locked) {
			self->AddLine(L"Huu Khang", L"Hey, keep your firewall up if you're heading down Thread Alley.");
			self->AddLine(L"Huu Khang", L"It used to be a shortcut for background processes, but lately... things go in and don't come out.");
			self->AddLine(L"Player", L"Malware?");
			self->AddLine(L"Huu Khang", L"Worse. The sneaky kind. It mimics friendly data to bypass your antivirus.");
			self->AddLine(L"Huu Khang", L"If you're heading that way, maybe you can flush the corruption out?");
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
			self->AddLine(L"Huu Khang", L"Sometimes it looks exactly like a friend in need.");
		}
		else {
			self->AddLine(L"Huu Khang", L"You actually deleted it? And your registry is still intact?");
			self->AddLine(L"Huu Khang", L"Not bad! Thread Alley is finally safe to route data through again.");
		}
		return nullptr;
		});
	mapNPCs.push_back(hKhang);

	// Rolled fresh for a new game; RestoreSaveData overwrites it when a save is loaded, which always
	// happens after every scene has been Init()ed.
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

	/*healingPoints.push_back(std::make_shared<HealingPoint>(transformManager, 984, -839));
	healingPoints.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healingPoints.back()->SetVisible(true);*/

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
		treasureChests.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	}

	//map enemies
	auto spawn = [&](float x, float y, std::string id, std::vector<std::string> types, bool isRand, bool isGlobal) {
		auto bgDraw = [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) {
			DrawCheckerBackground(gd, deltaTime);
			};

		std::string bgm = (id == "sec_miniboss_01") ? "bgm_boss" : "battle_loop";

		auto enemy = EnemyFactory::CreateMapEnemy(
			x, y, id, types, isRand, isGlobal, bgm, bgDraw,
			transformManager, game, colliderManager.get(), player
		);

		//token spawning area
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

	spawn(-490.f, -50.f, "th_intro_01", { "KeyeEnemy" }, false, false);
	spawn(-735.f, -160.f, "th_intro_02", { "VampireBatEnemy" }, false, false);
	spawn(-210.f, -180.f, "th_intro_03", { "DemonEyeEnemy" }, false, false);
	spawn(-40.f, -300.f, "th_intro_04", { "KeyeEnemy", "KernelEnemy" }, true, false);
	spawn(-540.f, -370.f, "th_mid_01", { "VampireBatEnemy", "VampireBatEnemy" }, false, false);
	spawn(-330.f, -400.f, "th_mid_02", { "MimicEnemy" }, false, false);
	spawn(-35.f, -470.f, "th_mid_03", { "KernelEnemy", "VampireBatEnemy" }, true, false);
	spawn(-230.f, -550.f, "th_mid_04", { "DemonEyeEnemy", "DemonEyeEnemy" }, false, false);
	spawn(200.f, -560.f, "th_mid_05", {}, false, true);
	spawn(-500.f, -640.f, "th_dark_01", { "WarlockEnemy" }, false, false);
	spawn(925.f, -605.f, "th_dark_02", { "WarlockEnemy", "KernelEnemy" }, false, false);
	spawn(-370.f, -910.f, "th_dark_03", { "DemonEyeEnemy", "WarlockEnemy" }, true, false);
	spawn(580.f, -910.f, "th_dark_04", { "KernelEnemy", "VampireBatEnemy" }, false, false);
	spawn(920.f, -950.f, "th_dark_05", { "WarlockEnemy", "WarlockEnemy" }, false, false);
	spawn(-730.f, -990.f, "th_dark_06", {}, false, true);
	spawn(-10.f, -1055.f, "th_deep_01", { "KeyeEnemy", "KernelEnemy" }, true, false);
	spawn(280.f, -1080.f, "th_deep_02", { "DemonEyeEnemy", "MimicEnemy" }, true, false);
	spawn(-480.f, -1110.f, "th_deep_03", { "VampireBatEnemy" }, false, false);
	spawn(-10.f, -1200.f, "th_deep_04", { "MimicEnemy" }, false, false);
	spawn(590.f, -1250.f, "th_deep_05", { "WarlockEnemy", "MimicEnemy" }, true, false);
	spawn(1150.f, -1280.f, "th_deep_06", { "KeyeEnemy", "DemonEyeEnemy", "KernelEnemy" }, true, false);
	spawn(-200.f, -1370.f, "th_end_01", { "WarlockEnemy", "DemonEyeEnemy" }, false, false);
	spawn(920.f, -1380.f, "th_end_02", {}, false, true);
	spawn(-700.f, -1430.f, "th_end_03", { "VampireBatEnemy", "VampireBatEnemy", "VampireBatEnemy" }, false, false);
	spawn(-410.f, -1500.f, "th_end_04", { "WarlockEnemy", "KernelEnemy", "DemonEyeEnemy" }, false, false);
	spawn(620.f, -165.f, "th_extra_01", { "KeyeEnemy" }, false, false);
	spawn(950.f, -220.f, "th_extra_02", { "DemonEyeEnemy" }, false, false);
	// Moved off (1160, -280): that spawn sat 32px from the AuthTerminal, so its 80px aggro bubble
	// fired before the player could ever reach the keypad.
	spawn(1024.f, -120.f, "th_extra_03", { "KernelEnemy" }, false, false);
	spawn(1135.f, -525.f, "th_extra_04", {}, false, true);

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

	audio->RegisterBank("step_concrete", { "step_c1", "step_c2", "step_c3", "step_c4" });
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

	// The password menu owns the keyboard while it is up. It runs before anything else and consumes
	// its own close key; the cooldown below stops a still-held ESC from opening the inventory on the
	// frame after it closes.
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

	// Update PopUpMessage
	if (popUpMessage) {
		popUpMessage->Update(deltaTime);
	}

	if (currentConversation) {
		isGamePaused = true;
		currentConversation->Execute(deltaTime);
		if (currentConversation->IsFinished()) {
			currentConversation = nullptr;
			// Clear before invoking: the callback may queue the next conversation.
			if (activeNPC && activeNPC->GetOnDialogueEnd()) {
				activeNPC->GetOnDialogueEnd()();
			}
			activeNPC = nullptr; // Reset

			// Callback state-machine
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

void Demo::ThreadAlleyScene::DrawWorld(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	if (SUCCEEDED(gd->BeginDraw())) {
		DrawCheckerBackground(gd, deltaTime);

		map->Draw(camera);

		struct DepthNode {
			float y;
			std::function<void()> drawCall;
			bool operator<(const DepthNode& other) const { return y < other.y; }
		};
		std::vector<DepthNode> depthNodes;

		for (auto& savePoint : savePoints) depthNodes.push_back({ savePoint->GetWorldY(), [&, savePoint]() { savePoint->Draw(camera, deltaTime); } });
		for (auto& shopPoint : shopPoints) depthNodes.push_back({ shopPoint->GetWorldY(), [&, shopPoint]() { shopPoint->Draw(camera, deltaTime); } });
		for (auto& healPoint : healingPoints) depthNodes.push_back({ healPoint->GetWorldY(), [&, healPoint]() { healPoint->Draw(camera, deltaTime); } });
		for (auto& chest : treasureChests) depthNodes.push_back({ chest->GetWorldY(), [&, chest]() { chest->Draw(camera, deltaTime); } });
		for (auto& enemy : mapEnemies) depthNodes.push_back({ enemy->GetWorldY(), [&, enemy]() { enemy->Draw(&camera, deltaTime); } });

		for (auto& npc : mapNPCs) {
			depthNodes.push_back({ npc->GetWorldY(), [&, npc]() { npc->Draw(camera, deltaTime); } });
		}

		if (authTerminal) depthNodes.push_back({ authTerminal->GetWorldY(), [&]() { authTerminal->Draw(camera, deltaTime); } });
		if (trojanNPC) depthNodes.push_back({ trojanNPC->GetWorldY(), [&]() { trojanNPC->Draw(camera, deltaTime); } });
		if (player) depthNodes.push_back({ player->GetWorldY(), [&]() { player->Draw(deltaTime); } });

		std::sort(depthNodes.begin(), depthNodes.end());
		for (auto& node : depthNodes) {
			node.drawCall();
		}

		gd->EndDraw();
	}
}

void Demo::ThreadAlleyScene::DrawUI(unsigned long long deltaTime)
{
	CreateImGuiDebugFrame(player, game);
	auto gd = game->GetGraphicsDevice();
	if (SUCCEEDED(gd->BeginDraw())) {

		for (auto& savePoint : savePoints) savePoint->DrawUI(&this->uiCamera, deltaTime);
		for (auto& chest : treasureChests) chest->DrawUI(&this->uiCamera, deltaTime);
		for (auto& healPoint : healingPoints) healPoint->DrawUI(&this->uiCamera, deltaTime);
		for (auto& shopPoint : shopPoints) shopPoint->DrawUI(&this->uiCamera, deltaTime);

		for (auto& npc : mapNPCs) {
			npc->DrawUI(&this->uiCamera, deltaTime);
		}

		if (trojanNPC && trojanNPC->GetPhase() != Demo::TrojanNPC::Phase::Defeated) trojanNPC->DrawUI(&this->uiCamera, deltaTime);
		if (playerHUD) playerHUD->Draw(gd, deltaTime);
		if (inventoryMenu) inventoryMenu->Draw(gd, deltaTime);
		if (draggableManager && inventoryMenu && inventoryMenu->IsOpen() && inventoryMenu->GetCurrentTab() == Demo::InventoryMenu::Tab::DECK) {
			draggableManager->Draw(deltaTime);
		}
		if (inventoryMenu) inventoryMenu->DrawKeyboardReticle(gd, deltaTime);

		// After the HUD and inventory so the password panel sits on top of them.
		if (authTerminal) authTerminal->DrawUI(&this->uiCamera, deltaTime);

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

	outData["hasSeenChapterIntro"] = hasSeenChapterIntro;

	outData["authTerminal"] = {
		{"password", authPassword},
		{"solved", authTerminalSolved}
	};
	if (trojanNPC) outData["trojanNPC"] = { {"phase", static_cast<int>(trojanNPC->GetPhase())} };

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

void Demo::ThreadAlleyScene::RestoreSaveData(const nlohmann::json& inData)
{
	player->RestoreSaveData(inData["player"]);
	camera.SetPosition(inData["camera"]["x"], inData["camera"]["y"]);
	camera.SetZoom(inData["camera"]["zoom"]);

	hasSeenChapterIntro = inData.value("hasSeenChapterIntro", false);

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

void Demo::ThreadAlleyScene::GiveTestItems()
{

}

void Demo::ThreadAlleyScene::StartTrojanConversation()
{
	auto phase = trojanNPC->GetPhase();

	// Coming back with the token in hand is what makes it drop the act.
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
		// Losing the fight leaves the NPC in Revealed, so talking again replays the taunt and rematches.
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
			trojanNPC->SetPhase(TrojanNPC::Phase::Defeated);
			auto result = QuestManager::GetInstance()->NotifyEvent("TROJAN_DEFEATED", "", player.get());
			if (result.hasReward && this->popUpMessage) {
				this->popUpMessage->ShowMessage(L"(+) " + result.rewardMessage, 5.0f);
			}
			});

		sceMan->InsertScene(sceMan->GetIndex() + 1, battleScene);
		sceMan->GoToNext();

		isTransitioning = false;
		markFinished();
		}));

	drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
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