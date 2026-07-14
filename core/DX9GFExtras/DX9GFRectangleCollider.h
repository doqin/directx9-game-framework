#pragma once
#include "DX9GFICollider.h"
#include <array>
namespace DX9GF {
	class RectangleCollider : public ICollider {
	private:
		float width;
		float height;
		float originX;
		float originY;
		bool CheckAxisIntersection(float current, float delta, float expandedMin, float expandedMax, float& tNear, float& tFar) const;
	protected:
	public:
		RectangleCollider(std::weak_ptr<TransformManager> transformManager) : ICollider(transformManager), width(0), height(0), originX(0), originY(0) {}
		RectangleCollider(
			std::weak_ptr<TransformManager> transformManager, 
			float width, 
			float height, 
			float x = 0, 
			float y = 0, 
			float rotation = 0, 
			float scaleX = 1, 
			float scaleY = 1
		) : ICollider(transformManager, x, y, rotation, scaleX, scaleY), width(width), height(height), originX(0), originY(0) {}
		RectangleCollider(
			std::weak_ptr<TransformManager> transformManager, 
			std::weak_ptr<IGameObject> parent, 
			float width, 
			float height, 
			float x = 0, 
			float y = 0, 
			float rotation = 0, 
			float scaleX = 1, 
			float scaleY = 1
		) : ICollider(transformManager, parent, x, y, rotation, scaleX, scaleY), width(width), height(height), originX(0), originY(0) {}
		void SetOrigin(float x, float y);
		void SetOriginCenter();
		float GetOriginX() const;
		float GetOriginY() const;
		bool IsCollidedRectangle(std::weak_ptr<RectangleCollider> other, bool checkOther);
		AABB GetWorldAABB() override;
		bool IsCollided(std::weak_ptr<ICollider> other) override;
		std::optional<std::tuple<float, float>> IsIntersecting(std::weak_ptr<ICollider> other, float newX, float newY) override;
		float GetWidth() const;
		float GetHeight() const;

		void Draw(GraphicsDevice* graphicsDevice, const Camera& camera) override;
	};
}