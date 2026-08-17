#include "pch.h"
#include "MultiTargetCard.h"
#include "DrawUtils.h"
#include "IBattleScene.h"
#include "VirtualBattleState.h"

namespace Demo {
	MultiTargetCard::MultiTargetCard(std::weak_ptr<DX9GF::TransformManager> tm, size_t maxTargets, std::wstring name, float x, float y, size_t dragAreaWidth, size_t dragAreaHeight)
		: IGameObject(tm, x, y), IStatementCard(tm, dragAreaWidth, dragAreaHeight, x, y), maxTargets(maxTargets), cardName(name) {
	}

	bool MultiTargetCard::OnDrop(std::shared_ptr<IDraggable> other) {
		if (targets.size() >= maxTargets) return false;

		auto incomingEnemyCard = std::dynamic_pointer_cast<EnemyCard>(other);
		if (!incomingEnemyCard) return false;

		auto [thisX, thisY] = this->GetWorldPosition();
		auto [otherX, otherY] = other->GetWorldPosition();
		otherY += other->GetTrigger().lock()->GetHeight() / 2.0f;

		if (otherX > thisX && otherX < thisX + GetWidth() &&
			otherY > thisY && otherY < thisY + GetHeight()) {
			return AttachEnemyCard(incomingEnemyCard);
		}
		return false;
	}

	bool MultiTargetCard::AttachEnemyCard(std::shared_ptr<EnemyCard> card) {
		if (!card || targets.size() >= maxTargets) return false;

		float localX = (float)GetWidth();
		for (auto& wp : targets) {
			if (auto lock = wp.lock()) localX += lock->GetWidth();
		}

		card->SetParent(shared_from_this());
		card->SetLocalPosition(localX, 0);
		targets.push_back(card);
		return true;
	}

	void MultiTargetCard::CollectHitsOnTargets(VirtualBattleState& state, float damage, size_t maxHits) {
		size_t hits = 0;
		for (auto& wp : targets) {
			if (maxHits > 0 && hits >= maxHits) break;
			if (auto lock = wp.lock()) {
				if (auto enemy = lock->GetValue()) {
					state.SimulateDamage(enemy.get(), damage);
					++hits;
				}
			}
		}
	}

	void MultiTargetCard::CollectEffectOnTargets(VirtualBattleState& state, ModifierType modifier, float value, int duration, size_t maxTargetsHit) {
		size_t applied = 0;
		for (auto& wp : targets) {
			if (maxTargetsHit > 0 && applied >= maxTargetsHit) break;
			if (auto lock = wp.lock()) {
				if (auto enemy = lock->GetValue()) {
					state.SimulateEnemyModifier(enemy.get(), modifier, value, duration);
					++applied;
				}
			}
		}
	}

	void MultiTargetCard::ReleaseEnemyCards() {
		for (auto& wp : targets) {
			if (auto lock = wp.lock()) {
				if (battleScene) {
					battleScene->DiscardEnemyCard(lock);
				}
			}
		}
		targets.clear();
	}

	std::tuple<float, float> MultiTargetCard::GetEnemyCardSlotWorldPosition() const {
		float localX = (float)dragAreaWidth;
		for (auto& wp : targets) {
			if (auto lock = wp.lock()) localX += lock->GetWidth();
		}
		return { GetWorldX() + localX, GetWorldY() };
	}

	void MultiTargetCard::Update(unsigned long long deltaTime) {
		IDraggable::Update(deltaTime);
		bool needsReposition = false;

		for (auto it = targets.begin(); it != targets.end(); ) {
			bool remove = true;
			if (auto lock = it->lock()) {
				if (auto parent = lock->GetParent(); parent.has_value()) {
					if (parent.value().lock().get() == this) {
						remove = false;
					}
				}
			}
			if (remove) {
				it = targets.erase(it);
				needsReposition = true;
			}
			else {
				++it;
			}
		}

		if (needsReposition) {
			float localX = dragAreaWidth;
			for (auto& wp : targets) {
				if (auto lock = wp.lock()) {
					lock->SetLocalPosition(localX, 0);
					localX += lock->GetWidth();
				}
			}
		}
	}

	void MultiTargetCard::Draw(unsigned long long deltaTime) {
		IStatementCard::Draw(deltaTime);
		for (auto& draggable : draggableManager->GetDraggingDraggables()) {
			if (auto draggedEnemyCard = std::dynamic_pointer_cast<EnemyCard>(draggable); draggableManager->GetDraggingDraggables().size() == 1 && draggedEnemyCard) {
				auto [draggedX, draggedY] = draggedEnemyCard->GetWorldPosition();
				auto draggedWidth = draggedEnemyCard->GetWidth();
				auto draggedHeight = draggedEnemyCard->GetHeight();
				auto width = GetWidth();
				auto height = GetHeight();
				auto thisX = GetWorldX();
				auto thisY = GetWorldY();
				if (targets.size() >= maxTargets) {
					goto skipHighlight;
				}
				for (auto& wp : targets) {
					if (auto lock = wp.lock()) {
						if (lock.get() == draggedEnemyCard.get()) {
							goto skipHighlight;
						}
					}
				}
				draggableManager->QueueDraw(std::make_shared<DX9GF::CustomCommand>([&, width, height, thisX, thisY, draggedX, draggedY, draggedWidth, draggedHeight](std::function<void(void)> markFinished) {
					graphicsDevice->SetAlphaBlending(true);
					Demo::DrawAnimatedDashedRectangle(
						graphicsDevice,
						*camera,
						thisX,
						thisY,
						width,
						height,
						3.f,
						0xFFFFFFFF,
						false,
						4.f,
						0xFFFFFFFF,
						20.f,
						10.f,
						40.f,
						GetTickCount64()
					);
					//Demo::DrawAnimatedDashedArrow(
					//	graphicsDevice,
					//	*camera,
					//	draggedX + draggedWidth / 2.0f,
					//	draggedY + draggedHeight / 2.0f,
					//	thisX + GetWidth() / 2.0f,
					//	thisY + GetHeight() / 2.0f,
					//	3.f,
					//	0x80FFFFFF,
					//	false,
					//	10.f,
					//	0xFFFFFFFF,
					//	20.f,
					//	10.f,
					//	40.f,
					//	GetTickCount64(),
					//	10.f,
					//	10.f
					//);
					graphicsDevice->SetAlphaBlending(false);
					markFinished();
				}));
				return;
			skipHighlight:
				continue;
			}
		}
		//if (!nameFont) {
		//	nameFont = std::make_shared<DX9GF::Font>(graphicsDevice, L"StatusPlz", 16);
		//	nameFontSprite = std::make_shared<DX9GF::FontSprite>(nameFont.get());
		//	nameFontSprite->SetColor(0xFF000000);
		//}
		//nameFontSprite->Begin();
		//nameFontSprite->SetPosition(GetWorldX() + 8.f, GetWorldY() + 8.f);

		//std::wstring displayText = cardName + L" (" + std::to_wstring(targets.size()) + L"/" + std::to_wstring(maxTargets) + L")";
		//nameFontSprite->SetText(std::move(displayText));
		//nameFontSprite->Draw(*camera, deltaTime);
		//nameFontSprite->End();
	}

	void MultiTargetCard::ResetExecution() {
		isDone = false;
	}

	std::wstring MultiTargetCard::GetInputsDescription() const {
		return std::to_wstring(targets.size()) + L"/" + std::to_wstring(maxTargets) + L" Enemies";
	}
}