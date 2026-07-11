#include "pch.h"
#include "MimicEnemy.h"
#include "resource.h"
#include "SpiralProjectile.h"
#include "BoomerangProjectile.h"
#include "RNG.h"

void Demo::MimicEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"assets/notresponding-Sheet.png");
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 12);
	sprite->SetOrigin(32, 32);
	sprite->SetScale(2.f, 2.f);

	projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	projTexture->LoadTexture(L"assets/xprojectile.png");

   SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
	InitCardSpawnTrigger(camera, 128.f, 128.f);
}

void Demo::MimicEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) {
	if (sprite) {
		sprite->Begin();
		auto [x, y] = GetWorldPosition();
		sprite->SetPosition(x, y);
		sprite->Draw(*camera, deltaTime);
		sprite->End();
	}
	IEnemy::Draw(graphicsDevice, camera, deltaTime);
}

int Demo::MimicEnemy::GetRandomPattern() {
	return RNG::Range(1, 2);
}

void Demo::MimicEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	this->player = player;
	float projDamage = GetOutgoingDamage(4.f);

	int patternId = GetSmartRandomPattern(1, 2);
	//PatternJunkVomit(projDamage, enemies);
	if (patternId == 1) PatternCoinCyclone(projDamage, enemies);
	else PatternJunkVomit(projDamage, enemies);

	commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(4.f));
}

void Demo::MimicEnemy::PatternCoinCyclone(float projDamage, std::vector<std::shared_ptr<IEnemy>>* enemies) {
	bool isAlone = enemies->size() == 1;
	const int ATTACK_COUNT = isAlone ? 10 : 5;
	const int RADICAL_SPEED = isAlone ? 100.f : 150.f;
	const int ANGULAR_SPEED = isAlone ? 1.2f : 0.6f;
	auto attack = std::make_shared<DX9GF::CustomCommand>([this, projDamage, RADICAL_SPEED, ANGULAR_SPEED](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			for (int i = 0; i < 12; i++) {
				float angle = i * (3.14159f * 2.f / 12.f);
			auto projSprite = std::make_shared<DX9GF::StaticSprite>(projTexture.get());
			projSprite->SetOrigin(8, 8);
			projectiles.push_back(
				SpiralProjectile::Builder(transformManager, lock, projSprite, 16, 16, 256.f, 0)
					.SetSpiralParams(angle, RADICAL_SPEED, ANGULAR_SPEED)
					.SetDelay(i * 0.05f)
					.SetDecayTime(6.f)
					.SetDamage(projDamage)
					.Build()
				);
				projectiles.back()->Init();
			}
			transformManager.lock()->RebuildHierarchy();
		}
		markFinished();
	});
	for (int i = 0; i < ATTACK_COUNT; i++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*attack));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(1.f));
	}
}

void Demo::MimicEnemy::PatternJunkVomit(float projDamage, std::vector<std::shared_ptr<IEnemy>>* enemies) {
	bool isAlone = enemies->size() == 1;
	const int JUNK_COUNT = isAlone ? 100 : 50;
	const float ATTACK_DELAY = isAlone ? 0.05f : 0.1f;

	for (int i = 0; i < JUNK_COUNT; i++) {
		float randY = RNG::Range(-196.f, 196.f);
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, randY](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [px, py] = lock->GetWorldPosition();
			auto projSprite = std::make_shared<DX9GF::StaticSprite>(projTexture.get());
			projSprite->SetOrigin(8, 8);
			projectiles.push_back(
				BoomerangProjectile::Builder(transformManager, lock, projSprite, 16, 16, 512.f, 0.f)
					.SetTargetPosition(px, py + randY)

					.SetInitialVelocity(450.f)
					.SetReturnAcceleration(150.f)

					.SetDelay(0.f)
					.SetDecayTime(6.f)
					.SetDamage(projDamage)
					.Build()
				);
				projectiles.back()->Init();
				transformManager.lock()->RebuildHierarchy();
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(ATTACK_DELAY));
	}
}