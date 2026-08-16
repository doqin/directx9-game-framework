#pragma once
#include "IContainer.h"
#include "IStatementCard.h"
#include "ICard.h"

namespace Demo {
	class CardContainer : public IContainer {
	private:
		std::vector<std::shared_ptr<ICard>>* playedPile = nullptr;
	public:
		CardContainer(std::weak_ptr<DX9GF::TransformManager> tm, size_t width, size_t height, float x = 0, float y = 0)
         : IGameObject(tm, x, y), IContainer(tm, width, height, x, y) {
		}
		void Init(std::shared_ptr<DraggableManager> manager, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) override;
        void Init(std::shared_ptr<DraggableManager> manager, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, std::vector<std::shared_ptr<ICard>>* playedPile);
		bool OnDrop(std::shared_ptr<IDraggable> other) override;
		void StoreCard(std::shared_ptr<ICard> card);
	};
}
