#include "pch.h"
#include "IBlockCard.h"
#include "IBattleScene.h"

bool Demo::IBlockCard::OnDrop(std::shared_ptr<IDraggable> other)
{
	auto statementCard = std::dynamic_pointer_cast<IStatementCard>(other);
	if (!statementCard) {
		return false;
	}

	if (battleScene) {
		int cost = static_cast<int>(statementCard->GetCost());
		if (battleScene->GetAvailableEnergy() < 0) {
			if (timeSinceLastEnergyPopUp >= energyPopUpCooldown) {
				battleScene->QueuePopUpMessage(L"Not enough energy");
				timeSinceLastEnergyPopUp = 0.f;
			}
			return false;
		}
	}

	if (IContainer::OnDrop(other)) {
		statementCards.push_back(statementCard);
		return true;
	}
	return false;
}

void Demo::IBlockCard::Update(unsigned long long deltaTime)
{
	IContainer::Update(deltaTime);

	timeSinceLastEnergyPopUp += deltaTime / 1000.f;

	// Removes invalid cards
	for (size_t i = 0; i < statementCards.size(); ++i) {
		auto lock = statementCards[i].lock();
		if (!lock) {
			statementCards.erase(statementCards.begin() + i);
			--i;
			continue;
		}
		if (auto parent = lock->GetParent(); !parent.has_value() || parent.value().lock().get() != this) {
			statementCards.erase(statementCards.begin() + i);
			if (executeIndex > i && executeIndex != 0) {
				--executeIndex;
			}
			--i;
		}
	}
	if (isExecuting) {
		ExecuteIteratively(deltaTime);
	}
}

void Demo::IBlockCard::StartExecution()
{
	executeIndex = 0;
	currentExecutingCard.reset();
	for (auto& card : statementCards) {
		if (auto lock = card.lock()) {
			lock->ResetExecution();
		}
	}
	isExecuting = !statementCards.empty();
}

void Demo::IBlockCard::ExecuteIteratively(unsigned long long deltaTime)
{
	if (executeIndex >= statementCards.size()) {
		isExecuting = false;
		currentExecutingCard.reset();
		return;
	}
	timeSinceLastExecution += deltaTime / 1000.f;
	if (timeSinceLastExecution <= timePerExecution) {
		return;
	}
	else {
		timeSinceLastExecution = 0;
	}
	auto current = statementCards[executeIndex].lock();
	if (!current) {
		++executeIndex;
		currentExecutingCard.reset();
		return;
	}
	currentExecutingCard = current;
	if (current->Execute()) {
		++executeIndex;
	}
}

bool Demo::IBlockCard::IsExecuting() const
{
	return isExecuting;
}

std::shared_ptr<Demo::IStatementCard> Demo::IBlockCard::GetCurrentExecutingCard() const
{
    return currentExecutingCard.lock();
}

void Demo::IBlockCard::ResetExecution()
{
	executeIndex = 0;
	isExecuting = false;
	currentExecutingCard.reset();
}

bool Demo::IBlockCard::InsertStatementCardAt(std::shared_ptr<IStatementCard> card, size_t index)
{
	if (!card) {
		return false;
	}

	bool alreadyAttached = false;
	for (auto& weak : statementCards) {
		if (weak.lock() == card) {
			alreadyAttached = true;
			break;
		}
	}

	if (!alreadyAttached && battleScene) {
		if (battleScene->GetAvailableEnergy() < 0) {
			if (timeSinceLastEnergyPopUp >= energyPopUpCooldown) {
				battleScene->QueuePopUpMessage(L"Not enough energy");
				timeSinceLastEnergyPopUp = 0.f;
			}
			return false;
		}
	}

	// Remove any existing bookkeeping for this card first (reordering case).
	for (size_t i = 0; i < statementCards.size(); ++i) {
		if (statementCards[i].lock() == card) {
			statementCards.erase(statementCards.begin() + i);
			break;
		}
	}
	for (size_t i = 0; i < children.size(); ++i) {
		if (children[i].lock() == card) {
			children.erase(children.begin() + i);
			break;
		}
	}

	index = (std::min)(index, statementCards.size());
	card->SetParent(shared_from_this());
	statementCards.insert(statementCards.begin() + index, card);
	children.insert(children.begin() + index, card);
	// IContainer::Update repositions all children by vector order every frame, so no explicit reposition needed here.
	return true;
}

std::tuple<float, float> Demo::IBlockCard::GetStatementSlotWorldPosition(size_t index, std::shared_ptr<IStatementCard> excluding)
{
	auto [thisX, thisY] = GetWorldPosition();
	float y = thisY + (float)dragAreaHeight;
	size_t counted = 0;
	for (auto& weak : statementCards) {
		auto lock = weak.lock();
		if (!lock || lock == excluding) {
			continue;
		}
		if (counted == index) {
			break;
		}
		y += (float)lock->GetHeight();
		++counted;
	}
	return { thisX, y };
}

bool Demo::IBlockCard::HasAllRequiredTargets() const
{
	for (auto& card : statementCards) {
		if (auto lock = card.lock()) {
			if (!lock->HasRequiredTargets()) {
				return false;
			}
		}
	}
	return true;
}