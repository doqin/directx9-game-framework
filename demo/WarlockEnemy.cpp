#include "pch.h"
#include "WarlockEnemy.h"
#include "resource.h"
#include "SpiralProjectile.h"
#include "TargetedProjectile.h"
#include <random>

void Demo::WarlockEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(IDB_PNG4);
	sprite = std::make_shared<DX9GF::StaticSprite>(texture.get());
	sprite->SetOrigin(32, 32);

	projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	projTexture->LoadTexture(IDB_PNG5);
	projSprite = std::make_shared<DX9GF::StaticSprite>(projTexture.get());
	projSprite->SetOrigin(8, 8);

   SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
	InitCardSpawnTrigger(camera, 128.f, 128.f);
}

void Demo::WarlockEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) {
	if (sprite) {
		sprite->Begin();
		auto [x, y] = GetWorldPosition();
		sprite->SetPosition(x, y);
		sprite->Draw(*camera, deltaTime);
		sprite->End();
	}
	IEnemy::Draw(graphicsDevice, camera, deltaTime);
}

int Demo::WarlockEnemy::GetRandomPattern() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(1, 2);
	return dist(gen);
}

void Demo::WarlockEnemy::StartAttack(std::shared_ptr<Player> player) {
	this->player = player;
	float projDamage = GetOutgoingDamage(5.f);

	if (GetRandomPattern() == 1) PatternDarkVortex(projDamage);
	else PatternHomingCurse(projDamage);

	commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(4.f));
}

void Demo::WarlockEnemy::PatternDarkVortex(float projDamage) {
	for (int i = 0; i < 4; i++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, i](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [px, py] = lock->GetWorldPosition();
				float angle = i * (3.14159f / 2.f);

				projectiles.push_back(
					SpiralProjectile::Builder(transformManager, lock, projSprite, 16, 16, px, py)
					.SetSpiralParams(angle, 40.f, 4.0f)
					.SetDelay(0.f)
					.SetDecayTime(4.f)
					.SetDamage(projDamage)
					.Build()
				);
				projectiles.back()->Init();
				transformManager.lock()->RebuildHierarchy();
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.4f));
	}
}

void Demo::WarlockEnemy::PatternHomingCurse(float projDamage) {
	for (int i = 0; i < 5; i++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [px, py] = lock->GetWorldPosition();
				projectiles.push_back(
					TargetedProjectile::Builder(transformManager, lock, projSprite, 16, 16, GetWorldX(), GetWorldY() - 80.f)
					.SetTrajectory(D3DXVECTOR2(0, -1))
					.SetHoming(2.0f)
					.SetDelay(0.f)
					.SetDecayTime(5.f)
					.SetVelocity(140.f)
					.SetDamage(projDamage)
					.Build()
				);
				projectiles.back()->Init();
				transformManager.lock()->RebuildHierarchy();
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.6f));
	}
}