#include "pch.h"
#include "EnergyToken.h"
#include "IBattleScene.h"
#include "Player.h"
#include "RNG.h"
#include <cmath>

namespace Demo {
	EnergyToken::EnergyToken(std::weak_ptr<DX9GF::TransformManager> tm, DX9GF::GraphicsDevice* gd, float x, float y)
		: IToken(tm, x, y)
	{
		texture = std::make_shared<DX9GF::Texture>(gd);
		texture->LoadTexture(L"assets/ui.png"); //TODO: Temporary reuse energy icon, change it if u have any idea about how energy fragments look like

		sprite = std::make_shared<DX9GF::StaticSprite>(texture.get());
		sprite->SetSrcRect({ 96, 0, 112, 16 });
		sprite->SetScale(1.5f, 1.5f);
		sprite->SetOrigin(8.f, 8.f);

		collider = std::make_shared<DX9GF::RectangleCollider>(tm, 24.f, 24.f, x, y);
		collider->SetOriginCenter();
	}

	void EnergyToken::Update(unsigned long long deltaTime) {
		if (isCollected) {
			floatOffset -= 0.1f * deltaTime;
			alpha = (std::max)(0, alpha - static_cast<int>(0.5f * deltaTime));
			SetLocalY(GetLocalY() + floatOffset * 0.01f);
			return;
		}

		lifetime -= static_cast<float>(deltaTime);
		if (lifetime <= 0.f) isExpired = true;

		floatTime += deltaTime * 0.005f;
		floatOffset = std::sin(floatTime) * 5.0f;
		collider->SetLocalPosition(GetWorldX(), GetWorldY() + floatOffset);
	}

	void EnergyToken::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) {
		if (alpha <= 0 || isExpired) return;

		int currentAlpha = alpha;
		if (!isCollected && lifetime < 500.f) {
			currentAlpha = (std::sin(floatTime * 10.f) > 0.f) ? 255 : 50;
		}

		sprite->Begin();
		sprite->SetPosition(GetWorldX(), GetWorldY() + floatOffset);
		sprite->SetColor(D3DCOLOR_ARGB(currentAlpha, 255, 255, 255));
		sprite->Draw(*camera, deltaTime);
		sprite->End();
	}

	void EnergyToken::OnCollect(Player* player, IBattleScene* scene) {
		if (isCollected) return;
		isCollected = true;

		scene->AddEnergyPieceToken();
		DX9GF::AudioManager::GetInstance()->PlayRandom("token_collect", 0.3f);
	}

	bool EnergyToken::IsDone() const {
		return (isCollected && alpha <= 0) || isExpired;
	}
}