#include "pch.h"
#include "SubScene.h"
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include <utility>

void SubScene::Init()
{
	inputManager = DX9GF::InputManager::GetInstance();
	auto app = DX9GF::Application::GetInstance();
	auto graphicsDevice = game->GetGraphicsDevice();
	transformManager = std::make_shared<DX9GF::TransformManager>();

	rectTrigger1 = std::make_shared<DX9GF::RectangleTrigger>(transformManager, 150, 150, 200, 200);
	rectTrigger1->Init(&camera);
	rectTrigger1->SetOriginCenter();
	rectTrigger1->SetOnHeldLeft([](DX9GF::ITrigger* t) {
		auto input = DX9GF::InputManager::GetInstance();
		t->SetLocalPosition(t->GetLocalX() + input->GetRelativeMouseX(), t->GetLocalY() + input->GetRelativeMouseY());
		});

	rectTrigger2 = std::make_shared<DX9GF::RectangleTrigger>(transformManager, 50, 200, 200, 200);
	rectTrigger2->Init(&camera);
	rectTrigger2->SetOriginCenter();
	rectTrigger2->SetOnHeldLeft([](DX9GF::ITrigger* t) {
		auto input = DX9GF::InputManager::GetInstance();
		t->SetLocalPosition(t->GetLocalX() + input->GetRelativeMouseX(), t->GetLocalY() + input->GetRelativeMouseY());
		});

	colliderManager = std::make_shared<DX9GF::ColliderManager>();

	rects.push_back(std::make_shared<GO::Rectangle>(transformManager, 100, 100, -200, 200));
	rects.push_back(std::make_shared<GO::Rectangle>(transformManager, 50, 200, -200, 60));
	for (auto& rect : rects) {
		rect->Init(game->GetGraphicsDevice(), &camera, colliderManager);
	}

	ellipses.push_back(std::make_shared<GO::Ellipse>(transformManager, 100, 100, 0, 0));
	ellipses.push_back(std::make_shared<GO::Ellipse>(transformManager, 200, 50, 0, -200));
	for (auto& ellipse : ellipses) {
		ellipse->Init(game->GetGraphicsDevice(), &camera, colliderManager);
	}

	transformManager->RebuildHierarchy();
}

void SubScene::Update(unsigned long long deltaTime)
{
	inputManager->ReadKeyboard(deltaTime);
	inputManager->ReadMouse(deltaTime);
	if (inputManager->KeyDown(DIK_F)) {
		game->GetSceneManager()->GoToPrevious();
		return;
	}
	if (inputManager->KeyPress(DIK_LCONTROL) && inputManager->KeyDown(DIK_S)) {
		game->GetSaveManager()->Save("./Save/temp.sav");
	}
	if (rectTrigger2) rectTrigger2->Update(deltaTime);
	if (rectTrigger1) rectTrigger1->Update(deltaTime);
	for (auto& rect : rects) {
		rect->Update(deltaTime);
	}
	for (auto& ellipse : ellipses) {
		ellipse->Update(deltaTime);
	}
	transformManager->UpdateAll();
	camera.Update();
}

void SubScene::DrawWorld(unsigned long long deltaTime)
{
	auto dev = game->GetGraphicsDevice();
	if (SUCCEEDED(dev->BeginDraw())) {
		auto app = DX9GF::Application::GetInstance();
		auto width = app->GetScreenWidth();
		auto height = app->GetScreenHeight();
		DX9GF::Debug::DrawGrid(
			dev,
			0,
			0,
			width,
			height,
			32,
			32,
			0xFFFF0000);
		for (auto& rect : rects) {
			rect->Draw(deltaTime);
		}
		for (auto& ellipse : ellipses) {
			ellipse->Draw(deltaTime);
		}

		dev->DrawRectangle(camera,
			rectTrigger1->GetWorldX(), rectTrigger1->GetWorldY(),
			rectTrigger1->GetWidth(), rectTrigger1->GetHeight(),
			rectTrigger1->GetWorldRotation(), rectTrigger1->GetWorldScaleX(), rectTrigger1->GetWorldScaleY(),
			rectTrigger1->GetOriginX(), rectTrigger1->GetOriginY(),
			0xAA00FF00, false);

		dev->DrawRectangle(camera,
			rectTrigger2->GetWorldX(), rectTrigger2->GetWorldY(),
			rectTrigger2->GetWidth(), rectTrigger2->GetHeight(),
			rectTrigger2->GetWorldRotation(), rectTrigger2->GetWorldScaleX(), rectTrigger2->GetWorldScaleY(),
			rectTrigger2->GetOriginX(), rectTrigger2->GetOriginY(),
			0xAA0000FF, false);
		dev->EndDraw();
	}
	dev->Present();
}
void SubScene::DrawUI(unsigned long long deltaTime)
{
}

void SubScene::Dispose()
{
}

std::string SubScene::GetSaveID() const
{
	return "SubScene";
}

void SubScene::GenerateSaveData(nlohmann::json& outData)
{
}

void SubScene::RestoreSaveData(const nlohmann::json& inData)
{
}
