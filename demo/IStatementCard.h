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

	public:
		virtual void Draw(unsigned long long deltaTime) override;
	};
}
