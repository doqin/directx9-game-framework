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
		void Spawn(float x, float y, float rotation, float scaleX, float scaleY, D3DCOLOR tint);
		void Draw(const Camera& camera, unsigned long long deltaTime);
	};

	void ConfigureFootprintEmitter(ParticleSystem& particleSystem);
	void ConfigureTrailEmitter(ParticleSystem& particleSystem);
	void ConfigureGhostTrailEmitter(ParticleSystem& particleSystem);
}
