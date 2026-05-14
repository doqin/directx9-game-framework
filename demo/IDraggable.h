#pragma once
#include "DX9GFExtras.h"
#include "DX9GF.h"

namespace Demo {
	class DraggableManager;
  class IDraggable : virtual public DX9GF::IGameObject {
	protected:
		inline static size_t nextID = 0;
		std::string id;
		size_t dragAreaWidth;
		size_t dragAreaHeight;
		std::shared_ptr<DX9GF::RectangleTrigger> trigger;
		std::shared_ptr<Demo::DraggableManager> draggableManager;
		DX9GF::Camera* camera = nullptr;
		DX9GF::GraphicsDevice* graphicsDevice = nullptr;
		D3DXCOLOR color;
		static std::shared_ptr<DX9GF::Font> debugFont;
		static std::shared_ptr<DX9GF::FontSprite> debugFontSprite;
		bool isDragging = false;
		RECT scissorRect;
		bool isCropped = false;
	public:
		inline static bool debug = false;
		inline IDraggable(std::weak_ptr<DX9GF::TransformManager> transformManager) : IGameObject(transformManager), dragAreaWidth(0), dragAreaHeight(0) { 
			id = "draggable" + std::to_string(nextID++); 
		}
		inline IDraggable(
			std::weak_ptr<DX9GF::TransformManager> transformManager, 
			size_t dragAreaWidth, 
			size_t dragAreaHeight, 
			float x = 0, 
			float y = 0, 
			float rotation = 0, 
			float scaleX = 1, 
			float scaleY = 1
		) : IGameObject(transformManager, x, y, rotation, scaleX, scaleY), 
			dragAreaWidth(dragAreaWidth), 
			dragAreaHeight(dragAreaHeight) { 
			id = "draggable" + std::to_string(nextID++);
		}
		inline IDraggable(
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
			dragAreaWidth(dragAreaWidth),
			dragAreaHeight(dragAreaHeight) {
			id = "draggable" + std::to_string(nextID++);
		}
		virtual void Init(std::shared_ptr<DraggableManager> manager, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera);
		void SetParent(std::weak_ptr<IGameObject> parent);
		void DetachParent();
		virtual void Update(unsigned long long deltaTime);
		virtual void Draw(unsigned long long deltaTime);
		virtual bool OnDrop(std::shared_ptr<IDraggable> other);
		virtual bool OnHover(std::shared_ptr<IDraggable> other);
		void SetScissorRect(const RECT& rect);
		void ClearScissorRect();
		std::string GetID() const;
		virtual size_t GetWidth() const;
		virtual size_t GetHeight() const;
		std::weak_ptr<DraggableManager> GetDraggableManager();
		std::weak_ptr<DX9GF::RectangleTrigger> GetTrigger();
		bool IsDragging() const;
	};

	class DraggableManager final {
	private:
		struct LevelBatch{
			size_t startIdx;
			size_t endIdx;
		};
		std::vector<std::weak_ptr<IDraggable>> hierarchy;
		std::vector<LevelBatch> levels;
		std::unordered_map<std::string, std::shared_ptr<IDraggable>> objectMap;
		DX9GF::CommandBuffer drawBuffer;
	public:
		void RebuildHierarchy();
		void Add(std::shared_ptr<IDraggable> obj);
		void Remove(std::shared_ptr<IDraggable> obj);
		void HoverDroppable(std::shared_ptr<IDraggable> obj);
		void AttachDroppable(std::shared_ptr<IDraggable> obj);
		void Update(unsigned long long deltaTime);
		void Draw(unsigned long long deltaTime);
		void QueueDraw(std::shared_ptr<DX9GF::ICommand> cmd);
	};
}