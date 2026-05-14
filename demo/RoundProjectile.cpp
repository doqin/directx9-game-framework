#include "pch.h"
#include "RoundProjectile.h"

Demo::RoundProjectile::Builder& Demo::RoundProjectile::Builder::SetTrajectory(D3DXVECTOR2 trajectory)
{
    this->instance->trajectory = trajectory;
    return *this;
}

Demo::RoundProjectile::Builder& Demo::RoundProjectile::Builder::SetTargetPosition(float x, float y)
{
    auto [currentX, currentY] = this->instance->GetWorldPosition();
    D3DXVECTOR2 trajectory{x - currentX, y - currentY};
    D3DXVECTOR2 finalTrajectory;
    D3DXVec2Normalize(&finalTrajectory, &trajectory);
    this->instance->trajectory = finalTrajectory;
    return *this;
}

Demo::RoundProjectile::Builder& Demo::RoundProjectile::Builder::SetVelocity(float velocity)
{
    this->instance->velocity = velocity;
    return *this;
}

Demo::RoundProjectile::Builder& Demo::RoundProjectile::Builder::SetDelay(float delay)
{
    this->instance->delay = delay;
    return *this;
}

Demo::RoundProjectile::Builder& Demo::RoundProjectile::Builder::SetDecayTime(float decayTime)
{
    this->instance->decayTime = decayTime;
    return *this;
}

Demo::RoundProjectile::Builder& Demo::RoundProjectile::Builder::SetDamage(float dmg)
{
    this->instance->damage = dmg;
    return *this;
}

std::shared_ptr<Demo::RoundProjectile> Demo::RoundProjectile::Builder::Build()
{
    return instance;
}

void Demo::RoundProjectile::Init()
{
    this->collider = std::make_shared<DX9GF::EllipseCollider>(transformManager, shared_from_this(), colliderWidth, colliderHeight);
    this->collider->SetOriginCenter();
}

void Demo::RoundProjectile::Update(unsigned long long deltaTime)
{
    if (decayTime != UNSPECIFIED && this->elapsed >= decayTime) {
        state = State::Destroyed;
        return;
    }
    if (this->elapsed >= delay) {
        auto [currentX, currentY] = GetWorldPosition();
        SetLocalPosition(currentX + trajectory.x * velocity * deltaTime / 1000.f, currentY + trajectory.y * velocity * deltaTime / 1000.f);
        float angle = atan2(trajectory.y, trajectory.x);
        SetLocalRotation(angle);
    }
    if (collider->IsCollided(player->GetCollider())) {
        player->TakeDamage(damage);
    }
    this->elapsed += deltaTime / 1000.f;
}

void Demo::RoundProjectile::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime)
{
    // i'm not sure if we should make the programmer specify when to begin and end
    this->sprite->Begin();
    auto [x, y] = GetWorldPosition();
    this->sprite->SetPosition(x, y);
    this->sprite->SetRotation(GetWorldRotation());
    this->sprite->SetScaleX(GetWorldScaleX());
    this->sprite->SetScaleY(GetWorldScaleY());
    this->sprite->SetRotation(GetWorldRotation());
    this->sprite->Draw(camera, deltaTime);
    this->sprite->End();
    collider->Draw(sprite->GetGraphicsDevice(), camera);
}
