#pragma once
#include "../DX9GFSprites.h"
#include "../DX9GFTexture.h"
#include "../DX9GFCamera.h"
#include <vector>
#include <memory>

namespace DX9GF {
	// Generic pool-based particle emitter. Bound to a single texture for its
	// lifetime; spawns short-lived, fading/tinted sprites either automatically
	// at a fixed interval (via Update) or on demand (via Spawn).
	class ParticleSystem {
	private:
		struct Particle {
			bool isActive = false;
			float x = 0, y = 0, rotation = 0;
			float scaleX = 1, scaleY = 1;
			// Velocity in units/second. Zero for the timer-driven emitters (footprints, trails),
			// non-zero for one-shot bursts like ConfigureExplosionEmitter. Integrated and damped
			// in Update, so a manual burst still needs Update called every frame.
			float vx = 0, vy = 0;
			float age = 0; // ms since spawn
			D3DCOLOR tint = 0xFFFFFFFF;
		};

		Texture* texture;
		std::unique_ptr<StaticSprite> sprite; // reused across all particles
		std::vector<Particle> pool;

		float emissionInterval = 100.f;
		float emissionTimer = 0.f;
		float lifeTime = 500.f;
		float fadeOutTime = 200.f;
		float startScale = 1.f;
		float endScale = 1.f;
		D3DCOLOR startColor = 0xFFFFFFFF;
		D3DCOLOR endColor = 0xFFFFFFFF;
		bool enabled = true;

		D3DCOLOR LerpColor(D3DCOLOR a, D3DCOLOR b, float t) const;
	public:
		ParticleSystem(Texture* texture, size_t maxParticles);
		~ParticleSystem() = default;

		void SetSrcRect(RECT rect);
		void SetOrigin(float x, float y);
		void SetEmissionInterval(float intervalMs);
		void SetLifeTime(float lifeTimeMs);
		void SetFadeOutTime(float fadeOutMs);
		void SetScaleRange(float startScale, float endScale);
		void SetColorRange(D3DCOLOR startColor, D3DCOLOR endColor);
		void SetEnabled(bool enabled);
		bool IsEnabled() const;
		void Clear();

		// Returns true if a particle was spawned this call.
		bool Update(unsigned long long deltaTime, float x, float y, float rotation = 0.f,
			float scaleX = 1.f, float scaleY = 1.f, D3DCOLOR tint = 0xFFFFFFFF, bool active = true);
		void Spawn(float x, float y, float rotation, float scaleX, float scaleY, D3DCOLOR tint,
			float vx = 0.f, float vy = 0.f);
		void Draw(const Camera& camera, unsigned long long deltaTime);
	};

	void ConfigureFootprintEmitter(ParticleSystem& particleSystem);
	void ConfigureTrailEmitter(ParticleSystem& particleSystem);
	void ConfigureGhostTrailEmitter(ParticleSystem& particleSystem);
	// One-shot burst: short-lived embers that expand as they fade from white to orange.
	// Pair with a ring of Spawn() calls that pass an outward velocity.
	void ConfigureExplosionEmitter(ParticleSystem& particleSystem);
}
