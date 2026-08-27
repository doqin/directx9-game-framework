#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Game.h"
#include "CardCatalog.h"
#include <string>
#include <vector>

namespace Demo {
	// Overlay scene that plays the card-pack purchase spectacle: the pack-opening animation, an
	// explosion particle burst, then the pulled cards revealed one at a time (click to fade in the
	// next). The cards are already in the player's inventory before this scene is pushed, so the
	// reveal is pure presentation - quitting mid-reveal loses nothing.
	class PackOpeningScene : public DX9GF::IScene {
	public:
		PackOpeningScene(Game* game, int screenWidth, int screenHeight, std::vector<std::string> cardIds);

		bool IsOverlay() const override { return true; }
		void Init() override;
		void Update(unsigned long long deltaTime) override;
		void DrawWorld(unsigned long long deltaTime) override;
		void DrawUI(unsigned long long deltaTime) override;

	private:
		enum class State { Opening, Revealing, Done };

		Game* game;
		std::vector<std::string> cardIds;

		std::shared_ptr<DX9GF::TransformManager> transformManager;
		std::shared_ptr<DX9GF::Texture> packTex;
		std::shared_ptr<DX9GF::Texture> uiTex;
		std::shared_ptr<DX9GF::Texture> particleTex;
		std::shared_ptr<DX9GF::AnimatedSprite> packSprite;
		std::shared_ptr<DX9GF::StaticSprite> cardFaceSprite;
		std::shared_ptr<DX9GF::Font> font;
		std::shared_ptr<DX9GF::FontSprite> fontSprite;
		std::unique_ptr<DX9GF::ParticleSystem> explosionEmitter;

		std::vector<RECT> faceRects;
		std::vector<CardCatalog::Rarity> rarities;

		State state = State::Opening;
		int revealIndex = 0;
		float cardAlpha = 0.f;
		float holdTimer = 0.f;      // brief pause between the burst and the first card
		bool burstFired = false;
		bool pendingCardSound = false; // play the card-draw sfx when the next card starts appearing
		bool shouldLeave = false;
	};
}
