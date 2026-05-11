#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Player.h"
#include "Game.h"
#include "IDraggable.h"
#include "TextButton.h"
#include "IconButton.h"
#include "MainBlockCard.h"
#include "StrikeCard.h"
#include "EnemyCard.h"
#include "HandContainer.h"
#include "TestEnemy.h"
#include "PopUpMessage.h"
#include "AdvancedCards.h"	

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
		// States
		State state = State::PlayerStandBy;
        State lastEnemyLayoutState = State::EnemyAttack;
		bool enemyLayoutInitialized = false;
		bool isExecutingAttacks = false;
		size_t currentTurn = 0;
		int energy = MAX_ENERGY;
		int usedEnergy = 0;
		bool isTransitioning = false;
        bool enemyAttackStartPending = false;
        size_t initialEnemyCount = 0;
        int battleGoldReward = 0;
        bool isBattleEnding = false;
        bool isDefeatSequence = false;
		float defeatElapsedMs = 0.f;
		float defeatFadeAlpha = 0.f;
		// Externals
		Game* game;
		std::shared_ptr<Player> player;
		std::shared_ptr<Player> battlePlayer;
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
		std::shared_ptr<PopUpMessage> popUpMessage;
		std::shared_ptr<DX9GF::StaticSprite> energyIcon;
		std::shared_ptr<DX9GF::StaticSprite> hourglassIcon;
		std::shared_ptr<DX9GF::StaticSprite> itemMenuBackground;
		void CreateEnemyCard(std::shared_ptr<IEnemy> enemy);
		void StartBattle();
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
		void PlayerOpenItemsUpdate(unsigned long long deltaTime);
		bool EnemyAttackUpdate(unsigned long long deltaTime);
        void QueueEnemyLayoutTransition(State targetState);
        void RemoveEnemyCardsInRemoveArea();
		// Draws
		void PlayerStandByDraw(unsigned long long deltaTime);
		void PlayerAttackDraw(unsigned long long deltaTime);
		void PlayerOpenItemsDraw(unsigned long long deltaTime);
		void EnemyAttackDraw(unsigned long long deltaTime);
	public:
		IBattleScene(Game* game, std::shared_ptr<Player> player, int screenWidth, int screenHeight) : IScene(screenWidth, screenHeight), game(game), player(player) {}
		virtual void Init() override;
		void Update(unsigned long long deltaTime) override;
		void Draw(unsigned long long deltaTime) override;
	};
}