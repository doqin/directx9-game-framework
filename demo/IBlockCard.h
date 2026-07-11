#pragma once
#include "ICard.h"
#include "IContainer.h"
#include "IStatementCard.h"

namespace Demo {
	class IBattleScene;

  class IBlockCard : public ICard, public IContainer {
	private:
		size_t executeIndex = 0;
		bool isExecuting = false;
		std::weak_ptr<IStatementCard> currentExecutingCard;
		float timeSinceLastExecution = 0;
		const float timePerExecution = .5f;
		IBattleScene* battleScene = nullptr;
		float timeSinceLastEnergyPopUp = 999.f;
		const float energyPopUpCooldown = 3.f;
	protected:
		std::vector<std::weak_ptr<IStatementCard>> statementCards;
	public:
		inline IBlockCard(std::weak_ptr<DX9GF::TransformManager> transformManager)
			: IGameObject(transformManager), ICard(transformManager), IContainer(transformManager) {}
		inline IBlockCard(
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
			IContainer(transformManager, dragAreaWidth, dragAreaHeight, x, y, rotation, scaleX, scaleY) {}
		inline IBlockCard(
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
			IContainer(transformManager, parent, dragAreaWidth, dragAreaHeight, x, y, rotation, scaleX, scaleY) {}
		bool OnDrop(std::shared_ptr<IDraggable> other) override;
		void Update(unsigned long long deltaTime) override;
		void StartExecution();
		void ExecuteIteratively(unsigned long long deltaTime);
		bool IsExecuting() const;
		std::shared_ptr<IStatementCard> GetCurrentExecutingCard() const;
		void ResetExecution();
		void SetBattleScene(IBattleScene* scene) { battleScene = scene; }
		bool HasAllRequiredTargets() const;
		// Inserts (or moves, if already attached) a statement card at the given position in the
		// execution queue, shifting the rest as needed (used by keyboard navigation).
		// Returns false if the card can't be attached (e.g. not enough energy for a brand new card).
		bool InsertStatementCardAt(std::shared_ptr<IStatementCard> card, size_t index);
		// World-space position of the slot at the given queue index (0 = first to execute).
		// Pass the card being moved as "excluding" so its own current slot isn't counted twice.
		std::tuple<float, float> GetStatementSlotWorldPosition(size_t index, std::shared_ptr<IStatementCard> excluding = nullptr);
		// Ordered list (execution order) of statement cards currently queued in this block.
		const std::vector<std::weak_ptr<IStatementCard>>& GetStatementCards() const { return statementCards; }
	};
}
