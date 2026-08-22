#pragma once
#include "INPC.h"

namespace Demo {
	// The "helpful stranger" blocking the alley. It wears the Trojan overworld sprite and walks the
	// player through a social-engineering setup: ask for a credential, take it, then drop the act.
	// Its collider is what physically blocks the corridor, and it is removed once the fight is won.
	class TrojanNPC : public INPC {
	public:
		enum class Phase {
			Friendly,       // not spoken to yet
			AwaitingToken,  // quest given, waiting for the player to bring the token back
			Revealed,       // token handed over, fight pending or lost
			Defeated        // beaten - despawned, path open
		};

	private:
		Phase phase = Phase::Friendly;
		std::weak_ptr<DX9GF::ColliderManager> colliderManager;

		std::vector<DialogueLine> friendlyLines;
		std::vector<DialogueLine> waitingLines;
		std::vector<DialogueLine> revealLines;

		void Append(std::vector<DialogueLine>& lines, std::wstring name, std::wstring content);
		void MapCharacterVoices(std::unordered_map<std::wstring, std::string>& voiceMap) override {
			voiceMap[L"???"] = "bleep21";
			voiceMap[L"Trojan"] = "bleep21";
		}
	public:
		TrojanNPC(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y);

		void Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p,
			std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font,
			std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) override;

		void Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) override;

		bool CanInteract() const override { return isPlayerNear && phase != Phase::Defeated; }

		void AddFriendlyLine(std::wstring name, std::wstring content) { Append(friendlyLines, name, content); }
		void AddWaitingLine(std::wstring name, std::wstring content) { Append(waitingLines, name, content); }
		void AddRevealLine(std::wstring name, std::wstring content) { Append(revealLines, name, content); }

		// Lines for the current phase. GetDialogueLines() is overridden so callers that only know
		// about INPC still get the right script.
		std::vector<DialogueLine> GetDialogueLines() override;

		Phase GetPhase() const { return phase; }
		void SetPhase(Phase newPhase);
	};
}
