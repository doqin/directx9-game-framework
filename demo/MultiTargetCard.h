#pragma once
#include "IStatementCard.h"
#include "EnemyCard.h"
#include <vector>
#include <string>

namespace Demo {
	class MultiTargetCard : public IStatementCard {
	protected:
		std::vector<std::weak_ptr<EnemyCard>> targets;
		size_t maxTargets;
		std::wstring cardName;
		bool isDone = false;
		std::shared_ptr<DX9GF::Font> nameFont;
		std::shared_ptr<DX9GF::FontSprite> nameFontSprite;
		// Appends one hit of `damage` per attached target, stopping after `maxHits` of them (0
		// means every target). Covers every damaging card built on this base - only Terminate,
		// whose damage depends on the target's health, needs to roll its own.
		void CollectHitsOnTargets(VirtualBattleState& state, float damage, size_t maxHits = 0);
		// The same walk for cards that apply a modifier to their targets rather than hitting them.
		void CollectEffectOnTargets(VirtualBattleState& state, ModifierType modifier, float value, int duration, size_t maxTargetsHit = 0);
	public:
		MultiTargetCard(std::weak_ptr<DX9GF::TransformManager> tm, size_t maxTargets, std::wstring name, float x = 0, float y = 0, size_t dragAreaWidth = 0, size_t dragAreaHeight = 0);

		bool OnDrop(std::shared_ptr<IDraggable> other) override;
		void Update(unsigned long long deltaTime) override;
		virtual void Draw(unsigned long long deltaTime) override;
		size_t GetWidth() const override;
		void ResetExecution() override;
		std::wstring GetInputsDescription() const override;
		bool HasRequiredTargets() const override { return !targets.empty(); }
		bool CanAcceptEnemyCard() const override { return targets.size() < maxTargets; }
		bool AttachEnemyCard(std::shared_ptr<EnemyCard> card) override;
		void ReleaseEnemyCards() override;
		std::tuple<float, float> GetEnemyCardSlotWorldPosition() const override;
		const std::vector<std::weak_ptr<EnemyCard>>& GetTargets() const { return targets; }
	};
}