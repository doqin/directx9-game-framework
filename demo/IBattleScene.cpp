#include "pch.h"
#include "IBattleScene.h"
#include <algorithm>
#include <random>
#include "DamageTextManager.h"
#include "GameItems.h"
#include "TransitionCommand.h"
#include "resource.h"
#include "TutorialWorldScene.h"
#include "SecretPuzzleScene.h"
#include "ThreadAlleyScene.h"
#include "BossWorldScene.h"
#include "SettingsManager.h"
#include "DrawUtils.h"

#include "RNG.h"
namespace {
	constexpr float HiddenPileX = -10000.f;
	constexpr float HiddenPileY = -10000.f;

	void HidePileCard(const std::shared_ptr<Demo::ICard>& card)
	{
		if (!card) {
			return;
		}
		auto draggable = dynamic_pointer_cast<Demo::IDraggable>(card);
		if (draggable) {
			if (auto parent = card->GetParent(); parent.has_value()) {
				draggable->DetachParent();
			}
			draggable->SetHidden(true);
		}

		card->SetLocalPosition(HiddenPileX, HiddenPileY);
	}

	constexpr float RAW_ITEM_W = 23.0f;
	constexpr float RAW_ITEM_H = 35.0f;
	constexpr float RAW_BG_W = 192.0f;
	constexpr float RAW_BG_H = 128.0f;

	constexpr float MENU_SCALE = 3.0f;

	constexpr float ITEM_W = RAW_ITEM_W * MENU_SCALE;
	constexpr float ITEM_H = RAW_ITEM_H * MENU_SCALE;
	constexpr float BG_W = RAW_BG_W * MENU_SCALE;
	constexpr float BG_H = RAW_BG_H * MENU_SCALE;

	constexpr float HALF_BG_W = BG_W / 2.0f;
	constexpr float HALF_BG_H = BG_H / 2.0f;

	constexpr float PADDING_X = 20.0f;
	constexpr float PADDING_Y = 30.0f;

}

void Demo::IBattleScene::StartBattle()
{
	for (auto& card : drawPile) {
		HidePileCard(card);
		card->ResetUses();
	}
	nullifiedPile.clear();
	energy = MAX_ENERGY;
	usedEnergy = 0;
	committedEnergy = 0;
	pendingBonusEnergy = 0;
	pendingBonusDraw = 0;
	initialEnemyCount = enemies.size();
	battleGoldReward = 0;
	isBattleEnding = false;
	isDefeatSequence = false;
	defeatElapsedMs = 0.f;
	defeatFadeAlpha = 0.f;
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::shuffle(drawPile.begin(), drawPile.end(), gen);
	currentTurn = 1;
	DrawCards(CARDS_DRAWN_PER_TURN);
	for (auto& enemy : enemies) {
		enemy->SetOnRequestLockCard([this](int turns) { this->LockRandomCard(turns); });
	}
	for (auto& enemy : enemies) {
		enemy->OnTurnBegin(this->battlePlayer, this->popUpMessage, this->currentTurn);
	}
	if (!customBGMName.empty()) {
		DX9GF::AudioManager::GetInstance()->PlayBGM_Fade(customBGMName, 0.3f, 1.5f);
	}
	else {
		DX9GF::AudioManager::GetInstance()->PlayRandomBGM_Fade("battle_bgm", 0.3f, 1.5f);
	}
}

void Demo::IBattleScene::LockRandomCard(int turns)
{
	std::vector<std::shared_ptr<ICard>> visibleCards;

	for (auto& card : this->cardHand) if (!card->IsLocked()) visibleCards.push_back(card);
	for (auto& card : this->queuedToDraw) if (!card->IsLocked()) visibleCards.push_back(card);

	// Lock a visible (hand/queued) card first
	if (!visibleCards.empty()) {
		int randIdx = RNG::Range(0, static_cast<int>(visibleCards.size() - 1));
		visibleCards[randIdx]->SetLocked(turns);
		this->pendingLockMessage = true;
		return;
	}

	// Otherwise fall back to locking a card still in the draw pile
	std::vector<std::shared_ptr<ICard>> hiddenCards;
	for (auto& card : this->drawPile) if (!card->IsLocked()) hiddenCards.push_back(card);

	if (!hiddenCards.empty()) {
		int randIdx = RNG::Range(0, static_cast<int>(hiddenCards.size() - 1));
		hiddenCards[randIdx]->SetLocked(turns);
		this->pendingLockMessage = true;
	}
}

void Demo::IBattleScene::CollectDeadEnemies()
{
	for (size_t i = 0; i < enemies.size(); ++i) {
		if (enemies[i]->IsDead() && enemies[i]->IsDoneAttacking()) {
			battleGoldReward += enemies[i]->GetGoldReward();
			enemies.erase(enemies.begin() + i);
			--i;
		}
	}
}

void Demo::IBattleScene::OnAllEnemiesDefeated()
{
	if (isBattleEnding) {
		return;
	}
	isBattleEnding = true;
	int finalGold = battleGoldReward;
	if (initialEnemyCount > 1) {
		const float multiplier = 1.25f * static_cast<float>(initialEnemyCount - 1);
		finalGold = static_cast<int>(std::round(finalGold * multiplier));
	}

	player->AddGold(finalGold);
	popUpMessage->QueueMessage(&commandBuffer, L"You earned " + std::to_wstring(finalGold) + L" gold!", 1.5f);
	DX9GF::AudioManager::GetInstance()->Play("coin_gather", false, 0.8f);

	if (onVictoryCallback != nullptr) {
		onVictoryCallback();
	}

	auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, true);
	drawBuffer->PushCommand(std::make_shared<DX9GF::DelayCommand>(2.5f));
	drawBuffer->PushCommand(transitionInCommand);
	commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, transitionInCommand](std::function<void(void)> markFinished) {
		if (!transitionInCommand->IsFinished()) {
			return;
		}
		player->SetHealth(battlePlayer->GetHealth());
		auto sceMan = game->GetSceneManager();
		auto prevScene = sceMan->GetScene(sceMan->GetIndex() - 1);
		auto audio = DX9GF::AudioManager::GetInstance();

		if (dynamic_cast<Demo::TutorialWorldScene*>(prevScene)) audio->PlayBGM_Fade("bgm_tutorial", 0.5f, 1.0f);
		else if (dynamic_cast<Demo::SecretPuzzleScene*>(prevScene)) audio->PlayBGM_Fade("bgm_secret", 0.3f, 1.0f);
		else if (dynamic_cast<Demo::ThreadAlleyScene*>(prevScene)) audio->PlayBGM_Fade("bgm_arcade", 0.2f, 1.0f);
		else if (dynamic_cast<Demo::BossWorldScene*>(prevScene)) audio->PlayBGM_Fade("bgm_boss", 0.3f, 1.0f);
		sceMan->RemoveScene(sceMan->GetIndex());
		sceMan->GoToPrevious();
		markFinished();
		}));
}

void Demo::IBattleScene::DrawCardsNow(int amount)
{
	if (amount <= 0) {
		return;
	}
	DrawCards(static_cast<size_t>(amount), true);
}

void Demo::IBattleScene::DrawCards(size_t count, bool midTurn)
{
	auto app = DX9GF::Application::GetInstance();
	float screenWidth = static_cast<float>(app->GetScreenWidth());
	float screenHeight = static_cast<float>(app->GetScreenHeight());
	auto x = -static_cast<float>(screenWidth) / 2.f + 20.f;
	auto y = -static_cast<float>(screenHeight) / 2.f + 20.f;
	for (size_t i = 0; i < count; ++i) {
		if (drawPile.empty()) {
			ShuffleDiscardIntoDrawPile();
		}
		if (drawPile.empty()) {
			break;
		}
		auto card = drawPile.back();
		drawPile.pop_back();
		this->queuedToDraw.push_back(card);
		card->SetOwner(battlePlayer.get());
		card->SetBattleScene(this);
		auto draggable = dynamic_pointer_cast<IDraggable>(card);
		if (draggable) {
			draggable->DetachParent();
			draggable->SetHidden(false);
		}
		y += 30;
		std::vector<std::shared_ptr<DX9GF::ICommand>> commands = {
			std::make_shared<DX9GF::SetPositionCommand>(card, -static_cast<float>(screenWidth), y),
			std::make_shared<DX9GF::CustomCommand>([this, card](std::function<void(void)> markFinished) {
				DX9GF::AudioManager::GetInstance()->PlayRandom("card_draw", 0.3f);
				markFinished();
			}),
			std::make_shared<DX9GF::DelayCommand>(midTurn ? 0.f : i * .2f),
			std::make_shared<DX9GF::GoToCommand>(card, x, y, 0.5f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}),
			std::make_shared<DX9GF::DelayCommand>(midTurn ? 0.f : 0.75f),
			std::make_shared<DX9GF::GoToCommand>(card, x, screenHeight, 0.75f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}),
			std::make_shared<DX9GF::CustomCommand>([this, card](std::function<void(void)> markFinished) {
				this->queuedToDraw.erase(std::find(queuedToDraw.begin(), queuedToDraw.end(), card));
				cardHand.push_back(card);
				if (handContainer) {
					handContainer->StoreCard(card);
				}
				markFinished();
			})
		};
		commandBuffer.StackCommand(std::make_shared<DX9GF::MultiCommand>(std::move(commands)));
	}
}

void Demo::IBattleScene::ShuffleDiscardIntoDrawPile()
{
	if (discardPile.empty()) {
		return;
	}
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::shuffle(discardPile.begin(), discardPile.end(), gen);
	for (auto& card : discardPile) {
		HidePileCard(card);
		drawPile.push_back(card);
	}
	discardPile.clear();
	DX9GF::AudioManager::GetInstance()->PlayRandom("card_draw", 0.6f);
}

void Demo::IBattleScene::MovePlayedPileToDiscardPileIfNeeded()
{
	if (currentTurn == 0 || currentTurn % 3 != 1 || playedPile.empty()) {
		return;
	}
	for (size_t i = 0; i < enemyCards.size(); ++i) {
		auto& enemyCard = enemyCards[i];
		if (auto manager = enemyCard->GetDraggableManager().lock()) {
			manager->Remove(enemyCard);
		}
		enemyCards.erase(enemyCards.begin() + i);
		--i;
	}
	for (auto& card : playedPile) {
		HidePileCard(card);
		discardPile.push_back(card);
	}
	playedPile.clear();
}

void Demo::IBattleScene::MoveExecutedHandCardsToPlayedPile()
{
	for (size_t i = 0; i < cardHand.size(); ++i) {
		auto parent = cardHand[i]->GetParent();
		if (parent.has_value() && parent.value().lock().get() == handContainer.get()) {
			continue;
		}
		playedPile.push_back(cardHand[i]);
		cardHand.erase(cardHand.begin() + i);
		--i;
	}
}

void Demo::IBattleScene::MoveDepletedCardsToNullifiedPile()
{
	auto drain = [this](std::vector<std::shared_ptr<ICard>>& pile) {
		for (size_t i = 0; i < pile.size(); ++i) {
			if (!pile[i]->IsDepleted()) {
				continue;
			}
			HidePileCard(pile[i]);
			nullifiedPile.push_back(pile[i]);
			pile.erase(pile.begin() + i);
			--i;
		}
		};
	// Played cards keep running from inside the block, so detaching them here is what
	// actually stops a depleted card from executing again next turn.
	drain(playedPile);
	drain(cardHand);
}

bool Demo::IBattleScene::CanPlaceCardInBlock(const std::shared_ptr<ICard>& card) const
{
	if (!card) {
		return false;
	}
	// Not in hand at all - queued from an earlier turn, so it was paid for when it landed and
	// moving it between blocks costs nothing.
	if (std::find(cardHand.begin(), cardHand.end(), card) == cardHand.end()) {
		return true;
	}
	bool countedAlready = true;
	if (auto parent = card->GetParent(); parent.has_value()) {
		if (auto lock = parent.value().lock(); lock && lock == handContainer) {
			countedAlready = false;
		}
	}
	const int cost = countedAlready ? 0 : static_cast<int>(card->GetCost());
	return GetAvailableEnergy() >= cost;
}

void Demo::IBattleScene::ReturnCardToHand(const std::shared_ptr<IStatementCard>& card)
{
	if (!card) {
		return;
	}
	commandBuffer.StackCommand(std::make_shared<DX9GF::CustomCommand>([this, card](std::function<void(void)> markFinished) {
		// A drop is offered to every droppable in turn, so a block that rejected this card may be
		// followed by one that accepts it - and the player may have grabbed it again. Only rescue
		// the card if it is still loose, or this would yank it out of wherever it ended up.
		if (handContainer && !card->GetParent().has_value() && !card->IsDragging()) {
			handContainer->StoreCard(card);
		}
		markFinished();
		}));
}

void Demo::IBattleScene::MoveNonPersistentCardsToDiscardPile()
{
	// Runs after the depleted sweep, so a card that is both depleted and non-persistent is
	// already gone to the nullified pile - depletion is permanent, non-persistence is not.
	// As with depleted cards, detaching from the block is what stops them executing again.
	for (size_t i = 0; i < playedPile.size(); ++i) {
		if (playedPile[i]->IsPersistent()) {
			continue;
		}
		HidePileCard(playedPile[i]);
		discardPile.push_back(playedPile[i]);
		playedPile.erase(playedPile.begin() + i);
		--i;
	}
}

void Demo::IBattleScene::FinishInitBlockExecution()
{
	if (!initBlockCard) {
		return;
	}
	std::vector<std::shared_ptr<ICard>> resolved;
	for (auto& weak : initBlockCard->GetStatementCards()) {
		if (auto card = weak.lock()) {
			resolved.push_back(card);
		}
	}
	for (auto& card : resolved) {
		// Only charge for cards the per-frame recompute was charging for. A card dragged in from
		// the main block is already in the played pile and was paid for on the turn it landed
		// there, so committing it again would bill the player twice.
		auto handIt = std::find(cardHand.begin(), cardHand.end(), card);
		if (handIt != cardHand.end()) {
			committedEnergy += static_cast<int>(card->GetCost());
			cardHand.erase(handIt);
		}
		else {
			auto playedIt = std::find(playedPile.begin(), playedPile.end(), card);
			if (playedIt != playedPile.end()) {
				playedPile.erase(playedIt);
			}
		}
		HidePileCard(card);
		// Executing here spends a use just as it does in the main block, so a card that ran out
		// has to be nullified rather than discarded - otherwise it would be drawn again with no
		// uses left and keep resolving for free.
		if (card->IsDepleted()) {
			nullifiedPile.push_back(card);
		}
		else {
			discardPile.push_back(card);
		}
	}
}

void Demo::IBattleScene::MoveInitBlockCardsToDiscardPile()
{
	if (!initBlockCard) {
		return;
	}
	// Paid for but never run. That is the player's call, so the cards are discarded rather than
	// refunded - but they must not stay attached to the block across the turn boundary.
	std::vector<std::shared_ptr<ICard>> leftovers;
	for (auto& weak : initBlockCard->GetStatementCards()) {
		if (auto card = weak.lock()) {
			leftovers.push_back(card);
		}
	}
	for (auto& card : leftovers) {
		auto handIt = std::find(cardHand.begin(), cardHand.end(), card);
		if (handIt != cardHand.end()) {
			cardHand.erase(handIt);
		}
		HidePileCard(card);
		if (card->IsDepleted()) {
			nullifiedPile.push_back(card);
		}
		else {
			discardPile.push_back(card);
		}
	}
}

void Demo::IBattleScene::MoveHandCardsToDiscardPile()
{
	for (size_t i = 0; i < cardHand.size(); ++i) {
		if (cardHand[i]->IsLocked()) {
			continue;
		}
		HidePileCard(cardHand[i]);
		discardPile.push_back(cardHand[i]);
		cardHand.erase(cardHand.begin() + i);
		--i;
	}
}

const std::vector<std::shared_ptr<Demo::ICard>>& Demo::IBattleScene::GetPile(PileKind kind) const
{
	switch (kind) {
	case PileKind::Discard:   return discardPile;
	case PileKind::Nullified: return nullifiedPile;
	default:                  return drawPile;
	}
}

std::wstring Demo::IBattleScene::GetPileName(PileKind kind)
{
	switch (kind) {
	case PileKind::Discard:   return L"Discard Pile";
	case PileKind::Nullified: return L"Nullified Pile";
	default:                  return L"Draw Pile";
	}
}

void Demo::IBattleScene::OpenPileView(PileKind kind)
{
	viewedPile = kind;
	currentPilePage = 0;
	state = State::PlayerViewPile;
	RefreshPileView();
	DX9GF::AudioManager::GetInstance()->PlayRandom("card_draw", 0.4f);
}

void Demo::IBattleScene::ClosePileView()
{
	// Drop anything the view queued this frame before its cards are destroyed.
	draggableManager->ClearQueuedDraws();
	for (auto& card : pileViewCards) {
		if (auto draggable = std::dynamic_pointer_cast<IDraggable>(card)) {
			draggableManager->Remove(draggable);
		}
	}
	pileViewCards.clear();
	state = State::PlayerAttack;
}

void Demo::IBattleScene::RefreshPileView()
{
	// Paging destroys the current page's cards, same hazard as closing the view.
	draggableManager->ClearQueuedDraws();
	for (auto& card : pileViewCards) {
		if (auto draggable = std::dynamic_pointer_cast<IDraggable>(card)) {
			draggableManager->Remove(draggable);
		}
	}
	pileViewCards.clear();

	const auto& pile = GetPile(viewedPile);

	const float targetWidth = itemMenuBackground->GetTargetWidth();
	const float targetHeight = itemMenuBackground->GetTargetHeight();
	const float columnWidth = 240.f;
	const float rowHeight = 40.f;

	int columns = static_cast<int>(std::floor((targetWidth - PADDING_X * 2.f) / columnWidth));
	int rows = static_cast<int>(std::floor((targetHeight - PADDING_Y * 2.f - 90.f) / rowHeight));
	if (columns < 1) columns = 1;
	if (rows < 1) rows = 1;

	const int perPage = columns * rows;
	maxPilePage = pile.empty() ? 0 : (static_cast<int>(pile.size()) - 1) / perPage;
	if (currentPilePage > maxPilePage) currentPilePage = maxPilePage;
	if (currentPilePage < 0) currentPilePage = 0;

	const int startIndex = currentPilePage * perPage;
	const int endIndex = (std::min)(static_cast<int>(pile.size()), startIndex + perPage);

	const float gridWidth = columns * columnWidth;
	const float startX = -gridWidth / 2.f;
	const float startY = -targetHeight / 2.f + PADDING_Y + 40.f;

	// The pile's own cards are mid-battle state (parented into the block, hidden off-screen,
	// bound to the world camera), so the view shows fresh copies bound to the UI camera.
	for (int i = startIndex; i < endIndex; ++i) {
		const int slot = i - startIndex;
		auto copy = ICard::CreateCard(pile[i]->GetSaveID(), transformManager, draggableManager, game->GetGraphicsDevice(), &uiCamera);
		if (!copy) {
			continue;
		}
		// Carry the use counter across so a half-spent card shows the right pips.
		if (pile[i]->HasLimitedUses()) {
			copy->SetMaxUses(pile[i]->GetMaxUses());
			for (int spent = pile[i]->GetRemainingUses(); spent < pile[i]->GetMaxUses(); ++spent) {
				copy->ConsumeUse();
			}
		}
		copy->SetLocalPosition(startX + (slot % columns) * columnWidth, startY + (slot / columns) * rowHeight);
		pileViewCards.push_back(copy);
	}

	const float pageY = targetHeight / 2.0f - PADDING_Y - 15.0f;
	btnPilePrevPage->SetLocalPosition(-targetWidth / 2.0f + PADDING_X, pageY);
	btnPileNextPage->SetLocalPosition(targetWidth / 2.0f - PADDING_X - btnPileNextPage->GetWidth(), pageY);

	if (transformManager) {
		transformManager->RebuildHierarchy();
		transformManager->UpdateAll();
	}
}

float Demo::IBattleScene::LayOutPileButtons(float screenWidth, float buttonY)
{
	const float sidePadding = 20.f;
	const float total = PILE_BUTTON_SIZE * 3.f + PILE_BUTTON_GAP * 2.f;
	const float x = screenWidth / 2.f - sidePadding - total;
	// Taller than the back/execute buttons, so sit them on the bar's baseline rather than its top.
	const float y = buttonY + backButton->GetHeight() - PILE_BUTTON_SIZE;
	drawPileButton->SetLocalPosition(x, y);
	discardPileButton->SetLocalPosition(x + PILE_BUTTON_SIZE + PILE_BUTTON_GAP, y);
	nullifiedPileButton->SetLocalPosition(x + (PILE_BUTTON_SIZE + PILE_BUTTON_GAP) * 2.f, y);
	return x;
}

void Demo::IBattleScene::BeginNextTurn()
{
	++currentTurn;
	MovePlayedPileToDiscardPileIfNeeded();
	DrawCards(CARDS_DRAWN_PER_TURN + pendingBonusDraw);
	pendingBonusDraw = 0;
	energy = MAX_ENERGY + pendingBonusEnergy;
	pendingBonusEnergy = 0;
	usedEnergy = 0;
	committedEnergy = 0;

	battlePlayer->TickDurations(TickPhase::EndOfRound);
	for (auto& enemy : enemies) {
		enemy->TickDurations(TickPhase::EndOfRound);
		enemy->OnTurnBegin(this->battlePlayer, this->popUpMessage, this->currentTurn);
	}
}

void Demo::IBattleScene::QueueEnemyLayoutTransition(State targetState)
{
	if (enemies.empty()) {
		return;
	}

	const auto app = DX9GF::Application::GetInstance();
	const float centerLineY = -120.f;
	const float horizontalSpacing = 120.f;
	const float verticalSpacing = 240.f;
	const float rightSideX = app->GetScreenWidth() / 2.f - 186.f;

	bool hasQueued = false;
	const size_t enemyCount = enemies.size();
	for (size_t i = 0; i < enemyCount; ++i) {
		float targetX = 0.f;
		float targetY = 0.f;

		if (targetState == State::PlayerStandBy) {
			const float totalWidth = (enemyCount > 1) ? (enemyCount - 1) * horizontalSpacing : 0.f;
			targetX = -totalWidth / 2.f + i * horizontalSpacing;
			targetY = centerLineY;
		}
		else if (targetState == State::EnemyAttack) {
			const float totalHeight = (enemyCount > 1) ? (enemyCount - 1) * verticalSpacing : 0.f;
			targetX = app->GetScreenWidth() / 1.5f;
			targetY = -totalHeight / 2.f + i * verticalSpacing;
		}
		else {
			const float totalHeight = (enemyCount > 1) ? (enemyCount - 1) * verticalSpacing : 0.f;
			targetX = rightSideX;
			targetY = -totalHeight / 2.f + i * verticalSpacing;
		}

		auto command = std::make_shared<DX9GF::GoToCommand>(enemies[i], targetX, targetY, 0.4f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{});
		if (!hasQueued) {
			commandBuffer.StackCommand(command);
			hasQueued = true;
		}
		else {
			commandBuffer.StackCommand(command);
		}
	}
}

void Demo::IBattleScene::CreateEnemyCard(std::shared_ptr<IEnemy> enemy)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<int> xdist(-10, 10);
	std::uniform_int_distribution<int> ydist(-10, 10);

	auto trigger = enemy->GetCardSpawnTrigger().lock();
	const float triggerHalfHeight = trigger ? trigger->GetHeight() * 0.5f : 32.f;
	const float closeOffsetY = triggerHalfHeight + 10.f;

	const float cardHalfWidth = 52.f;

	const float spawnX = enemy->GetWorldX() - cardHalfWidth + xdist(gen);
	const float spawnY = enemy->GetWorldY() + closeOffsetY + ydist(gen);

	auto card = std::make_shared<EnemyCard>(transformManager, enemy, spawnX, spawnY);
	card->Init(draggableManager, game->GetGraphicsDevice(), &camera);
	enemyCards.push_back(card);
}

void Demo::IBattleScene::RemoveEnemyCardsInRemoveArea()
{
	const float left = enemyCardRemoveAreaX;
	const float top = enemyCardRemoveAreaY;
	const float right = enemyCardRemoveAreaX + enemyCardRemoveAreaWidth;
	const float bottom = enemyCardRemoveAreaY + enemyCardRemoveAreaHeight;

	enemyCards.erase(std::remove_if(enemyCards.begin(), enemyCards.end(), [&](const std::shared_ptr<EnemyCard>& enemyCard) {
		if (!enemyCard || enemyCard->IsDragging()) {
			return false;
		}
		auto [x, y] = enemyCard->GetWorldPosition();
		if (x >= left && x <= right && y >= top && y <= bottom) {
			if (auto manager = enemyCard->GetDraggableManager().lock()) {
				manager->Remove(enemyCard);
			}
			return true;
		}
		return false;
		}), enemyCards.end());
}

void Demo::IBattleScene::RefreshItemMenu()
{
	buffItems.clear();

	auto& inventory = player->GetInventoryItems().GetSlots();

	std::vector<int> validIndices;

	for (int i = 0; i < inventory.size(); i++) {
		if (inventory[i].quantity > 0 && Demo::ItemData::GetInstance()->GetItemBlueprint(inventory[i].itemID)) {
			validIndices.push_back(i);
		}
	}

	float targetWidth = itemMenuBackground->GetTargetWidth();
	float targetHeight = itemMenuBackground->GetTargetHeight();

	int columns = std::floor((targetWidth - PADDING_X * 2.0f) / (ITEM_W + PADDING_X));
	int rows = std::floor((targetHeight - PADDING_Y * 2.0f - 90.0f) / (ITEM_H + PADDING_Y));
	if (columns < 1) columns = 1;
	if (rows < 1) rows = 1;

	int itemsPerPage = columns * rows;

	maxItemPage = validIndices.empty() ? 0 : (validIndices.size() - 1) / itemsPerPage;
	if (currentItemPage > maxItemPage) currentItemPage = maxItemPage;
	if (currentItemPage < 0) currentItemPage = 0;

	int startIndex = currentItemPage * itemsPerPage;
	int endIndex = std::min(static_cast<int>(validIndices.size()), startIndex + itemsPerPage);

	float totalGridWidth = (columns * ITEM_W) + ((columns - 1) * PADDING_X);
	float startX = -totalGridWidth / 2.0f;
	float startY = -targetHeight / 2.0f + PADDING_Y + 40.0f;

	int displayIndex = 0;
	for (int i = startIndex; i < endIndex; i++)
	{
		int originalIndex = validIndices[i];
		auto slot = inventory[originalIndex];
		auto blueprint = Demo::ItemData::GetInstance()->GetItemBlueprint(slot.itemID);

		int col = displayIndex % columns;
		int row = displayIndex / columns;

		float baseX = startX + col * (ITEM_W + PADDING_X);
		float baseY = startY + row * (ITEM_H + PADDING_Y);

		auto btn = std::make_shared<IconButton>(transformManager, 0, 0, ITEM_W, ITEM_H, itemsTex, 1);
		btn->Init(&uiCamera);
		btn->SetSpriteScale(MENU_SCALE, MENU_SCALE);
		btn->SetLocalPosition(baseX, baseY);
		btn->Update(0);

		btn->SetSpriteRects({ blueprint->GetItemRect() });

		btn->SetOnReleaseLeft([this, slot, blueprint](DX9GF::ITrigger* thisObj) {
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, slot, blueprint](std::function<void(void)> markFinished) {
				if (player->GetInventoryItems().ConsumeItem(slot.itemID)) {
					for (auto& mod : blueprint->GetModifiers()) {
						if (mod.type == Demo::ModifierType::HealHP) {
							battlePlayer->Heal(mod.value);
						}
						else {
							battlePlayer->AddModifier(mod.type, mod.duration, mod.value, mod.isBuff, mod.delayTurns);
						}
					}
					std::wstring msg = L"Used " + blueprint->GetName() + L"!";
					popUpMessage->QueueMessage(&commandBuffer, msg);

					this->RefreshItemMenu();
					this->QueueToEnemyAttack(0);
				}
				markFinished();
				}));
			});

		buffItems.push_back(btn);
		displayIndex++;
	}

	float btnY = targetHeight / 2.0f - PADDING_Y - 15.0f;
	btnPrevPage->SetLocalPosition(-targetWidth / 2.0f + PADDING_X, btnY);
	btnNextPage->SetLocalPosition(targetWidth / 2.0f - PADDING_X - btnNextPage->GetWidth(), btnY);

	transformManager->RebuildHierarchy();
	transformManager->UpdateAll();
}

void Demo::IBattleScene::PlayerStandByUpdate(unsigned long long deltaTime)
{
	auto app = DX9GF::Application::GetInstance();
	// Set position so that it's updated if screen is resized
	const float buttonY = app->GetScreenHeight() / 2.f - 50 - attackButton->GetHeight();
	const float spacing = 20.f;
	const float totalButtonsWidth = attackButton->GetWidth() + itemsButton->GetWidth() + fleeButton->GetWidth() + 2 * spacing;
	const float leftX = -totalButtonsWidth / 2;

	attackButton->SetLocalPosition(leftX, buttonY);
	itemsButton->SetLocalPosition(leftX + attackButton->GetWidth() + spacing, buttonY);
	fleeButton->SetLocalPosition(leftX + attackButton->GetWidth() + spacing + itemsButton->GetWidth() + spacing, buttonY);

	if (!isFleeing) {
		fleeButton->Update(deltaTime);
		itemsButton->Update(deltaTime);
		attackButton->Update(deltaTime);
	}
	for (auto& enemy : enemies) {
		enemy->Update(deltaTime);
	}
}

void Demo::IBattleScene::PlayerAttackUpdate(unsigned long long deltaTime)
{
	auto app = DX9GF::Application::GetInstance();
	float screenWidth = static_cast<float>(app->GetScreenWidth());
	float screenHeight = static_cast<float>(app->GetScreenHeight());
	handContainer->SetLocalPosition(-screenWidth / 2.f + 20.f, -screenHeight / 2.f + 20.f);
	usedEnergy = committedEnergy;
	for (auto& card : cardHand) {
		if (auto parent = card->GetParent(); parent.has_value()) {
			if (auto lock = parent.value().lock(); lock && lock == handContainer) {
				continue;
			}
		}
		usedEnergy += card->GetCost();
	}
	const float buttonY = screenHeight / 2.f - 20 - attackButton->GetHeight();
	const float sidePadding = 20.f;
	const float leftX = -screenWidth / 2.f + sidePadding;
	const float executeX = leftX + backButton->GetWidth() + sidePadding;

	const float runInitX = executeX + executeButton->GetWidth() + sidePadding;

	backButton->SetLocalPosition(leftX, buttonY);
	executeButton->SetLocalPosition(executeX, buttonY);
	runInitButton->SetLocalPosition(runInitX, buttonY);
	const float pileButtonsX = LayOutPileButtons(screenWidth, buttonY);
	enemyCardRemoveAreaX = runInitX + runInitButton->GetWidth() + sidePadding;
	enemyCardRemoveAreaY = buttonY;
	// Stops short of the pile buttons parked at the right end of the same bar.
	enemyCardRemoveAreaWidth = pileButtonsX - sidePadding - enemyCardRemoveAreaX;
	enemyCardRemoveAreaHeight = backButton->GetHeight();
	const bool initExecuting = initBlockCard && initBlockCard->IsExecuting();
	// Unlike the main block finishing, this does not end the turn - the cards are committed and
	// discarded and the player carries on programming.
	if (!initExecuting && isExecutingInit) {
		isExecutingInit = false;
		FinishInitBlockExecution();
	}
	if (!mainBlockCard->IsExecuting() && !initExecuting) {
		// When finished executing attack
		if (isExecutingAttacks) {
			isExecutingAttacks = false;
			return QueueToEnemyAttack(deltaTime);
		}
		if (!isTransitioning) {
			// queuedToDraw guard: retreating would end the turn and discard the hand while cards
			// from a mid-turn draw are still in flight, stranding them outside every pile.
			if (usedEnergy == 0 && queuedToDraw.empty()) backButton->Update(deltaTime);
			executeButton->Update(deltaTime);
			runInitButton->Update(deltaTime);
			drawPileButton->Update(deltaTime);
			discardPileButton->Update(deltaTime);
			nullifiedPileButton->Update(deltaTime);
		}
	}
	for (auto& enemy : enemies) {
		enemy->Update(deltaTime);
	}
	if (!isTransitioning) {
		draggableManager->Update(deltaTime);
		RemoveEnemyCardsInRemoveArea();
	}
}

void Demo::IBattleScene::StartAttackCountdown(std::shared_ptr<std::vector<std::shared_ptr<IEnemy>>> attackingEnemies)
{
	isAttackCountdownActive = true;
	attackCountdownNumber = 3;
	attackCountdownTimer = ATTACK_COUNTDOWN_STEP_SECONDS;
	countdownAttackingEnemies = attackingEnemies;
}

bool Demo::IBattleScene::UpdateAttackCountdown(unsigned long long deltaTime)
{
	if (!isAttackCountdownActive) {
		return false;
	}

	attackCountdownTimer -= deltaTime / 1000.0f;
	if (attackCountdownTimer <= 0.f) {
		attackCountdownNumber--;
		if (attackCountdownNumber <= 0) {
			isAttackCountdownActive = false;
			return true;
		}
		attackCountdownTimer = ATTACK_COUNTDOWN_STEP_SECONDS;
	}
	return false;
}

void Demo::IBattleScene::DrawAttackCountdown(unsigned long long deltaTime)
{
	if (!isAttackCountdownActive) {
		return;
	}

	float progress = 1.f - (attackCountdownTimer / ATTACK_COUNTDOWN_STEP_SECONDS);
	if (progress < 0.f) progress = 0.f;
	if (progress > 1.f) progress = 1.f;

	const float startScale = 4.5f;
	const float endScale = 3.0f;
	const float pulseT = (std::min)(1.f, progress / 0.35f);
	const float ease = 1.f - (1.f - pulseT) * (1.f - pulseT);
	const float scale = startScale - (startScale - endScale) * ease;

	float alpha = 1.f;
	if (progress > 0.75f) {
		alpha = 1.f - (progress - 0.75f) / 0.25f;
	}
	const int alphaByte = static_cast<int>((std::max)(0.f, (std::min)(1.f, alpha)) * 255.f);

	std::wstring text = std::to_wstring(attackCountdownNumber);

	fontSprite->Begin();
	fontSprite->SetScale(scale);
	fontSprite->SetBold(true);
	fontSprite->SetOutline(true, D3DCOLOR_ARGB(alphaByte, 0, 0, 0), 4.f);
	fontSprite->SetColor(D3DCOLOR_ARGB(alphaByte, 255, 60, 60));
	fontSprite->SetText(std::move(text));

	const float textWidth = fontSprite->GetWidth();
	const float textHeight = fontSprite->GetHeight();

	fontSprite->SetPosition(-textWidth / 2.f, -textHeight / 2.f);

	fontSprite->Draw(this->uiCamera, deltaTime);

	fontSprite->SetBold(false);
	fontSprite->SetScale(1.f);
	fontSprite->SetOutline(false);
	fontSprite->End();
}

std::vector<std::shared_ptr<Demo::IBlockCard>> Demo::IBattleScene::GetBlockCards() const
{
	std::vector<std::shared_ptr<IBlockCard>> blocks;
	if (mainBlockCard) {
		blocks.push_back(mainBlockCard);
	}
	if (initBlockCard) {
		blocks.push_back(initBlockCard);
	}
	return blocks;
}

std::vector<Demo::KeyboardNavigator::Candidate> Demo::IBattleScene::CollectKeyboardCandidates()
{
	std::vector<KeyboardNavigator::Candidate> candidates;

	auto addButton = [&](std::shared_ptr<IButton> button) {
		if (!button) {
			return;
		}
		candidates.push_back({
			button,
			button->GetWorldX(),
			button->GetWorldY(),
			(float)button->GetWidth(),
			(float)button->GetHeight(),
			[button]() { button->Activate(); }
			});
		};

	switch (state) {
	case State::PlayerStandBy: {
		if (!isFleeing) {
			addButton(attackButton);
			addButton(itemsButton);
			addButton(fleeButton);
		}
		break;
	}
	case State::PlayerAttack: {
		if (!mainBlockCard->IsExecuting() && !(initBlockCard && initBlockCard->IsExecuting()) && !isTransitioning) {
			if (usedEnergy == 0 && queuedToDraw.empty()) {
				addButton(backButton);
			}
			addButton(executeButton);
			addButton(runInitButton);
			addButton(drawPileButton);
			addButton(discardPileButton);
			addButton(nullifiedPileButton);
		}

		// Hand cards, plus orphaned cards floating in space (e.g. a card whose drop onto the
		// block was rejected for lack of energy) - block-parented cards are handled below.
		for (auto& card : cardHand) {
			if (card->IsLocked()) {
				continue;
			}

			auto statementCard = std::dynamic_pointer_cast<IStatementCard>(card);
			if (!statementCard) {
				continue;
			}
			auto parent = card->GetParent();
			const bool inHand = parent.has_value() && parent.value().lock().get() == handContainer.get();
			const bool orphaned = !parent.has_value();
			if (!inHand && !orphaned) {
				continue;
			}
			if (orphaned && statementCard->IsDragging()) {
				continue; // mid mouse-drag
			}
			candidates.push_back({
				statementCard,
				statementCard->GetWorldX(),
				statementCard->GetWorldY(),
				(float)statementCard->GetWidth(),
				(float)statementCard->GetHeight(),
				[this, statementCard]() { pickedUpCard = statementCard; placementSlotIndex = 0; }
				});
		}

		// StatementCards already queued in either block can also be targeted, to reorder them,
		// and each block itself is a candidate so it can be repositioned.
		for (auto& block : GetBlockCards()) {
			for (auto& weakChild : block->GetChildren()) {
				auto placedStatementCard = std::dynamic_pointer_cast<IStatementCard>(weakChild.lock());

				if (!placedStatementCard || placedStatementCard->IsLocked()) {
					continue;
				}

				candidates.push_back({
					placedStatementCard,
					placedStatementCard->GetWorldX(),
					placedStatementCard->GetWorldY(),
					(float)placedStatementCard->GetWidth(),
					(float)placedStatementCard->GetHeight(),
					[this, placedStatementCard]() { pickedUpCard = placedStatementCard; placementSlotIndex = 0; }
					});
			}

			candidates.push_back({
				block,
				block->GetWorldX(),
				block->GetWorldY(),
				(float)block->GetWidth(),
				(float)block->GetHeight(),
				[this]() { isDraggingBlockCardViaKeyboard = true; }
				});
		}

		for (auto& enemy : enemies) {
			if (enemy->IsDead() || enemy->IsOnStandby()) {
				continue;
			}
			auto trigger = enemy->GetCardSpawnTrigger().lock();
			if (!trigger) {
				continue;
			}
			// The trigger uses a center origin, so its top-left (not the enemy's own world position) is the correct bound.
			candidates.push_back({
				enemy,
				trigger->GetWorldX() - trigger->GetOriginX(),
				trigger->GetWorldY() - trigger->GetOriginY(),
				trigger->GetWidth(),
				trigger->GetHeight(),
				[this, enemy]() {
					CreateEnemyCard(enemy);
					// Skip having to separately navigate to the freshly spawned card - go straight
					// into placement mode so the player immediately picks a target or discards it.
					if (!enemyCards.empty()) {
						pickedUpCard = enemyCards.back();
						placementSlotIndex = 0;
					}
				}
				});
		}
		break;
	}
	case State::PlayerOpenItems: {
		if (!isTransitioning) {
			addButton(closeItemMenuButton);
			if (currentItemPage > 0) {
				addButton(btnPrevPage);
			}
			if (currentItemPage < maxItemPage) {
				addButton(btnNextPage);
			}
			for (auto& btn : buffItems) {
				addButton(btn);
			}
		}
		break;
	}
	case State::PlayerViewPile: {
		if (!isTransitioning) {
			addButton(closePileViewButton);
			if (currentPilePage > 0) {
				addButton(btnPilePrevPage);
			}
			if (currentPilePage < maxPilePage) {
				addButton(btnPileNextPage);
			}
		}
		break;
	}
	default:
		break;
	}

	return candidates;
}

void Demo::IBattleScene::UpdateKeyboardNavigation(unsigned long long deltaTime)
{
	auto inpMan = DX9GF::InputManager::GetInstance();
	auto sm = SettingsManager::GetInstance();

	const int keyUp = sm->GetKeybind("MOVE_UP");
	const int keyDown = sm->GetKeybind("MOVE_DOWN");
	const int keyLeft = sm->GetKeybind("MOVE_LEFT");
	const int keyRight = sm->GetKeybind("MOVE_RIGHT");
	const int keyAccept = sm->GetKeybind("ACCEPT");

	keyboardNavigator.UpdateMode();
	if (!keyboardNavigator.IsInKeyboardMode()) {
		isDraggingBlockCardViaKeyboard = false;
		pickedUpCard.reset();
		return;
	}

	if (pickedUpCard) {
		auto slots = CollectPlacementSlots();
		if (slots.empty()) {
			pickedUpCard.reset();
			return;
		}
		if (placementSlotIndex >= slots.size()) {
			placementSlotIndex = slots.size() - 1;
		}

		int slotDirX = 0, slotDirY = 0;
		if (inpMan->KeyDown(keyLeft)) slotDirX = -1;
		else if (inpMan->KeyDown(keyRight)) slotDirX = 1;
		if (inpMan->KeyDown(keyUp)) slotDirY = -1;
		else if (inpMan->KeyDown(keyDown)) slotDirY = 1;

		if (slotDirX != 0 || slotDirY != 0) {
			std::vector<std::pair<float, float>> positions;
			positions.reserve(slots.size());
			for (auto& slot : slots) {
				positions.push_back({ slot.x, slot.y });
			}
			const int best = KeyboardNavigator::PickDirectional(
				slots[placementSlotIndex].x, slots[placementSlotIndex].y,
				slotDirX, slotDirY, positions, static_cast<int>(placementSlotIndex));
			if (best >= 0) {
				placementSlotIndex = static_cast<size_t>(best);
			}
		}

		if (inpMan->KeyDown(keyAccept)) {
			auto place = slots[placementSlotIndex].place;
			pickedUpCard.reset();
			placementSlotIndex = 0;
			if (place) {
				place();
			}
		}
		return;
	}

	std::shared_ptr<IBlockCard> draggedBlock;
	if (isDraggingBlockCardViaKeyboard) {
		for (auto& block : GetBlockCards()) {
			if (keyboardNavigator.GetTarget() == block) {
				draggedBlock = block;
				break;
			}
		}
	}
	if (draggedBlock) {
		auto app = DX9GF::Application::GetInstance();
		const float halfScreenWidth = app->GetScreenWidth() / 2.f;
		const float halfScreenHeight = app->GetScreenHeight() / 2.f;

		float dx = 0.f, dy = 0.f;
		if (inpMan->KeyPress(keyLeft)) dx -= 1.f;
		if (inpMan->KeyPress(keyRight)) dx += 1.f;
		if (inpMan->KeyPress(keyUp)) dy -= 1.f;
		if (inpMan->KeyPress(keyDown)) dy += 1.f;

		if (dx != 0.f || dy != 0.f) {
			const float len = std::sqrt(dx * dx + dy * dy);
			const float step = KEYBOARD_BLOCK_CARD_SPEED * (deltaTime / 1000.f);
			float newX = draggedBlock->GetWorldX() + (dx / len) * step;
			float newY = draggedBlock->GetWorldY() + (dy / len) * step;
			newX = (std::max)(-halfScreenWidth, (std::min)(newX, halfScreenWidth - (float)draggedBlock->GetWidth()));
			newY = (std::max)(-halfScreenHeight, (std::min)(newY, halfScreenHeight - (float)draggedBlock->GetHeight()));
			draggedBlock->SetLocalPosition(newX, newY);
		}

		if (inpMan->KeyDown(keyAccept)) {
			isDraggingBlockCardViaKeyboard = false;
		}
		return;
	}
	// Target moved off the block card (e.g. it vanished from the candidates) - drop the grab.
	isDraggingBlockCardViaKeyboard = false;

	keyboardNavigator.Navigate(deltaTime, CollectKeyboardCandidates());
}

std::vector<Demo::IBattleScene::PlacementSlot> Demo::IBattleScene::CollectPlacementSlots()
{
	std::vector<PlacementSlot> slots;
	if (!pickedUpCard) {
		return slots;
	}

	if (auto statementCard = std::dynamic_pointer_cast<IStatementCard>(pickedUpCard)) {
		for (auto& block : GetBlockCards()) {
			size_t otherCount = 0;
			for (auto& weak : block->GetStatementCards()) {
				auto sibling = weak.lock();
				if (sibling && sibling != statementCard) {
					++otherCount;
				}
			}
			const float slotWidth = (float)block->GetWidth();
			const float slotHeight = (float)statementCard->GetHeight();
			for (size_t index = 0; index <= otherCount; ++index) {
				auto [slotX, slotY] = block->GetStatementSlotWorldPosition(index, statementCard);
				slots.push_back({
					slotX, slotY, slotWidth, slotHeight,
					[this, statementCard, index, block]() { PlaceStatementCardAt(statementCard, index, block); }
					});
			}
		}

		// A card not already in the hand (queued in the block, or orphaned in space after a
		// rejected drop) can also be sent back to the hand. GetHeight() is just the container's
		// header bar, not the visible card stack below it - offset past it.
		auto parent = statementCard->GetParent();
		const bool isInHand = parent.has_value() && parent.value().lock().get() == handContainer.get();
		if (!isInHand) {
			const float handDropHeight = handContainer->GetMaxHeight() > 0 ? (float)handContainer->GetMaxHeight() : 100.f;
			slots.push_back({
				handContainer->GetWorldX(), handContainer->GetWorldY() + (float)handContainer->GetHeight(),
				(float)handContainer->GetWidth(), handDropHeight,
				[this, statementCard]() { PlaceStatementCardInHand(statementCard); }
				});
		}
		return slots;
	}

	if (auto enemyCard = std::dynamic_pointer_cast<EnemyCard>(pickedUpCard)) {
		for (auto& block : GetBlockCards()) {
			for (auto& weakChild : block->GetChildren()) {
				auto statement = std::dynamic_pointer_cast<IStatementCard>(weakChild.lock());
				if (!statement || !statement->CanAcceptEnemyCard()) {
					continue;
				}
				slots.push_back({
					statement->GetWorldX(), statement->GetWorldY(), (float)statement->GetWidth(), (float)statement->GetHeight(),
					[this, enemyCard, statement]() { PlaceEnemyCardOn(enemyCard, statement); }
					});
			}
		}
		slots.push_back({
			enemyCardRemoveAreaX, enemyCardRemoveAreaY, enemyCardRemoveAreaWidth, enemyCardRemoveAreaHeight,
			[this, enemyCard]() { DiscardEnemyCard(enemyCard); },
			true // the discard bar is drawn in the UI pass
			});
		return slots;
	}

	return slots;
}

void Demo::IBattleScene::PlaceStatementCardAt(std::shared_ptr<IStatementCard> card, size_t index, std::shared_ptr<IBlockCard> block)
{
	if (!card || !block || block->IsExecuting()) {
		return;
	}

	auto parent = card->GetParent();
	const bool alreadyInBlock = parent.has_value() && parent.value().lock().get() == block.get();

	if (!alreadyInBlock && !CanPlaceCardInBlock(card)) {
		QueuePopUpMessage(L"Not enough energy");
		return;
	}

	const float curX = card->GetWorldX();
	const float curY = card->GetWorldY();
	card->DetachParent();
	card->SetLocalPosition(curX, curY);

	auto [slotX, slotY] = block->GetStatementSlotWorldPosition(index, card);

	std::vector<std::shared_ptr<DX9GF::ICommand>> commands = {
		std::make_shared<DX9GF::GoToCommand>(card, slotX, slotY, 0.3f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}),
		std::make_shared<DX9GF::CustomCommand>([card, index, block](std::function<void(void)> markFinished) {
			block->InsertStatementCardAt(card, index);
			DX9GF::AudioManager::GetInstance()->PlayRandom("card_snap", 0.2f);
			markFinished();
			})
	};
	commandBuffer.StackCommand(std::make_shared<DX9GF::MultiCommand>(std::move(commands)));
}

void Demo::IBattleScene::PlaceStatementCardInHand(std::shared_ptr<IStatementCard> card)
{
	if (!card) {
		return;
	}

	const float curX = card->GetWorldX();
	const float curY = card->GetWorldY();
	card->DetachParent();
	card->SetLocalPosition(curX, curY);

	const float handX = handContainer->GetWorldX();
	const float handY = handContainer->GetWorldY() + (float)handContainer->GetHeight();

	std::vector<std::shared_ptr<DX9GF::ICommand>> commands = {
		std::make_shared<DX9GF::GoToCommand>(card, handX, handY, 0.3f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}),
		std::make_shared<DX9GF::CustomCommand>([this, card](std::function<void(void)> markFinished) {
			handContainer->StoreCard(card);
			DX9GF::AudioManager::GetInstance()->PlayRandom("card_snap", 0.2f);
			markFinished();
			})
	};
	commandBuffer.StackCommand(std::make_shared<DX9GF::MultiCommand>(std::move(commands)));
}

void Demo::IBattleScene::PlaceEnemyCardOn(std::shared_ptr<EnemyCard> card, std::shared_ptr<IStatementCard> destination)
{
	if (!card || !destination) {
		return;
	}

	const float curX = card->GetWorldX();
	const float curY = card->GetWorldY();
	card->DetachParent();
	card->SetLocalPosition(curX, curY);

	auto [slotX, slotY] = destination->GetEnemyCardSlotWorldPosition();

	std::vector<std::shared_ptr<DX9GF::ICommand>> commands = {
		std::make_shared<DX9GF::GoToCommand>(card, slotX, slotY, 0.3f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}),
		std::make_shared<DX9GF::CustomCommand>([destination, card](std::function<void(void)> markFinished) {
			destination->AttachEnemyCard(card);
			DX9GF::AudioManager::GetInstance()->PlayRandom("card_snap", 0.2f);
			markFinished();
			})
	};
	commandBuffer.StackCommand(std::make_shared<DX9GF::MultiCommand>(std::move(commands)));
}

void Demo::IBattleScene::DiscardEnemyCard(std::shared_ptr<EnemyCard> card)
{
	if (!card) {
		return;
	}
	if (auto manager = card->GetDraggableManager().lock()) {
		manager->Remove(card);
	}
	enemyCards.erase(std::remove(enemyCards.begin(), enemyCards.end(), card), enemyCards.end());
}

void Demo::IBattleScene::QueueToEnemyAttack(unsigned long long deltaTime)
{
	isTransitioning = true;
	// Use shared_ptr to share state between multiple commands in the MultiCommand
	auto attackingEnemies = std::make_shared<std::vector<std::shared_ptr<IEnemy>>>();
	std::vector<std::shared_ptr<DX9GF::ICommand>> commands = {

		//trigger effects: poison
		std::make_shared<DX9GF::CustomCommand>([this](std::function<void(void)> markFinished) {
			for (auto& enemy : this->enemies) {
				enemy->TriggerEffects(TickPhase::EndOfTurn);
			}
			markFinished();
		}),

		std::make_shared<DX9GF::DelayCommand>(0.6f),

		std::make_shared<DX9GF::CustomCommand>([this, deltaTime, attackingEnemies](std::function<void(void)> markFinished) {
			// Wait for dead enemies to finish animations before collecting them
			for (auto& enemy : this->enemies) {
				if (enemy->IsDead() && !enemy->IsDoneAttacking()) {
					return; // wait
				}
			}

			this->MoveInitBlockCardsToDiscardPile();
			this->MoveExecutedHandCardsToPlayedPile();
			this->MoveDepletedCardsToNullifiedPile();
			this->MoveNonPersistentCardsToDiscardPile();
			this->MoveHandCardsToDiscardPile();

			for (auto& card : this->cardHand) {
				card->TickLock();
			}

			// Remove unused enemy cards or enemy cards of dead enemies
			for (size_t i = 0; i < this->enemyCards.size(); ++i) {
				auto& enemyCard = this->enemyCards[i];
				if (!enemyCard->GetParent().has_value() || enemyCard->GetValue()->IsDead()) {
					if (auto manager = enemyCard->GetDraggableManager().lock()) {
						manager->Remove(enemyCard);
					}
					this->enemyCards.erase(this->enemyCards.begin() + i);
					--i;
				}
			}
			for (auto enemy : this->enemies) {
				bool isStunned = enemy->HasModifier(ModifierType::Stun);

				if (enemy->IsDead()) {
					continue;
				}
				if (isStunned) {
					continue;
				}
				attackingEnemies->push_back(enemy);
			}

			// Remove enemy cards of dead enemies again in case they died from statuses
			for (size_t i = 0; i < this->enemyCards.size(); ++i) {
				auto& enemyCard = this->enemyCards[i];
				if (!enemyCard->GetParent().has_value() || enemyCard->GetValue()->IsDead()) {
					if (auto manager = enemyCard->GetDraggableManager().lock()) {
						manager->Remove(enemyCard);
					}
					this->enemyCards.erase(this->enemyCards.begin() + i);
					--i;
				}
			}
			markFinished();
			}),
		std::make_shared<DX9GF::DelayCommand>(0.5f),
		std::make_shared<DX9GF::CustomCommand>([this, deltaTime, attackingEnemies](std::function<void(void)> markFinished) {
			// Wait for enemies that died from statuses to finish animations
			for (auto& enemy : this->enemies) {
				if (!enemy->IsDoneAttacking()) {
					return; // wait
				}
			}

			this->CollectDeadEnemies();
			if (this->enemies.empty()) {
				this->OnAllEnemiesDefeated();
				this->isTransitioning = false;
				markFinished();
				return;
			}

			if (attackingEnemies->empty()) {

				//even if the enemy is stunned, poison effect on player still hits
				this->battlePlayer->TriggerEffects(TickPhase::EndOfTurn);

				std::vector<std::shared_ptr<DX9GF::ICommand>> commands = {
					std::make_shared<DX9GF::DelayCommand>(0.8f),
					std::make_shared<DX9GF::CustomCommand>([this](std::function<void(void)> markFinished2) {
						if (this->battlePlayer->IsDead()) {
							this->isDefeatSequence = true;
							this->defeatElapsedMs = 0.f;
							this->defeatFadeAlpha = 0.f;
							this->popUpMessage->QueueMessage(&this->commandBuffer, L"You were defeated", 1.5f);
						}
 else {
  this->BeginNextTurn();
  this->state = State::PlayerStandBy;
  this->QueueEnemyLayoutTransition(State::PlayerStandBy);
  this->lastEnemyLayoutState = State::PlayerStandBy;
  this->enemyLayoutInitialized = true;
}
this->isTransitioning = false;
markFinished2();
})
};
this->commandBuffer.PushCommand(std::make_shared<DX9GF::MultiCommand>(std::move(commands)));
markFinished();
return;
}

this->state = State::EnemyAttack;
this->QueueEnemyLayoutTransition(State::EnemyAttack);
for (auto& enemy : this->enemies) {
	enemy->SetState(true);
}

this->battlePlayer->SetLocalPosition(0, 0);
this->StartAttackCountdown(attackingEnemies);

this->isTransitioning = false;
markFinished();
})
	};
	commandBuffer.PushCommand(std::make_shared<DX9GF::MultiCommand>(std::move(commands)));
	return;
}

void Demo::IBattleScene::PlayerOpenItemsUpdate(unsigned long long deltaTime)
{
	float targetWidth = itemMenuBackground->GetTargetWidth();
	float targetHeight = itemMenuBackground->GetTargetHeight();

	float closeX = -closeItemMenuButton->GetWidth() / 2.0f;
	float closeY = targetHeight / 2.0f + 15.0f;
	closeItemMenuButton->SetLocalPosition(closeX, closeY);

	if (!isTransitioning) {
		closeItemMenuButton->Update(deltaTime);

		if (currentItemPage > 0) btnPrevPage->Update(deltaTime);
		if (currentItemPage < maxItemPage) btnNextPage->Update(deltaTime);

		for (auto& btn : buffItems) {
			btn->Update(deltaTime);
		}
	}
	for (auto& enemy : enemies) {
		enemy->Update(deltaTime);
	}
}

void Demo::IBattleScene::PlayerViewPileUpdate(unsigned long long deltaTime)
{
	const float targetHeight = itemMenuBackground->GetTargetHeight();

	closePileViewButton->SetLocalPosition(-closePileViewButton->GetWidth() / 2.0f, targetHeight / 2.0f + 15.0f);

	if (!isTransitioning) {
		closePileViewButton->Update(deltaTime);
		if (currentPilePage > 0) btnPilePrevPage->Update(deltaTime);
		if (currentPilePage < maxPilePage) btnPileNextPage->Update(deltaTime);
	}
	for (auto& enemy : enemies) {
		enemy->Update(deltaTime);
	}
}

bool Demo::IBattleScene::EnemyAttackUpdate(unsigned long long deltaTime)
{
	if (isDefeatSequence) {
		defeatElapsedMs += static_cast<float>(deltaTime);
		if (defeatElapsedMs >= 1000.f) {
			const float fadeDurationMs = 1000.f;
			defeatFadeAlpha = (std::min)(1.0f, (defeatElapsedMs - 1000.f) / fadeDurationMs);
		}
		if (defeatElapsedMs >= 2500.f) {
			auto sceMan = game->GetSceneManager();
			while (sceMan->GetSceneCount() > 1) {
				sceMan->PopScene();
			}
			DX9GF::AudioManager::GetInstance()->PlayBGM_Fade("bgm_sky", 0.9f, 1.0f);
			sceMan->GoToScene(0); // Go to main menu
			return true;
		}
		return false;
	}
	if (!isAttackCountdownActive) {
		battlePlayer->Update(deltaTime);
	}
	if (battlePlayer->IsDead()) {
		isDefeatSequence = true;
		defeatElapsedMs = 0.f;
		defeatFadeAlpha = 0.f;
		popUpMessage->QueueMessage(&commandBuffer, L"You were defeated", 1.5f);
		return false;
	}
	if (enemyAttackStartPending) {
		if (commandBuffer.IsBusy()) {
			return false;
		}
		for (auto& enemy : enemies) {
			enemy->SetState(true);
		}
		for (auto& enemy : enemies) {
			bool isStunned = enemy->HasModifier(ModifierType::Stun);

			if (enemy->IsDead()) {
				continue;
			}
			if (isStunned) {
				continue;
			}
			enemy->StartAttack(this->battlePlayer, &this->enemies, this->popUpMessage, game->GetGraphicsDevice(), &this->camera, this->currentTurn);
		}
		enemyAttackStartPending = false;
	}
	bool isDoneAttacking = true;
	for (auto& enemy : enemies) {
		enemy->Update(deltaTime);
		isDoneAttacking &= enemy->IsDoneAttacking();
	}
	if (isAttackCountdownActive) {
		isDoneAttacking = false;
	}
	if (isDoneAttacking && !isTransitioning) {
		CollectDeadEnemies();
		if (enemies.empty()) {
			OnAllEnemiesDefeated();
			return false;
		}

		isTransitioning = true;

		// poison hits at the end of enemy's turn
		battlePlayer->TriggerEffects(TickPhase::EndOfTurn);

		std::vector<std::shared_ptr<DX9GF::ICommand>> commands = {
			std::make_shared<DX9GF::DelayCommand>(0.8f),
			std::make_shared<DX9GF::CustomCommand>([this](std::function<void(void)> markFinished) {
				if (!battlePlayer->IsDead()) {
					BeginNextTurn();
					state = State::PlayerStandBy;
					QueueEnemyLayoutTransition(State::PlayerStandBy);
					lastEnemyLayoutState = State::PlayerStandBy;
					enemyLayoutInitialized = true;
				}
				isTransitioning = false;
				markFinished();
			})
		};
		commandBuffer.PushCommand(std::make_shared<DX9GF::MultiCommand>(std::move(commands)));
	}
	return false;
}

void Demo::IBattleScene::PlayerStandByDraw(unsigned long long deltaTime)
{
	fleeButton->Draw(game->GetGraphicsDevice(), deltaTime);
	itemsButton->Draw(game->GetGraphicsDevice(), deltaTime);
	attackButton->Draw(game->GetGraphicsDevice(), deltaTime);

	auto gd = game->GetGraphicsDevice();
	const float y = game->GetVirtualHeight() / 2.f - 128;
	DrawHealthAndDefenseBar(y, gd);
}

void Demo::IBattleScene::PlayerAttackDraw(unsigned long long deltaTime)
{
	game->GetGraphicsDevice()->DrawRectangle(
		this->uiCamera,
		enemyCardRemoveAreaX,
		enemyCardRemoveAreaY,
		enemyCardRemoveAreaWidth,
		enemyCardRemoveAreaHeight,
		D3DXCOLOR(0.7f, 0.1f, 0.1f, 0.5f),
		true
	);
	fontSprite->Begin();
	fontSprite->SetColor(0xFFFFFFFF);
	fontSprite->SetPosition(enemyCardRemoveAreaX + 8.f, enemyCardRemoveAreaY + 8.f);
	fontSprite->SetText(L"Discard Enemy Card Here");
	fontSprite->Draw(this->uiCamera, deltaTime);

	fontSprite->SetOutline(true, 0xFF000000, 2.f);
	fontSprite->SetPosition(backButton->GetWorldX() + 32.f, backButton->GetWorldY() - 30.f);
	// Denominator is this turn's actual maximum, not MAX_ENERGY - bonus and instant energy both
	// push it above the constant, which would otherwise read as "4/3".
	fontSprite->SetText(std::to_wstring(energy - usedEnergy) + L"/" + std::to_wstring(energy));
	fontSprite->Draw(this->uiCamera, deltaTime);

	fontSprite->SetPosition(executeButton->GetWorldX() + 32.f, executeButton->GetWorldY() - 30.f);
	fontSprite->SetText(std::to_wstring(3 - (currentTurn - 1) % 3) + L" turns left before program clears");
	fontSprite->Draw(this->uiCamera, deltaTime);
	fontSprite->End();

	energyIcon->SetPosition(backButton->GetWorldX(), backButton->GetWorldY() - 36.f);
	energyIcon->Begin();
	energyIcon->Draw(this->uiCamera, deltaTime);
	energyIcon->End();

	hourglassIcon->Begin();
	hourglassIcon->SetPosition(executeButton->GetWorldX(), executeButton->GetWorldY() - 36.f);
	hourglassIcon->Draw(this->uiCamera, deltaTime);
	hourglassIcon->End();

	if (!mainBlockCard->IsExecuting() && !(initBlockCard && initBlockCard->IsExecuting()) && !isTransitioning) {
		if (usedEnergy == 0 && queuedToDraw.empty()) backButton->Draw(game->GetGraphicsDevice(), deltaTime);
		executeButton->Draw(game->GetGraphicsDevice(), deltaTime);
		runInitButton->Draw(game->GetGraphicsDevice(), deltaTime);
		DrawPileButtons(deltaTime);
	}

	DrawKeyboardPlacementUI(deltaTime);
}

void Demo::IBattleScene::PlayerOpenItemsDraw(unsigned long long deltaTime)
{
	itemMenuBackground->Begin();
	itemMenuBackground->Draw(this->uiCamera, deltaTime);
	itemMenuBackground->End();

	closeItemMenuButton->Draw(game->GetGraphicsDevice(), deltaTime);

	auto& inventory = player->GetInventoryItems().GetSlots();
	std::wstring hoverDescription = L"";

	std::vector<int> validIndices;

	for (int i = 0; i < inventory.size(); i++) {
		if (inventory[i].quantity > 0 && Demo::ItemData::GetInstance()->GetItemBlueprint(inventory[i].itemID)) {
			validIndices.push_back(i);
		}
	}

	float targetWidth = itemMenuBackground->GetTargetWidth();
	float targetHeight = itemMenuBackground->GetTargetHeight();

	int columns = std::floor((targetWidth - PADDING_X * 2.0f) / (ITEM_W + PADDING_X));
	int rows = std::floor((targetHeight - PADDING_Y * 2.0f - 90.0f) / (ITEM_H + PADDING_Y));
	if (columns < 1) columns = 1;
	if (rows < 1) rows = 1;

	int itemsPerPage = columns * rows;
	int startIndex = currentItemPage * itemsPerPage;
	int endIndex = (std::min)(static_cast<int>(validIndices.size()), startIndex + itemsPerPage);

	int displayIndex = 0;
	for (int i = startIndex; i < endIndex; i++) {
		auto btn = buffItems[displayIndex];
		btn->Draw(game->GetGraphicsDevice(), deltaTime);
		displayIndex++;
	}

	if (currentItemPage > 0) {
		btnPrevPage->Draw(game->GetGraphicsDevice(), deltaTime);
	}
	if (currentItemPage < maxItemPage) {
		btnNextPage->Draw(game->GetGraphicsDevice(), deltaTime);
	}

	fontSprite->Begin();

	displayIndex = 0;
	for (int i = startIndex; i < endIndex; i++) {
		int originalIndex = validIndices[i];
		auto slot = inventory[originalIndex];
		auto btn = buffItems[displayIndex];

		fontSprite->SetColor(0xFFFFFFFF);
		fontSprite->SetOutline(true, 0xFF000000, 3.f);

		fontSprite->SetText(L"x" + std::to_wstring(slot.quantity));

		float textW = fontSprite->GetWidth();
		float textX = btn->GetWorldX() + (ITEM_W / 2.0f) - (textW / 2.0f);
		float textY = btn->GetWorldY() + ITEM_H + 5.0f;

		fontSprite->SetPosition(textX, textY);
		fontSprite->Draw(this->uiCamera, deltaTime);

		if (btn->GetTrigger()->IsHovering(deltaTime)) {
			auto blueprint = Demo::ItemData::GetInstance()->GetItemBlueprint(slot.itemID);
			if (blueprint) {
				hoverDescription = blueprint->GetDescription();
			}
		}
		displayIndex++;
	}

	if (maxItemPage > 0) {
		fontSprite->SetColor(0xFFFFFFFF);
		fontSprite->SetOutline(true, 0xFF000000, 3.f);
		std::wstring pageText = L"Page " + std::to_wstring(currentItemPage + 1) + L"/" + std::to_wstring(maxItemPage + 1);
		fontSprite->SetPosition(-35.0f, targetHeight / 2.0f - PADDING_Y - 9.0f);
		fontSprite->SetText(std::move(pageText));
		fontSprite->Draw(this->uiCamera, deltaTime);
	}

	if (!hoverDescription.empty()) {
		float descX = -targetWidth / 2.0f + PADDING_X;
		float descY = -targetHeight / 2.0f + 15.0f;

		fontSprite->SetColor(0xFFFFFFFF);
		fontSprite->SetOutline(true, 0xFF000000, 3.f);
		fontSprite->SetPosition(descX, descY);
		fontSprite->SetText(std::move(hoverDescription));
		fontSprite->Draw(this->uiCamera, deltaTime);
	}

	fontSprite->End();
	fontSprite->SetOutline(false);
}

void Demo::IBattleScene::DrawPileButtons(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	const std::pair<std::shared_ptr<IconButton>, PileKind> buttons[] = {
		{ drawPileButton,      PileKind::Draw },
		{ discardPileButton,   PileKind::Discard },
		{ nullifiedPileButton, PileKind::Nullified },
	};

	for (auto& [button, kind] : buttons) {
		button->Draw(gd, deltaTime);
	}

	fontSprite->Begin();
	fontSprite->SetColor(0xFFFFFFFF);
	fontSprite->SetOutline(true, 0xFF000000, 3.f);
	for (auto& [button, kind] : buttons) {
		fontSprite->SetText(std::to_wstring(GetPile(kind).size()));
		// Bottom-right corner of the icon, nudged outward so the digits clear the card art.
		const float textX = button->GetWorldX() + PILE_BUTTON_SIZE - fontSprite->GetWidth() + 2.f;
		const float textY = button->GetWorldY() + PILE_BUTTON_SIZE - fontSprite->GetHeight() + 2.f;
		fontSprite->SetPosition(textX, textY);
		fontSprite->Draw(this->uiCamera, deltaTime);
	}
	fontSprite->End();
	fontSprite->SetOutline(false);
}

void Demo::IBattleScene::PlayerViewPileDraw(unsigned long long deltaTime)
{
	itemMenuBackground->Begin();
	itemMenuBackground->Draw(this->uiCamera, deltaTime);
	itemMenuBackground->End();

	closePileViewButton->Draw(game->GetGraphicsDevice(), deltaTime);

	for (auto& card : pileViewCards) {
		if (auto draggable = std::dynamic_pointer_cast<IDraggable>(card)) {
			draggable->Draw(deltaTime);
		}
	}
	// These cards are drawn directly rather than through DraggableManager::Draw, so their
	// queued overlays (hover highlight, description tooltip) have to be flushed here. Leaving
	// them queued would defer them to the next battle frame, after these cards are gone.
	draggableManager->FlushQueuedDraws(deltaTime);

	if (currentPilePage > 0) {
		btnPilePrevPage->Draw(game->GetGraphicsDevice(), deltaTime);
	}
	if (currentPilePage < maxPilePage) {
		btnPileNextPage->Draw(game->GetGraphicsDevice(), deltaTime);
	}

	const float targetWidth = itemMenuBackground->GetTargetWidth();
	const float targetHeight = itemMenuBackground->GetTargetHeight();
	const size_t pileSize = GetPile(viewedPile).size();

	fontSprite->Begin();
	fontSprite->SetColor(0xFFFFFFFF);
	fontSprite->SetOutline(true, 0xFF000000, 3.f);

	std::wstring title = GetPileName(viewedPile) + L" (" + std::to_wstring(pileSize) + L")";
	fontSprite->SetText(std::move(title));
	fontSprite->SetPosition(-fontSprite->GetWidth() / 2.f, -targetHeight / 2.0f + 15.0f);
	fontSprite->Draw(this->uiCamera, deltaTime);

	if (pileSize == 0) {
		fontSprite->SetText(L"Empty");
		fontSprite->SetPosition(-fontSprite->GetWidth() / 2.f, -10.f);
		fontSprite->Draw(this->uiCamera, deltaTime);
	}

	if (maxPilePage > 0) {
		std::wstring pageText = L"Page " + std::to_wstring(currentPilePage + 1) + L"/" + std::to_wstring(maxPilePage + 1);
		fontSprite->SetText(std::move(pageText));
		fontSprite->SetPosition(-fontSprite->GetWidth() / 2.f, targetHeight / 2.0f - PADDING_Y - 9.0f);
		fontSprite->Draw(this->uiCamera, deltaTime);
	}

	fontSprite->End();
	fontSprite->SetOutline(false);
}

void Demo::IBattleScene::EnemyAttackDraw(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();
	const float battleHalfSize = battleBoxSize * 0.5f;
	gd->SetAlphaBlending(true);
	const float y = game->GetVirtualHeight() / 2.f - 40.f;
	DrawHealthAndDefenseBar(y, gd);
}

void Demo::IBattleScene::DrawHealthAndDefenseBar(const float y, DX9GF::GraphicsDevice* gd)
{
	const float spacing = 5.f;
	const float w = battlePlayer->GetMaxHealth() * spacing;
	const float defenseW = (std::max)(0.f, battlePlayer->GetTemporaryDefense()) * spacing;
	const float w_ = battlePlayer->GetHealth() * spacing;
	const float x = -w / 2;
	const float defenseY = y - 24.f;

	gd->DrawRectangle(this->uiCamera, x, defenseY, w, 16, 0xFF808080, true);
	gd->DrawRectangle(this->uiCamera, x, defenseY, defenseW, 16, 0xFFD0D0D0, true);
	gd->DrawRectangle(this->uiCamera, x, defenseY, w, 16, 0xFF000000, false);

	gd->DrawRectangle(this->uiCamera, x, y, w, 20, 0xFFb4202a, true);
	gd->DrawRectangle(this->uiCamera, x, y, w, 10, 0xFFdf3e23, true);
	gd->DrawRectangle(this->uiCamera, x, y, w_, 20, 0xFF59c135, true);
	gd->DrawRectangle(this->uiCamera, x, y, w_, 10, 0xFF9cdb43, true);
	gd->DrawRectangle(this->uiCamera, x, y, w, 20, 0xFF000000, false);

	auto modifiers = battlePlayer->GetModifiers();
	auto modY = y - 48.f;

	bool isTooltipActive = false;
	std::wstring activeTooltipText = L"";

	auto [screenX, screenY] = DX9GF::InputManager::GetInstance()->GetVirtualAbsoluteMousePos(&this->uiCamera);
	auto [mouseX, mouseY] = DX9GF::Utils::WindowToWorldCoords(this->uiCamera, screenX, screenY);

	for (const auto& mod : modifiers) {
		if (mod.duration <= 0) continue;

		std::wstring valueText = L"";
		std::wstring nameText = L"";
		std::wstring statusName = L"Unknown";
		std::wstring statusDescription = L"";
		D3DCOLOR textColor = 0xFFFFFFFF;
		RECT iconRect = { 0, 0, 0, 0 };

		if (mod.type == ModifierType::BuffDamage) {
			valueText = std::to_wstring(static_cast<int>(mod.value));
			textColor = 0xFFfa6a0a;
			iconRect = { 112, 240, 128, 256 };
			statusName = L"Atk Up";
			statusDescription = L"Increases attack damage.";
		}
		else if (mod.type == ModifierType::BuffDefense) {
			valueText = std::to_wstring(static_cast<int>(mod.value));
			textColor = 0xFF588dbe;
			iconRect = { 96, 240, 112, 256 };
			statusName = L"Def Up";
			statusDescription = L"Blocks incoming damage.";
		}
		else if (mod.type == ModifierType::Poison) {
			int poisonDmg = static_cast<int>(std::round((mod.value > 0.f) ? mod.value : static_cast<float>(mod.duration)));
			valueText = std::to_wstring(poisonDmg);
			textColor = 0xFFba4aed;
			iconRect = { 128, 240, 144, 256 };
			statusName = L"Poison";
			statusDescription = L"Takes " + std::to_wstring(poisonDmg) + L" damage at end of turn.";
		}
		else if (mod.type == ModifierType::Burn) {
			int burnDmg = static_cast<int>(std::round(mod.value));
			valueText = std::to_wstring(burnDmg);
			textColor = 0xFFff8800;
			iconRect = { 240, 288, 256, 304 };
			statusName = L"Burn";
			statusDescription = L"Takes " + std::to_wstring(burnDmg) + L" damage at end of turn, ignoring block.";
		}
		else if (mod.type == ModifierType::Regen) {
			int regenAmount = static_cast<int>(std::round(mod.value));
			valueText = std::to_wstring(regenAmount);
			textColor = 0xFF9cdb43;
			iconRect = { 272, 288, 288, 304 };
			statusName = L"Regen";
			statusDescription = L"Heals " + std::to_wstring(regenAmount) + L" at end of turn.";
		}
		else if (mod.type == ModifierType::Marked) {
			int markAmount = static_cast<int>(std::round(mod.value));
			valueText = std::to_wstring(markAmount);
			textColor = 0xFFfffc40;
			iconRect = { 128, 256, 144, 272 };
			statusName = L"Marked";
			statusDescription = L"Takes " + std::to_wstring(markAmount) + L" extra damage from every hit.";
		}
		else if (mod.type == ModifierType::Vulnerable) {
			iconRect = { 96, 256, 112, 272 };
			statusName = L"Vulnerable";
			statusDescription = L"Takes 50% more damage from attacks.";
		}
		else if (mod.type == ModifierType::Weak) {
			iconRect = { 112, 256, 128, 272 };
			statusName = L"Weak";
			statusDescription = L"Deals 25% less damage with attacks.";
		}
		else if (mod.type == ModifierType::Stun) {
			textColor = 0xFFfffc40;
			iconRect = { 224, 288, 240, 304 };
			statusName = L"Stun";
			statusDescription = L"Cannot take action this turn.";
		}
		else {
			continue;
		}

		float currentDrawX = x;
		float hitBoxX = x;
		float hitBoxY = modY - 8.f;
		float hitBoxW = 32.f;
		float hitBoxH = 32.f;

		//icons/texts
		if (iconRect.right > 0) {
			attackBuffIcon->SetSrcRect(iconRect);
			attackBuffIcon->SetPosition(currentDrawX, hitBoxY);
			attackBuffIcon->SetScale(2.f, 2.f);
			attackBuffIcon->Begin();
			attackBuffIcon->Draw(this->uiCamera, 0);
			attackBuffIcon->End();
			currentDrawX += 36.f;
		}
		else if (!nameText.empty()) {
			fontSprite->SetColor(textColor);
			fontSprite->SetOutline(true, 0xFF000000, 3.f);
			fontSprite->SetPosition(currentDrawX, modY);
			fontSprite->SetText(std::move(nameText));
			fontSprite->Begin();
			fontSprite->Draw(this->uiCamera, 0);
			fontSprite->End();

			hitBoxY = modY;
			hitBoxW = fontSprite->GetWidth();
			hitBoxH = fontSprite->GetHeight();
			currentDrawX += hitBoxW + 8.f;
		}

		//values
		if (!valueText.empty()) {
			fontSprite->SetColor(textColor);
			fontSprite->SetOutline(true, 0xFF000000, 3.f);
			fontSprite->SetPosition(currentDrawX, modY);
			fontSprite->SetText(std::move(valueText));
			fontSprite->Begin();
			fontSprite->Draw(this->uiCamera, 0);
			fontSprite->End();
			currentDrawX += fontSprite->GetWidth() + 8.f;
		}

		//turns left
		fontSprite->SetColor(0xFFFFFFFF);
		fontSprite->SetOutline(true, 0xFF000000, 3.f);
		fontSprite->SetPosition(currentDrawX, modY);
		fontSprite->SetText(std::to_wstring(mod.duration) + L" turns");
		fontSprite->Begin();
		fontSprite->Draw(this->uiCamera, 0);
		fontSprite->End();

		bool isHovered = mouseX >= hitBoxX && mouseX <= hitBoxX + hitBoxW && mouseY >= hitBoxY && mouseY <= hitBoxY + hitBoxH;
		if (isHovered) {
			isTooltipActive = true;
			activeTooltipText = statusName + L" (" + std::to_wstring(mod.duration) + L" turns remaining)\n" + statusDescription;
		}

		modY -= 32.f;
	}

	//draw tooltip after
	if (isTooltipActive) {
		fontSprite->SetText(std::move(activeTooltipText));
		float tooltipWidth = fontSprite->GetWidth() + 8.f;
		float tooltipHeight = fontSprite->GetHeight() + 8.f;

		auto [screenW, screenH] = this->uiCamera.GetScreenResolution();
		float targetScreenX = screenX;
		float targetScreenY = screenY - tooltipHeight;

		if (targetScreenX + tooltipWidth > screenW) targetScreenX = screenW - tooltipWidth;
		if (targetScreenY < 0) targetScreenY = screenY + 32.f;

		auto [worldDrawX, worldDrawY] = DX9GF::Utils::WindowToWorldCoords(this->uiCamera, targetScreenX, targetScreenY);

		gd->SetAlphaBlending(true);
		gd->DrawRectangle(this->uiCamera, worldDrawX, worldDrawY, tooltipWidth, tooltipHeight, 0, 1, 1, 0, 0, D3DCOLOR_ARGB(220, 0, 0, 0), true);
		gd->SetAlphaBlending(false);

		fontSprite->SetColor(0xFFFFFFFF);
		fontSprite->SetOutline(true, 0xFF000000, 1.f);
		fontSprite->SetPosition(worldDrawX + 4.f, worldDrawY + 4.f);
		fontSprite->Begin();
		fontSprite->Draw(this->uiCamera, 0);
		fontSprite->End();
	}
}

void Demo::IBattleScene::Init()
{
	// Init components
	draggableManager = std::make_shared<DraggableManager>();
	transformManager = std::make_shared<DX9GF::TransformManager>();
	font = std::make_shared<DX9GF::Font>(game->GetGraphicsDevice(), L"StatusPlz", 16);
	drawBuffer = std::make_shared<DX9GF::CommandBuffer>();
	// Setup player
	battlePlayer = std::make_shared<Player>(transformManager);
	battlePlayer->Init(game->GetGraphicsDevice(), &colliderManager, &camera, true);
	battlePlayer->SetFollowCamera(false);
	battlePlayer->SetVelocity(125);
	battlePlayer->SetHealth(player->GetHealth());

	// Initialize texture sheet
	uiSheetTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	uiSheetTex->LoadTexture(L"assets/ui.png");

	tempTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	tempTex->LoadTexture(L"assets/All-borders.png");

	itemsTex = std::make_shared<DX9GF::Texture>(game->GetGraphicsDevice());
	itemsTex->LoadTexture(L"assets/items.png");
	attackBuffIcon = std::make_shared<DX9GF::StaticSprite>(uiSheetTex.get());
	attackBuffIcon->SetSrcRect({ .left = 112, .top = 240, .right = 128, .bottom = 256 });
	defenseBuffIcon = std::make_shared<DX9GF::StaticSprite>(uiSheetTex.get());
	defenseBuffIcon->SetSrcRect({ .left = 96, .top = 240, .right = 112, .bottom = 256 });
	// Create buttons
	const auto buttonWidth = 96;
	const auto buttonHeight = 32;
	attackButton = std::make_shared<IconButton>(transformManager, 0, 0, buttonWidth, buttonHeight, uiSheetTex);
	attackButton->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([&](std::function<void(void)> markFinished) {
			this->state = State::PlayerAttack;
			for (auto& enemy : enemies) {
				enemy->SetState(false);
			}
			QueueEnemyLayoutTransition(State::PlayerAttack);
			lastEnemyLayoutState = State::PlayerAttack;
			//lock card message
			if (this->pendingLockMessage) {
				this->popUpMessage->QueueMessage(&this->commandBuffer, L"Enemy locked a card!", 2.0f);
				this->pendingLockMessage = false;
			}
			//popUpMessage->QueueMessage(&commandBuffer, L"Click on the enemy sprite to create an Enemy Card and drag it to your attacking card to target it!", 2.5f);
			markFinished();
			}));
		isExecutingAttacks = false;
		});
	attackButton->SetSpriteRects({
		{
			.left = 0,
			.top = 48,
			.right = 48,
			.bottom = 64
		},
		{
			.left = 0,
			.top = 64,
			.right = 48,
			.bottom = 80
		},
		{
			.left = 0,
			.top = 80,
			.right = 48,
			.bottom = 96
		}
		});
	attackButton->SetSpriteScale(2.f, 2.f);

	itemsButton = std::make_shared<IconButton>(transformManager, 0, 0, buttonWidth, buttonHeight, uiSheetTex);
	itemsButton->SetSpriteRects({
		{
			.left = 0,
			.top = 96,
			.right = 48,
			.bottom = 112
		},
		{
			.left = 0,
			.top = 112,
			.right = 48,
			.bottom = 128
		},
		{
			.left = 0,
			.top = 128,
			.right = 48,
			.bottom = 144
		}
		});
	itemsButton->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([&](std::function<void(void)> markFinished) {
			this->RefreshItemMenu();
			this->state = State::PlayerOpenItems;
			//turn off volume or play sfx volume when open a menu here
			markFinished();
			}));
		});
	itemsButton->SetSpriteScale(2.f, 2.f);
	fleeButton = std::make_shared<IconButton>(transformManager, 0, 0, buttonWidth, buttonHeight, uiSheetTex);
	fleeButton->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		popUpMessage->QueueMessage(&commandBuffer, L"You tried to flee...");
		isFleeing = true;
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void(void)> markFinished) {
			if (RNG::Range(0, 1) == 0) {
				popUpMessage->QueueMessage(&commandBuffer, L"You successfully fled!");
				auto transitionInCommand = std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, true);
				drawBuffer->PushCommand(std::make_shared<DX9GF::DelayCommand>(1.5f));
				drawBuffer->PushCommand(transitionInCommand);
				commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, transitionInCommand](std::function<void(void)> markFinished2) {
					if (!transitionInCommand->IsFinished()) {
						return;
					}
					player->SetHealth(battlePlayer->GetHealth());
					auto sceMan = game->GetSceneManager();

					auto prevScene = sceMan->GetScene(sceMan->GetIndex() - 1);
					auto audio = DX9GF::AudioManager::GetInstance();

					if (dynamic_cast<Demo::TutorialWorldScene*>(prevScene)) audio->PlayBGM_Fade("bgm_tutorial", 0.5f, 1.0f);
					else if (dynamic_cast<Demo::SecretPuzzleScene*>(prevScene)) audio->PlayBGM_Fade("bgm_secret", 0.3f, 1.0f);
					else if (dynamic_cast<Demo::ThreadAlleyScene*>(prevScene)) audio->PlayBGM_Fade("bgm_arcade", 0.2f, 1.0f);
					else if (dynamic_cast<Demo::BossWorldScene*>(prevScene)) audio->PlayBGM_Fade("bgm_boss", 0.3f, 1.0f);

					sceMan->RemoveScene(sceMan->GetIndex());
					sceMan->GoToPrevious();
					markFinished2();
					}));
			}
			else {
				popUpMessage->QueueMessage(&commandBuffer, L"You failed to flee!");
				commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void(void)> markFinished) {
					this->QueueToEnemyAttack(0);
					commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this](std::function<void(void)> markFinished2) {
						isFleeing = false;
						markFinished2();
						}));
					markFinished();
					}));
			}
			markFinished();
			}));
		});
	fleeButton->SetSpriteRects({
		{
			.left = 48,
			.top = 96,
			.right = 96,
			.bottom = 112
		},
		{
			.left = 48,
			.top = 112,
			.right = 96,
			.bottom = 128
		},
		{
			.left = 48,
			.top = 128,
			.right = 96,
			.bottom = 144
		}
		});
	fleeButton->SetSpriteScale(2.f, 2.f);
	backButton = std::make_shared<IconButton>(transformManager, 0, 0, buttonWidth, buttonHeight, uiSheetTex);
	backButton->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		commandBuffer.Clear();
		popUpMessage->Reset(); // Really bad hack, please never do this in a production game lol
		commandBuffer.StackCommand(std::make_shared<DX9GF::CustomCommand>([&](std::function<void(void)> markFinished) {
			this->state = State::PlayerStandBy;
			for (auto& enemy : enemies) {
				enemy->SetState(true);
			}
			markFinished();
			}));
		});
	backButton->SetSpriteRects({
		{
			.left = 96,
			.top = 48,
			.right = 144,
			.bottom = 64
		},
		{
			.left = 96,
			.top = 64,
			.right = 144,
			.bottom = 80
		},
		{
			.left = 96,
			.top = 80,
			.right = 144,
			.bottom = 96
		}
		});
	backButton->SetSpriteScale(2.f, 2.f);
	executeButton = std::make_shared<IconButton>(transformManager, 0, 0, buttonWidth, buttonHeight, uiSheetTex);
	executeButton->SetSpriteRects({
		{
			.left = 96,
			.top = 96,
			.right = 144,
			.bottom = 112
		},
		{
			.left = 96,
			.top = 112,
			.right = 144,
			.bottom = 128
		},
		{
			.left = 96,
			.top = 128,
			.right = 144,
			.bottom = 144
		}
		});
	executeButton->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		// Executing clears the command buffer, which is also what carries the deal animation of
		// a mid-turn draw. Clearing it mid-flight would strand those cards in queuedToDraw -
		// never reaching the hand, never discarded - so refuse until they have landed.
		if (!queuedToDraw.empty()) {
			popUpMessage->QueueMessage(&commandBuffer, L"Still drawing");
			DX9GF::AudioManager::GetInstance()->Play("error", false, 0.8f);
		}
		else if (usedEnergy > energy) {
			popUpMessage->QueueMessage(&commandBuffer, L"Not enough energy");
			DX9GF::AudioManager::GetInstance()->Play("error", false, 0.8f);
		}
		else if (mainBlockCard && !mainBlockCard->IsExecuting()) {
			if (!mainBlockCard->HasAllRequiredTargets()) {
				if (timeSinceLastTargetPopUp >= targetPopUpCooldown) {
					popUpMessage->QueueMessage(&commandBuffer, L"Some cards are missing a target!");
					DX9GF::AudioManager::GetInstance()->Play("error", false, 0.8f);
					timeSinceLastTargetPopUp = 0.f;
				}
				return;
			}
			commandBuffer.Clear();
			popUpMessage->Reset(); // Really bad hack, please never do this in a production game lol
			isExecutingAttacks = true;
			mainBlockCard->StartExecution();
		}
		});
	executeButton->SetSpriteScale(2.f, 2.f);

	runInitButton = std::make_shared<IconButton>(transformManager, 0, 0, buttonWidth, buttonHeight, uiSheetTex);
	runInitButton->SetSpriteRects({
		{ .left = 48, .top = 48, .right = 96, .bottom = 64 },
		{ .left = 48, .top = 64, .right = 96, .bottom = 80 },
		{ .left = 48, .top = 80, .right = 96, .bottom = 96 }
		});
	runInitButton->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		if (!initBlockCard || initBlockCard->GetStatementCards().empty()) {
			return;
		}
		// Running init does not clear the command buffer the way executing does, so a draw it
		// kicks off survives - but a draw already in flight still has to land first, because the
		// next Execute would clear the buffer out from under it.
		if (!queuedToDraw.empty()) {
			popUpMessage->QueueMessage(&commandBuffer, L"Still drawing");
			DX9GF::AudioManager::GetInstance()->Play("error", false, 0.8f);
			return;
		}
		if (usedEnergy > energy) {
			popUpMessage->QueueMessage(&commandBuffer, L"Not enough energy");
			DX9GF::AudioManager::GetInstance()->Play("error", false, 0.8f);
			return;
		}
		if (initBlockCard->IsExecuting() || (mainBlockCard && mainBlockCard->IsExecuting())) {
			return;
		}
		if (!initBlockCard->HasAllRequiredTargets()) {
			if (timeSinceLastTargetPopUp >= targetPopUpCooldown) {
				popUpMessage->QueueMessage(&commandBuffer, L"Some cards are missing a target!");
				DX9GF::AudioManager::GetInstance()->Play("error", false, 0.8f);
				timeSinceLastTargetPopUp = 0.f;
			}
			return;
		}
		isExecutingInit = true;
		initBlockCard->StartExecution();
		});
	runInitButton->SetSpriteScale(2.f, 2.f);

	// Pile buttons. Each group of three frames on the sheet is idle / hover / pressed.
	drawPileButton = std::make_shared<IconButton>(transformManager, 0, 0, PILE_BUTTON_SIZE, PILE_BUTTON_SIZE, uiSheetTex);
	drawPileButton->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(192, 160, 16, 16, 3));
	drawPileButton->SetSpriteScale(3.f, 3.f);
	drawPileButton->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		this->OpenPileView(PileKind::Draw);
		});

	discardPileButton = std::make_shared<IconButton>(transformManager, 0, 0, PILE_BUTTON_SIZE, PILE_BUTTON_SIZE, uiSheetTex);
	discardPileButton->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(144, 160, 16, 16, 3));
	discardPileButton->SetSpriteScale(3.f, 3.f);
	discardPileButton->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		this->OpenPileView(PileKind::Discard);
		});

	nullifiedPileButton = std::make_shared<IconButton>(transformManager, 0, 0, PILE_BUTTON_SIZE, PILE_BUTTON_SIZE, uiSheetTex);
	nullifiedPileButton->SetSpriteRects(DX9GF::Utils::CreateRectsHorizontal(240, 160, 16, 16, 3));
	nullifiedPileButton->SetSpriteScale(3.f, 3.f);
	nullifiedPileButton->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		this->OpenPileView(PileKind::Nullified);
		});

	closePileViewButton = std::make_shared<IconButton>(transformManager, 0, 0, buttonWidth, buttonHeight, uiSheetTex);
	closePileViewButton->SetSpriteRects({
		{ .left = 96, .top = 48, .right = 144, .bottom = 64 },
		{ .left = 96, .top = 64, .right = 144, .bottom = 80 },
		{ .left = 96, .top = 80, .right = 144, .bottom = 96 }
		});
	closePileViewButton->SetSpriteScale(2.f, 2.f);
	closePileViewButton->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		this->ClosePileView();
		});

	btnPilePrevPage = std::make_shared<Demo::TextIconButton>(transformManager, 0, 0, 30.0f, 30.0f, uiSheetTex, font.get(), L"<", 3);
	btnPilePrevPage->SetSpriteRects({ {0, 0, 16, 16}, {0, 16, 16, 32}, {0, 32, 16, 48} });
	btnPilePrevPage->SetSpriteScale(2.0f, 2.0f);
	btnPilePrevPage->SetTextColor(0xFF111111);
	btnPilePrevPage->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		if (currentPilePage > 0) {
			currentPilePage--;
			RefreshPileView();
		}
		});

	btnPileNextPage = std::make_shared<Demo::TextIconButton>(transformManager, 0, 0, 30.0f, 30.0f, uiSheetTex, font.get(), L">", 3);
	btnPileNextPage->SetSpriteRects({ {0, 0, 16, 16}, {0, 16, 16, 32}, {0, 32, 16, 48} });
	btnPileNextPage->SetSpriteScale(2.0f, 2.0f);
	btnPileNextPage->SetTextColor(0xFF111111);
	btnPileNextPage->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		if (currentPilePage < maxPilePage) {
			currentPilePage++;
			RefreshPileView();
		}
		});

	closeItemMenuButton = std::make_shared<IconButton>(transformManager, 0, 0, buttonWidth, buttonHeight, uiSheetTex);
	closeItemMenuButton->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([&](std::function<void(void)> markFinished) {
			this->state = State::PlayerStandBy;
			markFinished();
			}));
		});
	closeItemMenuButton->SetSpriteRects({
		{
			.left = 96,
			.top = 48,
			.right = 144,
			.bottom = 64
		},
		{
			.left = 96,
			.top = 64,
			.right = 144,
			.bottom = 80
		},
		{
			.left = 96,
			.top = 80,
			.right = 144,
			.bottom = 96
		}
		});
	closeItemMenuButton->SetSpriteScale(2.f, 2.f);

	btnPrevPage = std::make_shared<Demo::TextIconButton>(transformManager, 0, 0, 30.0f, 30.0f, uiSheetTex, font.get(), L"<", 3);
	btnPrevPage->SetSpriteRects({ {0, 0, 16, 16}, {0, 16, 16, 32}, {0, 32, 16, 48} });
	btnPrevPage->SetSpriteScale(2.0f, 2.0f);
	btnPrevPage->SetTextColor(0xFF111111);
	btnPrevPage->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		if (currentItemPage > 0) {
			currentItemPage--;
			RefreshItemMenu();
		}
		});

	btnNextPage = std::make_shared<Demo::TextIconButton>(transformManager, 0, 0, 30.0f, 30.0f, uiSheetTex, font.get(), L">", 3);
	btnNextPage->SetSpriteRects({ {0, 0, 16, 16}, {0, 16, 16, 32}, {0, 32, 16, 48} });
	btnNextPage->SetSpriteScale(2.0f, 2.0f);
	btnNextPage->SetTextColor(0xFF111111);
	btnNextPage->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		if (currentItemPage < maxItemPage) {
			currentItemPage++;
			RefreshItemMenu();
		}
		});

	// Init buttons
	attackButton->Init(&this->uiCamera);
	itemsButton->Init(&this->uiCamera);
	fleeButton->Init(&this->uiCamera);
	backButton->Init(&this->uiCamera);
	executeButton->Init(&this->uiCamera);
	runInitButton->Init(&this->uiCamera);
	closeItemMenuButton->Init(&this->uiCamera);
	btnPrevPage->Init(&this->uiCamera);
	btnNextPage->Init(&this->uiCamera);
	drawPileButton->Init(&this->uiCamera);
	discardPileButton->Init(&this->uiCamera);
	nullifiedPileButton->Init(&this->uiCamera);
	closePileViewButton->Init(&this->uiCamera);
	btnPilePrevPage->Init(&this->uiCamera);
	btnPileNextPage->Init(&this->uiCamera);

	// Init sprite
	fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
	fontSprite->SetColor(0xFFFFFFFF);
	energyIcon = std::make_shared<DX9GF::StaticSprite>(uiSheetTex.get());
	energyIcon->SetSrcRect({
		.left = 96,
		.top = 0,
		.right = 112,
		.bottom = 16
		});
	energyIcon->SetScale(2.f);
	hourglassIcon = std::make_shared<DX9GF::StaticSprite>(uiSheetTex.get());
	hourglassIcon->SetSrcRect({
		.left = 112,
		.top = 0,
		.right = 128,
		.bottom = 16
		});
	hourglassIcon->SetScale(2.f);


	itemMenuBackground = std::make_shared<DX9GF::NineSliceSprite>(
		tempTex.get(),
		RECT{ 0, 0, 45, 45 },
		8, 8, 8, 8
	);

	auto app = DX9GF::Application::GetInstance();
	float screenW = static_cast<float>(app->GetScreenWidth());
	float screenH = static_cast<float>(app->GetScreenHeight());
	float targetWidth = screenW * 0.8f;
	float targetHeight = screenH * 0.65f;

	itemMenuBackground->SetTargetSize(targetWidth, targetHeight);
	itemMenuBackground->SetOrigin(targetWidth / 2.0f, targetHeight / 2.0f);
	itemMenuBackground->SetPosition(0.0f, 0.0f);

	// Fetch Deck
	const auto& deckCards = player->GetDeck();
	for (const auto& cardId : deckCards) {
		auto newCard = ICard::CreateCard(cardId, transformManager, draggableManager, game->GetGraphicsDevice(), &camera);
		if (newCard) {
			drawPile.push_back(newCard);
		}
	}

	auto sh = DX9GF::Application::GetInstance()->GetScreenHeight();
	// Init draggables
	mainBlockCard = std::make_shared<MainBlockCard>(transformManager, -100.f, -140.f);
	mainBlockCard->Init(draggableManager, game->GetGraphicsDevice(), &camera);
	mainBlockCard->SetMaxHeight(sh * 0.5f);
	mainBlockCard->SetBattleScene(this);
	// Stacked directly above the main block, which reads as "init runs first". Not to its left:
	// the hand container occupies the top-left corner and would sit underneath. Both blocks are
	// draggable, so the player can rearrange them if a long init program reaches the main block.
	initBlockCard = std::make_shared<InitBlockCard>(transformManager, -100.f, -300.f);
	initBlockCard->Init(draggableManager, game->GetGraphicsDevice(), &camera);
	initBlockCard->SetMaxHeight(sh * 0.5f);
	initBlockCard->SetBattleScene(this);
	handContainer = std::make_shared<HandContainer>(transformManager, 180, 40, -250.f, -200.f);
	handContainer->Init(draggableManager, game->GetGraphicsDevice(), &camera, &playedPile);
	handContainer->SetMaxHeight(sh * 0.5f);

	const float battleHalfSize = battleBoxSize * 0.5f;
	const float battleBorder = 8.f;
	const float battleLeft = -battleHalfSize - battleBorder;
	const float battleTop = -battleHalfSize - battleBorder;
	const float battleRight = battleHalfSize;
	const float battleBottom = battleHalfSize;
	const float battleWidth = battleBoxSize + battleBorder * 2.f;
	const float battleHeight = battleBoxSize + battleBorder * 2.f;

	std::array<std::shared_ptr<DX9GF::RectangleCollider>, 4> bounds = {
		std::make_shared<DX9GF::RectangleCollider>(transformManager, battleWidth, battleBorder, battleLeft, battleTop),
		std::make_shared<DX9GF::RectangleCollider>(transformManager, battleWidth, battleBorder, battleLeft, battleBottom),
		std::make_shared<DX9GF::RectangleCollider>(transformManager, battleBorder, battleHeight, battleLeft, battleTop),
		std::make_shared<DX9GF::RectangleCollider>(transformManager, battleBorder, battleHeight, battleRight, battleTop)
	};

	for (auto& bound : bounds) {
		bound->SetOrigin(0.f, 0.f);
		colliderManager.Add(bound);
		battleBounds.push_back(bound);
	}

	// Init pop up message
	popUpMessage = std::make_shared<PopUpMessage>(transformManager);
	popUpMessage->Init(game->GetGraphicsDevice(), &camera);

	//Init BGM
	auto audio = DX9GF::AudioManager::GetInstance();
	audio->Load("battle_loop1", IDR_BGM_B1);
	audio->Load("battle_loop2", IDR_BGM_B2);
	audio->Load("battle_loop3", IDR_BGM_B3);
	audio->Load("battle_loop4", IDR_BGM_B4);
	audio->Load("battle_boss", IDR_BGM_BAT_BOSS);

	audio->RegisterBank("battle_bgm", { "battle_loop1","battle_loop2","battle_loop3","battle_loop4" });

	transformManager->RebuildHierarchy();
	DamageTextManager::GetInstance()->Init(this->game);
	ItemData::GetInstance()->LoadData();

	// Note: enemies are populated by subclasses after IBattleScene::Init() returns,
	// so their onRequestLockCard is wired centrally in StartBattle() instead.
	drawBuffer->PushCommand(std::make_shared<TransitionCommand>(game->GetGraphicsDevice(), &this->uiCamera, 1.f, false));
}

void Demo::IBattleScene::Update(unsigned long long deltaTime)
{
	auto inpMan = DX9GF::InputManager::GetInstance();
	inpMan->ReadMouse(deltaTime);
	inpMan->ReadKeyboard(deltaTime);
	this->camera.Update();
	this->uiCamera.Update();

	if (isDefeatSequence) {
		if (!EnemyAttackUpdate(deltaTime)) {
			DamageTextManager::GetInstance()->Update(deltaTime);
			transformManager->UpdateAll();
			commandBuffer.Update(deltaTime);
			return;
		}
		return;
	}
	if (isAttackCountdownActive) {
		bool countdownFinished = UpdateAttackCountdown(deltaTime);
		if (countdownFinished) {
			if (countdownAttackingEnemies) {
				for (auto& enemy : *countdownAttackingEnemies) {
					if (enemy->IsDead()) {
						continue;
					}
					enemy->StartAttack(this->battlePlayer, &this->enemies, this->popUpMessage, game->GetGraphicsDevice(), &this->camera, this->currentTurn);
				}
				countdownAttackingEnemies.reset();
			}
			CollectDeadEnemies();
			if (enemies.empty()) {
				OnAllEnemiesDefeated();
			}
			else {
				battlePlayer->SetLocalPosition(0, 0);
			}
		}
	}
	if (!enemyLayoutInitialized || lastEnemyLayoutState != state) {
		if (state == State::PlayerStandBy || state == State::PlayerAttack) {
			QueueEnemyLayoutTransition(state);
			lastEnemyLayoutState = state;
			enemyLayoutInitialized = true;
		}
	}
	switch (state) {
	case State::PlayerStandBy:
		PlayerStandByUpdate(deltaTime);
		break;
	case State::PlayerAttack:
		PlayerAttackUpdate(deltaTime);
		break;
	case State::PlayerOpenItems:
		PlayerOpenItemsUpdate(deltaTime);
		break;
	case State::PlayerViewPile:
		PlayerViewPileUpdate(deltaTime);
		break;
	case State::EnemyAttack:
		EnemyAttackUpdate(deltaTime);
		break;
	default:
		throw std::runtime_error("Unexpected state");
	}

	UpdateKeyboardNavigation(deltaTime);

	const float targetDim = (state == State::EnemyAttack) ? BACKGROUND_DIM_ALPHA : 0.f;
	const float dimStep = BACKGROUND_DIM_SPEED * (deltaTime / 1000.f);
	if (backgroundDimAlpha < targetDim) {
		backgroundDimAlpha = (std::min)(targetDim, backgroundDimAlpha + dimStep);
	}
	else if (backgroundDimAlpha > targetDim) {
		backgroundDimAlpha = (std::max)(targetDim, backgroundDimAlpha - dimStep);
	}

	DamageTextManager::GetInstance()->Update(deltaTime);
	transformManager->UpdateAll();
	commandBuffer.Update(deltaTime);
}

void Demo::IBattleScene::DrawKeyboardReticleUI(unsigned long long deltaTime)
{
	if (!keyboardNavigator.IsInKeyboardMode() || !keyboardNavigator.GetTarget()) {
		return;
	}
	auto button = std::dynamic_pointer_cast<IButton>(keyboardNavigator.GetTarget());
	if (!button) {
		return;
	}
	auto gd = game->GetGraphicsDevice();
	Demo::DrawKeyboardTargetReticle(gd, uiCamera, button->GetWorldX(), button->GetWorldY(), (float)button->GetWidth(), (float)button->GetHeight(), GetTickCount64());
}

void Demo::IBattleScene::DrawKeyboardReticleWorld(unsigned long long deltaTime)
{
	if (!keyboardNavigator.IsInKeyboardMode()) {
		return;
	}

	auto gd = game->GetGraphicsDevice();

	if (pickedUpCard) {
		auto slots = CollectPlacementSlots();
		if (slots.empty()) {
			return;
		}
		size_t selectedIndex = (placementSlotIndex < slots.size()) ? placementSlotIndex : slots.size() - 1;

		gd->SetAlphaBlending(true);
		for (auto& slot : slots) {
			if (slot.inUIPass) {
				continue; // drawn later by DrawKeyboardPlacementUI, above the UI it belongs to
			}
			Demo::DrawAnimatedDashedRectangle(
				gd, camera, slot.x, slot.y, slot.width, slot.height,
				3.f, 0xFFFFFFFF, false, 4.f, 0xFFFFFFFF, 20.f, 10.f, 40.f, GetTickCount64());
		}
		gd->SetAlphaBlending(false);

		auto& selected = slots[selectedIndex];
		if (!selected.inUIPass) {
			Demo::DrawKeyboardTargetReticle(gd, camera, selected.x, selected.y, selected.width, selected.height, GetTickCount64());
		}
		return;
	}

	if (!keyboardNavigator.GetTarget() || std::dynamic_pointer_cast<IButton>(keyboardNavigator.GetTarget())) {
		return;
	}

	// The navigator looks the target's bounds up from the same candidate list navigation
	// uses - some kinds (e.g. enemies, whose trigger uses a center origin) don't have
	// world position == top-left, so there's a single source of truth for that math.
	keyboardNavigator.Draw(gd, camera, CollectKeyboardCandidates());
}

void Demo::IBattleScene::DrawKeyboardPlacementUI(unsigned long long deltaTime)
{
	if (!keyboardNavigator.IsInKeyboardMode() || !pickedUpCard) {
		return;
	}
	auto slots = CollectPlacementSlots();
	if (slots.empty()) {
		return;
	}
	size_t selectedIndex = (placementSlotIndex < slots.size()) ? placementSlotIndex : slots.size() - 1;

	auto gd = game->GetGraphicsDevice();
	gd->SetAlphaBlending(true);
	for (auto& slot : slots) {
		if (!slot.inUIPass) {
			continue;
		}
		Demo::DrawAnimatedDashedRectangle(
			gd, uiCamera, slot.x, slot.y, slot.width, slot.height,
			3.f, 0xFFFFFFFF, false, 4.f, 0xFFFFFFFF, 20.f, 10.f, 40.f, GetTickCount64());
	}
	gd->SetAlphaBlending(false);

	auto& selected = slots[selectedIndex];
	if (selected.inUIPass) {
		Demo::DrawKeyboardTargetReticle(gd, uiCamera, selected.x, selected.y, selected.width, selected.height, GetTickCount64());
	}
}

void Demo::IBattleScene::DrawWorld(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();

	if (SUCCEEDED(gd->BeginDraw())) {
		if (customBackgroundDraw) {
			customBackgroundDraw(gd, deltaTime);
		}
		else {
			/* Cool wave grid effect */
			auto app = DX9GF::Application::GetInstance();
			float screenWidth = static_cast<float>(app->GetScreenWidth());
			float screenHeight = static_cast<float>(app->GetScreenHeight());

			const int spacingX = 32;
			const int spacingY = 32;
			const float segmentLength = 16.0f;
			const float amplitude = 12.0f;
			const float frequency = 0.05f;
			const D3DCOLOR gridColor = 0xFF9cdb43;
			static float waveTime = 0.0f;
			waveTime += static_cast<float>(deltaTime) * 0.002f;
			while (waveTime > 6.2831853f) {
				waveTime -= 6.2831853f;
			}

			for (int y = 0; y <= screenHeight; y += spacingY) {
				gd->DrawLine(0.0f, static_cast<float>(y), static_cast<float>(screenWidth), static_cast<float>(y), gridColor);
			}

			for (int x = 0; x <= screenWidth; x += spacingX) {
				float prevY = 0.0f;
				float prevX = static_cast<float>(x) + std::sinf(waveTime) * amplitude;
				for (float y = segmentLength; y <= static_cast<float>(screenHeight); y += segmentLength) {
					float offsetX = static_cast<float>(x) + std::sinf((y * frequency) + waveTime) * amplitude;
					gd->DrawLine(prevX, prevY, offsetX, y, gridColor);
					prevX = offsetX;
					prevY = y;
				}
			}
			/* End of cool wave grid effect */
		}

		if (backgroundDimAlpha > 0.001f) {
			auto app = DX9GF::Application::GetInstance();
			float screenWidth = static_cast<float>(app->GetScreenWidth());
			float screenHeight = static_cast<float>(app->GetScreenHeight());
			const int dimAlphaByte = static_cast<int>(backgroundDimAlpha * 255.f);
			gd->SetAlphaBlending(true);
			gd->DrawRectangle(0.f, 0.f, screenWidth, screenHeight, D3DCOLOR_ARGB(dimAlphaByte, 0, 0, 0), true);
			gd->SetAlphaBlending(false);
		}

		switch (state) {
		case State::PlayerStandBy:
		case State::PlayerOpenItems:
		case State::PlayerViewPile:
			for (auto& enemy : enemies) {
				enemy->Draw(gd, &camera, deltaTime);
			}
			break;
		case State::PlayerAttack:
			for (auto& enemy : enemies) {
				enemy->Draw(gd, &camera, deltaTime);
			}
			draggableManager->Draw(deltaTime);
			DrawKeyboardReticleWorld(deltaTime);
			break;
		case State::EnemyAttack:
			// DRAW BATTLE BOUNDS
			const float battleHalfSize = battleBoxSize * 0.5f;
			gd->SetAlphaBlending(true);
			gd->DrawRectangle(camera, -battleHalfSize, -battleHalfSize, battleBoxSize, battleBoxSize, 0xEE6d758d, true);
			gd->DrawRectangle(camera, -battleHalfSize, -battleHalfSize, battleBoxSize, battleBoxSize, 0xFF000000, false);
			const int outlineThickness = 4;
			for (int i = 1; i < outlineThickness; ++i) {
				gd->DrawRectangle(camera, -battleHalfSize - i, -battleHalfSize - i, battleBoxSize + 2 * i, battleBoxSize + 2 * i, 0xFFdae0ea, false);
			}
			gd->DrawRectangle(camera, -battleHalfSize - outlineThickness, -battleHalfSize - outlineThickness, battleBoxSize + 2 * outlineThickness, battleBoxSize + 2 * outlineThickness, 0xFF000000, false);
			gd->SetAlphaBlending(false);

			battlePlayer->Draw(deltaTime);
			for (auto& enemy : enemies) {
				enemy->Draw(gd, &camera, deltaTime);
			}
			break;
		}

		for (auto& card : queuedToDraw) {
			dynamic_pointer_cast<IDraggable>(card)->Draw(deltaTime);
		}

		popUpMessage->Draw(deltaTime);
		DamageTextManager::GetInstance()->Draw(this->camera, deltaTime);
		gd->EndDraw();
	}
}

void Demo::IBattleScene::DrawUI(unsigned long long deltaTime)
{
	auto gd = game->GetGraphicsDevice();

	if (SUCCEEDED(gd->BeginDraw())) {

		switch (state) {
		case State::PlayerStandBy:
			PlayerStandByDraw(deltaTime);
			break;
		case State::PlayerAttack:
			PlayerAttackDraw(deltaTime);
			break;
		case State::PlayerOpenItems:
			PlayerOpenItemsDraw(deltaTime);
			break;
		case State::PlayerViewPile:
			PlayerViewPileDraw(deltaTime);
			break;
		case State::EnemyAttack:
			float y = game->GetVirtualHeight() / 2.f - 40.f;
			DrawHealthAndDefenseBar(y, gd);
			break;
		}

		DrawKeyboardReticleUI(deltaTime);

		if (isDefeatSequence && defeatElapsedMs >= 1000.f) {
			auto [camW, camH] = uiCamera.GetScreenResolution();
			float w = static_cast<float>(camW);
			float h = static_cast<float>(camH);
			const int alpha = static_cast<int>((std::min)(1.0f, defeatFadeAlpha) * 255.f);

			gd->SetAlphaBlending(true);
			gd->DrawRectangle(uiCamera, -w / 2.f, -h / 2.f, w, h, D3DCOLOR_ARGB(alpha, 0, 0, 0), true);
			gd->SetAlphaBlending(false);
		}
		DrawAttackCountdown(deltaTime);

		drawBuffer->Update(deltaTime);

		if (!keyboardNavigator.IsInKeyboardMode()) {
			DX9GF::InputManager::GetInstance()->DrawCursor(&this->uiCamera, deltaTime);
		}
		gd->EndDraw();
	}
}