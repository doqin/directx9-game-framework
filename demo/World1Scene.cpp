#include "pch.h"
#include "World1Scene.h"

void Demo::World1Scene::Init()
{
	transformManager = std::make_shared<DX9GF::TransformManager>();
	player = std::make_shared<Player>(transformManager);
	player->Init(game->GetGraphicsDevice(), &colliderManager, &camera);
	transformManager->RebuildHierarchy();
	camera.SetZoom(2.f);
}

void Demo::World1Scene::Update(unsigned long long deltaTime)
{
	auto inpMan = DX9GF::InputManager::GetInstance();
	inpMan->ReadMouse(deltaTime);
	inpMan->ReadKeyboard(deltaTime);
	if (inpMan->KeyDown(DIK_F1)) DX9GF::ICollider::drawCollider = !DX9GF::ICollider::drawCollider;
	if (inpMan->KeyDown(DIK_F2)) DX9GF::Debug::drawGrid = !DX9GF::Debug::drawGrid;
	player->Update(deltaTime);
	transformManager->UpdateAll();
	camera.Update();
}

void Demo::World1Scene::DrawWorld(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	if (SUCCEEDED(gd->BeginDraw())) {
		auto app = DX9GF::Application::GetInstance();
		if (DX9GF::Debug::drawGrid) {
			DX9GF::Debug::DrawGrid(game->GetGraphicsDevice(), camera, 0, 0, app->GetScreenWidth(), app->GetScreenHeight(), 16, 16, 0x88888888);
		}
		player->Draw(deltaTime);
		gd->EndDraw();
	}
}
void Demo::World1Scene::DrawUI(unsigned long long deltaTime)
{
}
