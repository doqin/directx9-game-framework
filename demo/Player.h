#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "ICard.h"
#include "GameItems.h"
#include "DamageTextManager.h"
namespace Demo {
	class IEnemy;
	class Player : public DX9GF::IGameObject, public DX9GF::ISaveable {
	private:
		enum class State {
			Left,
			Right,
			Up,
			Down,
			UpLeft,
			UpRight,
			DownLeft,
			DownRight
		};
		// Constants
        const float MAX_HEALTH = 50;
		float VELOCITY = 85;
		const float SPRINT_MULTIPLIER = 1.5f;
		const float CAMERA_VELOCITY = 25;
		const float CAMERA_ACCELERATION = 50;
		const float CAMERA_SNAP_MARGIN = 1.f;
		const float INVINCIBILITY_DURATION = 1.f;
		const float BLINKING_DURATION = 0.1f;
		// States
		State state = State::Down;
		bool isWalking = false;
		float cameraDeltaTime = 0;
		bool followCamera = true;
		float health = MAX_HEALTH;
      float temporaryDefense = 0.f;
		bool isInvincible = false;
		float timeSinceTurnedInvincible = 0.f;
		// Sprites
		std::shared_ptr<DX9GF::Texture> spritesheet;
		std::shared_ptr<DX9GF::StaticSprite> idleDown;
		std::shared_ptr<DX9GF::StaticSprite> idleUp;
		std::shared_ptr<DX9GF::StaticSprite> idleRight;
		std::shared_ptr<DX9GF::StaticSprite> idleLeft;
		std::shared_ptr<DX9GF::StaticSprite> idleDownLeft;
		std::shared_ptr<DX9GF::StaticSprite> idleDownRight;
		std::shared_ptr<DX9GF::StaticSprite> idleUpLeft;
		std::shared_ptr<DX9GF::StaticSprite> idleUpRight;
		std::shared_ptr<DX9GF::AnimatedSprite> walkingDownLeft;
		std::shared_ptr<DX9GF::AnimatedSprite> walkingDownRight;
		std::shared_ptr<DX9GF::AnimatedSprite> walkingUpLeft;
		std::shared_ptr<DX9GF::AnimatedSprite> walkingUpRight;
		std::shared_ptr<DX9GF::AnimatedSprite> walkingDown;
		std::shared_ptr<DX9GF::AnimatedSprite> walkingUp;
		std::shared_ptr<DX9GF::AnimatedSprite> walkingRight;
		std::shared_ptr<DX9GF::AnimatedSprite> walkingLeft;
		// Colliders
		std::shared_ptr<DX9GF::RectangleCollider> collider;
		// Externals
		DX9GF::GraphicsDevice* graphicsDevice = nullptr;
		DX9GF::ColliderManager* colliderManager = nullptr;
		DX9GF::Camera* camera = nullptr;

		int gold = 100;
		std::vector<std::string> deck;

		std::vector<std::string> inventoryCards;
		ItemInventory inventoryItems;
		std::vector<ActiveBuff> activeBuffs;

		//for audio
		float stepTimer = 0.0f;
		std::string baseSurface = "default";
		std::string currentSurface = "default";
		float surfaceTimeout = 0.0f;
	public:
		Player(std::weak_ptr<DX9GF::TransformManager> transformManager) : IGameObject(transformManager) {}
		Player(
			std::weak_ptr<DX9GF::TransformManager> transformManager,
			float x,
			float y,
			float rotation = 0,
			float scaleX = 1,
			float scaleY = 1
		) : IGameObject(transformManager, x, y, rotation, scaleX, scaleY) {}
		Player(
			std::weak_ptr<DX9GF::TransformManager> transformManager,
			std::weak_ptr<DX9GF::IGameObject> parent,
			float x,
			float y,
			float rotation = 0,
			float scaleX = 1,
			float scaleY = 1
		) : IGameObject(transformManager, parent, x, y, rotation, scaleX, scaleY) {}
		~Player();
		void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::ColliderManager* colliderManager, DX9GF::Camera* camera, bool isBattling = false);
		void Update(unsigned long long deltaTime);
		void Draw(unsigned long long deltaTime);
		void SetFollowCamera(bool followCamera);
		float GetVelocity() const;
		float SetVelocity(float velocity);
		bool TakeDamage(float damage);
		void DealDamage(IEnemy* target, float cardBaseDamage);
		void Heal(float value);
		bool IsDead() const;
		void SetHealth(float hp);
		float GetMaxHealth() const;
		float GetHealth() const;
		float GetTemporaryDefense() const;
		std::weak_ptr<DX9GF::RectangleCollider> GetCollider();

		int GetGold() const { return gold; }
		void AddGold(int amount) { gold += amount; }
		void AddCardToDeck(const std::string& card) { deck.push_back(card); }
		const std::vector<std::string>& GetDeck() const { return deck; }
		void ClearDeck() { deck.clear(); }
		const std::vector<std::string>& GetInventoryCards() const { return inventoryCards; }
		ItemInventory& GetInventoryItems() { return inventoryItems; }
		const std::vector<ActiveBuff>& GetActiveBuffs() { return activeBuffs; }
		void AddCardToInventory(const std::string& card) { inventoryCards.push_back(card); }
		void ClearInventory() { inventoryCards.clear(); }
		virtual std::string GetSaveID() const override;
		virtual void GenerateSaveData(nlohmann::json& outData) override;
		virtual void RestoreSaveData(const nlohmann::json& inData) override;
		void GenerateSaveGlobalData(nlohmann::json& outData) const;
		void RestoreSaveGlobalData(const nlohmann::json& inData);

		float GetBuffStat(ItemBuffType targetType) const;
		void UpdateBuffs();
		void AddActiveBuff(const ActiveBuff& buff);
		bool IsWalking() const { return isWalking; }

		void SetSurface(std::string surface);
		void SetBaseSurface(std::string surface) {
			this->baseSurface = surface;
			this->currentSurface = surface;
		}
	};
}