#include "pch.h"
#include "MultiTargetCard.h"

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

		if (otherX > thisX && otherX < thisX + dragAreaWidth &&
			otherY > thisY && otherY < thisY + dragAreaHeight) {

			float localX = dragAreaWidth;
			for (auto& wp : targets) {
				if (auto lock = wp.lock()) localX += lock->GetWidth();
			}

			other->SetParent(shared_from_this());
			other->SetLocalPosition(localX, 0);
			targets.push_back(incomingEnemyCard);
			return true;
		}
		return false;
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

	size_t MultiTargetCard::GetWidth() const {
		size_t totalWidth = IDraggable::GetWidth();
		for (auto& wp : targets) {
			if (auto lock = wp.lock()) totalWidth += lock->GetWidth();
		}
		return totalWidth;
	}

	void MultiTargetCard::Draw(unsigned long long deltaTime) {
		IStatementCard::Draw(deltaTime);
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