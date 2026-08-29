#include "pch.h"
#include "CupidNPC.h"

namespace Demo {
	CupidNPC::CupidNPC(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y)
		: INPC(tm, x, y) {
	}

	void CupidNPC::Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p,
		std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font,
		std::shared_ptr<DX9GF::CommandBuffer> drawBuffer)
	{
		INPC::Init(gd, camera, p, cm, font, drawBuffer);
		this->gd = gd;
		this->colliderManager = cm;

		// 64x64 centred on the object covers the tile it is standing on.
		collider = std::make_shared<DX9GF::RectangleCollider>(transformManager, 64.f, 64.f, GetWorldX(), GetWorldY());
		collider->SetOriginCenter();
		cm->Add(collider);

		spritesheet = std::make_shared<DX9GF::Texture>(gd);
		spritesheet->LoadTexture(L"assets/bubble-Sheet.png");
		sprite = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 11), 12, true);
		sprite->SetOrigin(32.f, 48.f);
		sprite->SetPosition(GetWorldX(), GetWorldY());

		// A save restored before Init would have set the phase already; re-apply it so a defeated
		// NPC does not get its collider back.
		if (phase == Phase::Defeated) SetPhase(Phase::Defeated);
	}

	void CupidNPC::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime)
	{
		if (phase == Phase::Defeated) return;

		sprite->Begin();
		sprite->Draw(camera, deltaTime);
		sprite->End();

		INPC::Draw(camera, deltaTime);
	}

	void CupidNPC::SetPhase(Phase newPhase)
	{
		phase = newPhase;

		if (phase == Phase::Defeated && collider) {
			if (auto cm = colliderManager.lock()) {
				cm->Remove(collider);
			}
			collider = nullptr;
		}
	}
}
