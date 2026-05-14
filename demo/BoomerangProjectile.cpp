#include "pch.h"
#include "BoomerangProjectile.h"

Demo::BoomerangProjectile::Builder& Demo::BoomerangProjectile::Builder::SetTrajectory(D3DXVECTOR2 trajectory) {
	this->instance->trajectory = trajectory;
	return *this;
}

Demo::BoomerangProjectile::Builder& Demo::BoomerangProjectile::Builder::SetTargetPosition(float x, float y) {
	auto [currentX, currentY] = this->instance->GetWorldPosition();
	D3DXVECTOR2 trajectory{ x - currentX, y - currentY };
	D3DXVECTOR2 finalTrajectory;
	D3DXVec2Normalize(&finalTrajectory, &trajectory);
	this->instance->trajectory = finalTrajectory;
	return *this;
}

Demo::BoomerangProjectile::Builder& Demo::BoomerangProjectile::Builder::SetInitialVelocity(float velocity) {
	this->instance->velocity = velocity;
	return *this;
}

Demo::BoomerangProjectile::Builder& Demo::BoomerangProjectile::Builder::SetReturnAcceleration(float acceleration) {
	this->instance->returnAcceleration = acceleration;
	return *this;
}

Demo::BoomerangProjectile::Builder& Demo::BoomerangProjectile::Builder::SetDelay(float delay) {
	this->instance->delay = delay;
	return *this;
}

Demo::BoomerangProjectile::Builder& Demo::BoomerangProjectile::Builder::SetDecayTime(float decayTime) {
	this->instance->decayTime = decayTime;
	return *this;
}

Demo::BoomerangProjectile::Builder& Demo::BoomerangProjectile::Builder::SetDamage(float dmg) {
	this->instance->damage = dmg;
	return *this;
}

std::shared_ptr<Demo::BoomerangProjectile> Demo::BoomerangProjectile::Builder::Build() {
	return instance;
}

void Demo::BoomerangProjectile::Init() {
	this->collider = std::make_shared<DX9GF::EllipseCollider>(transformManager, shared_from_this(), colliderWidth, colliderHeight);
	this->collider->SetOriginCenter();
}

void Demo::BoomerangProjectile::Update(unsigned long long deltaTime) {
	if (decayTime != UNSPECIFIED && this->elapsed >= decayTime) {
		state = State::Destroyed;
		return;
	}
	if (this->elapsed >= delay) {
		float dt = deltaTime / 1000.f;

		velocity -= returnAcceleration * dt;

		auto [currentX, currentY] = GetWorldPosition();
		D3DXVECTOR2 moveVector = trajectory * velocity;
		if (D3DXVec2LengthSq(&moveVector) > 0.0001f) {
			float angle = atan2(moveVector.y, moveVector.x);
			SetLocalRotation(angle);
		}

		SetLocalPosition(currentX + trajectory.x * velocity * dt, currentY + trajectory.y * velocity * dt);
	}
	if (collider->IsCollided(player->GetCollider())) {
		player->TakeDamage(damage);
		state = State::Destroyed;
	}
	this->elapsed += deltaTime / 1000.f;
}

void Demo::BoomerangProjectile::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) {
	if (this->elapsed < delay) return;
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