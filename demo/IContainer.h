#pragma once
#include "DX9GFExtras.h"
#include "IDraggable.h"

namespace Demo {
	class IContainer : public IDraggable {
	private:
		size_t GetMaxWidthOfChildren();
		size_t GetHeightOfChildren();
		size_t maxHeight = 0;
		float scrollOffset = 0;
	protected:
		std::vector<std::weak_ptr<IDraggable>> children;
		bool isHovered = false;
	public:
       inline IContainer(std::weak_ptr<DX9GF::TransformManager> transformManager)
			: IGameObject(transformManager), IDraggable(transformManager), maxHeight(0), scrollOffset(0) { }
		inline IContainer(
			std::weak_ptr<DX9GF::TransformManager> transformManager, 
			size_t dragAreaWidth, 
			size_t dragAreaHeight,
			size_t maxHeight = 0,
			float x = 0,
			float y = 0,
			float rotation = 0,
			float scaleX = 1,
			float scaleY = 1
     ) : IGameObject(transformManager, x, y, rotation, scaleX, scaleY), 
		 IDraggable(transformManager, dragAreaWidth, dragAreaHeight, x, y, rotation, scaleX, scaleY),
		 maxHeight(maxHeight), scrollOffset(0) { }
		inline IContainer(
			std::weak_ptr<DX9GF::TransformManager> transformManager,
			std::weak_ptr<DX9GF::IGameObject> parent,
			size_t dragAreaWidth,
			size_t dragAreaHeight,
			size_t maxHeight = 0,
			float x = 0,
			float y = 0,
			float rotation = 0,
			float scaleX = 1,
			float scaleY = 1
     ) : IGameObject(transformManager, parent, x, y, rotation, scaleX, scaleY), 
		 IDraggable(transformManager, parent, dragAreaWidth, dragAreaHeight, x, y, rotation, scaleX, scaleY),
		 maxHeight(maxHeight), scrollOffset(0) { }
		bool OnHover(std::shared_ptr<IDraggable> other) override;
		bool OnDrop(std::shared_ptr<IDraggable> other) override;
		void Update(unsigned long long deltaTime) override;
		void Draw(unsigned long long deltaTime) override;
		void AddChildProgrammatically(std::shared_ptr<IDraggable> child);
		const std::vector<std::weak_ptr<IDraggable>>& GetChildren() const { return children; }
		void ClearChildren();
		void SetMaxHeight(size_t height) { maxHeight = height; }
		size_t GetMaxHeight() const { return maxHeight; }
	};
}