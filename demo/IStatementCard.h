#pragma once
#include "ICard.h"
#include "IDraggable.h"

namespace Demo {
	class EnemyCard;
	class IStatementCard : public ICard, public IDraggable {
	public:
		inline IStatementCard(std::weak_ptr<DX9GF::TransformManager> transformManager)
			: IGameObject(transformManager), ICard(transformManager), IDraggable(transformManager) {
		}
		inline IStatementCard(
			std::weak_ptr<DX9GF::TransformManager> transformManager,
			size_t dragAreaWidth,
			size_t dragAreaHeight,
			float x = 0,
			float y = 0,
			float rotation = 0,
			float scaleX = 1,
			float scaleY = 1
		) : IGameObject(transformManager, x, y, rotation, scaleX, scaleY),
			ICard(transformManager, x, y, rotation, scaleX, scaleY),
			IDraggable(transformManager, dragAreaWidth, dragAreaHeight, x, y, rotation, scaleX, scaleY) {
		}
		inline IStatementCard(
			std::weak_ptr<DX9GF::TransformManager> transformManager,
			std::weak_ptr<DX9GF::IGameObject> parent,
			size_t dragAreaWidth,
			size_t dragAreaHeight,
			float x = 0,
			float y = 0,
			float rotation = 0,
			float scaleX = 1,
			float scaleY = 1
		) : IGameObject(transformManager, parent, x, y, rotation, scaleX, scaleY),
			ICard(transformManager, parent, x, y, rotation, scaleX, scaleY),
			IDraggable(transformManager, parent, dragAreaWidth, dragAreaHeight, x, y, rotation, scaleX, scaleY) {
		}
		virtual bool Execute() = 0;
		virtual void ResetExecution() {}

		virtual std::wstring GetInputsDescription() const { return L"None"; }
		virtual bool HasRequiredTargets() const { return true; }

		// EnemyCard targeting - overridden by cards that consume enemy targets (StrikeCard,
		// MultiTargetCard) so keyboard navigation can treat them uniformly as destinations.
		virtual bool CanAcceptEnemyCard() const { return false; }
		// Attaches without the position hit-test used by OnDrop. Returns false if full.
		virtual bool AttachEnemyCard(std::shared_ptr<EnemyCard> card) { return false; }
		// World-space position the next attached enemy card would occupy.
		virtual std::tuple<float, float> GetEnemyCardSlotWorldPosition() const { return { GetWorldX(), GetWorldY() }; }
	protected:
		std::shared_ptr<DX9GF::Font> descFont;
		std::shared_ptr<DX9GF::FontSprite> descFontSprite;

		// Cover panel drawn off the card's right edge for cards with limited uses, holding one
		// pip per use. Geometry below is in ui.png pixels; cards render at USES_COVER_SCALE.
		static constexpr int USES_COVER_SCALE = 2;
		static constexpr int USES_COVER_CAP_WIDTH = 3;
		// The card art's right edge is a black outline with rounded corners, so the panel is
		// pulled back over it - otherwise the outline reads as a seam and the corners leave
		// notches of background between the two.
		static constexpr int USES_COVER_OVERLAP = 2;
		// Asymmetric because the rounded cap already supplies visual margin on the right.
		static constexpr int USES_COVER_PAD_LEFT = 5;
		static constexpr int USES_COVER_PAD_RIGHT = 2;
		static constexpr int USES_PIP_SIZE = 6;
		static constexpr int USES_PIP_GAP = 2;
		static constexpr int USES_PIP_TOP = 5;

		std::shared_ptr<DX9GF::Texture> usesTexture;
		std::shared_ptr<DX9GF::StaticSprite> usesCoverBody;
		std::shared_ptr<DX9GF::StaticSprite> usesCoverCap;
		std::shared_ptr<DX9GF::StaticSprite> usesPipFull;
		std::shared_ptr<DX9GF::StaticSprite> usesPipSpent;

		// Panel width in sheet pixels, sized to the pips so there is no dead grey space.
		int GetUsesCoverPanelWidth() const;
		void DrawUsesCover(unsigned long long deltaTime);

		// The card's own artwork. Override this rather than Draw() so the face lands on top of
		// the uses cover - the cover is pulled back over the card's right edge, and drawing the
		// face second is what keeps that edge visible instead of buried under the panel.
		virtual void DrawCardFace(unsigned long long deltaTime) {}

	public:
		// How far the uses cover extends past the card on screen, net of its overlap, or 0 when
		// the card has no use limit. Included in GetWidth() so containers size and crop to it
		// instead of clipping it off.
		size_t GetUsesCoverWidth() const;
		size_t GetWidth() const override;
		virtual void Draw(unsigned long long deltaTime) override;
	};
}
