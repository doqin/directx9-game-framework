#include "pch.h"
#include "Player.h"
#include "resource.h"
#include "DamageTextManager.h"
#include "AdvancedCards.h"
#include "StrikeCard.h"
#include "MainBlockCard.h"

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
	else if (id == "MainBlockCard") card = std::make_shared<MainBlockCard>(transformManager);

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

void Demo::Player::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::ColliderManager* colliderManager, DX9GF::Camera* camera) {
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
	walkingDown = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateFrames(128, 128, 32, 32, 4, 0));
	walkingUp = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateFrames(128, 128, 32, 32, 4, 4));
	walkingRight = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateFrames(128, 128, 32, 32, 4, 8));
	walkingLeft = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateFrames(128, 128, 32, 32, 4, 12));
	// Align sprites
	idleDown->SetOrigin(16, 16);
	idleUp->SetOrigin(16, 16);
	idleRight->SetOrigin(16, 16);
	idleLeft->SetOrigin(16, 16);
	walkingDown->SetOrigin(16, 16);
	walkingUp->SetOrigin(16, 16);
	walkingRight->SetOrigin(16, 16);
	walkingLeft->SetOrigin(16, 16);
	// Set framerate
	walkingDown->SetFrameRate(12);
	walkingUp->SetFrameRate(12);
	walkingRight->SetFrameRate(12);
	walkingLeft->SetFrameRate(12);
	// Create collider
	collider = std::make_shared<DX9GF::RectangleCollider>(transformManager, shared_from_this(), 8, 4, 0, 14);
	collider->SetOriginCenter();
	deck = { "StrikeCard", "StrikeCard", "StrikeCard" };
	this->colliderManager->Add(collider);
	inventoryItems.InitFixedInventory(10);
}

void Demo::Player::Update(unsigned long long deltaTime) {
	auto inpMan = DX9GF::InputManager::GetInstance();
	// Movement
	D3DXVECTOR2 dir{ 0, 0 };
	if (inpMan->KeyPress(DIK_D)) dir.x += 1;
	if (inpMan->KeyPress(DIK_A)) dir.x -= 1;
	if (inpMan->KeyPress(DIK_S)) dir.y += 1;
	if (inpMan->KeyPress(DIK_W)) dir.y -= 1;
	if (dir.x == 1) state = State::Right;
	if (dir.x == -1) state = State::Left;
	if (dir.y == 1) state = State::Down;
	if (dir.y == -1) state = State::Up;
	D3DXVECTOR2 dirNorm;
	D3DXVec2Normalize(&dirNorm, &dir);
	bool isRunning = false;
	if (inpMan->KeyPress(DIK_LSHIFT)) {
		isRunning = true;
		dirNorm.x *= SPRINT_MULTIPLIER;
		dirNorm.y *= SPRINT_MULTIPLIER;
		walkingDown->SetFrameRate(12 * SPRINT_MULTIPLIER);
		walkingUp->SetFrameRate(12 * SPRINT_MULTIPLIER);
		walkingRight->SetFrameRate(12 * SPRINT_MULTIPLIER);
		walkingLeft->SetFrameRate(12 * SPRINT_MULTIPLIER);
	}
	else {
		walkingDown->SetFrameRate(12);
		walkingUp->SetFrameRate(12);
		walkingRight->SetFrameRate(12);
		walkingLeft->SetFrameRate(12);
	}
	if (dirNorm.x != 0 || dirNorm.y != 0) isWalking = true;
	else {
		isWalking = false;
		walkingDown->SetFrame(0);
		walkingUp->SetFrame(0);
		walkingRight->SetFrame(0);
		walkingLeft->SetFrame(0);
	}
	// if (dirNorm.x == 0 && dirNorm.y == 0) return;
	float dX = dirNorm.x * VELOCITY * deltaTime / 1000.f;
	float dY = dirNorm.y * VELOCITY * deltaTime / 1000.f;
	auto [finalDX, finalDY] = colliderManager->GetSlidingDeltas(collider, dX, dY);
	auto [currentX, currentY] = GetLocalPosition();
	SetLocalPosition(currentX + finalDX, currentY + finalDY);
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
				const float maxSpeed = isRunning ? VELOCITY * SPRINT_MULTIPLIER : VELOCITY;
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

bool Demo::Player::TakeDamage(float damage)
{
	if (isInvincible) return IsDead();

  const float blockedDamage = (std::min)(temporaryDefense, damage);
	temporaryDefense = (std::max)(0.f, temporaryDefense - blockedDamage);
	const float actualDamage = (std::max)(0.f, damage - blockedDamage);

	health -= actualDamage;
	isInvincible = true;
	timeSinceTurnedInvincible = 0.f;
	if (health < 0) health = 0;
	auto [x, y] = GetWorldPosition();
	Demo::DamageTextManager::GetInstance()->Spawn(actualDamage, x, y - 16.0f, Demo::TextType::TakeDamage); //-16.0f from y so the text pops up from the head
	return IsDead();
}

void Demo::Player::DealDamage(IEnemy* target, float cardBaseDamage)
{
	if (!target) return;

	float currentAtk = GetBuffStat(ItemBuffType::BuffDamage);
	float finalDamage = cardBaseDamage + currentAtk;

	target->TakeDamage(finalDamage);
}

bool Demo::Player::IsDead() const
{
	return health <= 0;
}

void Demo::Player::SetHealth(float hp)
{
	health = hp;
}

float Demo::Player::GetMaxHealth() const
{
	return MAX_HEALTH;
}

float Demo::Player::GetHealth() const
{
	return health;
}

float Demo::Player::GetTemporaryDefense() const
{
	return temporaryDefense;
}

std::weak_ptr<DX9GF::RectangleCollider> Demo::Player::GetCollider()
{
	return collider;
}

void Demo::Player::Heal(float value)
{
	if (IsDead()) return;
	health += value;
	if (health > MAX_HEALTH) health = MAX_HEALTH;
}

float Demo::Player::GetBuffStat(ItemBuffType targetType) const
{
	float val = 0;
	for (const auto& buff : activeBuffs)
	{
		if (buff.type == targetType)
		{
			val += buff.value;
		}
	}
	return val;
}
void Demo::Player::UpdateBuffs()
{
	for (int i = activeBuffs.size() - 1; i >= 0; i--)
	{
		if (activeBuffs[i].isNewlyAdded)
		{
			activeBuffs[i].isNewlyAdded = false;
			continue;
		}

		activeBuffs[i].turnsLeft--;

		if (activeBuffs[i].turnsLeft <= 0)
		{
			activeBuffs.erase(activeBuffs.begin() + i);
		}
	}
   temporaryDefense = (std::max)(0.f, GetBuffStat(ItemBuffType::BuffDefense));
}
void Demo::Player::AddActiveBuff(const ActiveBuff& buff) {
	activeBuffs.push_back(buff);
   if (buff.type == ItemBuffType::BuffDefense) {
		temporaryDefense = (std::max)(0.f, GetBuffStat(ItemBuffType::BuffDefense));
	}
}