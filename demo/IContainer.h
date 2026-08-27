#pragma once
#include "DX9GFExtras.h"
#include "IDraggable.h"

namespace Demo {
	class IContainer : public IDraggable {
	private:
		size_t maxHeight = 0;
		float scrollOffset = 0;
		// When on, Draw() hides children scrolled fully outside the visible band so the
		// DraggableManager skips their draw entirely (default off: battle containers are small).
		bool cullOffscreen = false;
		// Re-stacks children from dragAreaHeight, applying scrollOffset. Shared by Update and
		// ScrollChildIntoView so a programmatic scroll takes effect the same frame.
		void LayoutChildren();
	protected:
		size_t GetMaxWidthOfChildren();
		size_t GetHeightOfChildren();
		std::vector<std::weak_ptr<IDraggable>> children;
		bool isHovered = false;
		// Parents, positions (at the next stacking slot) and appends child to the children list, skipping the hit-test in OnDrop.
		void AttachChild(std::shared_ptr<IDraggable> child);
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
		// Parents `child` to this container without touching the children list (caller pairs it
		// with SetChildList). Cheap under DraggableManager::SetDeferRebuild.
		void AdoptChild(const std::shared_ptr<IDraggable>& child);
		// Replaces the children list wholesale with an already-parented set (see AdoptChild).
		void SetChildList(const std::vector<std::shared_ptr<IDraggable>>& newChildren);
		// Nudges scrollOffset so `child` sits fully inside the visible band. No-op unless
		// maxHeight is set, the list overflows, and `child` belongs to this container.
		void ScrollChildIntoView(const std::shared_ptr<IDraggable>& child);
		void SetCulling(bool enabled) { cullOffscreen = enabled; }
		void SetMaxHeight(size_t height) { maxHeight = height; }
		size_t GetMaxHeight() const { return maxHeight; }
	};
}