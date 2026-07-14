#include "pch.h"
#include "DX9GFColliderManager.h"
#include <algorithm>
#include <mutex>

namespace {
    constexpr float broadPhaseEpsilon = 0.01f;
}

namespace DX9GF {

    void ColliderManager::Add(std::shared_ptr<ICollider> collider) {
        if (collider) {
            std::scoped_lock<std::mutex> lock{vectorMutex};
            colliders.push_back(collider);
        }
    }

    void ColliderManager::Remove(std::shared_ptr<ICollider> collider) {
        auto it = std::find(colliders.begin(), colliders.end(), collider);
        if (it != colliders.end()) {
            colliders.erase(it);
        }
    }

    void ColliderManager::Clear() {
        colliders.clear();
    }

    std::optional<std::tuple<float, float>> ColliderManager::GetSafePosition(
        std::shared_ptr<ICollider> target,
        float newX,
        float newY)
    {
        if (!target) return std::nullopt;

        float finalX = newX;
        float finalY = newY;
        bool hasCollision = false;

        // Broad phase: target's AABB over the whole current -> new span
        ICollider::AABB sweptAABB = target->GetWorldAABB();
        float offsetX = newX - target->GetWorldX();
        float offsetY = newY - target->GetWorldY();
        sweptAABB.minX += (std::min)(offsetX, 0.0f) - broadPhaseEpsilon;
        sweptAABB.minY += (std::min)(offsetY, 0.0f) - broadPhaseEpsilon;
        sweptAABB.maxX += (std::max)(offsetX, 0.0f) + broadPhaseEpsilon;
        sweptAABB.maxY += (std::max)(offsetY, 0.0f) + broadPhaseEpsilon;

        for (const auto& other : colliders) {
            if (other == target) {
                continue;
            }
            if (!sweptAABB.Overlaps(other->GetWorldAABB())) {
                continue;
            }

            auto result = target->IsIntersecting(other, finalX, finalY);
            if (result.has_value()) {
                auto& [correctedX, correctedY] = result.value();
                finalX = correctedX;
                finalY = correctedY;
                hasCollision = true;
            }
        }

        if (hasCollision) {
            return std::make_tuple(finalX, finalY);
        }

        return std::nullopt;
    }

    const std::vector<std::shared_ptr<ICollider>>& ColliderManager::GetAllColliders() const {
        return colliders;
    }

    std::tuple<float, float> DX9GF::ColliderManager::GetSlidingDeltas(std::shared_ptr<ICollider> target, float deltaX, float deltaY)
    {
        if (!target) return { deltaX, deltaY };

        float currentX = target->GetWorldX();
        float currentY = target->GetWorldY();

        float finalDx = deltaX;
        float finalDy = deltaY;

        // Broad phase: target's AABB expanded by the movement deltas
        ICollider::AABB sweptAABB = target->GetWorldAABB();
        sweptAABB.minX += (std::min)(deltaX, 0.0f) - broadPhaseEpsilon;
        sweptAABB.minY += (std::min)(deltaY, 0.0f) - broadPhaseEpsilon;
        sweptAABB.maxX += (std::max)(deltaX, 0.0f) + broadPhaseEpsilon;
        sweptAABB.maxY += (std::max)(deltaY, 0.0f) + broadPhaseEpsilon;

        for (const auto& other : colliders) {
            if (other == target) {
                continue;
            }
            if (!sweptAABB.Overlaps(other->GetWorldAABB())) {
                continue;
            }

            if (auto pos = target->IsIntersecting(other, currentX + finalDx, currentY); pos.has_value()) {
                auto& [correctedX, correctedY] = pos.value();
                finalDx = correctedX - currentX;
            }

            if (auto pos = target->IsIntersecting(other, currentX, currentY + finalDy); pos.has_value()) {
                auto& [correctedX, correctedY] = pos.value();
                finalDy = correctedY - currentY;
            }
        }

        return std::make_tuple(finalDx, finalDy);
    }
}
