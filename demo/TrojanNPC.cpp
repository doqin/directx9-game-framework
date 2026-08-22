#include "pch.h"
#include "TrojanNPC.h"

namespace Demo {
	TrojanNPC::TrojanNPC(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y)
		: INPC(tm, x, y) {
	}

	void TrojanNPC::Append(std::vector<DialogueLine>& lines, std::wstring name, std::wstring content)
	{
		DialogueLine line;
		line.name = std::move(name);
		line.content = std::move(content);
		lines.push_back(std::move(line));
	}

	void TrojanNPC::Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p,
		std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font,
		std::shared_ptr<DX9GF::CommandBuffer> drawBuffer)
	{
		INPC::Init(gd, camera, p, cm, font, drawBuffer);
		this->gd = gd;
		this->colliderManager = cm;

		// 32x32 centred on the object covers the 2x2 tile block it is standing on.
		collider = std::make_shared<DX9GF::RectangleCollider>(transformManager, 32.f, 32.f, GetWorldX(), GetWorldY());
		collider->SetOriginCenter();
		cm->Add(collider);

		spritesheet = std::make_shared<DX9GF::Texture>(gd);
		spritesheet->LoadTexture(L"assets/Trojan_outside.png");
		sprite = std::make_shared<DX9GF::AnimatedSprite>(spritesheet.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 6, true);
		sprite->SetOrigin(32.f, 48.f);
		sprite->SetPosition(GetWorldX(), GetWorldY());

		// A save restored before Init would have set the phase already; re-apply it so a defeated
		// NPC does not get its collider back.
		if (phase == Phase::Defeated) SetPhase(Phase::Defeated);
	}

	void TrojanNPC::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime)
	{
		if (phase == Phase::Defeated) return;

		sprite->Begin();
		sprite->Draw(camera, deltaTime);
		sprite->End();

		INPC::Draw(camera, deltaTime);
	}

	std::vector<DialogueLine> TrojanNPC::GetDialogueLines()
	{
		switch (phase) {
		case Phase::Friendly:      return friendlyLines;
		case Phase::AwaitingToken: return waitingLines;
		case Phase::Revealed:      return revealLines;
		case Phase::Defeated:      return {};
		}
		return {};
	}

	void TrojanNPC::SetPhase(Phase newPhase)
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
