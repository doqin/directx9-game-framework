#include "pch.h"
#include "KeyboardNavigator.h"
#include "SettingsManager.h"
#include "DrawUtils.h"
#include "IButton.h"
#include <cmath>
#include "ICard.h"

void Demo::KeyboardNavigator::UpdateMode()
{
	auto inpMan = DX9GF::InputManager::GetInstance();
	auto sm = SettingsManager::GetInstance();

	if (inpMan->KeyDown(sm->GetKeybind("MOVE_UP")) || inpMan->KeyDown(sm->GetKeybind("MOVE_DOWN"))
		|| inpMan->KeyDown(sm->GetKeybind("MOVE_LEFT")) || inpMan->KeyDown(sm->GetKeybind("MOVE_RIGHT"))) {
		keyboardMode = true;
	}
	if (inpMan->GetRelativeMouseX() != 0 || inpMan->GetRelativeMouseY() != 0) {
		keyboardMode = false;
		target.reset();
	}
}

void Demo::KeyboardNavigator::Navigate(unsigned long long deltaTime, const std::vector<Candidate>& candidates)
{
	if (!keyboardMode) {
		return;
	}

	auto inpMan = DX9GF::InputManager::GetInstance();
	auto sm = SettingsManager::GetInstance();
	const auto previousTarget = target;

	if (hasPendingRetarget) {
		hasPendingRetarget = false;
		const Candidate* best = nullptr;
		float bestDist = 0.f;
		for (auto& c : candidates) {
			const float dx = c.x - retargetX;
			const float dy = c.y - retargetY;
			const float dist = dx * dx + dy * dy;
			if (!best || dist < bestDist) {
				best = &c;
				bestDist = dist;
			}
		}
		if (best) {
			target = best->anchor;
		}
	}

	const Candidate* current = nullptr;
	int currentIndex = -1;
	for (size_t i = 0; i < candidates.size(); ++i) {
		if (candidates[i].anchor == target) {
			current = &candidates[i];
			currentIndex = static_cast<int>(i);
			break;
		}
	}

	if (!current) {
		if (candidates.empty()) {
			target.reset();
			return;
		}
		// Default entry point: whichever candidate is closest to the screen center
		// (0,0 in this engine's camera-centered coordinate system).
		const Candidate* best = nullptr;
		float bestDist = 0.f;
		for (auto& c : candidates) {
			const float cx = c.x + c.width / 2.f;
			const float cy = c.y + c.height / 2.f;
			const float dist = cx * cx + cy * cy;
			if (!best || dist < bestDist) {
				best = &c;
				bestDist = dist;
			}
		}
		target = best->anchor;
		ApplyButtonVisuals(previousTarget);
		return;
	}

	int dirX = 0, dirY = 0;
	if (inpMan->KeyDown(sm->GetKeybind("MOVE_LEFT"))) dirX = -1;
	else if (inpMan->KeyDown(sm->GetKeybind("MOVE_RIGHT"))) dirX = 1;
	if (inpMan->KeyDown(sm->GetKeybind("MOVE_UP"))) dirY = -1;
	else if (inpMan->KeyDown(sm->GetKeybind("MOVE_DOWN"))) dirY = 1;

	if (dirX != 0 || dirY != 0) {
		std::vector<std::pair<float, float>> positions;
		positions.reserve(candidates.size());
		for (auto& c : candidates) {
			positions.push_back({ c.x, c.y });
		}
		const int best = PickDirectional(current->x, current->y, dirX, dirY, positions, currentIndex);
		if (best >= 0) {
			target = candidates[best].anchor;
		}
	}

	if (inpMan->KeyDown(sm->GetKeybind("ACCEPT")) && current->activate) {
		current->activate();
	}

	ApplyButtonVisuals(previousTarget);
}

void Demo::KeyboardNavigator::ApplyButtonVisuals(const std::shared_ptr<DX9GF::IGameObject>& previousTarget)
{
	auto button = std::dynamic_pointer_cast<IButton>(target);
	if (!button) {
		return;
	}
	const auto state = button->GetState();
	if (state == IButton::ButtonState::DISABLED || state == IButton::ButtonState::LISTENING) {
		return;
	}
	if (target != previousTarget) {
		button->PlayHoverSfx();
	}
	// Buttons recompute their state from the mouse every frame in their own Update
	// (which runs before the navigator), so this override only lasts while targeted.
	const bool acceptHeld = DX9GF::InputManager::GetInstance()->KeyPress(SettingsManager::GetInstance()->GetKeybind("ACCEPT"));
	button->SetState(acceptHeld ? IButton::ButtonState::CLICKED : IButton::ButtonState::HOVER);
}

void Demo::KeyboardNavigator::Update(unsigned long long deltaTime, const std::vector<Candidate>& candidates)
{
	UpdateMode();
	Navigate(deltaTime, candidates);
}

void Demo::KeyboardNavigator::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera& camera, const std::vector<Candidate>& candidates)
{
	if (!keyboardMode || !target) {
		return;
	}
	for (auto& c : candidates) {
		if (c.anchor == target) {
			Demo::DrawKeyboardTargetReticle(graphicsDevice, camera, c.x, c.y, c.width, c.height, GetTickCount64());
			if (dynamic_cast<Demo::ICard*>(c.anchor.get()) != nullptr) {
				graphicsDevice->SetAlphaBlending(true);
				float alphaPhase = (std::sin(GetTickCount64() / 150.0f) + 1.0f) / 2.0f;
				int alpha = static_cast<int>(alphaPhase * 80.0f);
				D3DCOLOR color = D3DCOLOR_ARGB(alpha, 255, 255, 255);
				graphicsDevice->DrawRectangle(camera, c.x, c.y, c.width, c.height, color, true);
				graphicsDevice->SetAlphaBlending(false);
			}
			return;
		}
	}
}

void Demo::KeyboardNavigator::Reset()
{
	keyboardMode = false;
	target.reset();
	hasPendingRetarget = false;
}

void Demo::KeyboardNavigator::RetargetNearest(float x, float y)
{
	hasPendingRetarget = true;
	retargetX = x;
	retargetY = y;
}

int Demo::KeyboardNavigator::PickDirectional(float currentX, float currentY, int dirX, int dirY, const std::vector<std::pair<float, float>>& positions, int excludeIndex)
{
	const float dirLen = std::sqrt(static_cast<float>(dirX * dirX + dirY * dirY));
	if (dirLen <= 0.f) {
		return -1;
	}
	const float dirUnitX = dirX / dirLen;
	const float dirUnitY = dirY / dirLen;

	int best = -1;
	float bestScore = 0.f;
	for (size_t i = 0; i < positions.size(); ++i) {
		if (static_cast<int>(i) == excludeIndex) {
			continue;
		}
		const float vx = positions[i].first - currentX;
		const float vy = positions[i].second - currentY;
		const float dist = std::sqrt(vx * vx + vy * vy);
		if (dist <= 0.0001f) {
			continue;
		}
		const float dot = (vx / dist) * dirUnitX + (vy / dist) * dirUnitY;
		if (dot < DIRECTION_DOT_THRESHOLD) {
			continue;
		}
		const float primaryAxisDist = vx * dirUnitX + vy * dirUnitY;
		const float perpDist = std::sqrt((std::max)(0.f, dist * dist - primaryAxisDist * primaryAxisDist));
		const float score = primaryAxisDist + perpDist * PERPENDICULAR_PENALTY;
		if (best < 0 || score < bestScore) {
			best = static_cast<int>(i);
			bestScore = score;
		}
	}
	return best;
}
