#include "pch.h"
#include "NPC.h"

namespace Demo {
	NPC::NPC(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y, const NPCConfig& config)
		: INPC(tm, x, y), config(config) {
	}

	void NPC::Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) {
		INPC::Init(gd, camera, p, cm, font, drawBuffer);

		collider = std::make_shared<DX9GF::RectangleCollider>(transformManager, config.colW, config.colH, this->GetWorldX(), this->GetWorldY() + config.colOffsetY);
		collider->SetOriginCenter();
		cm->Add(collider);

		spritesheet = std::make_shared<DX9GF::Texture>(gd);
		spritesheet->LoadTexture(config.texturePath.c_str());
		sprite = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, config.frameW, config.frameH, config.frameCount), config.animSpeed, true);
		sprite->SetOrigin(config.frameW / 2.0f, config.frameH / 2.0f);
		sprite->SetPosition(this->GetWorldX(), this->GetWorldY());
	}

	void NPC::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) {
		sprite->Begin();
		sprite->Draw(camera, deltaTime);
		sprite->End();

		INPC::Draw(camera, deltaTime);
	}
}