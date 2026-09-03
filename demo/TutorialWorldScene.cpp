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
#include "CardShop.h"
#include "ItemShop.h"
#include "backends/imgui_impl_dx9.h"
#include "backends/imgui_impl_win32.h"

void Demo::TutorialWorldScene::OnInit()
{
	InitCore(248, 184, L"./assets/tutorial.tmx");

	map->SetAreaUpdateHandler("trigger_p", [this](const DX9GF::Map::ObjectArea& area) {
		CreatePortalTransition(2, -417.f, 144.f, "bgm_arcade", 0.2f);
	});

	map->SetAreaUpdateHandler("trigger_secret", [this](const DX9GF::Map::ObjectArea& area) {
		CreatePortalTransition(1, -84 * 16, -39 * 16, "bgm_secret", 0.3f);
	});

	NPCConfig daudauConfig = { L"assets/daudau-Sheet.png", 32, 32, 5, 12, 24.f, 8.f, 12.f };
	NPCConfig kakoConfig = { L"assets/kako-Sheet.png", 32, 32, 2, 6, 24.f, 8.f, 12.f };

	auto npcIntroduction = std::make_shared<NPC>(transformManager, 167.0f, -18.0f, kakoConfig);
	npcIntroduction->AttachQuestMarker("Quest_Tutorial", Demo::QuestMarkerRole::Giver);
	npcIntroduction->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	npcIntroduction->RegisterVoice(L"Kako", "bleep28");
	npcIntroduction->RegisterVoice(L"???", "bleep28");
	npcIntroduction->SetInteractLogic([](NPC* self) -> std::function<void()> {
		auto qState = QuestManager::GetInstance()->GetQuestState("Quest_Tutorial");
		if (qState == Demo::QuestState::Locked) {
			self->AddLine(L"???", L"Hello! Welcome.");
			self->AddLine(L"Player", L"Who are you?");
			self->AddLine(L"Kako", L"My name is Kako. I am a digital entity that exists in this world. Created by the developers as a guide.");
			self->AddLine(L"Player", L"Where am I?");
			self->AddLine(L"Kako", L"This is a cyber world! An abandoned project made by a bunch of UIT students.");
			self->AddLine(L"Kako", L"Unfortunately, this world is not safe for humans. You are trapped here now.");
			self->AddLine(L"Player", L"How do I get out?");
			self->AddLine(L"Kako", L"How to escape this world? I don't know.");
			self->AddLine(L"Kako", L"But I can teach you how to survive here! Explore around a bit and I'll explain further.");
			self->AddLine(L"Kako", L"By the way, use the floppy disk icon over there to save your progress.");

			return []() {
				std::vector<std::pair<std::wstring, std::function<void()>>> buttons = {
					{ L"Yes(Y)", []() { QuestManager::GetInstance()->AcceptQuest("Quest_Tutorial"); } },
					{ L"No(N)", []() {} }
				};
				PopupManager::GetInstance()->Show("stepped_blue", L"New Quest", L"Accept Beginner's Quest?", buttons);
			};
		}
		else if (qState == Demo::QuestState::Active) {
			self->AddLine(L"Kako", L"Explore around a bit!");
		}
		else {
			self->AddLine(L"Kako", L"Great job! You know the basics of survival now.");
		}
		return nullptr;
	});
	mapNPCs.push_back(npcIntroduction);

	auto npcKako = std::make_shared<NPC>(transformManager, 592.0f, -912.0f, kakoConfig);
	npcKako->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	npcKako->RegisterVoice(L"Kako", "bleep28");
	npcKako->SetInteractLogic([](NPC* self) -> std::function<void()> {
		auto prevQuestState = QuestManager::GetInstance()->GetQuestState("Quest_Tutorial");
		auto questState = QuestManager::GetInstance()->GetQuestState("Quest_KakosLab");
		if (questState == Demo::QuestState::Locked) {
			if (prevQuestState == Demo::QuestState::Active || prevQuestState == Demo::QuestState::Completed) {
				self->AddLine(L"Kako", L"Oh hi again!");
				self->AddLine(L"Player", L"Hi Kako! What are you doing here?");
			}
			else {
				self->AddLine(L"Player", L"Hello! Who are you?");
				self->AddLine(L"Kako", L"Hi! I am Kako, a digital entity that exists in this world.");
				self->AddLine(L"Player", L"Hi Kako! What are you doing here?");
			}
			self->AddLine(L"Kako", L"Hm.. I was doing some research inside the lab right there until a monster appeared!");
			self->AddLine(L"Kako", L"Luckily, I managed to escape. But I think the lab is not safe anymore.");
			self->AddLine(L"Kako", L"If you can clear the area for me, I'll pay a handsome reward :).");
			return []() {
				std::vector<std::pair<std::wstring, std::function<void()>>> buttons = {
					{ L"Yes(Y)", []() { QuestManager::GetInstance()->AcceptQuest("Quest_KakosLab"); } },
					{ L"No(N)", []() {} }
				};
				PopupManager::GetInstance()->Show("stepped_blue", L"New Quest", L"Accept Kako's Request?", buttons);
			};
		}
		else if (questState == Demo::QuestState::Active) {
			self->AddLine(L"Kako", L"I'll be waiting for you to clear the lab!");
		}
		else {
			self->AddLine(L"Kako", L"Thanks for helping me clear the lab!");
		}
		return nullptr;
	});
	mapNPCs.push_back(npcKako);

	auto npcExplainingEnemyEncounters = std::make_shared<NPC>(transformManager, 544.0f, -56.0f, daudauConfig);
	npcExplainingEnemyEncounters->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	npcExplainingEnemyEncounters->RegisterVoice(L"Dau Dau", "bleep12");
	npcExplainingEnemyEncounters->SetInteractLogic([](NPC* self) -> std::function<void()> {
		self->AddLine(L"Dau Dau", L"Look out ahead! See those digital creeps roaming around?");
		self->AddLine(L"Dau Dau", L"If they spot you, they will chase you down! Touching them will drag you into a battle.");
		self->AddLine(L"Dau Dau", L"You can try to outrun them or hide behind walls to break their line of sight.");
		self->AddLine(L"Dau Dau", L"Don't worry, you can run away from battles if you want.\n But you won't get any rewards if you do that!");
		return nullptr;
	});
	mapNPCs.push_back(npcExplainingEnemyEncounters);

	auto npcExplainingHealingPoint = std::make_shared<NPC>(transformManager, 289.0f, -496.0f, daudauConfig);
	npcExplainingHealingPoint->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	npcExplainingHealingPoint->RegisterVoice(L"Dau Dau", "bleep12");
	npcExplainingHealingPoint->SetInteractLogic([this](NPC* self) -> std::function<void()> {
		if (player->GetHealth() == player->GetMaxHealth()) {
			self->AddLine(L"Dau Dau", L"Hey, you look healthy!");
			self->AddLine(L"Dau Dau", L"Remember, you can use healing points to restore your health.");
			self->AddLine(L"Dau Dau", L"If you want to heal in combat, you can use healing items! Check out my shop up ahead for some.");
			return nullptr;
		}
		self->AddLine(L"Dau Dau", L"Hey, you look hurt.");
		self->AddLine(L"Player", L"Yeah, I feel dizzy...");
		self->AddLine(L"Dau Dau", L"This is a healing point. You can use it to restore your health. Just interact with it like you do with me.");
		self->AddLine(L"Dau Dau", L"If you want to heal in combat, you can use healing items! Check out my shop up ahead for some.");
		return nullptr;
	});
	mapNPCs.push_back(npcExplainingHealingPoint);

	auto npcExplainingPortal = std::make_shared<NPC>(transformManager, 630.f, -639.f, daudauConfig);
	npcExplainingPortal->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	npcExplainingPortal->RegisterVoice(L"Dau Dau", "bleep12");
	npcExplainingPortal->SetInteractLogic([](NPC* self) -> std::function<void()> {
		self->AddLine(L"Dau Dau", L"Up this hill, there will be a foe waiting for you. It's guarding a portal that'll take you to the next area.");
		self->AddLine(L"Dau Dau", L"Good luck!");
		return nullptr;
	});
	mapNPCs.push_back(npcExplainingPortal);

	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 248.0f, -70.0f));
	savePoints.push_back(std::make_shared<SavePoint>(transformManager, -64.0f, -592.0f));
	savePoints.push_back(std::make_shared<SavePoint>(transformManager, 456.0f, -903.0f));
	for (auto& sp : savePoints) {
		sp->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, saveManager, font, drawBuffer);
		sp->SetVisible(true);
	}

	auto shopCard = std::make_shared<ShopPoint>(transformManager, 183.0f, -460.0f);
	shopCard->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) {
			return new CardShop(g, p, w, h, ShopTier::BASIC);
		}
	);
	shopCard->SetVisible(true);
	shopPoints.push_back(shopCard);

	auto shopItem = std::make_shared<ShopPoint>(transformManager, 320.0f, -660.0f);
	shopItem->Init(game, game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer,
		[](Game* g, Player* p, int w, int h) {
			return new ItemShop(g, p, w, h, ShopTier::BASIC);
		}
	);
	shopItem->SetVisible(true);
	shopPoints.push_back(shopItem);

	auto healPt = std::make_shared<HealingPoint>(transformManager, 368.0f, -400.0f);
	healPt->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);
	healPt->SetVisible(true);
	healingPoints.push_back(healPt);

	treasureChests.push_back(std::make_shared<TreasureChestNPC>(
		transformManager, 174.f, -629.f,
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

	auto bgDraw = [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) {
		DrawBackground(gd, deltaTime);
	};

	SpawnMapEnemy(615.f, -170.f, "tutorial_keye_01", { "KeyeEnemy" }, false, false, bgDraw);
	SpawnMapEnemy(510.f, -380.f, "tutorial_demoneye_01", { "DemonEyeEnemy" }, false, false, bgDraw);
	SpawnMapEnemy(500.f, -590.f, "tutorial_random_01", { "KeyeEnemy", "DemonEyeEnemy" }, true, false, bgDraw, 30);
	SpawnMapEnemy(-45.f, -409.f, "tutorial_random_02", { "KeyeEnemy", "DemonEyeEnemy", "MimicEnemy" }, true, false, bgDraw, 30);

	player->SetBaseSurface("default");

	transformManager->RebuildHierarchy();
	this->GiveTestItems();
	drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
}

std::string Demo::TutorialWorldScene::GetSaveID() const
{
	return "TutorialWorldScene";
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
