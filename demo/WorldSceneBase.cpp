#include "pch.h"
#include "SettingsManager.h"
#include "WorldSceneBase.h"
#include "MainMenu.h"
#include "SaveGameState.h"
#include "TransitionCommand.h"
#include "resource.h"
#include "PopupManager.h"
#include "QuestManager.h"
#include "EnemyFactory.h"
#include "MapBattleScene.h"
#include "RNG.h"
#include "backends/imgui_impl_dx9.h"
#include "backends/imgui_impl_win32.h"

Demo::WorldSceneBase::WorldSceneBase(Game* game, std::shared_ptr<DX9GF::SaveManager> sm, UINT sw, UINT sh)
	: IScene(sw, sh), game(game), saveManager(sm) {
}

void Demo::WorldSceneBase::Init() {
	OnInit();
}

void Demo::WorldSceneBase::InitCore(float playerX, float playerY, const wchar_t* mapFile)
{
	camera.SetZoom(2.0f);
	transformManager = std::make_shared<DX9GF::TransformManager>();
	colliderManager = std::make_shared<DX9GF::ColliderManager>();
	player = std::make_shared<Player>(transformManager, playerX, playerY);
	camera.SetPosition(playerX, playerY);
	player->Init(game->GetGraphicsDevice(), colliderManager.get(), &camera);
	drawBuffer = std::make_shared<DX9GF::CommandBuffer>();
	commandBuffer = std::make_shared<DX9GF::CommandBuffer>();
	popUpMessage = std::make_shared<PopUpMessage>(transformManager, game);
	popUpMessage->SetLocalPosition(0.0f, 0.0f);
	popUpMessage->Init(game->GetGraphicsDevice(), &this->uiCamera);
	map = std::make_shared<DX9GF::Map>(game->GetGraphicsDevice());
	std::wstring wMapFile(mapFile);
	std::string sMapFile(wMapFile.begin(), wMapFile.end());
	map->Create(transformManager, colliderManager, sMapFile);

	font = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 16);
	chapterTitleUI = std::make_shared<ChapterTitleUI>(font);

	auto borderTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	borderTex->LoadTexture(L"assets/popup-borders.png");
	auto uiTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	uiTex->LoadTexture(L"assets/ui.png");
	PopupManager::GetInstance()->Init(game->GetGraphicsDevice(), borderTex, uiTex, font);
	QuestManager::GetInstance()->SetVirtualResolution(game->GetVirtualWidth(), game->GetVirtualHeight());
	QuestManager::GetInstance()->Init(game->GetGraphicsDevice(), transformManager, &this->uiCamera, font);

	draggableManager = std::make_shared<Demo::DraggableManager>();
	inventoryMenu = std::make_shared<InventoryMenu>(game, player, transformManager, draggableManager, &this->uiCamera, font.get());
	inventoryMenu->Init();
	playerHUD = std::make_shared<PlayerHUD>(game, player, transformManager, &this->uiCamera, font.get());
	playerHUD->SetOnInventoryOpen([this]() {
		if (inventoryMenu && !inventoryMenu->IsOpen()) inventoryMenu->Toggle();
	});
	playerHUD->Init();

	map->SetAreaUpdateHandler("audio_zone_leaves", [this](const DX9GF::Map::ObjectArea&) {
		GetPlayer()->SetSurface("leaves");
	});
	map->SetAreaUpdateHandler("audio_zone_metal", [this](const DX9GF::Map::ObjectArea&) {
		GetPlayer()->SetSurface("metal");
	});

	ItemData::GetInstance()->LoadData();
}

void Demo::WorldSceneBase::Update(unsigned long long deltaTime)
{
	PopupManager::GetInstance()->SetUICamera(&this->uiCamera);
	QuestManager::GetInstance()->SetUICamera(&this->uiCamera);
	QuestManager::GetInstance()->SetVirtualResolution(game->GetVirtualWidth(), game->GetVirtualHeight());
	QuestManager::GetInstance()->SetVisible(!(inventoryMenu && inventoryMenu->IsOpen()));
	QuestManager::GetInstance()->Update(deltaTime);

	if (!hasSeenChapterIntro && !isTransitioning) {
		hasSeenChapterIntro = true;
		if (chapterTitleUI) {
			chapterTitleUI->Show(L"CHAPTER", L"< Subtitle >", 4.0f, 0xFFFFFFFF, 0xFFFFF200, 0x000000);
		}
	}
	if (chapterTitleUI) {
		chapterTitleUI->Update(deltaTime);
	}

	auto OpenChestWithDialogLocal = [&](std::shared_ptr<TreasureChestNPC>& chest) {
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

	if (popUpMessage) {
		popUpMessage->Update(deltaTime);
	}

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

	for (auto& savePoint : savePoints) {
		savePoint->Update(deltaTime);
	}
	for (auto& shopPoint : shopPoints) {
		shopPoint->Update(deltaTime);
	}
	for (auto& healingPoint : healingPoints) {
		healingPoint->Update(deltaTime);
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

	OnUpdate(deltaTime);

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

void Demo::WorldSceneBase::DrawWorld(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	if (SUCCEEDED(gd->BeginDraw())) {

		DrawBackground(gd, deltaTime);
		map->Draw(camera);

		std::vector<DepthNode> depthNodes;

		for (auto& savePoint : savePoints) {
			depthNodes.push_back({ savePoint->GetWorldY(), [&, savePoint]() { savePoint->Draw(camera, deltaTime); } });
		}
		for (auto& shopPoint : shopPoints) {
			depthNodes.push_back({ shopPoint->GetWorldY(), [&, shopPoint]() { shopPoint->Draw(camera, deltaTime); } });
		}
		for (auto& healingPoint : healingPoints) {
			depthNodes.push_back({ healingPoint->GetWorldY(), [&, healingPoint]() { healingPoint->Draw(camera, deltaTime); } });
		}
		for (auto& chest : treasureChests) {
			depthNodes.push_back({ chest->GetWorldY(), [&, chest]() { chest->Draw(camera, deltaTime); } });
		}
		for (auto& enemy : mapEnemies) {
			depthNodes.push_back({ enemy->GetWorldY(), [&, enemy]() { enemy->Draw(&camera, deltaTime); } });
		}
		for (auto& npc : mapNPCs) {
			depthNodes.push_back({ npc->GetWorldY(), [&, npc]() { npc->Draw(camera, deltaTime); } });
		}
		if (player) depthNodes.push_back({ player->GetWorldY(), [&]() { player->Draw(deltaTime); } });

		OnDrawWorld(depthNodes, deltaTime);

		std::sort(depthNodes.begin(), depthNodes.end());
		for (auto& node : depthNodes) {
			node.drawCall();
		}

		gd->EndDraw();
	}
}

void Demo::WorldSceneBase::DrawUI(unsigned long long deltaTime)
{
	CreateImGuiDebugFrame(player, game);
	auto gd = game->GetGraphicsDevice();

	if (SUCCEEDED(gd->BeginDraw())) {

		for (auto& savePoint : savePoints) savePoint->DrawUI(&this->uiCamera, deltaTime);
		for (auto& shopPoint : shopPoints) shopPoint->DrawUI(&this->uiCamera, deltaTime);
		for (auto& healingPoint : healingPoints) healingPoint->DrawUI(&this->uiCamera, deltaTime);
		for (auto& chest : treasureChests) chest->DrawUI(&this->uiCamera, deltaTime);
		for (auto& npc : mapNPCs) npc->DrawUI(&this->uiCamera, deltaTime);

		OnDrawUI(deltaTime);

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

void Demo::WorldSceneBase::OpenChestWithDialog(std::shared_ptr<TreasureChestNPC>& chest)
{
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
}

void Demo::WorldSceneBase::AddDepthNode(std::vector<DepthNode>& nodes, float y, std::function<void()> drawCall)
{
	nodes.push_back({ y, std::move(drawCall) });
}

void Demo::WorldSceneBase::CreatePortalTransition(int sceneOffset, float targetX, float targetY, const char* bgm, float bgmVol)
{
	if (isTransitioning) return;
	isTransitioning = true;

	auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, true);
	drawBuffer->PushCommand(transitionInCommand);
	commandBuffer->PushCommand(std::make_shared<DX9GF::CustomCommand>([this, transitionInCommand, sceneOffset, targetX, targetY, bgm, bgmVol](std::function<void(void)> markFinished) {
		if (!transitionInCommand->IsFinished()) {
			return;
		}
		auto sceMan = game->GetSceneManager();
		auto targetScene = sceMan->GetScene(static_cast<size_t>(sceMan->GetIndex()) + sceneOffset);
		auto targetPlayer = MainMenu::gameSaveState->GetPlayerFromScene(targetScene);
		targetPlayer->SetLocalPosition(targetX, targetY);
		if (bgm) {
			DX9GF::AudioManager::GetInstance()->PlayBGM_Fade(bgm, bgmVol, 1.5f);
		}
		if (sceneOffset > 0) sceMan->GoToNext();
		else if (sceneOffset < 0) sceMan->GoToPrevious();
		else sceMan->GoToScene(sceMan->GetIndex() + sceneOffset);
		isTransitioning = false;
		markFinished();
	}));
	drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
}

void Demo::WorldSceneBase::SpawnMapEnemy(float x, float y, std::string id, std::vector<std::string> types,
	bool isRand, bool isGlobal, std::function<void(DX9GF::GraphicsDevice*, unsigned long long)> bgDraw,
	int tokenChance)
{
	auto enemy = EnemyFactory::CreateMapEnemy(
		x, y, id, types, isRand, isGlobal, "battle_loop", bgDraw,
		transformManager, game, colliderManager.get(), player
	);

	Demo::EventType generatedEvent = Demo::EventType::None;
	if (static_cast<int>(Demo::RNG::Range(1, 100)) <= tokenChance) {
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
}

void Demo::WorldSceneBase::GenerateSaveData(nlohmann::json& outData)
{
	player->GenerateSaveData(outData["player"]);
	auto pos = camera.GetPosition();
	outData["camera"] = {
		{"x", pos.x},
		{"y", pos.y},
		{"zoom", camera.GetZoom()}
	};
	outData["hasSeenChapterIntro"] = hasSeenChapterIntro;

	nlohmann::json chestStates = nlohmann::json::array();
	for (auto& c : treasureChests) chestStates.push_back(c->GetIsOpened());
	outData["treasureChests"] = chestStates;

	nlohmann::json enemiesState = nlohmann::json::object();
	for (auto& enemy : mapEnemies) {
		enemiesState[enemy->GetEnemyID()] = {
			{"isDefeated", enemy->IsDefeated()},
			{"respawnTimer", enemy->GetRespawnTimer()},
			{"eventType", static_cast<int>(enemy->GetEncounterData().eventType)}
		};
	}
	outData["mapEnemies"] = enemiesState;

	OnGenerateSaveData(outData);
}

void Demo::WorldSceneBase::RestoreSaveData(const nlohmann::json& inData)
{
	player->RestoreSaveData(inData["player"]);
	camera.SetPosition(inData["camera"]["x"], inData["camera"]["y"]);
	camera.SetZoom(inData["camera"]["zoom"]);
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

	OnRestoreSaveData(inData);
}
