#include "pch.h"
#include "HandContainer.h"
#include "IBlockCard.h"
#include "MainBlockCard.h"
#include <algorithm>

void Demo::HandContainer::Init(std::shared_ptr<DraggableManager> manager, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera)
{
	IContainer::Init(manager, graphicsDevice, camera);
}

void Demo::HandContainer::Init(std::shared_ptr<DraggableManager> manager, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, std::vector<std::shared_ptr<ICard>>* playedPile)
{
	this->Init(manager, graphicsDevice, camera);
	this->playedPile = playedPile;
	if (trigger) {
		trigger->SetOnHeldLeft([](DX9GF::ITrigger* thisObj) {
			});
		trigger->SetOnReleaseLeft([](DX9GF::ITrigger* thisObj) {
			});
	}
}

bool Demo::HandContainer::OnDrop(std::shared_ptr<IDraggable> other)
{
	if (!std::dynamic_pointer_cast<ICard>(other) || std::dynamic_pointer_cast<MainBlockCard>(other)) {
		return false;
	}

	if (std::find_if(playedPile->begin(), playedPile->end(), [&](const std::weak_ptr<ICard>& playedCard) {
		auto lock = playedCard.lock();
		return lock && lock.get() == dynamic_pointer_cast<ICard>(other).get();
	}) != playedPile->end()) {
		return false;
	}
	if (!IContainer::OnDrop(other)) {
		return false;
	}
	// Only once the drop is committed - a card the hand rejected is still in play and keeps its
	// targets.
	if (auto statement = std::dynamic_pointer_cast<IStatementCard>(other)) {
		statement->ReleaseEnemyCards();
	}
	return true;
}

void Demo::HandContainer::StoreCard(std::shared_ptr<ICard> card)
{
	if (!card) {
		return;
	}
	auto statement = std::dynamic_pointer_cast<IStatementCard>(card);
	if (!statement) {
		return;
	}
	// Before the reparent, so the card's width no longer counts its enemy cards when the
	// container measures its children.
	statement->ReleaseEnemyCards();
	if (auto parent = card->GetParent(); parent.has_value()) {
		dynamic_pointer_cast<IDraggable>(card)->DetachParent();
	}

	children.erase(std::remove_if(children.begin(), children.end(), [&](const std::weak_ptr<IDraggable>& child) {
		auto lock = child.lock();
		return !lock || lock.get() == dynamic_pointer_cast<IDraggable>(card).get();
	}), children.end());

	float yPos = static_cast<float>(dragAreaHeight);
	for (const auto& child : children) {
		if (auto lock = child.lock()) {
			yPos += static_cast<float>(lock->GetHeight());
		}
	}

	dynamic_pointer_cast<IDraggable>(card)->SetParent(shared_from_this());
	card->SetLocalPosition(0.f, yPos);
	children.push_back(dynamic_pointer_cast<IDraggable>(card));
}
