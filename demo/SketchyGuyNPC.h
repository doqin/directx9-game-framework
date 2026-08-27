#pragma once
#include "INPC.h"

namespace Demo {
	// The card-pack peddler: a shifty little process that sells sealed packs of "acquired" cards.
	// First meeting is a short conversation (he's sketchy, the player goes along with it); every
	// meeting after that opens the buy prompt. All of his flavour text is baked into the class so
	// any scene can drop him in with just a position - see SketchyGuyGlobalData for the shared
	// met / first-pack state.
	class SketchyGuyNPC : public INPC {
	private:
		std::weak_ptr<DX9GF::ColliderManager> colliderManager;

		void MapCharacterVoices(std::unordered_map<std::wstring, std::string>& voiceMap) override {
			voiceMap[L"???"] = "bleep21";
			voiceMap[L"Peddler"] = "bleep21";
		}
	public:
		SketchyGuyNPC(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y);

		void Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p,
			std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font,
			std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) override;

		void Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) override;

		bool CanInteract() const override { return isPlayerNear; }

		// Buy-prompt wording, kept here so the popup the scene shows stays in his voice.
		std::wstring GetBuyPromptTitle() const;
		std::wstring GetBuyPromptText(int cost) const;
		std::wstring GetBuyConfirmLabel(int cost) const;
	};
}
