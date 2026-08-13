#include "pch.h"
#include "Player.h"
#include "resource.h"
#include "DamageTextManager.h"
#include "AdvancedCards.h"
#include "StrikeCard.h"
#include "EnergyCard.h"
#include "UtilityCards.h"
#include "FinisherCards.h"
#include "MainBlockCard.h"
#include "SettingsManager.h"
#include "IDraggable.h"
#include "IEnemy.h"

std::shared_ptr<Demo::ICard> Demo::ICard::CreateCard(const std::string& id, std::weak_ptr<DX9GF::TransformManager> transformManager, std::shared_ptr<DraggableManager> draggableManager, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	std::shared_ptr<ICard> card;
	if (id == "HeavyStrikeCard") card = std::make_shared<HeavyStrikeCard>(transformManager);
	else if (id == "TwinStrikeCard") card = std::make_shared<TwinStrikeCard>(transformManager);
	else if (id == "CleaveCard") card = std::make_shared<CleaveCard>(transformManager);
	else if (id == "ChainLightningCard") card = std::make_shared<ChainLightningCard>(transformManager);
	else if (id == "PoisonCard") card = std::make_shared<PoisonCard>(transformManager);
	else if (id == "VulnerableCard") card = std::make_shared<VulnerableCard>(transformManager);
	else if (id == "WeaknessCard") card = std::make_shared<WeaknessCard>(transformManager);
	else if (id == "StunCard") card = std::make_shared<StunCard>(transformManager);
	else if (id == "StrikeCard") card = std::make_shared<StrikeCard>(transformManager);
	else if (id == "EnergyCard") card = std::make_shared<EnergyCard>(transformManager);
	else if (id == "JabCard") card = std::make_shared<JabCard>(transformManager);
	else if (id == "MarkCard") card = std::make_shared<MarkCard>(transformManager);
	else if (id == "BraceCard") card = std::make_shared<BraceCard>(transformManager);
	else if (id == "PrefetchCard") card = std::make_shared<PrefetchCard>(transformManager);
	else if (id == "OverclockCard") card = std::make_shared<OverclockCard>(transformManager);
	else if (id == "JumpstartCard") card = std::make_shared<JumpstartCard>(transformManager);
	else if (id == "ForesightCard") card = std::make_shared<ForesightCard>(transformManager);
	else if (id == "TerminateCard") card = std::make_shared<TerminateCard>(transformManager);
	else if (id == "InfernoCard") card = std::make_shared<InfernoCard>(transformManager);
	else if (id == "SystemPurgeCard") card = std::make_shared<SystemPurgeCard>(transformManager);
	else if (id == "OverdriveCard") card = std::make_shared<OverdriveCard>(transformManager);
	else if (id == "MainBlockCard") card = std::make_shared<MainBlockCard>(transformManager);
	else if (id == "IgniteCard") card = std::make_shared<IgniteCard>(transformManager);
	else if (id == "FireDetonationCard") card = std::make_shared<FireDetonationCard>(transformManager);
	else if (id == "RagingStrikeCard") card = std::make_shared<RagingStrikeCard>(transformManager);
	else if (id == "OverloadCard") card = std::make_shared<OverloadCard>(transformManager);
	else if (id == "ChainReactionCard") card = std::make_shared<ChainReactionCard>(transformManager);
	else if (id == "LethalHarvestCard") card = std::make_shared<LethalHarvestCard>(transformManager);
	else if (id == "ShieldBashCard") card = std::make_shared<ShieldBashCard>(transformManager);
	else if (id == "CruelStrikeCard") card = std::make_shared<CruelStrikeCard>(transformManager);
	else if (id == "ArmorPiercerCard") card = std::make_shared<ArmorPiercerCard>(transformManager);
	else if (id == "ExecuteCard") card = std::make_shared<ExecuteCard>(transformManager);
	if (card && draggableManager && graphicsDevice && camera) {
		if (auto dragCard = std::dynamic_pointer_cast<IDraggable>(card)) {
			dragCard->Init(draggableManager, graphicsDevice, camera);
		}
	}
	return card;
}

std::string Demo::Player::GetSaveID() const {
	return "Player_Data";
}

void Demo::Player::GenerateSaveData(nlohmann::json& outData) {
	auto [x, y] = GetLocalPosition();

	outData["x"] = x;
	outData["y"] = y;
	outData["health"] = health;
}

void Demo::Player::RestoreSaveData(const nlohmann::json& inData) {
	auto [currentX, currentY] = GetLocalPosition();
	float savedX = currentX, savedY = currentY;

	if (inData.contains("x")) savedX = inData["x"];
	if (inData.contains("y")) savedY = inData["y"];
	if (inData.contains("health")) health = inData["health"];

	SetLocalPosition(savedX, savedY);
}

void Demo::Player::GenerateSaveGlobalData(nlohmann::json& outData) const {
	outData["gold"] = gold;
	outData["deck"] = nlohmann::json::array();
	for (auto& card : deck) {
		outData["deck"].push_back(card);
	}
	outData["inventoryCards"] = nlohmann::json::array();
	for (auto& card : inventoryCards) {
		outData["inventoryCards"].push_back(card);
	}
	auto inventorySlots = inventoryItems.GetSlots();
	for (size_t i = 0; i < inventorySlots.size(); i++) {
		outData["inventoryItems"][i]["id"] = inventorySlots[i].itemID;
		outData["inventoryItems"][i]["quantity"] = inventorySlots[i].quantity;
	}
}

void Demo::Player::RestoreSaveGlobalData(const nlohmann::json& inData) {
	if (inData.contains("gold")) gold = inData["gold"];
	if (inData.contains("deck")) {
		deck.clear();
		for (auto& item : inData["deck"]) {
			deck.push_back(item.get<std::string>());
		}
	}
	if (inData.contains("inventoryCards")) {
		inventoryCards.clear();
		for (auto& item : inData["inventoryCards"]) {
			inventoryCards.push_back(item.get<std::string>());
		}
	}
	if (inData.contains("inventoryItems")) {
		inventoryItems.Clear();
		for (auto& item : inData["inventoryItems"]) {
			int id = item["id"];
			int quantity = item["quantity"];
			inventoryItems.AddItem(id, quantity);
		}
	}
}

Demo::Player::~Player() {
	colliderManager->Remove(collider);
}

void Demo::Player::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::ColliderManager* colliderManager, DX9GF::Camera* camera, bool isBattling) {
	// External components
	this->graphicsDevice = graphicsDevice;
	this->colliderManager = colliderManager;
	this->camera = camera;
	// Create texture
	spritesheet = std::make_shared<DX9GF::Texture>(graphicsDevice);
	spritesheet->LoadTexture(IDB_PNG1);
	// Create sprites
	idleDown = std::make_shared<DX9GF::StaticSprite>(spritesheet.get());
	idleDown->SetSrcRect({ .left = 0, .top = 0, .right = 32, .bottom = 32 });
	idleUp = std::make_shared<DX9GF::StaticSprite>(spritesheet.get());
	idleUp->SetSrcRect({ .left = 0, .top = 32, .right = 32, .bottom = 64 });
	idleRight = std::make_shared<DX9GF::StaticSprite>(spritesheet.get());
	idleRight->SetSrcRect({ .left = 0, .top = 64, .right = 32, .bottom = 96 });
	idleLeft = std::make_shared<DX9GF::StaticSprite>(spritesheet.get());
	idleLeft->SetSrcRect({ .left = 0, .top = 96, .right = 32, .bottom = 128 });
	idleDownRight = std::make_shared<DX9GF::StaticSprite>(spritesheet.get());
	idleDownRight->SetSrcRect({ .left = 0, .top = 128, .right = 32, .bottom = 160 });
	idleUpRight = std::make_shared<DX9GF::StaticSprite>(spritesheet.get());
	idleUpRight->SetSrcRect({ .left = 0, .top = 160, .right = 32, .bottom = 192 });
	idleDownLeft = std::make_shared<DX9GF::StaticSprite>(spritesheet.get());
	idleDownLeft->SetSrcRect({ .left = 0, .top = 192, .right = 32, .bottom = 224 });
	idleUpLeft = std::make_shared<DX9GF::StaticSprite>(spritesheet.get());
	idleUpLeft->SetSrcRect({ .left = 0, .top = 224, .right = 32, .bottom = 256 });
	walkingDown = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateFrames(128, 128, 32, 32, 4, 0));
	walkingUp = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateFrames(128, 128, 32, 32, 4, 4));
	walkingRight = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateFrames(128, 128, 32, 32, 4, 8));
	walkingLeft = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateFrames(128, 128, 32, 32, 4, 12));
	walkingDownRight = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateFrames(128, 128, 32, 32, 4, 16));
	walkingUpRight = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateFrames(128, 128, 32, 32, 4, 20));
	walkingDownLeft = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateFrames(128, 128, 32, 32, 4, 24));
	walkingUpLeft = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateFrames(128, 128, 32, 32, 4, 28));
	// Align sprites
	idleDown->SetOrigin(16, 16);
	idleUp->SetOrigin(16, 16);
	idleRight->SetOrigin(16, 16);
	idleLeft->SetOrigin(16, 16);
	idleDownRight->SetOrigin(16, 16);
	idleUpRight->SetOrigin(16, 16);
	idleDownLeft->SetOrigin(16, 16);
	idleUpLeft->SetOrigin(16, 16);
	walkingDown->SetOrigin(16, 16);
	walkingUp->SetOrigin(16, 16);
	walkingRight->SetOrigin(16, 16);
	walkingLeft->SetOrigin(16, 16);
	walkingDownRight->SetOrigin(16, 16);
	walkingUpRight->SetOrigin(16, 16);
	walkingDownLeft->SetOrigin(16, 16);
	walkingUpLeft->SetOrigin(16, 16);
	// Set framerate
	walkingDown->SetFrameRate(12);
	walkingUp->SetFrameRate(12);
	walkingRight->SetFrameRate(12);
	walkingLeft->SetFrameRate(12);
	walkingDownRight->SetFrameRate(12);
	walkingUpRight->SetFrameRate(12);
	walkingDownLeft->SetFrameRate(12);
	walkingUpLeft->SetFrameRate(12);
	// Create collider
	collider = std::make_shared<DX9GF::RectangleCollider>(transformManager, shared_from_this(), 8, 4, 0, isBattling ? 6 : 14);
	collider->SetOriginCenter();
	// Create footprint particle emitter
	footprintTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	footprintTexture->CreatePlainTexture(D3DCOLOR_ARGB(160, 90, 70, 50), 4, 4);
	footprintEmitter = std::make_unique<DX9GF::ParticleSystem>(footprintTexture.get(), 32);
	footprintEmitter->SetOrigin(2, 2);
	DX9GF::ConfigureFootprintEmitter(*footprintEmitter);
	footprintsEnabled = !isBattling;
	footprintEmitter->SetEnabled(footprintsEnabled);
	deck = { "StrikeCard", "TwinStrikeCard", "IgniteCard", "FireDetonationCard", "RagingStrikeCard", "OverloadCard", "ChainReactionCard", "LethalHarvestCard", "ShieldBashCard", "CruelStrikeCard", "ArmorPiercerCard","WeaknessCard"};
	this->colliderManager->Add(collider);
	inventoryItems.InitFixedInventory(12);
}

void Demo::Player::Update(unsigned long long deltaTime) {
	auto inpMan = DX9GF::InputManager::GetInstance();
	auto sm = Demo::SettingsManager::GetInstance();
	int keyUp = sm->GetKeybind("MOVE_UP");
	int keyDown = sm->GetKeybind("MOVE_DOWN");
	int keyLeft = sm->GetKeybind("MOVE_LEFT");
	int keyRight = sm->GetKeybind("MOVE_RIGHT");
	// Movement
	D3DXVECTOR2 dir{ 0, 0 };
	if (inpMan->KeyPress(keyRight)) dir.x += 1;
	if (inpMan->KeyPress(keyLeft))  dir.x -= 1;
	if (inpMan->KeyPress(keyDown))  dir.y += 1;
	if (inpMan->KeyPress(keyUp))    dir.y -= 1;
	if (dir.x == 1) state = State::Right;
	if (dir.x == -1) state = State::Left;
	if (dir.y == 1) state = State::Down;
	if (dir.y == -1) state = State::Up;
	if (dir.x == 1 && dir.y == 1) state = State::DownRight;
	if (dir.x == 1 && dir.y == -1) state = State::UpRight;
	if (dir.x == -1 && dir.y == 1) state = State::DownLeft;
	if (dir.x == -1 && dir.y == -1) state = State::UpLeft;
	D3DXVECTOR2 dirNorm;
	D3DXVec2Normalize(&dirNorm, &dir);
	D3DXVECTOR2 moveDir = dirNorm; // unscaled direction, used for footprint placement
	bool isRunning = false;
	if (inpMan->KeyPress(SettingsManager::GetInstance()->GetKeybind("SPRINT"))) {
		isRunning = true;
		dirNorm.x *= SPRINT_MULTIPLIER;
		dirNorm.y *= SPRINT_MULTIPLIER;
		walkingDown->SetFrameRate(12 * SPRINT_MULTIPLIER);
		walkingUp->SetFrameRate(12 * SPRINT_MULTIPLIER);
		walkingRight->SetFrameRate(12 * SPRINT_MULTIPLIER);
		walkingLeft->SetFrameRate(12 * SPRINT_MULTIPLIER);
		walkingDownRight->SetFrameRate(12 * SPRINT_MULTIPLIER);
		walkingUpRight->SetFrameRate(12 * SPRINT_MULTIPLIER);
		walkingDownLeft->SetFrameRate(12 * SPRINT_MULTIPLIER);
		walkingUpLeft->SetFrameRate(12 * SPRINT_MULTIPLIER);
	}
	else {
		walkingDown->SetFrameRate(12);
		walkingUp->SetFrameRate(12);
		walkingRight->SetFrameRate(12);
		walkingLeft->SetFrameRate(12);
		walkingDownRight->SetFrameRate(12);
		walkingUpRight->SetFrameRate(12);
		walkingDownLeft->SetFrameRate(12);
		walkingUpLeft->SetFrameRate(12);
	}
	if (dirNorm.x != 0 || dirNorm.y != 0) isWalking = true;
	else {
		isWalking = false;
		walkingDown->SetFrame(0);
		walkingUp->SetFrame(0);
		walkingRight->SetFrame(0);
		walkingLeft->SetFrame(0);
		walkingDownRight->SetFrame(0);
		walkingUpRight->SetFrame(0);
		walkingDownLeft->SetFrame(0);
		walkingUpLeft->SetFrame(0);
	}
	if (currentSurface != baseSurface) {
		surfaceTimeout -= deltaTime / 1000.0f;
		if (surfaceTimeout <= 0) {
			currentSurface = baseSurface;
		}
	}

	if (isWalking) {
		float stepInterval = isRunning ? 0.25f : 0.4f;
		stepTimer -= deltaTime / 1000.0f;

		if (stepTimer <= 0) {
			std::string bankName = "step_" + currentSurface;
			DX9GF::AudioManager::GetInstance()->PlayRandom(bankName, 0.5f);
			stepTimer = stepInterval;
		}
	}
	else {
		stepTimer = 0.0f;
	}
	// if (dirNorm.x == 0 && dirNorm.y == 0) return;
	float speedMultiplier = 1.f - GetModifierValue(ModifierType::Freeze);
	speedMultiplier = (std::max)(0.f, speedMultiplier);
	float dX = dirNorm.x * VELOCITY * speedMultiplier * deltaTime / 1000.f;
	float dY = dirNorm.y * VELOCITY * speedMultiplier * deltaTime / 1000.f;
	auto [finalDX, finalDY] = colliderManager->GetSlidingDeltas(collider, dX, dY);
	auto [currentX, currentY] = GetLocalPosition();
	SetLocalPosition(currentX + finalDX, currentY + finalDY);
	// Footprint particles, spawned at the collider (feet) position, alternating left/right
	auto [colliderX, colliderY] = collider->GetWorldPosition();
	D3DXVECTOR2 perp{ -moveDir.y, moveDir.x };
	float footSign = nextFootLeft ? -1.f : 1.f;
	float footX = colliderX + perp.x * FOOTPRINT_OFFSET * footSign;
	float footY = colliderY + perp.y * FOOTPRINT_OFFSET * footSign;
	if (footprintEmitter->Update(deltaTime, footX, footY, 0.f, 1.f, 1.f, 0xFFFFFFFF, isWalking && footprintsEnabled)) {
		nextFootLeft = !nextFootLeft;
	}
	// Camera movement
	if (followCamera) {
		auto cameraPos = camera->GetPosition();
		auto [playerPosX, playerPosY] = GetLocalPosition();
		D3DXVECTOR2 vec{ playerPosX - cameraPos.x, playerPosY - cameraPos.y };
		const float EPSILON = 0.0001f;
		const float CAMERA_EASE_IN_TIME_MS = 220.f;
		const float CAMERA_EASE_OUT_DISTANCE = 64.f;
		auto smoothStep = [](float t) {
			t = (std::max)(0.f, (std::min)(1.f, t));
			return t * t * (3.f - 2.f * t);
			};

		const float distanceSq = vec.x * vec.x + vec.y * vec.y;
		if (distanceSq > EPSILON * EPSILON) {
			const float distance = std::sqrt(distanceSq);
			if (distance <= CAMERA_SNAP_MARGIN + EPSILON) {
				camera->SetPosition(playerPosX, playerPosY);
				cameraDeltaTime = 0.f;
			}
			else {
				cameraDeltaTime += deltaTime;
				const float maxSpeed = 3000 /*isRunning ? VELOCITY * SPRINT_MULTIPLIER : VELOCITY*/;
				const float easeIn = smoothStep(cameraDeltaTime / CAMERA_EASE_IN_TIME_MS);
				const float easeOut = smoothStep(distance / CAMERA_EASE_OUT_DISTANCE);
				const float easeFactor = easeIn * easeOut;
				const float stepDistance = (std::min)(distance, maxSpeed * (std::max)(0.05f, easeFactor) * deltaTime / 1000.f);

				const float invDistance = 1.f / distance;
				const float stepX = vec.x * invDistance * stepDistance;
				const float stepY = vec.y * invDistance * stepDistance;
				camera->SetPosition(cameraPos.x + stepX, cameraPos.y + stepY);
			}
		}
		else {
			cameraDeltaTime = 0.f;
		}
	}
	if (isInvincible) {
		if (timeSinceTurnedInvincible > INVINCIBILITY_DURATION) {
			isInvincible = false;
		}
		else {
			timeSinceTurnedInvincible += deltaTime / 1000.f;
		}
	}
}

void Demo::Player::Draw(unsigned long long deltaTime) {
	footprintEmitter->Draw(*camera, deltaTime);
	if (!isInvincible || static_cast<int>(timeSinceTurnedInvincible / BLINKING_DURATION) % 2) {
		switch (state) {
		case State::Down: {
			if (isWalking) {
				walkingDown->Begin();
				auto [x, y] = GetWorldPosition();
				walkingDown->SetPosition(x, y);
				walkingDown->Draw(*camera, deltaTime);
				walkingDown->End();
			}
			else {
				idleDown->Begin();
				auto [x, y] = GetWorldPosition();
				idleDown->SetPosition(x, y);
				idleDown->Draw(*camera, deltaTime);
				idleDown->End();
			}
		}
						break;
		case State::Up: {
			if (isWalking) {
				walkingUp->Begin();
				auto [x, y] = GetWorldPosition();
				walkingUp->SetPosition(x, y);
				walkingUp->Draw(*camera, deltaTime);
				walkingUp->End();
			}
			else {
				idleUp->Begin();
				auto [x, y] = GetWorldPosition();
				idleUp->SetPosition(x, y);
				idleUp->Draw(*camera, deltaTime);
				idleUp->End();
			}
		}
					  break;
		case State::Right: {
			if (isWalking) {
				walkingRight->Begin();
				auto [x, y] = GetWorldPosition();
				walkingRight->SetPosition(x, y);
				walkingRight->Draw(*camera, deltaTime);
				walkingRight->End();
			}
			else {
				idleRight->Begin();
				auto [x, y] = GetWorldPosition();
				idleRight->SetPosition(x, y);
				idleRight->Draw(*camera, deltaTime);
				idleRight->End();
			}
		}
						 break;
		case State::Left: {
			if (isWalking) {
				walkingLeft->Begin();
				auto [x, y] = GetWorldPosition();
				walkingLeft->SetPosition(x, y);
				walkingLeft->Draw(*camera, deltaTime);
				walkingLeft->End();
			}
			else {
				idleLeft->Begin();
				auto [x, y] = GetWorldPosition();
				idleLeft->SetPosition(x, y);
				idleLeft->Draw(*camera, deltaTime);
				idleLeft->End();
			}
		}
						break;
		case State::DownRight: {
			if (isWalking) {
				walkingDownRight->Begin();
				auto [x, y] = GetWorldPosition();
				walkingDownRight->SetPosition(x, y);
				walkingDownRight->Draw(*camera, deltaTime);
				walkingDownRight->End();
			}
			else {
				idleDownRight->Begin();
				auto [x, y] = GetWorldPosition();
				idleDownRight->SetPosition(x, y);
				idleDownRight->Draw(*camera, deltaTime);
				idleDownRight->End();
			}
		}
							 break;
		case State::UpRight: {
			if (isWalking) {
				walkingUpRight->Begin();
				auto [x, y] = GetWorldPosition();
				walkingUpRight->SetPosition(x, y);
				walkingUpRight->Draw(*camera, deltaTime);
				walkingUpRight->End();
			}
			else {
				idleUpRight->Begin();
				auto [x, y] = GetWorldPosition();
				idleUpRight->SetPosition(x, y);
				idleUpRight->Draw(*camera, deltaTime);
				idleUpRight->End();
			}
		}
						   break;
		case State::DownLeft: {
			if (isWalking) {
				walkingDownLeft->Begin();
				auto [x, y] = GetWorldPosition();
				walkingDownLeft->SetPosition(x, y);
				walkingDownLeft->Draw(*camera, deltaTime);
				walkingDownLeft->End();
			}
			else {
				idleDownLeft->Begin();
				auto [x, y] = GetWorldPosition();
				idleDownLeft->SetPosition(x, y);
				idleDownLeft->Draw(*camera, deltaTime);
				idleDownLeft->End();
			}
		}
							break;
		case State::UpLeft: {
			if (isWalking) {
				walkingUpLeft->Begin();
				auto [x, y] = GetWorldPosition();
				walkingUpLeft->SetPosition(x, y);
				walkingUpLeft->Draw(*camera, deltaTime);
				walkingUpLeft->End();
			}
			else {
				idleUpLeft->Begin();
				auto [x, y] = GetWorldPosition();
				idleUpLeft->SetPosition(x, y);
				idleUpLeft->Draw(*camera, deltaTime);
				idleUpLeft->End();
			}
		}
						  break;
		default:
			break;
		}
	}
	collider->Draw(graphicsDevice, *camera);
}

void Demo::Player::SetFollowCamera(bool followCamera)
{
	this->followCamera = followCamera;
}

float Demo::Player::GetVelocity() const
{
	return VELOCITY;
}

float Demo::Player::SetVelocity(float velocity)
{
	return this->VELOCITY = velocity;
}

bool Demo::Player::TakeDamage(float damage) {
	if (isInvincible) return IsDead();

	float actualDamage = CalculateActualDamage(damage);
	health -= actualDamage;

	isInvincible = true;
	timeSinceTurnedInvincible = 0.f;

	if (health <= 0) {
		if (health + actualDamage > 0) DX9GF::AudioManager::GetInstance()->Play("player_dead", false, 0.3f);
		health = 0;
	}
	if (actualDamage > 0) DX9GF::AudioManager::GetInstance()->PlayRandom("take_dmg", 0.8f);

	auto [x, y] = GetWorldPosition();
	Demo::DamageTextManager::GetInstance()->Spawn(actualDamage, x, y - 16.0f, Demo::TextType::TakeDamage);
	return IsDead();
}

bool Demo::Player::TakeIndirectDamage(float damage, DamageType type) {
	health -= damage;
	if (health < 0) {
		if (health + damage > 0) DX9GF::AudioManager::GetInstance()->Play("player_dead", false, 0.3f);
		health = 0;
	}

	if (damage > 0) {
		//TODO: change audio resource here
		if (type == DamageType::Poison) {
			DX9GF::AudioManager::GetInstance()->PlayRandom("take_dmg", 0.4f);
		}
		else if (type == DamageType::Burn) {
			DX9GF::AudioManager::GetInstance()->PlayRandom("take_dmg", 0.4f);
		}
		else {
			DX9GF::AudioManager::GetInstance()->PlayRandom("take_dmg", 0.4f);
		}
	}


	auto [x, y] = GetWorldPosition();
	Demo::DamageTextManager::GetInstance()->Spawn(damage, x, y - 16.0f, Demo::TextType::TakeDamage);

	return IsDead();
}

void Demo::Player::DealDamage(IEnemy* target, float cardBaseDamage) {
	if (!target) return;
	float finalDamage = CalculateOutgoingDamage(cardBaseDamage);
	target->TakeDamage(finalDamage);
}

std::weak_ptr<DX9GF::RectangleCollider> Demo::Player::GetCollider()
{
	return collider;
}

void Demo::Player::SetSurface(std::string surface)
{
	this->currentSurface = surface;
	this->surfaceTimeout = 0.1f;
}

void Demo::Player::SetFootprintsEnabled(bool enabled)
{
	this->footprintsEnabled = enabled;
	footprintEmitter->SetEnabled(enabled);
}
