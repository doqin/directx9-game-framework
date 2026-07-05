#include "pch.h"
#include "MainScene.h"
#include <DX9GFUtils.h>
#include "DX9GFGraphicsDevice.h"
#include "DX9GFSceneManager.h"
#include "resource.h"

void MainScene::Init()
{
	inputManager = DX9GF::InputManager::GetInstance();
	audioManager = DX9GF::AudioManager::GetInstance();
	audioManager->Load("ALOVU", IDR_WAVE_ALOVU);
	transformManager = std::make_shared<DX9GF::TransformManager>();
	colliderManager = std::make_shared<DX9GF::ColliderManager>();
	map = std::make_shared<DX9GF::Map>(game->GetGraphicsDevice());
	map->Create(transformManager, colliderManager, "./Resources/map.tmx");
	fontArial = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"Arial", 32);
	whiteTexture = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	whiteTexture->CreatePlainTexture(0xFFFFFFFF, 500, 500);
	dawgTexture = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	dawgTexture->LoadTexture(IDR_RCDATA1, 100, 100);
	fontSpriteArial = std::make_shared<DX9GF::FontSprite>(fontArial.get());
	fontSpriteArial->SetOrigin(16, 16);
	fontSpriteArial->SetPosition(64, -64);
	fontSpriteArial->SetText(L"Hello world!");

	DX9GF::Font::AddFont(L"./Resources/detemination.ttf");
	fontdetemination = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"Determination", 32);
	fontSpritedetemination = std::make_shared<DX9GF::FontSprite>(fontdetemination.get());
	fontSpritedetemination->SetOrigin(16, 16);
	fontSpritedetemination->SetPosition(64, -128);
	fontSpritedetemination->SetText(L"Hello world!!!!!");

	DX9GF::Debug::Init(game->GetGraphicsDevice());

	players.push_back(std::make_shared<GO::Player>(transformManager));
	for (auto& player : players) {
		player->Init(game->GetGraphicsDevice(), &commandBuffer, &camera, colliderManager);
	}
	rect = std::make_shared<GO::Rectangle>(transformManager, 64, 128, 64, 128);
	rect->Init(game->GetGraphicsDevice(), &camera, colliderManager);
	colorRec = std::make_shared<DX9GF::StaticSprite>(whiteTexture.get());
	textureRec = std::make_shared<DX9GF::StaticSprite>(dawgTexture.get());
	auto app = DX9GF::Application::GetInstance();
	transformManager->RebuildHierarchy();
}

void MainScene::Update(unsigned long long deltaTime)
{
	inputManager->ReadKeyboard(deltaTime);
	inputManager->ReadMouse(deltaTime);
	if (inputManager->KeyDown(DIK_ESCAPE)) PostMessage(game->GetHwnd(), WM_DESTROY, 0, 0);
	if (!commandBuffer.IsBusy()) {
		if (inputManager->KeyDown(DIK_F)) game->GetSceneManager()->GoToNext();
		if (inputManager->KeyDown(DIK_C)) {
			players.push_back(std::make_shared<GO::Player>(transformManager));
			players.back()->Init(game->GetGraphicsDevice(), &commandBuffer, &camera, colliderManager);
			transformManager->RebuildHierarchy();
		}

		if (inputManager->KeyDown(DIK_X)) {
			auto originalCamPos = camera.GetPosition();
			float originalZoom = camera.GetZoom();
			float originalRectRotation = rect->GetLocalRotation();

			commandBuffer.PushCommand(std::make_shared<DX9GF::SetCameraPositionCommand>(&camera, 0.0f, 0.0f));
			commandBuffer.StackCommand(std::make_shared<DX9GF::ZoomCameraCommand>(&camera, 0.5f, 2.0f));
			commandBuffer.PushCommand(std::make_shared<DX9GF::PlaySoundCommand>("ALOVU"));
			commandBuffer.PushCommand(std::make_shared<DX9GF::SetCameraPositionCommand>(&camera, rect->GetWorldX(), rect->GetWorldY()));
			commandBuffer.StackCommand(std::make_shared<DX9GF::ZoomCameraCommand>(&camera, 3.0f, 3.0f));
			commandBuffer.StackCommand(std::make_shared<DX9GF::RotateCommand>(rect, originalRectRotation + 720.0f, 360.0f));
			commandBuffer.StackCommand(std::make_shared<DX9GF::PlaySoundCommand>("ALOVU"));
			commandBuffer.PushCommand(std::make_shared<DX9GF::SetCameraPositionCommand>(&camera, originalCamPos.x, originalCamPos.y));
			commandBuffer.StackCommand(std::make_shared<DX9GF::ZoomCameraCommand>(&camera, originalZoom, 2.0f));
			commandBuffer.StackCommand(std::make_shared<DX9GF::RotateCommand>(rect, originalRectRotation, 360.0f));
		}

		float cameraXDir = 0;
		float cameraYDir = 0;
		const float cameraVelocity = 200;
		if (inputManager->KeyPress(DIK_LEFT)) cameraXDir -= 1;
		if (inputManager->KeyPress(DIK_RIGHT)) cameraXDir += 1;
		if (inputManager->KeyPress(DIK_UP)) cameraYDir -= 1;
		if (inputManager->KeyPress(DIK_DOWN)) cameraYDir += 1;
		if (inputManager->KeyDown(DIK_SPACE)) audioManager->Play("ALOVU");
		if (inputManager->KeyPress(DIK_LCONTROL)) {
			if (inputManager->KeyDown(DIK_S)) {
				game->GetSaveManager()->Save("./Save/temp.sav");
			}
			inputManager->ConsumeKey(DIK_S);
		}

		if (cameraXDir != 0 || cameraYDir != 0) {
			auto cameraPos = camera.GetPosition();
			cameraPos.x += cameraXDir * cameraVelocity * deltaTime / 1000;
			cameraPos.y += cameraYDir * cameraVelocity * deltaTime / 1000;
			camera.SetPosition(cameraPos);
		}
		if (inputManager->KeyDown(DIK_F1))  DX9GF::ICollider::drawCollider = !DX9GF::ICollider::drawCollider;
		if (inputManager->KeyDown(DIK_F2))  DX9GF::ITrigger::drawTrigger = !DX9GF::ITrigger::drawTrigger;
		if (inputManager->KeyDown(DIK_F3))  DX9GF::Debug::drawGrid = !DX9GF::Debug::drawGrid;
		if (inputManager->KeyDown(DIK_F4))  DX9GF::Debug::drawAxis = !DX9GF::Debug::drawAxis;
		if (inputManager->MousePress(DX9GF::InputManager::MouseButton::Middle)) {
			auto [dX, dY] = inputManager->GetRelativeMousePos();
			auto cameraPos = camera.GetPosition();
			cameraPos.x -= dX;
			cameraPos.y -= dY;
			camera.SetPosition(cameraPos);
		}

		auto dScroll = inputManager->GetMouseScroll();
		camera.SetZoom(camera.GetZoom() + dScroll / static_cast<float>(1000));
		rect->Update(deltaTime);
	}
	fontSpriteArial->Rotate(D3DXToRadian(0.1f * deltaTime));
	// Using task flow because I can't create a parallel programming system better than them (T_T)
	tf::Executor executor;
	tf::Taskflow taskflow;
	taskflow.for_each(players.begin(), players.end(), [deltaTime](std::shared_ptr<GO::Player> player) {
		player->Update(deltaTime);
	});
	executor.run(taskflow).wait();

	if (!players.empty()) {
		auto mainPlayer = players[0];
		float px = mainPlayer->GetWorldX();
		float py = mainPlayer->GetWorldY();

		float dx = px - triggerX;
		float dy = py - triggerY;
		bool isInside = (dx * dx + dy * dy) <= (triggerRadius * triggerRadius);
		if (isInside && !wasInTrigger) {

			auto originalCamPos = camera.GetPosition();
			float originalZoom = camera.GetZoom();
			float originalRectRotation = rect->GetLocalRotation();
			float chestX = rect->GetWorldX();
			float chestY = rect->GetWorldY();

			// Cutscene
			commandBuffer.PushCommand(std::make_shared<DX9GF::SetCameraPositionCommand>(&camera, chestX, chestY));
			commandBuffer.PushCommand(std::make_shared<DX9GF::ZoomCameraCommand>(&camera, 2.0f, 1.5f));
			commandBuffer.PushCommand(std::make_shared<DX9GF::RotateCommand>(rect, rect->GetLocalRotation() + 360.0f, 180.0f));
			commandBuffer.PushCommand(std::make_shared<DX9GF::ZoomCameraCommand>(&camera, originalZoom, 2.0f));
			commandBuffer.PushCommand(std::make_shared<DX9GF::RotateCommand>(rect, originalRectRotation, 180.0f));
			commandBuffer.PushCommand(std::make_shared<DX9GF::SetCameraPositionCommand>(&camera, originalCamPos.x, originalCamPos.y));
		}

		wasInTrigger = isInside;
	}

	camera.Update();
	commandBuffer.Update(deltaTime);
	for (auto& player : players) {
		if (player && player->GetState() == DX9GF::IGameObject::State::Destroyed) {
			auto it = std::find(players.begin(), players.end(), player);
			players.erase(it);
		}
	}
	transformManager->UpdateAll();
}

void MainScene::Dispose()
{
}

std::string MainScene::GetSaveID() const
{
	return "MainScene";
}

void MainScene::GenerateSaveData(nlohmann::json& outData)
{
	std::vector<nlohmann::json> playerData;
	for (auto& player : players) {
		nlohmann::json data;
		player->GenerateSaveData(data);
		playerData.push_back(data);
	}
	outData["playerData"] = playerData;
	nlohmann::json cameraData;
	auto cameraPosition = camera.GetPosition();
	cameraData["position"] = { cameraPosition.x, cameraPosition.y };
	cameraData["zoom"] = camera.GetZoom();
	outData["cameraData"] = cameraData;
}

void MainScene::RestoreSaveData(const nlohmann::json& inData)
{
	players.clear();
	if (auto playerData = inData["playerData"]; playerData.is_array()) {
		for (auto& data : playerData) {
			players.push_back(std::make_shared<GO::Player>(transformManager));
			players.back()->Init(game->GetGraphicsDevice(), &commandBuffer, &camera, colliderManager);
			players.back()->RestoreSaveData(data);
		}
		transformManager->RebuildHierarchy();
	}
	if (inData.contains("cameraData")) {
		auto& cameraData = inData["cameraData"];
		auto& cameraPosition = cameraData["position"];
		auto& cameraZoom = cameraData["zoom"];
		camera.SetPosition(cameraPosition[0], cameraPosition[1]);
		camera.SetZoom(cameraZoom);
	}
}
void MainScene::DrawUI(unsigned long long deltaTime)
{
}
void MainScene::DrawWorld(unsigned long long deltaTime)
{
	auto dev = game->GetGraphicsDevice();

	if (SUCCEEDED(dev->BeginDraw())) {
		auto app = DX9GF::Application::GetInstance();
		auto width = app->GetScreenWidth();
		auto height = app->GetScreenHeight();
		map->Draw(camera);
		if(DX9GF::Debug::drawGrid){
			DX9GF::Debug::DrawGrid(
				dev,
				camera,
				0,
				0,
				width,
				height,
				32,
				32,
				0xFFFF0000);
		}
		if (DX9GF::Debug::drawAxis) {
			DX9GF::Debug::DrawAxis(camera, 3000, 32, 0xFFFF0000);
		}
		textureRec->Begin();
		textureRec->Draw(camera, deltaTime);
		textureRec->End();
		rect->Draw(deltaTime);
		for (auto& collider : map->GetColliders()) {
			collider->Draw(game->GetGraphicsDevice(), camera);
		}
		dev->DrawEllipse(
			camera,
			170.0f, 170.0f,
			40.0f, 40.0f,  
			0.0f,         
			1.0f, 1.0f,    
			20.0f, 20.0f,   
			0x880000FF,   
			true
		);
		for (auto& player : players) player->Draw(deltaTime);
		fontSpriteArial->Begin();
		fontSpriteArial->Draw(camera, deltaTime);
		fontSpriteArial->End();

		fontSpritedetemination->Begin();
		fontSpritedetemination->Draw(camera, deltaTime);
		fontSpritedetemination->End();

		dev->EndDraw();
	}

	dev->Present();
}
