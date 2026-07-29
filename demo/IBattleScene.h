#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Player.h"
#include "Game.h"
#include "IDraggable.h"
#include "IconButton.h"
#include "MainBlockCard.h"
#include "StrikeCard.h"
#include "EnemyCard.h"
#include "HandContainer.h"
#include "TestEnemy.h"
#include "PopUpMessage.h"
#include "AdvancedCards.h"
#include "TextIconButton.h"
#include "KeyboardNavigator.h"

namespace Demo {
	class IBattleScene : public DX9GF::IScene {
	protected:
		enum class State {
			PlayerStandBy,
			PlayerAttack,
			PlayerOpenItems,
			EnemyAttack
		};
		// Constants
		const int MAX_ENERGY = 3;
		const float BACKGROUND_DIM_ALPHA = 0.55f;
		const float BACKGROUND_DIM_SPEED = 2.f; // alpha units per second
		// States
		State state = State::PlayerStandBy;
		float backgroundDimAlpha = 0.f;
		State lastEnemyLayoutState = State::EnemyAttack;
		bool enemyLayoutInitialized = false;
		bool isExecutingAttacks = false;
		size_t currentTurn = 0;
		int energy = MAX_ENERGY;
		int usedEnergy = 0;
		bool isTransitioning = false;
		bool enemyAttackStartPending = false;
		bool isFleeing = false;
		float timeSinceLastTargetPopUp = 999.f;
		const float targetPopUpCooldown = 3.f;
		bool isAttackCountdownActive = false;
		int attackCountdownNumber = 3;
		float attackCountdownTimer = 0.f;
		const float ATTACK_COUNTDOWN_STEP_SECONDS = 0.7f;
		std::shared_ptr<std::vector<std::shared_ptr<IEnemy>>> countdownAttackingEnemies;
		size_t initialEnemyCount = 0;
		int battleGoldReward = 0;
		bool isBattleEnding = false;
		bool isDefeatSequence = false;
		float defeatElapsedMs = 0.f;
		float defeatFadeAlpha = 0.f;
		// Keyboard navigation
		// A destination a picked-up card can be placed at (a slot in the block's execution queue,
		// or an eligible MultiTargetCard to lock an EnemyCard onto).
		struct PlacementSlot {
			float x;
			float y;
			float width;
			float height;
			std::function<void()> place;
			// Slots whose visuals live in the UI pass (e.g. the discard bar, drawn in
			// PlayerAttackDraw) must have their highlight drawn there too, or the bar
			// drawn later would cover it.
			bool inUIPass = false;
		};
		KeyboardNavigator keyboardNavigator;
		bool isDraggingBlockCardViaKeyboard = false;
		// Set while a StatementCard/EnemyCard has been "picked up" (first Enter) and is awaiting
		// a second Enter to confirm which highlighted destination it should be placed at.
		std::shared_ptr<DX9GF::IGameObject> pickedUpCard;
		size_t placementSlotIndex = 0;
		const float KEYBOARD_BLOCK_CARD_SPEED = 300.f; // px/sec
		// Externals
		Game* game;
		std::shared_ptr<Player> player;
		std::shared_ptr<Player> battlePlayer;

		std::function<void(DX9GF::GraphicsDevice*, unsigned long long)> customBackgroundDraw;
		// Managers
		DX9GF::CommandBuffer commandBuffer;
		std::shared_ptr<DX9GF::CommandBuffer> drawBuffer;
		std::shared_ptr<DraggableManager> draggableManager;
		std::shared_ptr<DX9GF::TransformManager> transformManager;
		DX9GF::ColliderManager colliderManager;
		// Battle cards
		std::shared_ptr<MainBlockCard> mainBlockCard;
		std::shared_ptr<HandContainer> handContainer;
		std::vector<std::shared_ptr<ICard>> cardHand;
		std::vector<std::shared_ptr<EnemyCard>> enemyCards;
		std::vector<std::shared_ptr<ICard>> drawPile;
		std::vector<std::shared_ptr<ICard>> playedPile;
		std::vector<std::shared_ptr<ICard>> discardPile;
		std::vector<std::shared_ptr<ICard>> queuedToDraw;
		std::vector<std::shared_ptr<IEnemy>> enemies;
		float enemyCardRemoveAreaX = 220.f;
		float enemyCardRemoveAreaY = -140.f;
		float enemyCardRemoveAreaWidth = 180.f;
		float enemyCardRemoveAreaHeight = 80.f;
		float battleBoxSize = 256.f;
		std::vector<std::shared_ptr<DX9GF::RectangleCollider>> battleBounds;
		// UI
		std::shared_ptr<DX9GF::Font> font;
		std::shared_ptr<DX9GF::FontSprite> fontSprite;
		std::shared_ptr<DX9GF::Texture> uiSheetTex;
		std::shared_ptr<DX9GF::Texture> tempTex;
		std::shared_ptr<DX9GF::Texture> itemsTex;
		std::shared_ptr<IconButton> attackButton;
		std::shared_ptr<IconButton> itemsButton;
		std::shared_ptr<IconButton> fleeButton;
		std::shared_ptr<IconButton> backButton;
		std::shared_ptr<IconButton> executeButton;
		std::shared_ptr<IconButton> closeItemMenuButton;

		std::vector<std::shared_ptr<IconButton>> buffItems;
		std::shared_ptr<TextIconButton> btnNextPage;
		std::shared_ptr<TextIconButton> btnPrevPage;
		int currentItemPage = 0;
		int maxItemPage = 0;

		std::shared_ptr<PopUpMessage> popUpMessage;
		std::shared_ptr<DX9GF::StaticSprite> energyIcon;
		std::shared_ptr<DX9GF::StaticSprite> hourglassIcon;
		std::shared_ptr<DX9GF::NineSliceSprite> itemMenuBackground;
		std::shared_ptr<DX9GF::StaticSprite> attackBuffIcon;
		std::shared_ptr<DX9GF::StaticSprite> defenseBuffIcon;
		bool pendingLockMessage = false;
		std::function<void()> onVictoryCallback = nullptr;
		std::string customBGMName;

		void CreateEnemyCard(std::shared_ptr<IEnemy> enemy);
		void StartBattle();
		// Locks a random available card (hand first, then queued/draw pile) for the given turns.
		void LockRandomCard(int turns);
		void OnAllEnemiesDefeated();

	private:
		void DrawCards(size_t count);
		void ShuffleDiscardIntoDrawPile();
		void MovePlayedPileToDiscardPileIfNeeded();
		void MoveExecutedHandCardsToPlayedPile();
		void MoveHandCardsToDiscardPile();
		void BeginNextTurn();
		void RefreshItemMenu();
		void CollectDeadEnemies();
		// Updates
		void PlayerStandByUpdate(unsigned long long deltaTime);
		void PlayerAttackUpdate(unsigned long long deltaTime);
		void QueueToEnemyAttack(unsigned long long deltaTime);
		void PlayerOpenItemsUpdate(unsigned long long deltaTime);
		bool EnemyAttackUpdate(unsigned long long deltaTime);
		void QueueEnemyLayoutTransition(State targetState);
		void RemoveEnemyCardsInRemoveArea();
		void StartAttackCountdown(std::shared_ptr<std::vector<std::shared_ptr<IEnemy>>> attackingEnemies);
		bool UpdateAttackCountdown(unsigned long long deltaTime);
		void DrawAttackCountdown(unsigned long long deltaTime);
		// Keyboard navigation
		void UpdateKeyboardNavigation(unsigned long long deltaTime);
		std::vector<KeyboardNavigator::Candidate> CollectKeyboardCandidates();
		std::vector<PlacementSlot> CollectPlacementSlots();
		void PlaceStatementCardAt(std::shared_ptr<IStatementCard> card, size_t index);
		void PlaceStatementCardInHand(std::shared_ptr<IStatementCard> card);
		void PlaceEnemyCardOn(std::shared_ptr<EnemyCard> card, std::shared_ptr<IStatementCard> destination);
		void DiscardEnemyCard(std::shared_ptr<EnemyCard> card);
		void DrawKeyboardReticleUI(unsigned long long deltaTime);
		void DrawKeyboardReticleWorld(unsigned long long deltaTime);
		void DrawKeyboardPlacementUI(unsigned long long deltaTime);
		// Draws
		void PlayerStandByDraw(unsigned long long deltaTime);
		void PlayerAttackDraw(unsigned long long deltaTime);
		void PlayerOpenItemsDraw(unsigned long long deltaTime);
		void EnemyAttackDraw(unsigned long long deltaTime);
		void DrawHealthAndDefenseBar(const float y, DX9GF::GraphicsDevice* gd);
	public:
		IBattleScene(Game* game, std::shared_ptr<Player> player, int screenWidth, int screenHeight) : IScene(screenWidth, screenHeight), game(game), player(player) {}
		virtual void Init() override;
		void Update(unsigned long long deltaTime) override;
		void DrawWorld(unsigned long long deltaTime) override;
		void DrawUI(unsigned long long deltaTime) override;
		void SetCustomBackgroundDraw(std::function<void(DX9GF::GraphicsDevice*, unsigned long long)> drawFunc) { customBackgroundDraw = drawFunc; }
		//
		void SetOnVictoryCallback(std::function<void()> cb) { onVictoryCallback = cb; }
		void SetCustomBGM(const std::string& name) { customBGMName = name; }
		int GetAvailableEnergy() const { return energy - usedEnergy; }
		void QueuePopUpMessage(const std::wstring& msg) { if (popUpMessage) popUpMessage->QueueMessage(&commandBuffer, msg); }
	};
}