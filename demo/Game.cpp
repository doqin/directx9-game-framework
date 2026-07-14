#include "pch.h"
#include "Game.h"
#include "DebugScene.h"
#include "World1Scene.h"
#include "MainMenu.h"
#include "SettingsScene.h"
#include "DX9GFAudioManager.h"
#include "resource.h"
#include "SettingsManager.h"
#include "SplashScene.h"
void Demo::Game::Init()
{
	srand(static_cast<unsigned int>(time(NULL)));
	IGame::Init();
	SettingsManager::GetInstance()->LoadSettings();
	SettingsManager::GetInstance()->ApplyResolution();

	auto audio = DX9GF::AudioManager::GetInstance();
	audio->Init();
	audio->Load("btn_hover", IDR_WAV_HOVER);
	audio->Load("btn_click", IDR_WAV_CLICK);
	audio->Load("open_inv", IDR_WAVE_OPEN_INV);
	audio->Load("close_inv", IDR_WAVE_CLOSE_INV);

	audio->Load("step_d1", IDR_STEP_D1);
	audio->Load("step_d2", IDR_STEP_D2);
	audio->Load("step_d3", IDR_STEP_D3);
	audio->Load("step_d4", IDR_STEP_D4);

	audio->Load("step_l1", IDR_STEP_L1);
	audio->Load("step_l2", IDR_STEP_L2);
	audio->Load("step_l3", IDR_STEP_L3);
	audio->Load("step_l4", IDR_STEP_L4);

	audio->Load("step_m1", IDR_STEP_M1);
	audio->Load("step_m2", IDR_STEP_M2);
	audio->Load("step_m3", IDR_STEP_M3);
	audio->Load("step_m4", IDR_STEP_M4);
	audio->Load("step_m5", IDR_STEP_M5);

	audio->RegisterBank("step_default", { "step_d1", "step_d2", "step_d3", "step_d4" });
	audio->RegisterBank("step_leaves", { "step_l1", "step_l2", "step_l3", "step_l4" });
	audio->RegisterBank("step_metal", { "step_m1", "step_m2", "step_m3", "step_m4", "step_m5" });

	audio->Load("shop_buy", IDR_SHOP_BUY);
	audio->Load("error", IDR_WAV_ERROR);

	audio->Load("card_draw1", IDR_CARD_DRAW1);
	audio->Load("card_draw2", IDR_CARD_DRAW2);
	audio->Load("card_draw3", IDR_CARD_DRAW3);
	audio->Load("card_draw4", IDR_CARD_DRAW4);

	audio->RegisterBank("card_draw", { "card_draw1", "card_draw2", "card_draw3", "card_draw4" });

	audio->Load("hurt1", IDR_HURT1);
	audio->Load("hurt2", IDR_HURT2);
	audio->Load("hurt3", IDR_HURT3);
	audio->Load("hurt4", IDR_HURT4);
	audio->Load("hurt5", IDR_HURT5);

	audio->RegisterBank("take_dmg", { "hurt1", "hurt2", "hurt3", "hurt4", "hurt5" });

	audio->Load("player_dead", IDR_PLAYER_DEAD);
	audio->Load("coin_gather", IDR_COIN_GATHER);

	audio->Load("card_snap1", IDR_CARD_SNAP);
	audio->Load("card_snap2", IDR_CARD_SNAP2);
	audio->RegisterBank("card_snap", { "card_snap1", "card_snap2" });

	audio->Load("dialog_voice1", IDR_DIALOG_VOICE1);
	audio->Load("dialog_voice2", IDR_DIALOG_VOICE2);
	audio->Load("dialog_voice3", IDR_DIALOG_VOICE3);
	audio->Load("dialog_voice4", IDR_DIALOG_VOICE4);
	audio->Load("dialog_voice5", IDR_DIALOG_VOICE5);

	audio->RegisterBank("dialog_voice", { "dialog_voice1", "dialog_voice2", "dialog_voice3", "dialog_voice4", "dialog_voice5" });

	audio->Load("power_up1", IDR_POWERUP1);
	audio->Load("power_up2", IDR_POWERUP2);
	audio->Load("power_up3", IDR_POWERUP3);

	audio->RegisterBank("power_up", { "power_up1", "power_up2", "power_up3" });

	audio->Load("checkpoint", IDR_CHECKPOINT);

	//load maps
	audio->Load("bgm_tutorial", IDR_BGM_TUTORIAL);
	audio->Load("bgm_secret", IDR_BGM_SECRET);
	audio->Load("bgm_boss", IDR_BGM_BOSS);
	audio->Load("bgm_arcade", IDR_BGM_ARCADE);

	auto app = DX9GF::Application::GetInstance();
	DX9GF::Font::AddFont(L"assets/arcade-among-2-r46pv.ttf");
	DX9GF::Font::AddFont(L"assets/statusplz.ttf");
	//Load cursor textures
	auto input = DX9GF::InputManager::GetInstance();
	auto gd = GetGraphicsDevice();

	//hotspot (hX, hY) must be multiplied by the scale factor
	input->AddCursor(DX9GF::InputManager::CursorType::CURSOR, gd, L"assets/cursor.png", 0.2f, 0.0f, 0.0f);
	input->AddCursor(DX9GF::InputManager::CursorType::POINTER, gd, L"assets/pointer.png", 0.2f, 0.0f, 0.0f);
	input->AddCursor(DX9GF::InputManager::CursorType::CLICK, gd, L"assets/click.png", 0.2f, 4.0f, 6.0f);
	input->AddCursor(DX9GF::InputManager::CursorType::GRAB, gd, L"assets/grab.png", 0.2f, 14.8f, 14.8);
	input->AddCursor(DX9GF::InputManager::CursorType::TEXTSELECT, gd, L"assets/text-select.png", 0.2f, 10.0f, 16.0f);
#ifdef TESTING
	this->sceneManager->PushScene(new DebugScene(this, app->GetScreenWidth(), app->GetScreenHeight()));
#else
	// this->sceneManager->PushScene(new World1Scene(this, app->GetScreenWidth(), app->GetScreenHeight()));
	this->sceneManager->PushScene(new SplashScene(this, app->GetScreenWidth(), app->GetScreenHeight()));

#endif
	this->sceneManager->GoToNext();
}

void Demo::Game::Update(unsigned long long deltaTime)
{
	IGame::Update(deltaTime);

	fpsAccumulator += deltaTime;
	fpsFrames++;

	if (fpsAccumulator >= 1000) {
		float msPerFrame = (float)fpsAccumulator / fpsFrames;
		char buf[128];
		sprintf_s(buf, "Demo - FPS: %d (%.2f ms/frame)", fpsFrames, msPerFrame);
		SetWindowTextA(GetHwnd(), buf);

		fpsAccumulator = 0;
		fpsFrames = 0;
	}
}
