#include "pch.h"
#include "MimicEnemy.h"
#include "resource.h"
#include "SpiralProjectile.h"
#include "BoomerangProjectile.h"
#include <random>

void Demo::MimicEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"notresponding-Sheet.png");
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 12);
	sprite->SetOrigin(32, 32);
	sprite->SetScale(2.f, 2.f);

	projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	projTexture->LoadTexture(L"xprojectile.png");
	projSprite = std::make_shared<DX9GF::StaticSprite>(projTexture.get());
	projSprite->SetOrigin(8, 8);

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
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(1, 2);
	return dist(gen);
}

void Demo::MimicEnemy::StartAttack(std::shared_ptr<Player> player) {
	this->player = player;
	float projDamage = GetOutgoingDamage(4.f);

	if (GetRandomPattern() == 1) PatternCoinCyclone(projDamage);
	else PatternJunkVomit(projDamage);

	commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(4.f));
}

void Demo::MimicEnemy::PatternCoinCyclone(float projDamage) {
	auto attack = std::make_shared<DX9GF::CustomCommand>([this, projDamage](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			for (int i = 0; i < 12; i++) {
				float angle = i * (3.14159f * 2.f / 12.f);
				projectiles.push_back(
					SpiralProjectile::Builder(transformManager, lock, projSprite, 16, 16, 256.f, 0)
					.SetSpiralParams(angle, 100.f, 1.2f)
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
	for (int i = 0; i < 10; i++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*attack));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(1.f));
	}
}

void Demo::MimicEnemy::PatternJunkVomit(float projDamage) {
	static std::random_device rd;
	static std::mt19937 gen(rd());

	std::uniform_real_distribution<float> yDist(-196.f, 196.f);

	for (int i = 0; i < 100; i++) {
		float randY = yDist(gen);
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, randY](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [px, py] = lock->GetWorldPosition();
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
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.05f));
	}
}