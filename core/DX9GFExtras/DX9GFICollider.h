#pragma once
#include "DX9GFIGameObject.h"
#include "../DX9GFGraphicsDevice.h"
#include <optional>
#include <tuple>

namespace DX9GF {
	class RectangleCollider;
	class EllipseCollider;

	class ICollider : public IGameObject {
	private:
	protected:
		ICollider(std::weak_ptr<TransformManager> transformManager) : IGameObject(transformManager) {}
		ICollider(std::weak_ptr<TransformManager> transformManager, float x, float y, float rotation = 0, float scaleX = 1, float scaleY = 1) : IGameObject(transformManager, x, y, rotation, scaleX, scaleY) {}
		ICollider(std::weak_ptr<TransformManager> transformManager, std::weak_ptr<IGameObject> parent, float x, float y, float rotation = 0, float scaleX = 1, float scaleY = 1) : IGameObject(transformManager, parent, x, y, rotation, scaleX, scaleY) {}
	public:
		// Conservative world-space bounding box, used for broad-phase pruning
		struct AABB {
			float minX;
			float minY;
			float maxX;
			float maxY;

			bool Overlaps(const AABB& other) const {
				return minX <= other.maxX && maxX >= other.minX
					&& minY <= other.maxY && maxY >= other.minY;
			}
		};
		virtual AABB GetWorldAABB() = 0;
		virtual bool IsCollided(std::weak_ptr<ICollider> other) = 0;
		virtual std::optional<std::tuple<float, float>> IsIntersecting(std::weak_ptr<ICollider> other, float newX, float newY) = 0;
		static std::optional<std::tuple<float, float>> IsIntersecting(std::weak_ptr<RectangleCollider> targetCollider, std::weak_ptr<EllipseCollider> otherCollider, float newX, float newY);
		static std::optional<std::tuple<float, float>> IsIntersecting(std::weak_ptr<EllipseCollider> targetCollider, std::weak_ptr<RectangleCollider> otherCollider, float newX, float newY);

		static bool IsCollided(std::weak_ptr<RectangleCollider> targetCollider, std::weak_ptr<EllipseCollider> otherCollider);
		static bool IsCollided(std::weak_ptr<EllipseCollider> targetCollider, std::weak_ptr<RectangleCollider> otherCollider);

		static bool drawCollider;
		virtual void Draw(GraphicsDevice* graphicsDevice, const Camera& camera) {}
	};
}