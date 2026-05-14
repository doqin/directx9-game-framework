#include "pch.h"
#include "SineWaveProjectile.h"
#include <cmath>

Demo::SineWaveProjectile::Builder& Demo::SineWaveProjectile::Builder::SetTrajectory(D3DXVECTOR2 trajectory)
{
    this->instance->trajectory = trajectory;
    return *this;
}

Demo::SineWaveProjectile::Builder& Demo::SineWaveProjectile::Builder::SetVelocity(float velocity)
{
    this->instance->velocity = velocity;
    return *this;
}

Demo::SineWaveProjectile::Builder& Demo::SineWaveProjectile::Builder::SetDelay(float delay)
{
    this->instance->delay = delay;
    return *this;
}

Demo::SineWaveProjectile::Builder& Demo::SineWaveProjectile::Builder::SetDecayTime(float decayTime)
{
    this->instance->decayTime = decayTime;
    return *this;
}

Demo::SineWaveProjectile::Builder& Demo::SineWaveProjectile::Builder::SetDamage(float dmg)
{
    this->instance->damage = dmg;
    return *this;
}

Demo::SineWaveProjectile::Builder& Demo::SineWaveProjectile::Builder::SetWave(float amplitude, float frequency)
{
    this->instance->amplitude = amplitude;
    this->instance->frequency = frequency;
    return *this;
}

std::shared_ptr<Demo::SineWaveProjectile> Demo::SineWaveProjectile::Builder::Build()
{
    return instance;
}

void Demo::SineWaveProjectile::Init()
{
    this->collider = std::make_shared<DX9GF::EllipseCollider>(transformManager, shared_from_this(), colliderWidth, colliderHeight);
    this->collider->SetOriginCenter();

    auto [x, y] = GetWorldPosition();
    this->basePosition = D3DXVECTOR2(x, y);
}

void Demo::SineWaveProjectile::Update(unsigned long long deltaTime)
{
    if (decayTime != UNSPECIFIED && this->elapsed >= decayTime) {
        state = State::Destroyed;
        return;
    }

    //Move if delay has expired
    if (this->elapsed >= delay) {
        float dtSec = deltaTime / 1000.f;

        this->basePosition.x += trajectory.x * velocity * dtSec;
        this->basePosition.y += trajectory.y * velocity * dtSec;
        D3DXVECTOR2 perpendicular(-trajectory.y, trajectory.x);
        float moveTime = this->elapsed - delay;
        float waveOffset = std::sin(moveTime * frequency) * amplitude;
        float finalX = this->basePosition.x + perpendicular.x * waveOffset;
        float finalY = this->basePosition.y + perpendicular.y * waveOffset;

        float waveDerivative = std::cos(moveTime * frequency) * frequency * amplitude;
        D3DXVECTOR2 instantaneousVelocity = trajectory * velocity + perpendicular * waveDerivative;

        if (D3DXVec2LengthSq(&instantaneousVelocity) > 0.0001f) {
            float angle = atan2(instantaneousVelocity.y, instantaneousVelocity.x);
            SetLocalRotation(angle);
        }

        SetLocalPosition(finalX, finalY);
    }

    if (collider->IsCollided(player->GetCollider())) {
        player->TakeDamage(damage);
    }

    this->elapsed += deltaTime / 1000.f;
}

void Demo::SineWaveProjectile::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime)
{
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