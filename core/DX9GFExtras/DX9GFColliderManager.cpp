#include "pch.h"
#include "DX9GFColliderManager.h"
#include "taskflow/taskflow.hpp"
#include "taskflow/algorithm/for_each.hpp"
#include <mutex>
#include <atomic>

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

        tf::Executor executor;
        tf::Taskflow taskflow;
        taskflow.for_each(colliders.begin(), colliders.end(), [&](std::shared_ptr<ICollider> other) {
            if (other == target) {
                return;
            }

            float x;
            float y;
            {
                x = finalX;
                y = finalY;
            }
            auto result = target->IsIntersecting(other, x, y);
            if (result.has_value()) {
                auto& [correctedX, correctedY] = result.value();
                finalX = correctedX;
                finalY = correctedY;
                hasCollision = true;
            }
        });
        executor.run(taskflow).wait();

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

        tf::Executor executor;
        tf::Taskflow taskflow;
        taskflow.for_each(colliders.begin(), colliders.end(), [&](std::shared_ptr<ICollider> other) {
            if (other == target) {
                return;
            }

            float dx;
            float dy;
            {
                dx = finalDx;
                dy = finalDy;
            }

            if (auto pos = target->IsIntersecting(other, currentX + dx, currentY); pos.has_value()) {
                auto& [correctedX, correctedY] = pos.value();
                finalDx = correctedX - currentX;
            }

            if (auto pos = target->IsIntersecting(other, currentX, currentY + dy); pos.has_value()) {
                auto& [correctedX, correctedY] = pos.value();
                finalDy = correctedY - currentY;
            }
        });
        executor.run(taskflow).wait();

        return std::make_tuple(finalDx, finalDy);
    }
}
