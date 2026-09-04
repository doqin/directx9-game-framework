#pragma once
#include "INPC.h"

namespace Demo {
	// The tutorial exit's gatekeeper: a junk-mail entity that blocks the path to trigger_p,
	// talks once, and the scene starts the fight when the conversation ends. Its collider is
	// removed once beaten so the portal becomes reachable.
	class SpamNPC : public INPC {
	public:
		enum class Phase {
			Waiting,   // not yet defeated (also covers "lost and can try again")
			Defeated   // beaten - despawned
		};

	private:
		Phase phase = Phase::Waiting;
		std::weak_ptr<DX9GF::ColliderManager> colliderManager;

		void MapCharacterVoices(std::unordered_map<std::wstring, std::string>& voiceMap) override {
			voiceMap[L"???"] = "bleep21";
			voiceMap[L"SPAM"] = "bleep21";
		}

	public:
		SpamNPC(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y);

		void Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p,
			std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font,
			std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) override;

		void Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) override;

		bool CanInteract() const override { return isPlayerNear && phase != Phase::Defeated; }

		Phase GetPhase() const { return phase; }
		void SetPhase(Phase newPhase);
	};
}
