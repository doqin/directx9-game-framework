#pragma once
#include "INPC.h"

namespace Demo {
	// Stands in the doorway to Keyepro's chamber. Talks once about what this sector really is,
	// then the scene starts the boss fight when the conversation ends. Its collider is what
	// physically blocks the corridor, and it is removed once the fight is won.
	class KeyeproNPC : public INPC {
	public:
		enum class Phase {
			Waiting,   // not yet defeated (also covers "lost and can try again")
			Defeated   // beaten - despawned, path open
		};

	private:
		Phase phase = Phase::Waiting;
		std::weak_ptr<DX9GF::ColliderManager> colliderManager;

		void MapCharacterVoices(std::unordered_map<std::wstring, std::string>& voiceMap) override {
			voiceMap[L"Anonymous"] = "bleep25";
		}
	public:
		KeyeproNPC(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y);

		void Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p,
			std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font,
			std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) override;

		void Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) override;

		bool CanInteract() const override { return isPlayerNear && phase != Phase::Defeated; }

		Phase GetPhase() const { return phase; }
		void SetPhase(Phase newPhase);
	};
}
