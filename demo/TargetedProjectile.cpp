#include "pch.h"
#include "TargetedProjectile.h"

Demo::TargetedProjectile::Builder& Demo::TargetedProjectile::Builder::SetTrajectory(D3DXVECTOR2 trajectory) {
	this->instance->trajectory = trajectory; return *this;
}
Demo::TargetedProjectile::Builder& Demo::TargetedProjectile::Builder::SetVelocity(float velocity) {
	this->instance->velocity = velocity; return *this;
}
Demo::TargetedProjectile::Builder& Demo::TargetedProjectile::Builder::SetDelay(float delay) {
	this->instance->delay = delay; return *this;
}
Demo::TargetedProjectile::Builder& Demo::TargetedProjectile::Builder::SetDecayTime(float decayTime) {
	this->instance->decayTime = decayTime; return *this;
}
Demo::TargetedProjectile::Builder& Demo::TargetedProjectile::Builder::SetDamage(float dmg) {
	this->instance->damage = dmg; return *this;
}
Demo::TargetedProjectile::Builder& Demo::TargetedProjectile::Builder::SetHoming(float turnSpeed) {
	this->instance->turnSpeed = turnSpeed; return *this;
}
std::shared_ptr<Demo::TargetedProjectile> Demo::TargetedProjectile::Builder::Build() {
	return instance;
}

void Demo::TargetedProjectile::Init() {
	this->collider = std::make_shared<DX9GF::EllipseCollider>(transformManager, shared_from_this(), colliderWidth, colliderHeight);
	this->collider->SetOriginCenter();

	auto graphicsDevice = sprite->GetGraphicsDevice();
	trailTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	trailTexture->CreatePlainTexture(0xFFFFFFFF, 4, 4);
	trailEmitter = std::make_unique<DX9GF::ParticleSystem>(trailTexture.get(), 32);
	trailEmitter->SetOrigin(2, 2);
	DX9GF::ConfigureTrailEmitter(*trailEmitter);
}

void Demo::TargetedProjectile::Update(unsigned long long deltaTime) {
	if (decayTime != UNSPECIFIED && this->elapsed >= decayTime) {
		state = State::Destroyed;
		return;
	}

	if (this->elapsed >= delay) {
		float dtSec = deltaTime / 1000.f;
		auto [currentX, currentY] = GetWorldPosition();
		auto [playerX, playerY] = player->GetWorldPosition();

		D3DXVECTOR2 idealTrajectory{ playerX - currentX, playerY - currentY };
		D3DXVec2Normalize(&idealTrajectory, &idealTrajectory);

		//gradually steer (Lerp) from current trajectory to idealTrajectory
		trajectory.x += (idealTrajectory.x - trajectory.x) * turnSpeed * dtSec;
		trajectory.y += (idealTrajectory.y - trajectory.y) * turnSpeed * dtSec;
		D3DXVec2Normalize(&trajectory, &trajectory);

		//move and update sprite rotation to ensure the bullet always faces the flight direction
		SetLocalPosition(currentX + trajectory.x * velocity * dtSec, currentY + trajectory.y * velocity * dtSec);

		float angle = atan2(trajectory.y, trajectory.x);
		SetLocalRotation(angle);
	}

	if (collider->IsCollided(player->GetCollider())) {
		player->TakeDamage(damage);
	}
	this->elapsed += deltaTime / 1000.f;

	auto [trailX, trailY] = GetWorldPosition();
	trailEmitter->Update(deltaTime, trailX, trailY, GetWorldRotation(), 1.f, 1.f, 0xFFFFFFFF, state != State::Destroyed && elapsed >= delay);
}

void Demo::TargetedProjectile::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) {
	trailEmitter->Draw(camera, deltaTime);
	this->sprite->Begin();
	auto [x, y] = GetWorldPosition();
	this->sprite->SetPosition(x, y);
	this->sprite->SetRotation(GetWorldRotation());
	this->sprite->SetScaleX(GetWorldScaleX());
	this->sprite->SetScaleY(GetWorldScaleY());
	this->sprite->Draw(camera, deltaTime);
	this->sprite->End();
	collider->Draw(sprite->GetGraphicsDevice(), camera);
}