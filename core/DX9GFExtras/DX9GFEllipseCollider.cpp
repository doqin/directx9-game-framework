#include "pch.h"
#include "DX9GFEllipseCollider.h"
#include "DX9GFRectangleCollider.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
	constexpr float collisionEpsilon = 0.001f;
	constexpr float floatEpsilon = 0.000001f;
	constexpr float pi = 3.14159265358979323846f;
	constexpr int overlapSamples = 48;

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
			return { 1.0f, 0.0f };
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

	Vec2 GetEllipseCenter(DX9GF::EllipseCollider& collider, float originWorldX, float originWorldY)
	{
		float rx = collider.GetWidth() * 0.5f;
		float ry = collider.GetHeight() * 0.5f;
		float r = collider.GetWorldRotation();
		float sx = collider.GetWorldScaleX();
		float sy = collider.GetWorldScaleY();
		float ox = collider.GetOriginX();
		float oy = collider.GetOriginY();
		Vec2 centerOffset = Rotate({ (rx - ox) * sx, (ry - oy) * sy }, r);
		return { originWorldX + centerOffset.x, originWorldY + centerOffset.y };
	}

	Vec2 TransformEllipseLocalPointToWorld(DX9GF::EllipseCollider& collider, float originWorldX, float originWorldY, const Vec2& localPoint)
	{
		float r = collider.GetWorldRotation();
		float sx = collider.GetWorldScaleX();
		float sy = collider.GetWorldScaleY();
		float ox = collider.GetOriginX();
		float oy = collider.GetOriginY();
		Vec2 p = Rotate({ (localPoint.x - ox) * sx, (localPoint.y - oy) * sy }, r);
		return { originWorldX + p.x, originWorldY + p.y };
	}

	bool EllipseContainsPoint(DX9GF::EllipseCollider& collider, float originWorldX, float originWorldY, const Vec2& worldPoint)
	{
		float rx = collider.GetWidth() * 0.5f;
		float ry = collider.GetHeight() * 0.5f;
		if (rx <= floatEpsilon || ry <= floatEpsilon) {
			return false;
		}

		float r = collider.GetWorldRotation();
		float sx = collider.GetWorldScaleX();
		float sy = collider.GetWorldScaleY();
		if (std::abs(sx) <= floatEpsilon || std::abs(sy) <= floatEpsilon) {
			return false;
		}

		Vec2 centerWorld = GetEllipseCenter(collider, originWorldX, originWorldY);
		Vec2 d = worldPoint - centerWorld;
		Vec2 localScaled = Rotate(d, -r);
		Vec2 local = { localScaled.x / sx, localScaled.y / sy };

		float nx = local.x / rx;
		float ny = local.y / ry;
		return (nx * nx + ny * ny) <= (1.0f + floatEpsilon);
	}

	bool AreEllipsesOverlapping(
		DX9GF::EllipseCollider& a,
		float aOriginX,
		float aOriginY,
		DX9GF::EllipseCollider& b,
		float bOriginX,
		float bOriginY
	)
	{
		float arx = a.GetWidth() * 0.5f;
		float ary = a.GetHeight() * 0.5f;
		float brx = b.GetWidth() * 0.5f;
		float bry = b.GetHeight() * 0.5f;
		if (arx <= floatEpsilon || ary <= floatEpsilon || brx <= floatEpsilon || bry <= floatEpsilon) {
			return false;
		}

		for (int i = 0; i < overlapSamples; i++) {
			float t = static_cast<float>(i) / static_cast<float>(overlapSamples);
			float theta = t * 2.0f * pi;
			Vec2 localPoint = { arx + std::cos(theta) * arx, ary + std::sin(theta) * ary };
			Vec2 worldPoint = TransformEllipseLocalPointToWorld(a, aOriginX, aOriginY, localPoint);
			if (EllipseContainsPoint(b, bOriginX, bOriginY, worldPoint)) {
				return true;
			}
		}

		for (int i = 0; i < overlapSamples; i++) {
			float t = static_cast<float>(i) / static_cast<float>(overlapSamples);
			float theta = t * 2.0f * pi;
			Vec2 localPoint = { brx + std::cos(theta) * brx, bry + std::sin(theta) * bry };
			Vec2 worldPoint = TransformEllipseLocalPointToWorld(b, bOriginX, bOriginY, localPoint);
			if (EllipseContainsPoint(a, aOriginX, aOriginY, worldPoint)) {
				return true;
			}
		}

		return false;
	}
}

bool DX9GF::EllipseCollider::IsCollidedEllipse(std::weak_ptr<DX9GF::EllipseCollider> other) {
    auto lock = other.lock();
    if (!lock) {
        throw std::runtime_error("lock failed");
    }

	return AreEllipsesOverlapping(*this, GetWorldX(), GetWorldY(), *lock, lock->GetWorldX(), lock->GetWorldY());
}

bool DX9GF::EllipseCollider::IsCollided(std::weak_ptr<ICollider> other)
{
	auto lock = other.lock();
	if (auto ellipseCollider = dynamic_pointer_cast<EllipseCollider>(lock)) {
		return IsCollidedEllipse(ellipseCollider);
	}
	else if (auto rectangleCollider = dynamic_pointer_cast<RectangleCollider>(lock)) {
		return ICollider::IsCollided(dynamic_pointer_cast<EllipseCollider>(shared_from_this()), rectangleCollider);
	}
	else {
		throw std::runtime_error("Unknown collider, method has not implemented logic");
	}
}

float DX9GF::EllipseCollider::GetWidth() const
{
    return width;
}

float DX9GF::EllipseCollider::GetHeight() const
{
    return height;
}

std::optional<std::tuple<float, float>> DX9GF::EllipseCollider::IsIntersecting(std::weak_ptr<ICollider> other, float newX, float newY)
{
    auto lock = other.lock();
    if (auto ellipseCollider = dynamic_pointer_cast<EllipseCollider>(lock)) {
		float currentX = GetWorldX();
		float currentY = GetWorldY();

		float otherX = ellipseCollider->GetWorldX();
		float otherY = ellipseCollider->GetWorldY();

		auto IsOverlappingAt = [&](float testX, float testY) {
			return AreEllipsesOverlapping(*this, testX, testY, *ellipseCollider, otherX, otherY);
		};

		bool isCurrentlyOverlapping = IsOverlappingAt(currentX, currentY);
		if (isCurrentlyOverlapping) {
			bool isOverlappingAtNewPosition = IsOverlappingAt(newX, newY);
			if (!isOverlappingAtNewPosition) {
				return std::nullopt;
			}

			Vec2 centerSelf = GetEllipseCenter(*this, newX, newY);
			Vec2 centerOther = GetEllipseCenter(*ellipseCollider, otherX, otherY);
			Vec2 dir = centerSelf - centerOther;
			if (LengthSq(dir) <= floatEpsilon) {
				dir = { newX - currentX, newY - currentY };
			}
			dir = Normalize(dir);

			float selfRadius = (std::max)(width * std::abs(GetWorldScaleX()), height * std::abs(GetWorldScaleY())) * 0.5f;
			float otherRadius = (std::max)(ellipseCollider->width * std::abs(ellipseCollider->GetWorldScaleX()), ellipseCollider->height * std::abs(ellipseCollider->GetWorldScaleY())) * 0.5f;
			float high = (std::max)(10.0f, selfRadius + otherRadius + 10.0f);
			for (int tries = 0; tries < 6 && IsOverlappingAt(newX + dir.x * high, newY + dir.y * high); tries++) {
				high *= 2.0f;
			}

			if (IsOverlappingAt(newX + dir.x * high, newY + dir.y * high)) {
				return std::nullopt;
			}

			float low = 0.0f;
			for (int i = 0; i < 16; i++) {
				float mid = (low + high) * 0.5f;
				if (IsOverlappingAt(newX + dir.x * mid, newY + dir.y * mid)) {
					low = mid;
				}
				else {
					high = mid;
				}
			}

			return std::make_tuple(newX + dir.x * (high + collisionEpsilon), newY + dir.y * (high + collisionEpsilon));
		}

		float dx = newX - currentX;
		float dy = newY - currentY;
		if (dx == 0.0f && dy == 0.0f) {
			return std::nullopt;
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
					float mid = (low + high) * 0.5f;
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
    else if (auto rectangleCollider = dynamic_pointer_cast<RectangleCollider>(lock)) {
        return ICollider::IsIntersecting(dynamic_pointer_cast<EllipseCollider>(shared_from_this()), rectangleCollider, newX, newY);
    }
    else {
        throw std::runtime_error("Unknown collider, method has not implemented logic");
    }
}

DX9GF::ICollider::AABB DX9GF::EllipseCollider::GetWorldAABB()
{
	// Conservative circle bound around the world center, radius = largest
	// scaled half-extent (over-approximates rotated/stretched ellipses)
	Vec2 center = GetEllipseCenter(*this, GetWorldX(), GetWorldY());
	float radius = 0.5f * (std::max)(
		width * std::abs(GetWorldScaleX()),
		height * std::abs(GetWorldScaleY())
	);
	return { center.x - radius, center.y - radius, center.x + radius, center.y + radius };
}

void DX9GF::EllipseCollider::SetOrigin(float x, float y)
{
	originX = x;
	originY = y;
}

void DX9GF::EllipseCollider::SetOriginCenter()
{
	originX = width * 0.5f;
	originY = height * 0.5f;
}

float DX9GF::EllipseCollider::GetOriginX() const
{
	return originX;
}

float DX9GF::EllipseCollider::GetOriginY() const
{
	return originY;
}

void DX9GF::EllipseCollider::Draw(GraphicsDevice* graphicsDevice, const Camera& camera)
{
	if (!drawCollider) return;
	graphicsDevice->DrawEllipse(
		camera, 
		GetWorldX(), 
		GetWorldY(), 
		width, 
		height, 
		GetWorldRotation(), 
		GetWorldScaleY(), 
		GetWorldScaleX(), 
		GetOriginX(), 
		GetOriginY(), 
		0x550000FF,
		false
	);
}
