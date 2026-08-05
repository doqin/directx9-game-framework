#include "pch.h"
#include "IBlockCard.h"
#include "IBattleScene.h"
#include "DrawUtils.h"

bool Demo::IBlockCard::OnDrop(std::shared_ptr<IDraggable> other)
{
	auto statementCard = std::dynamic_pointer_cast<IStatementCard>(other);
	if (!statementCard) {
		return false;
	}

	if (battleScene && !battleScene->CanPlaceCardInBlock(statementCard)) {
		if (timeSinceLastEnergyPopUp >= energyPopUpCooldown) {
			battleScene->QueuePopUpMessage(L"Not enough energy");
			timeSinceLastEnergyPopUp = 0.f;
		}
		// Dragging detached it from the hand, and rejecting the drop leaves it parented to
		// nothing - floating in space, not executing, yet still swept into the played pile at
		// end of turn. Put it back where it came from.
		battleScene->ReturnCardToHand(statementCard);
		return false;
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

void Demo::IBlockCard::Draw(unsigned long long deltaTime)
{
	IContainer::Draw(deltaTime);
	if (!blockTexture) {
		blockTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		blockTexture->LoadTexture(L"assets/ui.png");
		blockSprite = std::make_shared<DX9GF::StaticSprite>(blockTexture.get());
		blockSprite->SetSrcRect(GetBlockFaceRect());
		pointerSprite = std::make_shared<DX9GF::StaticSprite>(blockTexture.get());
		pointerSprite->SetSrcRect({ .left = 128, .top = 0, .right = 144, .bottom = 16 });
		pointerSprite->SetScale(2.f, 2.f);
		pointerSprite->SetOrigin(8.f, 8.f);
	}
	blockSprite->Begin();
	blockSprite->SetPosition(GetWorldX(), GetWorldY());
	blockSprite->SetScale(2.f, 2.f);
	blockSprite->Draw(*camera, deltaTime);
	blockSprite->End();
	if (!nameFont) {
		nameFont = std::make_shared<DX9GF::Font>(graphicsDevice, L"StatusPlz", 16);
		nameFontSprite = std::make_shared<DX9GF::FontSprite>(nameFont.get());
		nameFontSprite->SetColor(0xFF000000);
	}
	if (auto currentCard = GetCurrentExecutingCard()) {
		pointerSprite->Begin();
		pointerSprite->SetPosition(currentCard->GetWorldX() - 32.f, currentCard->GetWorldY() + 16.f);
		pointerSprite->Draw(*camera, deltaTime);
		pointerSprite->End();
	}
	for (auto& draggable : draggableManager->GetDraggingDraggables()) {
		if (auto lock = draggable->GetParent(); lock.has_value()) {
			if (auto parentLock = lock.value().lock()) {
				if (parentLock.get() == this) {
					continue;
				}
			}
		}
		if (auto draggedStatementCard = std::dynamic_pointer_cast<Demo::IStatementCard>(draggable)) {
			auto thisX = GetWorldX();
			auto thisY = GetWorldY() + GetHeight() + GetHeightOfChildren();
			auto width = (std::max)(this->GetMaxWidthOfChildren(), GetWidth());
			auto height = 32.f;
			draggableManager->QueueDraw(std::make_shared<DX9GF::CustomCommand>([&, width, height, thisX, thisY](std::function<void(void)> markFinished) {
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
				graphicsDevice->SetAlphaBlending(false);
				markFinished();
				}));
			return;
		}
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
		// A card queued in the block runs again on every following turn until the program
		// clears, so a use is spent per completed execution, not per time it was played.
		current->ConsumeUse();
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

	if (!alreadyAttached && battleScene && !battleScene->CanPlaceCardInBlock(card)) {
		if (timeSinceLastEnergyPopUp >= energyPopUpCooldown) {
			battleScene->QueuePopUpMessage(L"Not enough energy");
			timeSinceLastEnergyPopUp = 0.f;
		}
		battleScene->ReturnCardToHand(card);
		return false;
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