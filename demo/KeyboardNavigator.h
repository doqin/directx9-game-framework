#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include <functional>
#include <memory>
#include <vector>

namespace Demo {
	// Shared keyboard-navigation cursor: pressing a movement keybind enters keyboard
	// mode and cycles a target reticle between on-screen candidates, ACCEPT activates
	// the target, and any mouse motion returns to mouse mode.
	class KeyboardNavigator {
	public:
		struct Candidate {
			std::shared_ptr<DX9GF::IGameObject> anchor; // identity + lifetime
			float x; // top-left world position; NOT always anchor->GetWorldX() (e.g. center-origin triggers)
			float y;
			float width;
			float height;
			std::function<void()> activate;
		};

		static constexpr float DIRECTION_DOT_THRESHOLD = 0.3f;
		// How much a candidate's off-axis (perpendicular) offset counts against it relative
		// to its along-axis distance, so a candidate roughly "in front of" the cursor wins
		// over one that's merely closer in raw Euclidean distance but well off to the side.
		static constexpr float PERPENDICULAR_PENALTY = 2.5f;

		// Enters keyboard mode on a movement keybind press, exits (and clears the target)
		// on mouse motion. Call once per frame before Navigate.
		void UpdateMode();
		// Target upkeep + directional cycling + ACCEPT activation over this frame's
		// candidates. No-op outside keyboard mode.
		void Navigate(unsigned long long deltaTime, const std::vector<Candidate>& candidates);
		// Convenience for screens with no extra input layers: UpdateMode + Navigate.
		void Update(unsigned long long deltaTime, const std::vector<Candidate>& candidates);
		// Draws the pulsing reticle over the current target (no-op outside keyboard mode
		// or when the target isn't among the given candidates).
		void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera& camera, const std::vector<Candidate>& candidates);

		bool IsInKeyboardMode() const { return keyboardMode; }
		std::shared_ptr<DX9GF::IGameObject> GetTarget() const { return target; }
		void Reset();
		// On the next Navigate call, move the target to the candidate nearest (x, y)
		// instead of following the current target object. Used by activations that
		// teleport the target elsewhere (e.g. moving a card to another container) so
		// the cursor stays put for the next item in a row.
		void RetargetNearest(float x, float y);

		// Directional pick shared with bespoke navigation loops (e.g. the battle scene's
		// placement slots): returns the index of the best candidate in the pressed
		// direction, or -1. Compares top-left corners (centers would inject false
		// horizontal offsets between left-aligned items of differing widths).
		static int PickDirectional(
			float currentX,
			float currentY,
			int dirX,
			int dirY,
			const std::vector<std::pair<float, float>>& positions,
			int excludeIndex);

	private:
		bool keyboardMode = false;
		std::shared_ptr<DX9GF::IGameObject> target;
		bool hasPendingRetarget = false;
		float retargetX = 0.f;
		float retargetY = 0.f;
		// Reflects the keyboard cursor in button sprites: HOVER while targeted (with the
		// hover sfx when the cursor lands), CLICKED while the ACCEPT key is held.
		void ApplyButtonVisuals(const std::shared_ptr<DX9GF::IGameObject>& previousTarget);
	};
}
