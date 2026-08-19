#pragma once
#include "DX9GFITrigger.h"

namespace DX9GF {
	class RectangleTrigger : public ITrigger {
	private:
		float width;
		float height;
		float originX;
		float originY;
	protected:
	public:
		RectangleTrigger(std::weak_ptr<TransformManager> transformManager) : width(0), height(0), originX(0), originY(0), ITrigger(transformManager) {}
		RectangleTrigger(
			std::weak_ptr<TransformManager> transformManager, 
			float width, 
			float height, 
			float x = 0, 
			float y = 0, 
			float rotation = 0, 
			float scaleX = 1, 
			float scaleY = 1
		) : width(width), height(height), originX(0), originY(0), ITrigger(transformManager, x, y, rotation, scaleX, scaleY) {}
		RectangleTrigger(
			std::weak_ptr<TransformManager> transformManager, 
			std::weak_ptr<IGameObject> parent, 
			float width, 
			float height, 
			float x = 0, 
			float y = 0, 
			float rotation = 0, 
			float scaleX = 1, 
			float scaleY = 1
		) : width(width), height(height), originX(0), originY(0), ITrigger(transformManager, parent, x, y, rotation, scaleX, scaleY) {}
		void SetOrigin(float x, float y);
		void SetOriginCenter();
		void SetSize(float width, float height);
		float GetOriginX() const;
		float GetOriginY() const;
		bool IsHovering(unsigned long long deltaTime) override;
		float GetWidth() const;
		float GetHeight() const;
		void SetWidth(float width);
		void SetHeight(float height);

		void Draw(GraphicsDevice* graphicsDevice, const Camera& camera) override;
	};
}