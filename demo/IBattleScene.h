#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Player.h"
#include "Game.h"
#include "IDraggable.h"
#include "IconButton.h"
#include "MainBlockCard.h"
#include "InitBlockCard.h"
#include "StrikeCard.h"
#include "EnemyCard.h"
#include "CardContainer.h"
#include "TestEnemy.h"
#include "PopUpMessage.h"
#include "AdvancedCards.h"
#include "TextIconButton.h"
#include "KeyboardNavigator.h"
#include "BattleMenu.h"
#include "BattleEncounter.h"
#include "EnergyToken.h"
#include "GoldToken.h"
namespace Demo {
	class IBattleScene : public DX9GF::IScene {
	protected:
		enum class State {
			PlayerStandBy,
			PlayerAttack,
			PlayerOpenItems,
			PlayerViewPile,
			EnemyAttack
		};
		enum class PileKind {
			Draw,
			Discard,
			Nullified
		};
		// Constants
		const int MAX_ENERGY = 3;
		static constexpr size_t CARDS_DRAWN_PER_TURN = 5;
		const float BACKGROUND_DIM_ALPHA = 0.55f;
		const float BACKGROUND_DIM_SPEED = 2.f; // alpha units per second
		// States
		State state = State::PlayerStandBy;
		float backgroundDimAlpha = 0.f;
		State lastEnemyLayoutState = State::EnemyAttack;
		bool enemyLayoutInitialized = false;
		bool isExecutingAttacks = false;
		// Set while the init block is resolving. Unlike isExecutingAttacks this never routes into
		// the enemy turn - that is the whole point of the init block.
		bool isExecutingInit = false;
		// Set when that init run was kicked off by Execute rather than by Run Init, so the main
		// block starts as soon as init finishes. One press should resolve the whole turn.
		bool executeAfterInit = false;
		size_t currentTurn = 0;
		int energy = MAX_ENERGY;
		int usedEnergy = 0;
		// Energy spent by cards that have already resolved out of the init block. usedEnergy is
		// rebuilt from the hand every frame, so without this the cost of an init card would be
		// refunded the moment it left the hand - and the init block could be run forever.
		int committedEnergy = 0;
		// Extra energy granted at the start of the next turn (e.g. by EnergyCard).
		int pendingBonusEnergy = 0;
		// Extra cards drawn at the start of the next turn (e.g. by ForesightCard).
		int pendingBonusDraw = 0;
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
		std::shared_ptr<InitBlockCard> initBlockCard;
		std::shared_ptr<CardContainer> handContainer;
		std::vector<std::shared_ptr<ICard>> cardHand;
		std::vector<std::shared_ptr<EnemyCard>> enemyCards;
		std::vector<std::shared_ptr<ICard>> drawPile;
		std::vector<std::shared_ptr<ICard>> playedPile;
		std::vector<std::shared_ptr<ICard>> discardPile;
		// Cards whose limited uses ran out. They stay here for the rest of the battle and are
		// never shuffled back into the draw pile.
		std::vector<std::shared_ptr<ICard>> nullifiedPile;
		std::vector<std::shared_ptr<ICard>> queuedToDraw;
		std::vector<std::shared_ptr<IEnemy>> enemies;
		// Enemies spawned by other enemies wait here until FlushPendingEnemies runs. Spawns are
		// triggered from deferred commands that execute inside the loops walking `enemies`, so
		// appending directly would reallocate the vector out from under the running iteration.
		std::vector<std::shared_ptr<IEnemy>> pendingEnemies;
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
		std::shared_ptr<IconButton> runInitButton;
		std::shared_ptr<IconButton> closeItemMenuButton;
		std::shared_ptr<BattleMenu> battleMenu;

		std::vector<std::shared_ptr<IconButton>> buffItems;
		std::shared_ptr<TextIconButton> btnNextPage;
		std::shared_ptr<TextIconButton> btnPrevPage;
		int currentItemPage = 0;
		int maxItemPage = 0;

		// A consumable used this battle whose effect is still running. The effect itself lives in
		// battlePlayer's modifier list - this only records which item put it there, so the HUD can
		// show the source next to the anonymous buff icons.
		struct ActiveItemEffect {
			int itemID;
			std::wstring name;
			std::wstring description;
			RECT iconRect;
			// The lingering modifiers this item applied. Instant ones (HealHP) are not listed -
			// they leave nothing to report once the popup is gone.
			std::vector<ModifierType> modifierTypes;
			// The entry's own countdown. Without it a card that happens to apply the same modifier
			// type would keep a long-spent item on screen.
			int turnsRemaining;
		};
		std::vector<ActiveItemEffect> activeItems;

		// Pile inspection. The buttons live on the PlayerAttack bar; opening one swaps to
		// State::PlayerViewPile, which lists throwaway copies of that pile's cards.
		std::shared_ptr<IconButton> drawPileButton;
		std::shared_ptr<IconButton> discardPileButton;
		std::shared_ptr<IconButton> nullifiedPileButton;
		std::shared_ptr<IconButton> closePileViewButton;
		std::shared_ptr<TextIconButton> btnPileNextPage;
		std::shared_ptr<TextIconButton> btnPilePrevPage;
		PileKind viewedPile = PileKind::Draw;
		std::vector<std::shared_ptr<ICard>> pileViewCards;
		int currentPilePage = 0;
		int maxPilePage = 0;
		// 16x16 sheet icons at 3x - big enough that the count in the corner doesn't swamp the art.
		static constexpr float PILE_BUTTON_SIZE = 48.f;
		static constexpr float PILE_BUTTON_GAP = 8.f;

		// PlayerAttack enemy grid. Columns are anchored to the right edge and march toward the
		// screen midpoint, so the enemy area never reaches the program blocks or the card hand.
		// Clear of the right edge, leaving room for the status icons an enemy draws at +64..+140.
		static constexpr float ENEMY_GRID_RIGHT_PAD = 140.f;
		static constexpr float ENEMY_GRID_COLUMN_PITCH = 170.f;
		// Extra clearance when a wide body (the boss is 256px) pushes the next column over.
		static constexpr float ENEMY_GRID_COLUMN_GAP = 16.f;
		// An enemy draws its HP text 120px above its centre, so that - not the sprite - is what
		// binds at the top of the screen. Includes room for the glyphs themselves.
		static constexpr float ENEMY_GRID_HP_TEXT_HEADROOM = 130.f;
		static constexpr float ENEMY_GRID_BOTTOM_PAD = 12.f;
		static constexpr float ENEMY_GRID_ROW_GAP = 16.f;
		// Leftmost pixel an enemy body may reach. The void main() / void Init() blocks sit at
		// x = +70 by default, so columns stop just clear of them rather than at the midpoint.
		static constexpr float ENEMY_GRID_LEFT_EDGE_X = 76.f;

		std::shared_ptr<PopUpMessage> popUpMessage;
		std::shared_ptr<DX9GF::StaticSprite> energyIcon;
		std::shared_ptr<DX9GF::StaticSprite> hourglassIcon;
		std::shared_ptr<DX9GF::NineSliceSprite> itemMenuBackground;
		std::shared_ptr<DX9GF::StaticSprite> attackBuffIcon;
		std::shared_ptr<DX9GF::StaticSprite> defenseBuffIcon;
		std::shared_ptr<DX9GF::StaticSprite> activeItemIcon;
		bool pendingLockMessage = false;
		std::function<void()> onVictoryCallback = nullptr;
		std::string customBGMName;

		void CreateEnemyCard(std::shared_ptr<IEnemy> enemy);
		void StartBattle();
		// Locks a random available card (hand first, then queued/draw pile) for the given turns.
		void LockRandomCard(int turns);
		void OnAllEnemiesDefeated();

		//Tokens
		std::vector<std::shared_ptr<Demo::IToken>> activeTokens;
		int pendingGoldTokenReward = 0;
		float tokenSpawnTimer = 0.f;
		int tokensSpawnedThisTurn = 0;
		int maxTokensThisTurn = 0;
		EventType battleEventType = EventType::None;
		int targetTokenTurn = -1;

		bool isFlawlessChallengeActive = false;
		bool isFlawlessCompleted = false;
		float startingHealthForFlawless = 0.f;
		float startingDefenseForFlawless = 0.f;

		std::shared_ptr<DX9GF::Texture> energyTokenTex;
		std::shared_ptr<DX9GF::StaticSprite> energyTokenUI;
		std::shared_ptr<DX9GF::StaticSprite> goldTokenUI;
		std::shared_ptr<DX9GF::Texture> goldTex;
		int currentEnergyPieces = 0;
	private:
		// midTurn drops the hold and stagger from the deal animation - a mid-turn draw blocks the
		// Execute/Run buttons until it lands, so it should not linger.
		void DrawCards(size_t count, bool midTurn = false);
		void ShuffleDiscardIntoDrawPile();
		void MovePlayedPileToDiscardPileIfNeeded();
		void MoveExecutedHandCardsToPlayedPile();
		// Pulls cards that have run out of limited uses out of the hand/played piles so they
		// stop executing and can't be drawn again this battle.
		void MoveDepletedCardsToNullifiedPile();
		// Pulls non-persistent cards out of the block so they fire once per placement instead of
		// re-executing every turn until the program clears. They return to the discard pile, so
		// unlike depleted cards they can be drawn and played again.
		void MoveNonPersistentCardsToDiscardPile();
		// Discards whatever is still sitting in the init block at end of turn. Runs before the
		// played-pile sweep, which would otherwise treat those cards as played and leave them
		// attached to the init block - runnable for free next turn once usedEnergy resets.
		void MoveInitBlockCardsToDiscardPile();
		void MoveHandCardsToDiscardPile();
		// Commits the energy of the cards that just resolved out of the init block and sends them
		// to the discard pile, so nothing run from init can be run again.
		void FinishInitBlockExecution();
		void BeginNextTurn();
		void RefreshItemMenu();
		// Records a consumable the player just used so the HUD can list it while its modifiers
		// last. A no-op for items that only apply instant effects.
		void RegisterActiveItem(const ConsumableItem& item);
		// Turns the item's effect has left, read back from the player's live modifiers so the
		// number always agrees with the status icons drawn beside it.
		int GetActiveItemTurnsLeft(const ActiveItemEffect& entry) const;
		// Ages the entries alongside the player's modifiers and drops the spent ones.
		void TickActiveItems();
		void CollectDeadEnemies();
		// Moves anything queued by onRequestSpawnEnemy into `enemies`. Safe to call between
		// iterations, never during one.
		void FlushPendingEnemies();
		// Wires the scene callbacks an enemy needs. Applied to the starting line-up and to
		// every enemy spawned mid-battle.
		void WireEnemyCallbacks(const std::shared_ptr<IEnemy>& enemy);
		// The two program bodies, in the order keyboard navigation should walk them. Skips any
		// that have not been created yet.
		std::vector<std::shared_ptr<IBlockCard>> GetBlockCards() const;
		// Pile inspection
		const std::vector<std::shared_ptr<ICard>>& GetPile(PileKind kind) const;
		static std::wstring GetPileName(PileKind kind);
		void OpenPileView(PileKind kind);
		void ClosePileView();
		// Rebuilds the display-only copies shown for the current pile and page.
		void RefreshPileView();
		// Parks the three pile buttons at the right end of the action bar; returns their left edge.
		float LayOutPileButtons(float screenWidth, float buttonY);
		// Updates
		void PlayerStandByUpdate(unsigned long long deltaTime);
		void PlayerAttackUpdate(unsigned long long deltaTime);
		void QueueToEnemyAttack(unsigned long long deltaTime);
		void PlayerOpenItemsUpdate(unsigned long long deltaTime);
		void PlayerViewPileUpdate(unsigned long long deltaTime);
		bool EnemyAttackUpdate(unsigned long long deltaTime);
		void QueueEnemyLayoutTransition(State targetState);
		// One target position per entry in `enemies`, packed into right-anchored columns that
		// grow leftward. Columns are filled in list order, so a mid-battle spawn takes the next
		// free slot instead of reshuffling the enemies already placed.
		std::vector<D3DXVECTOR2> BuildEnemyGridSlots() const;
		// Vertical band the enemy grid may occupy, in world coordinates.
		void GetEnemyGridBand(float& top, float& bottom) const;
		void RemoveEnemyCardsInRemoveArea();
		void StartAttackCountdown(std::shared_ptr<std::vector<std::shared_ptr<IEnemy>>> attackingEnemies);
		bool UpdateAttackCountdown(unsigned long long deltaTime);
		void DrawAttackCountdown(unsigned long long deltaTime);
		// Keyboard navigation
		void UpdateKeyboardNavigation(unsigned long long deltaTime);
		std::vector<KeyboardNavigator::Candidate> CollectKeyboardCandidates();
		std::vector<PlacementSlot> CollectPlacementSlots();
		void PlaceStatementCardAt(std::shared_ptr<IStatementCard> card, size_t index, std::shared_ptr<IBlockCard> block);
		void PlaceStatementCardInHand(std::shared_ptr<IStatementCard> card);
		void PlaceEnemyCardOn(std::shared_ptr<EnemyCard> card, std::shared_ptr<IStatementCard> destination);
		void DrawKeyboardReticleUI(unsigned long long deltaTime);
		void DrawKeyboardReticleWorld(unsigned long long deltaTime);
		void DrawKeyboardPlacementUI(unsigned long long deltaTime);
		// Draws
		void PlayerStandByDraw(unsigned long long deltaTime);
		void PlayerAttackDraw(unsigned long long deltaTime);
		void PlayerOpenItemsDraw(unsigned long long deltaTime);
		void PlayerViewPileDraw(unsigned long long deltaTime);
		// The three pile buttons plus their card counts, drawn on the PlayerAttack bar.
		void DrawPileButtons(unsigned long long deltaTime);
		void EnemyAttackDraw(unsigned long long deltaTime);
		// Damage each enemy would take if the program were executed now. Replays the queue step by
		// step - init block then main block, the order Execute resolves them in - so a Vulnerable,
		// Mark or attack buff played earlier in the same program is already standing when the hits
		// after it are counted, and block spent absorbing one hit is not available to the next.
		void ComputeProjectedDamage(std::unordered_map<IEnemy*, float>& out);
		// That total, drawn at each enemy's bottom-left through the attack phase.
		void DrawProjectedDamage(unsigned long long deltaTime);
		void DrawHealthAndDefenseBar(const float y, DX9GF::GraphicsDevice* gd);
		void DrawTooltip(std::wstring& activeTooltipText, float screenX, float screenY, DX9GF::GraphicsDevice* gd);
		void DrawModifierIcons(const float x, const float y, DX9GF::GraphicsDevice* gd);
		// The row of used-item icons beside the health bar. Returns the tooltip text for whichever
		// icon the mouse is over (empty if none), so it can share the bar's tooltip renderer.
		std::wstring DrawActiveItems(const float barRightX, const float y);
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
		// Whether this card can go into a block without overspending.
		//
		// Read this before touching it: usedEnergy is rebuilt every frame from every cardHand card
		// whose parent is NOT the hand container, and dragging a card detaches it first. So by the
		// time a drop is tested, the dragged card's cost is already inside usedEnergy, and testing
		// GetAvailableEnergy() against its cost again double-bills it - which rejects plays the
		// player can afford (3 energy could never place a 3-cost card). A card still sitting in the
		// hand container has not been counted yet, and that one does pay its cost here.
		bool CanPlaceCardInBlock(const std::shared_ptr<ICard>& card) const;
		// Snaps a card back to the hand after a rejected drop. Deferred, because drops resolve
		// while the draggable manager is iterating its object map.
		void ReturnCardToHand(const std::shared_ptr<IStatementCard>& card);
		// Detaches the card from whatever it was slotted into and takes it out of the battle for
		// good. Public because statement cards call it when they give up their targets.
		void DiscardEnemyCard(std::shared_ptr<EnemyCard> card);
		// For cards that hit everything without the player attaching an EnemyCard per target.
		// Callers must skip IsDead() enemies - they linger here until CollectDeadEnemies runs.
		const std::vector<std::shared_ptr<IEnemy>>& GetEnemies() const { return enemies; }
		// Pending resources: applied at the start of the next turn.
		void QueueBonusEnergy(int amount) { pendingBonusEnergy += amount; }
		void QueueBonusDraw(int amount) { pendingBonusDraw += amount; }
		// Instant resources: applied immediately. Only meaningful from the Init block - the main
		// block executes as the turn ends, so anything gained there is never spent.
		void GainEnergyNow(int amount) { energy += amount; }
		void DrawCardsNow(int amount);
		void QueuePopUpMessage(const std::wstring& msg) { if (popUpMessage) popUpMessage->QueueMessage(&commandBuffer, msg); }
		void AddPendingGoldTokenReward(int amount) { pendingGoldTokenReward += amount; }
		void SpawnRandomToken(float x, float y);
		void AddEnergyPieceToken();
	};
}