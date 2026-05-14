#include "pch.h"
#include "DebugScene.h"
#include "TestBattleScene.h"

Demo::DebugScene::DebugScene(Game* game, int sw, int sh) : IScene(sw, sh), game(game), uiCamera(sw, sh)
{
	uiCamera.SetPosition(sw / 2.0f, sh / 2.0f);
}

void Demo::DebugScene::Init()
{
	draggableManager = std::make_shared<DraggableManager>();
	transformManager = std::make_shared<DX9GF::TransformManager>();
	player = std::make_shared<Player>(transformManager);
	player->Init(game->GetGraphicsDevice(), &colliderManager, &camera);
	draggables.push_back(std::make_shared<IDraggable>(transformManager, 50, 70));
	draggables.back()->Init(draggableManager, game->GetGraphicsDevice(), &camera);
	draggables.push_back(std::make_shared<IDraggable>(transformManager, 75, 90, 90, 100));
	draggables.back()->Init(draggableManager, game->GetGraphicsDevice(), &camera);
	draggables.push_back(std::make_shared<IDraggable>(transformManager, 25, 60, -90, -100));
	draggables.back()->Init(draggableManager, game->GetGraphicsDevice(), &camera);
	containers.push_back(std::make_shared<IContainer>(transformManager, 100, 30, -100, -100));
	containers.back()->Init(draggableManager, game->GetGraphicsDevice(), &camera);

	//just load resource (texture, font) 1 time and reuse it all over again
	uiSheetTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	uiSheetTex->LoadTexture(L"ui-pack.png");
	//Everything works fine with .png, but .bmp is causing coordinate issues. Idk how to fix bruh
	myFont = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"Arcade Among 2 R46PV", 24);
	myFontSprite = std::make_shared<DX9GF::FontSprite>(myFont.get());

	//create 1 text button and 2 icon buttons
	auto btnTextExit = std::make_shared<Demo::TextButton>(
		transformManager,
		0, 0, 150, 50, //x,y,w,h
		"EXIT GAME", //display text
		myFont.get(), //reuse myFont
		[](DX9GF::ITrigger* t)
		{
			PostQuitMessage(0);
		}
	);
	btnTextExit->Init(&uiCamera);
	uiButtons.push_back(btnTextExit);

	// Test TextButton setter
	btnTextExit->SetBackgroundColors(D3DXCOLOR(0.8f, 0, 0, 1), D3DXCOLOR(1, 0, 0, 1), D3DXCOLOR(0, 1, 0, 1))
		->SetTextColors(0xFFFFA500, 0xFF000000, 0xFFFFFFFF);

	btnTextExit->Init(&camera);
	uiButtons.push_back(btnTextExit);

	auto quitBtn = std::make_shared<Demo::IconButton>(transformManager, 200, 200, 62, 30, uiSheetTex, 3);
	quitBtn->SetSpriteCoords(385, 449, 62, 30, 2);
	quitBtn->SetOnReleaseLeft([](DX9GF::ITrigger* t) { PostQuitMessage(0); });

	auto continueBtn = std::make_shared<Demo::IconButton>(transformManager, 150, 150, 94, 30, uiSheetTex, 3);
	continueBtn->SetSpriteCoords(577, 193, 94, 30, 34);

	continueBtn->SetOnReleaseLeft([this](DX9GF::ITrigger* t) {
		auto [sw, sh] = this->camera.GetScreenResolution();
		auto sceMan = this->game->GetSceneManager();
		this->game->GetSceneManager()->PushScene(
          new Demo::CardShop(this->game, this->player.get(), sw, sh, ShopTier::BASIC)
		);
		sceMan->GoToNext();

		});

	continueBtn->Init(&camera);
	uiButtons.push_back(continueBtn);

	//test function
	//continueBtn->ChangeSpriteCoords(500, 2, 60, 32, 20);

	transformManager->RebuildHierarchy();
	DX9GF::ITrigger::drawTrigger = true;
	DX9GF::ICollider::drawCollider = true;
}

void Demo::DebugScene::Update(unsigned long long deltaTime)
{
	auto inpMan = DX9GF::InputManager::GetInstance();
	inpMan->ReadMouse(deltaTime);
	inpMan->ReadKeyboard(deltaTime);
	if (!commandBuffer.IsBusy()) {
		uiCamera.Update();
		if (inpMan->KeyDown(DIK_SPACE)) {
			auto [w, h] = camera.GetScreenResolution();
			activeConversation = std::make_shared<IConversation>(myFontSprite, w, h);

			activeConversation->AddLine({ std::nullopt, std::nullopt, L"Player", L"Bấm chuột trái để qua câu nha!" });
			activeConversation->AddLine({ std::nullopt, std::nullopt, L"Chó", L"Gâu Gâu!" });

			commandBuffer.PushCommand(activeConversation);
		}
		auto inputChars = Demo::TextInputManager::GetInstance()->ReadInput();
		for (char c : inputChars) {
			if (c == '\b') {
				if (typedText.length() > 16) {
					typedText.pop_back();
				}
			}
			else {
				typedText += c;
			}
		}
		if (inpMan->KeyDown(DIK_F2)) {
			DX9GF::ITrigger::drawTrigger = !DX9GF::ITrigger::drawTrigger;
		}
		if (inpMan->KeyDown(DIK_F5)) {
			auto app = DX9GF::Application::GetInstance();
			auto sceMan = game->GetSceneManager();
			sceMan->PushScene(new TestBattleScene(game, player, app->GetScreenWidth(), app->GetScreenHeight()));
			sceMan->GoToNext();
			return;
		}

		if (inpMan->MousePress(DX9GF::InputManager::MouseButton::Middle)) {
			auto dX = inpMan->GetRelativeMouseX();
			auto dY = inpMan->GetRelativeMouseY();
			auto camPos = camera.GetPosition();
			camera.SetPosition(camPos.x - dX, camPos.y - dY);
		}
		auto dZ = inpMan->GetMouseScroll();
		auto camZoom = camera.GetZoom();
		camera.SetZoom(camZoom + dZ / static_cast<float>(1000));
		draggableManager->Update(deltaTime);
		transformManager->UpdateAll();

		//update all UI button
		for (auto& btn : uiButtons) {
			btn->Update(deltaTime);
		}
	}

	commandBuffer.Update(deltaTime);
	transformManager->UpdateAll();
	camera.Update();
}

void Demo::DebugScene::Draw(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	gd->Clear();
	if (SUCCEEDED(gd->BeginDraw())) {
		draggableManager->Draw(deltaTime);

		//draw all UI button
		for (auto& btn : uiButtons)
		{
			btn->Draw(gd, deltaTime);
		}

		if (myFontSprite) {
			myFontSprite->Begin();
			myFontSprite->SetPosition(20.0f, 50.0f);
			myFontSprite->SetColor(0xFF00FF00);
			myFontSprite->SetText(std::wstring(typedText.begin(), typedText.end()));
			myFontSprite->Draw(uiCamera, deltaTime);
			myFontSprite->End();
		}

		if (activeConversation && !activeConversation->IsFinished()) {
			activeConversation->Draw(gd, deltaTime);
		}

		gd->EndDraw();
	}
	gd->Present();
}