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
			battleScene->QueuePopUpMessage(L"Not enough energy");
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
