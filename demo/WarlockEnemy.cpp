#include "pch.h"
#include "WarlockEnemy.h"
#include "resource.h"
#include "SpiralProjectile.h"
#include "TargetedProjectile.h"
#include "RoundProjectile.h"
#include <random>

void Demo::WarlockEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"crawler-Sheet.png");
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 12);
	sprite->SetOrigin(32, 32);
	sprite->SetScale(2.f, 2.f);

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
	float projDamage = GetOutgoingDamage(4.f);

	if (GetRandomPattern() == 1) PatternDarkVortex(projDamage);
	else PatternHomingCurse(projDamage);

	commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(4.f));
}

void Demo::WarlockEnemy::PatternDarkVortex(float projDamage) {
	std::random_device rd;
	std::default_random_engine gen(rd());
	std::uniform_real_distribution<float> dist(0.f, 1.f);
	for (int ring = 0; ring < 15; ring++) {
		float angle = dist(gen);
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, ring, dist, angle](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				int numProjectiles = 16;
				float radius = 640.f;
				float gapAngle = 3.14159f * 2.f * angle;
				float gapSize = 3.14159f / 4.f;

				for (int i = 0; i < numProjectiles; ++i) {
					float angle = i * (2.f * 3.14159f / numProjectiles);

					// define gap
					float angleDiff = std::abs(angle - gapAngle);
					if (angleDiff > 3.14159f) {
						angleDiff = 2.f * 3.14159f - angleDiff;
					}

					if (angleDiff < gapSize) {
						continue;
					}

					projectiles.push_back(
						RoundProjectile::Builder(transformManager, lock, projSprite, 16, 16, radius * std::cos(angle), radius * std::sin(angle))
						.SetDelay(0.2f)
						.SetDecayTime(2.2f)
						.SetVelocity(320.f)
						.SetTrajectory(D3DXVECTOR2(-std::cos(angle), -std::sin(angle)))
						.SetDamage(projDamage)
						.Build()
					);

					projectiles.back()->Init();
				}
				transformManager.lock()->RebuildHierarchy();
			}
			markFinished();
		}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.7f));
	}
}

void Demo::WarlockEnemy::PatternHomingCurse(float projDamage) {
	auto topRightAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			auto [px, py] = lock->GetWorldPosition();
			projectiles.push_back(
				TargetedProjectile::Builder(transformManager, lock, projSprite, 16, 16, 512, 128)
				.SetTrajectory(D3DXVECTOR2(-1, 1))
				.SetHoming(2.0f)
				.SetDelay(0.f)
				.SetDecayTime(5.f)
				.SetVelocity(250.f)
				.SetDamage(projDamage)
				.Build()
			);
			projectiles.back()->Init();
			transformManager.lock()->RebuildHierarchy();
		}
		markFinished();
	});
	auto topLeftAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			auto [px, py] = lock->GetWorldPosition();
			projectiles.push_back(
				TargetedProjectile::Builder(transformManager, lock, projSprite, 16, 16, -512, 128)
				.SetTrajectory(D3DXVECTOR2(1, 1))
				.SetHoming(2.0f)
				.SetDelay(0.f)
				.SetDecayTime(5.f)
				.SetVelocity(250.f)
				.SetDamage(projDamage)
				.Build()
			);
			projectiles.back()->Init();
			transformManager.lock()->RebuildHierarchy();
		}
		markFinished();
		});
	auto bottomRightAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			auto [px, py] = lock->GetWorldPosition();
			projectiles.push_back(
				TargetedProjectile::Builder(transformManager, lock, projSprite, 16, 16, 512, -128)
				.SetTrajectory(D3DXVECTOR2(-1, -1))
				.SetHoming(2.0f)
				.SetDelay(0.f)
				.SetDecayTime(5.f)
				.SetVelocity(250.f)
				.SetDamage(projDamage)
				.Build()
			);
			projectiles.back()->Init();
			transformManager.lock()->RebuildHierarchy();
		}
		markFinished();
		});
	auto bottomLeftAttack = std::make_shared<DX9GF::CustomCommand>([this, projDamage](std::function<void(void)> markFinished) {
		if (auto lock = this->player.lock()) {
			auto [px, py] = lock->GetWorldPosition();
			projectiles.push_back(
				TargetedProjectile::Builder(transformManager, lock, projSprite, 16, 16, -512, -128)
				.SetTrajectory(D3DXVECTOR2(1, -1))
				.SetHoming(2.0f)
				.SetDelay(0.f)
				.SetDecayTime(5.f)
				.SetVelocity(250.f)
				.SetDamage(projDamage)
				.Build()
			);
			projectiles.back()->Init();
			transformManager.lock()->RebuildHierarchy();
		}
		markFinished();
		});
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(0, 3);
	for (int i = 0; i < 40; i++) {
		int attackIndex = dist(gen);
		switch (attackIndex) {
			case 0:
				commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*topRightAttack));
				break;
			case 1:
				commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*topLeftAttack));
				break;
			case 2:
				commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*bottomRightAttack));
				break;
			case 3:
				commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>(*bottomLeftAttack));
				break;
		}
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.3f));
	}
}