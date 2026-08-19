#include "pch.h"
#include "GoldToken.h"
#include "IBattleScene.h"
#include "Player.h"
#include <cmath>

namespace Demo {
	GoldToken::GoldToken(std::weak_ptr<DX9GF::TransformManager> tm, DX9GF::GraphicsDevice* gd, float x, float y, int value)
		: IToken(tm, x, y), goldValue(value)
	{
		texture = std::make_shared<DX9GF::Texture>(gd);
		texture->LoadTexture(L"assets/gold-token.png");

		std::vector<RECT> frames = {
			{7, 7, 26, 26},
			{40, 7, 59, 26},
			{73, 7, 92, 26},
			{106, 7, 125, 26},
			{139, 7, 158, 26},
			{172, 7, 191, 26},
			{206, 7, 225, 26},
			{239, 7, 258, 26}
		};

		sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), frames, 12, true);
		sprite->SetOrigin(9.5f, 9.5f);

		collider = std::make_shared<DX9GF::RectangleCollider>(tm, 19.f, 19.f, x, y);
		collider->SetOriginCenter();
	}

	void GoldToken::Update(unsigned long long deltaTime) {
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

	void GoldToken::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) {
		if (alpha <= 0 || isExpired) return;

		int currentAlpha = alpha;

		//disappear warning
		if (!isCollected && lifetime < 1500.f) {
			int blinkRate = (lifetime < 500.f) ? 50 : 150;

			if ((static_cast<int>(lifetime) / blinkRate) % 2 == 0) {
				currentAlpha = alpha;
			}
			else {
				currentAlpha = 50;
			}
		}

		sprite->Begin();
		sprite->SetPosition(GetWorldX(), GetWorldY() + floatOffset);
		sprite->SetColor(D3DCOLOR_ARGB(currentAlpha, 255, 255, 255));
		sprite->Draw(*camera, deltaTime);
		sprite->End();
	}

	void GoldToken::OnCollect(Player* player, IBattleScene* scene) {
		if (isCollected) return;
		isCollected = true;
		scene->AddPendingGoldTokenReward(goldValue);
		DX9GF::AudioManager::GetInstance()->PlayRandom("token_collect", 0.3f);
	}

	bool GoldToken::IsDone() const {
		return (isCollected && alpha <= 0) || isExpired;
	}
}