#include "pch.h"
#include "TutorialWorldScene.h"
#include "RandomEncounter.h"
#include "MainMenu.h"
#include "SaveGameState.h"
#include "TransitionCommand.h"
#include "resource.h"
#include "PopupManager.h"
#include "EncounterGenerator.h"
#include "EnemyFactory.h"
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
	npcExplainingEnemyEncounters->AddLine(L"Dau Dau", L"Look out ahead! Those green patches are combat zones.");
	npcExplainingEnemyEncounters->AddLine(L"Dau Dau", L"If you step on them, there is a chance you'll be ambushed.\n If so, you'll have to fight enemies.");
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

	// --- KHỞI TẠO MAP ENEMIES ---

	// 1. Con thứ nhất: Chỉ có KeyeEnemy (Tọa độ 615, -170)
	BattleEncounter encounter1;
	encounter1.mapEnemyID = "tutorial_keye_01";
	encounter1.enemyTypes = { "KeyeEnemy" };
	encounter1.bgmName = "battle_loop1";
	encounter1.bgDrawFunc = [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) { DrawBackground(gd, deltaTime); };

	// AUTO LOAD SPRITE: Lấy hình của con KeyeEnemy
	auto sprite1 = EnemyFactory::GetOverworldSprite(encounter1.enemyTypes[0]);
	encounter1.mapTexturePath = sprite1.texturePath;
	encounter1.spriteWidth = sprite1.width;
	encounter1.spriteHeight = sprite1.height;
	encounter1.frameCount = sprite1.frameCount;

	auto roamingEnemy1 = std::make_shared<MapEnemy>(transformManager, 615.f, -170.f, encounter1);
	roamingEnemy1->Init(game, game->GetGraphicsDevice(), colliderManager.get(), player);
	mapEnemies.push_back(roamingEnemy1);


	// 2. Con thứ hai: Chỉ có DemonEyeEnemy (Tọa độ 510, -380)
	BattleEncounter encounter2;
	encounter2.mapEnemyID = "tutorial_demoneye_01";
	encounter2.enemyTypes = { "DemonEyeEnemy" };
	encounter2.bgmName = "battle_loop1";
	encounter2.bgDrawFunc = [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) { DrawBackground(gd, deltaTime); };

	// AUTO LOAD SPRITE: Lấy hình của con DemonEyeEnemy
	auto sprite2 = EnemyFactory::GetOverworldSprite(encounter2.enemyTypes[0]);
	encounter2.mapTexturePath = sprite2.texturePath;
	encounter2.spriteWidth = sprite2.width;
	encounter2.spriteHeight = sprite2.height;
	encounter2.frameCount = sprite2.frameCount;

	auto roamingEnemy2 = std::make_shared<MapEnemy>(transformManager, 510.f, -380.f, encounter2);
	roamingEnemy2->Init(game, game->GetGraphicsDevice(), colliderManager.get(), player);
	mapEnemies.push_back(roamingEnemy2);


	// 3. Con thứ ba: Random 1-2 con Keye/Demon (Tọa độ 500, -590)
	BattleEncounter encounter3;
	encounter3.mapEnemyID = "tutorial_random_01";

	// FIX: Nạp đạn vào rổ trước
	encounter3.randomPool = { "KeyeEnemy", "DemonEyeEnemy" };
	// Sau đó mới bốc thăm từ rổ
	encounter3.enemyTypes = EncounterGenerator::GenerateFromTypes(encounter3.randomPool);

	encounter3.bgmName = "battle_loop1";
	encounter3.bgDrawFunc = [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) { DrawBackground(gd, deltaTime); };

	// AUTO LOAD DẤU CHẤM HỎI
	auto sprite3 = EnemyFactory::GetOverworldSprite("Random");
	encounter3.mapTexturePath = sprite3.texturePath;
	encounter3.spriteWidth = sprite3.width;
	encounter3.spriteHeight = sprite3.height;
	encounter3.frameCount = sprite3.frameCount;

	auto roamingEnemy3 = std::make_shared<MapEnemy>(transformManager, 500.f, -590.f, encounter3);
	roamingEnemy3->Init(game, game->GetGraphicsDevice(), colliderManager.get(), player);
	mapEnemies.push_back(roamingEnemy3);

	// 4. Con thứ tư: Đội hình Cố định (Keye + DemonEye) - Ép người chơi xử lý đa mục tiêu
	// Tọa độ: -90, -400
	BattleEncounter encounter4;
	encounter4.mapEnemyID = "tutorial_mixed_01";
	encounter4.enemyTypes = { "KeyeEnemy", "DemonEyeEnemy" }; // Gặp chắc chắn 2 con khác loại
	encounter4.bgmName = "battle_loop1";
	encounter4.bgDrawFunc = [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) { DrawBackground(gd, deltaTime); };

	// Lấy hình con Keye làm "Leader" đại diện ngoài map
	auto sprite4 = EnemyFactory::GetOverworldSprite(encounter4.enemyTypes[0]);
	encounter4.mapTexturePath = sprite4.texturePath;
	encounter4.spriteWidth = sprite4.width;
	encounter4.spriteHeight = sprite4.height;
	encounter4.frameCount = sprite4.frameCount;

	auto roamingEnemy4 = std::make_shared<MapEnemy>(transformManager, -90.f, -400.f, encounter4);
	roamingEnemy4->Init(game, game->GetGraphicsDevice(), colliderManager.get(), player);
	mapEnemies.push_back(roamingEnemy4);


	// 5. Con thứ năm: Bể ngẫu nhiên Mở rộng (Có rủi ro gặp Mimic)
	// Tọa độ: 50, -330
	BattleEncounter encounter5;
	encounter5.mapEnemyID = "tutorial_random_02";

	// FIX: Nạp đạn vào rổ chứa cả Mimic
	encounter5.randomPool = { "KeyeEnemy", "DemonEyeEnemy", "MimicEnemy" };
	// Sau đó bốc thăm
	encounter5.enemyTypes = EncounterGenerator::GenerateFromTypes(encounter5.randomPool);
	encounter5.bgmName = "battle_loop1";
	encounter5.bgDrawFunc = [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) { DrawBackground(gd, deltaTime); };

	// Gắn sprite dấu chấm hỏi
	auto sprite5 = EnemyFactory::GetOverworldSprite("Random");
	encounter5.mapTexturePath = sprite5.texturePath;
	encounter5.spriteWidth = sprite5.width;
	encounter5.spriteHeight = sprite5.height;
	encounter5.frameCount = sprite5.frameCount;

	auto roamingEnemy5 = std::make_shared<MapEnemy>(transformManager, 50.f, -330.f, encounter5);
	roamingEnemy5->Init(game, game->GetGraphicsDevice(), colliderManager.get(), player);
	mapEnemies.push_back(roamingEnemy5);

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

	if (inpMan->KeyPress(DIK_ESCAPE) && escCooldown <= 0) {
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
		if (!currentConversation && npcIntroduction->CanInteract() && inpMan->KeyPress(DIK_E)) {
			float sw = game->GetVirtualWidth();
			float sh = game->GetVirtualHeight();
			currentConversation = std::make_shared<IConversation>(std::make_shared<DX9GF::FontSprite>(font.get()), sw, sh);
			for (auto& line : npcIntroduction->GetDialogueLines()) {
				currentConversation->AddLine(line);
			}
		}
	}
	if (npcExplainingHealingPoint) {
		npcExplainingHealingPoint->Update(deltaTime);
		if (!currentConversation && npcExplainingHealingPoint->CanInteract() && inpMan->KeyPress(DIK_E)) {
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
		if (!currentConversation && npcExplainingEnemyEncounters->CanInteract() && inpMan->KeyPress(DIK_E)) {
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
		if (!currentConversation && npcExplainingPortal->CanInteract() && inpMan->KeyPress(DIK_E)) {
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
		// 3. THÊM VÒNG LẶP NÀY ĐỂ QUÁI CHẠY
		for (auto& enemy : mapEnemies) {
			enemy->Update(deltaTime);
		}
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

void Demo::TutorialWorldScene::DrawWorld(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	if (SUCCEEDED(gd->BeginDraw())) {

		DrawBackground(gd, deltaTime);
		map->Draw(camera);
		if (npcIntroduction) npcIntroduction->Draw(camera, deltaTime);
		if (npcExplainingHealingPoint) npcExplainingHealingPoint->Draw(camera, deltaTime);
		if (npcExplainingEnemyEncounters) npcExplainingEnemyEncounters->Draw(camera, deltaTime);
		for (auto& savePoint : savePoints) {
			savePoint->Draw(camera, deltaTime);
		}
		if (shopPoint_Card) shopPoint_Card->Draw(camera, deltaTime);
		if (shopPoint_BSItem) shopPoint_BSItem->Draw(camera, deltaTime);
		if (healingPoint) healingPoint->Draw(camera, deltaTime);
		for (auto& chest : treasureChests)
			chest->Draw(camera, deltaTime);

		if (npcExplainingPortal) npcExplainingPortal->Draw(camera, deltaTime);

		// 4. THÊM VÒNG LẶP VẼ QUÁI
		for (auto& enemy : mapEnemies) {
			enemy->Draw(&camera, deltaTime);
		}
		player->Draw(deltaTime);

		gd->EndDraw();
	}
}

void Demo::TutorialWorldScene::DrawUI(unsigned long long deltaTime)
{
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

		if (currentConversation) {
			currentConversation->Draw(gd, &this->uiCamera, deltaTime);
		}

		if (drawBuffer) {
			drawBuffer->Update(deltaTime);
		}

		PopupManager::GetInstance()->DrawUI(deltaTime, &this->uiCamera);

		DX9GF::InputManager::GetInstance()->DrawCursor(&this->uiCamera, deltaTime);

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

	nlohmann::json chestStates = nlohmann::json::array();
	for (auto& c : treasureChests) chestStates.push_back(c->GetIsOpened());
	outData["treasureChests"] = chestStates;

	// THÊM MỚI: Quét toàn bộ mapEnemies và lưu trạng thái dựa trên mapEnemyID
	nlohmann::json enemiesState = nlohmann::json::object();
	for (auto& enemy : mapEnemies) {
		enemiesState[enemy->GetEnemyID()] = {
			{"isDefeated", enemy->IsDefeated()},
			{"respawnTimer", enemy->GetRespawnTimer()}
		};
	}
	outData["mapEnemies"] = enemiesState;
}

void Demo::TutorialWorldScene::RestoreSaveData(const nlohmann::json& inData)
{
	player->RestoreSaveData(inData["player"]);
	camera.SetPosition(inData["camera"]["x"], inData["camera"]["y"]);
	camera.SetZoom(inData["camera"]["zoom"]);

	if (inData.contains("treasureChests")) {
		auto& arr = inData["treasureChests"];
		for (size_t i = 0; i < treasureChests.size() && i < arr.size(); ++i)
			treasureChests[i]->SetOpened(arr[i].get<bool>());
	}

	// THÊM MỚI: Khôi phục trạng thái chết/sống của quái vật
	if (inData.contains("mapEnemies")) {
		auto& enemiesState = inData["mapEnemies"];
		for (auto& enemy : mapEnemies) {
			std::string id = enemy->GetEnemyID();
			// Nếu tìm thấy ID quái trong file Save, tiến hành set trạng thái
			if (enemiesState.contains(id)) {
				bool def = enemiesState[id]["isDefeated"].get<bool>();
				float timer = enemiesState[id]["respawnTimer"].get<float>();
				enemy->SetDefeatedState(def, timer);
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