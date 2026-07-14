#include "pch.h"
#include "DX9GFRectangleCollider.h"
#include "DX9GFEllipseCollider.h"
#include "../DX9GFUtils.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace {
	constexpr float collisionEpsilon = 0.001f;
	constexpr float floatEpsilon = 0.000001f;
	constexpr float rotationEpsilon = 0.0001f;

	struct Vec2 {
		float x;
		float y;
	};

	Vec2 operator+(const Vec2& a, const Vec2& b) { return { a.x + b.x, a.y + b.y }; }
	Vec2 operator-(const Vec2& a, const Vec2& b) { return { a.x - b.x, a.y - b.y }; }
	Vec2 operator*(const Vec2& v, float s) { return { v.x * s, v.y * s }; }

	float Dot(const Vec2& a, const Vec2& b)
	{
		return a.x * b.x + a.y * b.y;
	}

	float LengthSq(const Vec2& v)
	{
		return v.x * v.x + v.y * v.y;
	}

	Vec2 Normalize(const Vec2& v)
	{
		float lenSq = LengthSq(v);
		if (lenSq <= floatEpsilon) {
			return { 0.0f, 0.0f };
		}
		float invLen = 1.0f / std::sqrt(lenSq);
		return { v.x * invLen, v.y * invLen };
	}

	Vec2 Rotate(const Vec2& v, float radians)
	{
		float c = std::cos(radians);
		float s = std::sin(radians);
		return { v.x * c - v.y * s, v.x * s + v.y * c };
	}

	Vec2 GetCenter(const std::array<Vec2, 4>& corners)
	{
		return (corners[0] + corners[1] + corners[2] + corners[3]) * 0.25f;
	}

	void ProjectOntoAxis(const std::array<Vec2, 4>& corners, const Vec2& axis, float& outMin, float& outMax)
	{
		float p0 = Dot(corners[0], axis);
		outMin = p0;
		outMax = p0;
		for (size_t i = 1; i < corners.size(); i++) {
			float p = Dot(corners[i], axis);
			outMin = (std::min)(outMin, p);
			outMax = (std::max)(outMax, p);
		}
	}

	bool ComputeObbMtv(
		const std::array<Vec2, 4>& a,
		const std::array<Vec2, 4>& b,
		Vec2& outMtv
	)
	{
		Vec2 axes[4] = {
			Normalize(a[1] - a[0]),
			Normalize(a[3] - a[0]),
			Normalize(b[1] - b[0]),
			Normalize(b[3] - b[0])
		};

		float bestOverlap = (std::numeric_limits<float>::infinity)();
		Vec2 bestAxis = { 0.0f, 0.0f };

		for (const auto& axisRaw : axes) {
			if (LengthSq(axisRaw) <= floatEpsilon) {
				continue;
			}

			Vec2 axis = axisRaw;
			float minA, maxA, minB, maxB;
			ProjectOntoAxis(a, axis, minA, maxA);
			ProjectOntoAxis(b, axis, minB, maxB);

			float overlap = (std::min)(maxA, maxB) - (std::max)(minA, minB);
			if (overlap <= floatEpsilon) {
				return false;
			}

			if (overlap < bestOverlap) {
				bestOverlap = overlap;
				bestAxis = axis;
			}
		}

		Vec2 centerA = GetCenter(a);
		Vec2 centerB = GetCenter(b);
		Vec2 delta = centerB - centerA;
		if (Dot(delta, bestAxis) < 0.0f) {
			bestAxis = bestAxis * -1.0f;
		}

		// bestAxis points from A to B, so push A away from B
		outMtv = bestAxis * (-(bestOverlap + collisionEpsilon));
		return true;
	}

	std::array<Vec2, 4> BuildCorners(DX9GF::RectangleCollider& collider, float worldX, float worldY)
	{
		float r = collider.GetWorldRotation();
		float sx = collider.GetWorldScaleX();
		float sy = collider.GetWorldScaleY();
		float ox = collider.GetOriginX();
		float oy = collider.GetOriginY();

		std::array<Vec2, 4> local = {
			Vec2{ 0.0f, 0.0f },
			Vec2{ collider.GetWidth(), 0.0f },
			Vec2{ collider.GetWidth(), collider.GetHeight() },
			Vec2{ 0.0f, collider.GetHeight() }
		};

		std::array<Vec2, 4> worldCorners;
		for (size_t i = 0; i < local.size(); i++) {
			Vec2 p = { (local[i].x - ox) * sx, (local[i].y - oy) * sy };
			p = Rotate(p, r);
			worldCorners[i] = { worldX + p.x, worldY + p.y };
		}
		return worldCorners;
	}

	// Extents of an unrotated collider relative to its world position
	// (handles negative scale)
	void GetAxisAlignedExtents(DX9GF::RectangleCollider& collider, Vec2& outMin, Vec2& outMax)
	{
		float sx = collider.GetWorldScaleX();
		float sy = collider.GetWorldScaleY();
		float ox = collider.GetOriginX();
		float oy = collider.GetOriginY();

		float x0 = (0.0f - ox) * sx;
		float x1 = (collider.GetWidth() - ox) * sx;
		float y0 = (0.0f - oy) * sy;
		float y1 = (collider.GetHeight() - oy) * sy;

		outMin = { (std::min)(x0, x1), (std::min)(y0, y1) };
		outMax = { (std::max)(x0, x1), (std::max)(y0, y1) };
	}
}

DX9GF::ICollider::AABB DX9GF::RectangleCollider::GetWorldAABB()
{
	float worldX = GetWorldX();
	float worldY = GetWorldY();

	if (std::abs(GetWorldRotation()) < rotationEpsilon) {
		Vec2 min, max;
		GetAxisAlignedExtents(*this, min, max);
		return { worldX + min.x, worldY + min.y, worldX + max.x, worldY + max.y };
	}

	auto corners = BuildCorners(*this, worldX, worldY);
	AABB aabb = { corners[0].x, corners[0].y, corners[0].x, corners[0].y };
	for (size_t i = 1; i < corners.size(); i++) {
		aabb.minX = (std::min)(aabb.minX, corners[i].x);
		aabb.minY = (std::min)(aabb.minY, corners[i].y);
		aabb.maxX = (std::max)(aabb.maxX, corners[i].x);
		aabb.maxY = (std::max)(aabb.maxY, corners[i].y);
	}
	return aabb;
}

bool DX9GF::RectangleCollider::IsCollidedRectangle(std::weak_ptr<RectangleCollider> other, bool checkOther)
{
	auto lock = other.lock();
	if (!lock) {
		return false;
	}

	auto a = BuildCorners(*this, GetWorldX(), GetWorldY());
	auto b = BuildCorners(*lock, lock->GetWorldX(), lock->GetWorldY());

	Vec2 mtv;
	return ComputeObbMtv(a, b, mtv);
}

bool DX9GF::RectangleCollider::IsCollided(std::weak_ptr<ICollider> other)
{
	auto lock = other.lock();
	if (auto rectangleCollider = dynamic_pointer_cast<RectangleCollider>(lock)) {
		return IsCollidedRectangle(rectangleCollider, true);
	}
	else if (auto ellipseCollider = dynamic_pointer_cast<EllipseCollider>(lock)) {
		return ICollider::IsCollided(dynamic_pointer_cast<RectangleCollider>(shared_from_this()), ellipseCollider);
	}
	else {
		throw std::runtime_error("Unknown collider, method has not implemented logic");
	}
}

bool DX9GF::RectangleCollider::CheckAxisIntersection(float current, float delta, float expandedMin, float expandedMax, float& tNear, float& tFar) const
{
    if (delta == 0) {
        if (current < expandedMin || current > expandedMax) {
            return false;
        }
    }
    else {
        float t1 = (expandedMin - current) / delta;
        float t2 = (expandedMax - current) / delta;
        if (t1 > t2) std::swap(t1, t2);
        tNear = (std::max)(tNear, t1);
        tFar = (std::min)(tFar, t2);
        if (tNear > tFar) return false;
    }
    return true;
}

// Claude did my homework :DDDD
std::optional<std::tuple<float, float>> DX9GF::RectangleCollider::IsIntersecting(std::weak_ptr<ICollider> other, float newX, float newY)
{
    auto lock = other.lock();
    if (auto rectangleCollider = dynamic_pointer_cast<RectangleCollider>(lock)) {
		float currentX = GetWorldX();
		float currentY = GetWorldY();

		float otherX = rectangleCollider->GetWorldX();
		float otherY = rectangleCollider->GetWorldY();

		auto otherCorners = BuildCorners(*rectangleCollider, otherX, otherY);
		auto IsOverlappingAt = [&](float testX, float testY) {
			auto selfCorners = BuildCorners(*this, testX, testY);
			Vec2 mtv;
			return ComputeObbMtv(selfCorners, otherCorners, mtv);
		};

		bool isOverlappingNow = IsOverlappingAt(currentX, currentY);
		if (isOverlappingNow) {
			bool isOverlappingAtNewPosition = IsOverlappingAt(newX, newY);
			if (!isOverlappingAtNewPosition) {
				return std::nullopt;
			}

			auto selfCornersAtNew = BuildCorners(*this, newX, newY);
			Vec2 mtv;
			if (!ComputeObbMtv(selfCornersAtNew, otherCorners, mtv)) {
				return std::nullopt;
			}

			return std::make_tuple(newX + mtv.x, newY + mtv.y);
		}

		float dx = newX - currentX;
		float dy = newY - currentY;
		if (dx == 0.0f && dy == 0.0f) {
			return std::nullopt;
		}

		// Fast path: both rects are axis-aligned, so the sweep has an exact
		// analytic solution (Minkowski-expanded slab test) — no stepping needed
		if (std::abs(GetWorldRotation()) < rotationEpsilon
			&& std::abs(rectangleCollider->GetWorldRotation()) < rotationEpsilon) {
			Vec2 selfMin, selfMax, otherMin, otherMax;
			GetAxisAlignedExtents(*this, selfMin, selfMax);
			GetAxisAlignedExtents(*rectangleCollider, otherMin, otherMax);

			// Expand the other rect by self's half extents and sweep self's
			// center point against it
			Vec2 selfHalf = { (selfMax.x - selfMin.x) * 0.5f, (selfMax.y - selfMin.y) * 0.5f };
			Vec2 selfCenter = {
				currentX + (selfMin.x + selfMax.x) * 0.5f,
				currentY + (selfMin.y + selfMax.y) * 0.5f
			};
			float expandedMinX = otherX + otherMin.x - selfHalf.x;
			float expandedMaxX = otherX + otherMax.x + selfHalf.x;
			float expandedMinY = otherY + otherMin.y - selfHalf.y;
			float expandedMaxY = otherY + otherMax.y + selfHalf.y;

			float tNear = 0.0f;
			float tFar = 1.0f;
			if (!CheckAxisIntersection(selfCenter.x, dx, expandedMinX, expandedMaxX, tNear, tFar)
				|| !CheckAxisIntersection(selfCenter.y, dy, expandedMinY, expandedMaxY, tNear, tFar)) {
				return std::nullopt;
			}

			float hitDistance = std::sqrt(dx * dx + dy * dy);
			float correctedT = (std::max)(0.0f, tNear - collisionEpsilon / hitDistance);
			return std::make_tuple(currentX + dx * correctedT, currentY + dy * correctedT);
		}

		float distance = std::sqrt(dx * dx + dy * dy);
		float minFeature = (std::min)(width * std::abs(GetWorldScaleX()), height * std::abs(GetWorldScaleY()));
		int steps = 16;
		if (minFeature > 0.0f) {
			steps = (std::max)(steps, static_cast<int>(std::ceil((distance / minFeature) * 4.0f)));
		}
		steps = (std::min)(steps, 512);

		float previousT = 0.0f;
		for (int i = 1; i <= steps; i++) {
			float t = static_cast<float>(i) / static_cast<float>(steps);
			float testX = currentX + dx * t;
			float testY = currentY + dy * t;
			if (IsOverlappingAt(testX, testY)) {
				float low = previousT;
				float high = t;
				for (int j = 0; j < 12; j++) {
					float mid = (low + high) / 2.0f;
					float midX = currentX + dx * mid;
					float midY = currentY + dy * mid;
					if (IsOverlappingAt(midX, midY)) {
						high = mid;
					}
					else {
						low = mid;
					}
				}

				float correctedT = (std::max)(0.0f, high - collisionEpsilon);
				return std::make_tuple(currentX + dx * correctedT, currentY + dy * correctedT);
			}

			previousT = t;
		}

		return std::nullopt;
    }
    else if (auto ellipseCollider = dynamic_pointer_cast<EllipseCollider>(lock)) {
        return ICollider::IsIntersecting(dynamic_pointer_cast<RectangleCollider>(shared_from_this()), ellipseCollider, newX, newY);
    }
    else {
        throw std::runtime_error("Unknown collider, method has not implemented logic");
    }
}

void DX9GF::RectangleCollider::SetOrigin(float x, float y)
{
	originX = x;
	originY = y;
}

void DX9GF::RectangleCollider::SetOriginCenter()
{
	originX = width * 0.5f;
	originY = height * 0.5f;
}

float DX9GF::RectangleCollider::GetOriginX() const
{
	return originX;
}

float DX9GF::RectangleCollider::GetOriginY() const
{
	return originY;
}

float DX9GF::RectangleCollider::GetWidth() const
{
    return width;
}

float DX9GF::RectangleCollider::GetHeight() const
{
    return height;
}

void DX9GF::RectangleCollider::Draw(GraphicsDevice* graphicsDevice, const Camera& camera)
{
	if (!ICollider::drawCollider) return;

	float x = GetWorldX();
	float y = GetWorldY();
	float a = GetWorldRotation();
	float sx = GetWorldScaleX();
	float sy = GetWorldScaleY();
	float ox = GetOriginX();
	float oy = GetOriginY();
	graphicsDevice->DrawRectangle(
		camera,
		x,
		y,
		width,
		height,
		a,
		sx,
		sy,
		ox,
		oy,
		0xFF00FF00,
		false
	);

}
