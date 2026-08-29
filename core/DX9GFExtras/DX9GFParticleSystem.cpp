#include "pch.h"
#include "DX9GFParticleSystem.h"
#include <algorithm>

namespace DX9GF {

	namespace {
		BYTE ChannelA(D3DCOLOR c) { return (BYTE)((c >> 24) & 0xFF); }
		BYTE ChannelR(D3DCOLOR c) { return (BYTE)((c >> 16) & 0xFF); }
		BYTE ChannelG(D3DCOLOR c) { return (BYTE)((c >> 8) & 0xFF); }
		BYTE ChannelB(D3DCOLOR c) { return (BYTE)(c & 0xFF); }

		D3DCOLOR ModulateColor(D3DCOLOR a, D3DCOLOR b) {
			BYTE alpha = (BYTE)(ChannelA(a) * ChannelA(b) / 255);
			BYTE red = (BYTE)(ChannelR(a) * ChannelR(b) / 255);
			BYTE green = (BYTE)(ChannelG(a) * ChannelG(b) / 255);
			BYTE blue = (BYTE)(ChannelB(a) * ChannelB(b) / 255);
			return D3DCOLOR_ARGB(alpha, red, green, blue);
		}

		D3DCOLOR ScaleAlpha(D3DCOLOR c, float factor) {
			factor = std::clamp(factor, 0.f, 1.f);
			BYTE alpha = (BYTE)(ChannelA(c) * factor);
			return D3DCOLOR_ARGB(alpha, ChannelR(c), ChannelG(c), ChannelB(c));
		}
	}

	D3DCOLOR ParticleSystem::LerpColor(D3DCOLOR a, D3DCOLOR b, float t) const {
		t = std::clamp(t, 0.f, 1.f);
		BYTE alpha = (BYTE)(ChannelA(a) + (ChannelA(b) - ChannelA(a)) * t);
		BYTE red = (BYTE)(ChannelR(a) + (ChannelR(b) - ChannelR(a)) * t);
		BYTE green = (BYTE)(ChannelG(a) + (ChannelG(b) - ChannelG(a)) * t);
		BYTE blue = (BYTE)(ChannelB(a) + (ChannelB(b) - ChannelB(a)) * t);
		return D3DCOLOR_ARGB(alpha, red, green, blue);
	}

	ParticleSystem::ParticleSystem(Texture* texture, size_t maxParticles) : texture(texture) {
		sprite = std::make_unique<StaticSprite>(texture);
		pool.resize(maxParticles);
	}

	void ParticleSystem::SetSrcRect(RECT rect) {
		sprite->SetSrcRect(rect);
	}

	void ParticleSystem::SetOrigin(float x, float y) {
		sprite->SetOrigin(x, y);
	}

	void ParticleSystem::SetEmissionInterval(float intervalMs) {
		emissionInterval = intervalMs;
	}

	void ParticleSystem::SetLifeTime(float lifeTimeMs) {
		lifeTime = lifeTimeMs;
	}

	void ParticleSystem::SetFadeOutTime(float fadeOutMs) {
		fadeOutTime = fadeOutMs;
	}

	void ParticleSystem::SetScaleRange(float startScale, float endScale) {
		this->startScale = startScale;
		this->endScale = endScale;
	}

	void ParticleSystem::SetColorRange(D3DCOLOR startColor, D3DCOLOR endColor) {
		this->startColor = startColor;
		this->endColor = endColor;
	}

	void ParticleSystem::SetEnabled(bool enabled) {
		this->enabled = enabled;
	}

	bool ParticleSystem::IsEnabled() const {
		return enabled;
	}

	void ParticleSystem::Clear() {
		for (auto& particle : pool) {
			particle.isActive = false;
		}
	}

	bool ParticleSystem::Update(unsigned long long deltaTime, float x, float y, float rotation,
		float scaleX, float scaleY, D3DCOLOR tint, bool active) {
		// Per-tick velocity damping. Tuned for ~60fps ticks; particles carrying no velocity
		// (the timer-driven emitters) are unaffected.
		constexpr float kDrag = 0.90f;
		const float dtSeconds = (float)deltaTime / 1000.f;
		for (auto& particle : pool) {
			if (!particle.isActive) continue;
			particle.age += (float)deltaTime;
			if (particle.age >= lifeTime) { particle.isActive = false; continue; }
			particle.x += particle.vx * dtSeconds;
			particle.y += particle.vy * dtSeconds;
			particle.vx *= kDrag;
			particle.vy *= kDrag;
		}

		if (enabled && active) {
			emissionTimer -= (float)deltaTime;
			if (emissionTimer <= 0.f) {
				Spawn(x, y, rotation, scaleX, scaleY, tint);
				emissionTimer = emissionInterval;
				return true;
			}
		}
		return false;
	}

	void ParticleSystem::Spawn(float x, float y, float rotation, float scaleX, float scaleY, D3DCOLOR tint,
		float vx, float vy) {
		for (auto& particle : pool) {
			if (particle.isActive) continue;
			particle.isActive = true;
			particle.x = x;
			particle.y = y;
			particle.rotation = rotation;
			particle.scaleX = scaleX;
			particle.scaleY = scaleY;
			particle.vx = vx;
			particle.vy = vy;
			particle.age = 0.f;
			particle.tint = tint;
			return;
		}
	}

	void ParticleSystem::Draw(const Camera& camera, unsigned long long deltaTime) {
		sprite->Begin();
		for (auto& particle : pool) {
			if (!particle.isActive) continue;

			float t = lifeTime > 0.f ? particle.age / lifeTime : 1.f;
			float scale = startScale + (endScale - startScale) * std::clamp(t, 0.f, 1.f);
			D3DCOLOR color = ModulateColor(LerpColor(startColor, endColor, t), particle.tint);

			float lifeRemaining = lifeTime - particle.age;
			if (fadeOutTime > 0.f && lifeRemaining < fadeOutTime) {
				color = ScaleAlpha(color, lifeRemaining / fadeOutTime);
			}

			sprite->SetPosition(particle.x, particle.y);
			sprite->SetRotation(particle.rotation);
			sprite->SetScale(particle.scaleX * scale, particle.scaleY * scale);
			sprite->SetColor(color);
			sprite->Draw(camera, deltaTime);
		}
		sprite->End();
	}

	void ConfigureFootprintEmitter(ParticleSystem& particleSystem) {
		particleSystem.SetEmissionInterval(180.f);
		particleSystem.SetLifeTime(900.f);
		particleSystem.SetFadeOutTime(900.f);
		particleSystem.SetScaleRange(1.f, 0.8f);
		particleSystem.SetColorRange(0xFFFFFFFF, 0xFFFFFFFF);
	}

	void ConfigureTrailEmitter(ParticleSystem& particleSystem) {
		particleSystem.SetEmissionInterval(16.f);
		particleSystem.SetLifeTime(250.f);
		particleSystem.SetFadeOutTime(250.f);
		particleSystem.SetScaleRange(1.f, 0.4f);
		particleSystem.SetColorRange(0xFFFFFFFF, 0xFFFFFFFF);
	}

	void ConfigureGhostTrailEmitter(ParticleSystem& particleSystem) {
		particleSystem.SetEmissionInterval(70.f);
		particleSystem.SetLifeTime(250.f);
		particleSystem.SetFadeOutTime(250.f);
		particleSystem.SetScaleRange(1.f, 1.f);
		particleSystem.SetColorRange(D3DCOLOR_ARGB(120, 200, 220, 255), D3DCOLOR_ARGB(0, 200, 220, 255));
	}

	void ConfigureExplosionEmitter(ParticleSystem& particleSystem) {
		particleSystem.SetLifeTime(500.f);
		particleSystem.SetFadeOutTime(500.f);
		particleSystem.SetScaleRange(1.4f, 0.3f);
		particleSystem.SetColorRange(0xFFFFFFFF, 0xFFFF6A00);
		particleSystem.SetEnabled(false); // burst-only: no timer emission
	}
}
