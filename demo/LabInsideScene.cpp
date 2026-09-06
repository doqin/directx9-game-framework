#include "pch.h"
#include "SettingsManager.h"
#include "LabInsideScene.h"
#include "MainMenu.h"
#include "SaveGameState.h"
#include "TransitionCommand.h"
#include "resource.h"
#include "PopupManager.h"
#include "QuestManager.h"
#include "MapEnemy.h"
#include "EnemyFactory.h"
#include "MapBattleScene.h"
#include "RNG.h"
#include "backends/imgui_impl_dx9.h"
#include "backends/imgui_impl_win32.h"

void Demo::LabInsideScene::OnInit()
{
	InitCore(64, -1, L"./assets/labinside.tmx");

	SetChapterTitle(L"KAKOS LAB", L"< What scared her off? >");
	map->SetAreaUpdateHandler("trigger_back", [this](const DX9GF::Map::ObjectArea& area) {
		CreatePortalTransition(-1, 544.f, -928.f, "bgm_tutorial", 0.5f);
	});

	treasureChests.push_back(std::make_shared<TreasureChestNPC>(
		transformManager, 8.f, 4.f,
		std::vector<ChestReward>{
			ChestReward::Item(0, 1),
			ChestReward::Card("BraceCard")
		}, true));
	treasureChests.back()->Init(game->GetGraphicsDevice(), &camera, player, colliderManager, font, drawBuffer);

	auto bgDraw = [this](DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) {
		DrawBackground(gd, deltaTime);
	};

	SpawnMapEnemy(64.f, 125.f, "lab_kakos_01", { "KeyeEnemy", "DemonEyeEnemy", "VampireBatEnemy" }, false, false, bgDraw, 30,
		"KAKOS_LAB_CLEARED", "Quest_KakosLab");

	player->SetBaseSurface("default");

	transformManager->RebuildHierarchy();
	drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
}

std::string Demo::LabInsideScene::GetSaveID() const
{
	return "LabInsideScene";
}

void Demo::LabInsideScene::OnGenerateSaveData(nlohmann::json& outData)
{
	outData["isBossDead"] = isBossDead;
}

void Demo::LabInsideScene::OnRestoreSaveData(const nlohmann::json& inData)
{
	if (inData.contains("isBossDead")) {
		isBossDead = inData["isBossDead"].get<bool>();
	}
}

void Demo::LabInsideScene::DrawBackground(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
{
	auto [screenWidth, screenHeight] = camera.GetScreenResolution();

	gd->DrawRectangle(0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0x000000, true);

	const int spacingX = 32;
	const int spacingY = 32;
	const float segmentLength = 16.0f;
	const float amplitude = 8.0f;
	const float frequency = 0.08f;
	const D3DCOLOR gridColor = 0xFF1b1c1b;
	static float waveTime = 0.0f;
	waveTime += static_cast<float>(deltaTime) * 0.001f;
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
