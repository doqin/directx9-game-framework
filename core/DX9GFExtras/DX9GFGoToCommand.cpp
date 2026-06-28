#include "pch.h"
#include "DX9GFGoToCommand.h"
#include <algorithm>
#include <cmath>

namespace DX9GF {
   static constexpr float POSITION_EPSILON = 0.0001f;

	GoToCommand::GoToCommand(std::weak_ptr<IGameObject> gameObject, float x, float y, float velocity)
       : targetObject(gameObject), targetX(x), targetY(y), velocity(velocity), useEaseInOut(false), duration(0.0f), elapsedTime(0.0f), startX(0.0f), startY(0.0f), initialized(false) {}

	GoToCommand::GoToCommand(std::weak_ptr<IGameObject> gameObject, float x, float y, float velocity, VelocityTag) // for legacy reasons
		: GoToCommand(gameObject, x, y, velocity) {}

	GoToCommand::GoToCommand(std::weak_ptr<IGameObject> gameObject, float x, float y, float duration, TimeTag)
     : targetObject(gameObject), targetX(x), targetY(y), velocity(0), useEaseInOut(false), duration(0.0f), elapsedTime(0.0f), startX(0.0f), startY(0.0f), initialized(false)
	{
		if (auto lock = gameObject.lock()) {
			auto [currentX, currentY] = lock->GetWorldPosition();
			auto dX = x - currentX;
			auto dY = y - currentY;
			auto distance = std::sqrt(dX * dX + dY * dY);
			velocity = distance / duration;
		}
	}

	GoToCommand::GoToCommand(std::weak_ptr<IGameObject> gameObject, float x, float y, float duration, TimeTag, EaseInOutTag)
		: targetObject(gameObject), targetX(x), targetY(y), velocity(0.0f), useEaseInOut(true), duration(duration), elapsedTime(0.0f), startX(0.0f), startY(0.0f), initialized(false) {}

	void GoToCommand::Execute(unsigned long long deltaTime) {
		auto obj = targetObject.lock();
		if (!obj) {
			MarkFinished();
			return;
		}

		if (useEaseInOut) {
			if (!initialized) {
				startX = obj->GetWorldX();
				startY = obj->GetWorldY();
				initialized = true;
			}

			if (duration <= POSITION_EPSILON) {
				obj->SetLocalPosition(targetX, targetY);
				MarkFinished();
				return;
			}

			elapsedTime += deltaTime / 1000.0f;
			float t = std::clamp(elapsedTime / duration, 0.0f, 1.0f);
			float easedT = t * t * (3.0f - 2.0f * t);

			float newX = startX + (targetX - startX) * easedT;
			float newY = startY + (targetY - startY) * easedT;
			obj->SetLocalPosition(newX, newY);

			if (t >= 1.0f - POSITION_EPSILON) {
				obj->SetLocalPosition(targetX, targetY);
				MarkFinished();
			}
			return;
		}

		float currentX = obj->GetWorldX();
		float currentY = obj->GetWorldY();

		float dx = targetX - currentX;
		float dy = targetY - currentY;
		float distance = std::sqrt(dx * dx + dy * dy);
		float moveStep = velocity * (deltaTime / 1000.0f);

		if (distance <= moveStep + POSITION_EPSILON) {
			obj->SetLocalPosition(targetX, targetY);
			MarkFinished();
		}
		else {
			float nx = dx / distance;
			float ny = dy / distance;
			obj->SetLocalPosition(currentX + nx * moveStep, currentY + ny * moveStep);
		}
	}
}
