#pragma once
#include "IProjectile.h"

namespace Demo {

	class SineWaveProjectile : public IProjectile {
	public:
		class Builder;
		friend class Builder;
	private:
		std::shared_ptr<DX9GF::EllipseCollider> collider;
		std::shared_ptr<DX9GF::ISprite> sprite;
		float colliderWidth;
		float colliderHeight;

		float velocity;
		D3DXVECTOR2 trajectory;

		//sine wave parameters
		float amplitude;
		float frequency;
		D3DXVECTOR2 basePosition;

		float delay = 0.f;
		float elapsed = 0.f;
		float decayTime = UNSPECIFIED;
		float damage = 0.f;

		// Ghost trail (optional, populated via Builder::SetGhostSprite)
		DX9GF::Texture* ghostTexture = nullptr;
		RECT ghostSrcRect{};
		bool ghostHasSrcRect = false;
		float ghostOriginX = 0.f;
		float ghostOriginY = 0.f;
		std::unique_ptr<DX9GF::ParticleSystem> ghostEmitter;
	protected:
		SineWaveProjectile(
			std::weak_ptr<DX9GF::TransformManager> tm,
			std::shared_ptr<Player> player,
			std::shared_ptr<DX9GF::ISprite> sprite,
			float colliderWidth,
			float colliderHeight
		) : IProjectile(tm, player),
			sprite(sprite),
			colliderWidth(colliderWidth),
			colliderHeight(colliderHeight) {
		}

		SineWaveProjectile(
			std::weak_ptr<DX9GF::TransformManager> tm,
			std::shared_ptr<Player> player,
			std::shared_ptr<DX9GF::ISprite> sprite,
			float colliderWidth,
			float colliderHeight,
			float x,
			float y,
			float rotation = 0,
			float scaleX = 1,
			float scaleY = 1
		) : IProjectile(tm, player, x, y, rotation, scaleX, scaleY),
			sprite(sprite),
			colliderWidth(colliderWidth),
			colliderHeight(colliderHeight) {
		}

	public:
		void Init() override;
		void Update(unsigned long long deltaTime) override;
		void Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) override;
	};

	class SineWaveProjectile::Builder {
		std::shared_ptr<SineWaveProjectile> instance;
	public:
		Builder(
			std::weak_ptr<DX9GF::TransformManager> tm,
			std::shared_ptr<Player> player,
			std::shared_ptr<DX9GF::ISprite> sprite,
			float colliderWidth,
			float colliderHeight
		) : instance(std::shared_ptr<SineWaveProjectile>(new SineWaveProjectile(tm, player, sprite, colliderWidth, colliderHeight))) {
		}

		Builder(
			std::weak_ptr<DX9GF::TransformManager> tm,
			std::shared_ptr<Player> player,
			std::shared_ptr<DX9GF::ISprite> sprite,
			float colliderWidth,
			float colliderHeight,
			float x,
			float y,
			float rotation = 0,
			float scaleX = 1,
			float scaleY = 1
		) : instance(std::shared_ptr<SineWaveProjectile>(new SineWaveProjectile(tm, player, sprite, colliderWidth, colliderHeight, x, y, rotation, scaleX, scaleY))) {
		}

		Builder& SetTrajectory(D3DXVECTOR2 trajectory);
		Builder& SetVelocity(float velocity);
		Builder& SetDelay(float delay);
		Builder& SetDecayTime(float decayTime);
		Builder& SetDamage(float dmg);

		Builder& SetWave(float amplitude, float frequency);
		// Enables the ghost trail effect, reusing the given texture/region so the
		// afterimages match the projectile's own appearance.
		Builder& SetGhostSprite(DX9GF::Texture* texture, RECT srcRect, float originX, float originY);

		std::shared_ptr<SineWaveProjectile> Build();
	};
}