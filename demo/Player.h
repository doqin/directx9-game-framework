#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "ICard.h"
#include "GameItems.h"
#include "DamageTextManager.h"
#include "PlayerGlobalData.h"
#include <memory>
#include "ICombatant.h"
namespace Demo {
	class IEnemy;
	class Player : public ICombatant, public DX9GF::ISaveable {
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
		// Gear Drone
		std::shared_ptr<DX9GF::Texture> gearTex;
		std::shared_ptr<DX9GF::AnimatedSprite> gearDroneAnim;
		float gearAnimTimer = 0.0f;
		int lastEquippedGearID = -2;
		// Particles
		const float FOOTPRINT_OFFSET = 3.f;
		std::shared_ptr<DX9GF::Texture> footprintTexture;
		std::unique_ptr<DX9GF::ParticleSystem> footprintEmitter;
		bool footprintsEnabled = true;
		bool nextFootLeft = true;
		// Colliders
		std::shared_ptr<DX9GF::RectangleCollider> collider;
		// Externals
		DX9GF::GraphicsDevice* graphicsDevice = nullptr;
		DX9GF::ColliderManager* colliderManager = nullptr;
		DX9GF::Camera* camera = nullptr;

		//for audio
		float stepTimer = 0.0f;
		std::string baseSurface = "default";
		std::string currentSurface = "default";
		float surfaceTimeout = 0.0f;
	public:
		static bool ignoreCollisions; // For debugging purposes
		Player(std::weak_ptr<DX9GF::TransformManager> tm) : ICombatant(tm, 50.0) {}
		Player(std::weak_ptr<DX9GF::TransformManager> tm,
			float x, float y, float rot = 0, float sx = 1, float sy = 1)
			: ICombatant(tm, 50.f, x, y, rot, sx, sy) {
		}
		Player(std::weak_ptr<DX9GF::TransformManager> tm,
			std::weak_ptr<DX9GF::IGameObject> parent, float x, float y, float rot = 0, float sx = 1, float sy = 1)
			: ICombatant(tm, parent, 50.f, x, y, rot, sx, sy) {
		}
		~Player();
		void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::ColliderManager* colliderManager, DX9GF::Camera* camera, bool isBattling = false);
		void Update(unsigned long long deltaTime);
		void Draw(unsigned long long deltaTime);
		void SetFollowCamera(bool followCamera);
		float GetVelocity() const;
		float SetVelocity(float velocity);
		bool TakeDamage(float damage, bool ignoreArmor = false) override;
		void DealDamage(IEnemy* target, float cardBaseDamage, bool ignoreArmor = false);
		std::weak_ptr<DX9GF::RectangleCollider> GetCollider();

		float GetHealth() const { return PlayerGlobalData::GetInstance()->GetHealth(); }
		float GetMaxHealth() const { return PlayerGlobalData::GetInstance()->GetMaxHealth(); }
		void SetHealth(float hp) { PlayerGlobalData::GetInstance()->SetHealth(hp); }
		bool IsDead() const { return PlayerGlobalData::GetInstance()->IsDead(); }
		void Heal(float value) override;

		int GetGold() const { return PlayerGlobalData::GetInstance()->GetGold(); }
		void SetGold(int amount) { PlayerGlobalData::GetInstance()->SetGold(amount); }
		void AddGold(int amount) { PlayerGlobalData::GetInstance()->AddGold(amount); }
		void AddCardToDeck(const std::string& card) { PlayerGlobalData::GetInstance()->AddCardToDeck(card); } // Should probably validate that the card exists in the game, but for now we just add it
		const std::vector<std::string>& GetDeck() const { return PlayerGlobalData::GetInstance()->GetDeck(); }
		void ClearDeck() { PlayerGlobalData::GetInstance()->ClearDeck(); }
		const std::vector<std::string>& GetInventoryCards() const { return PlayerGlobalData::GetInstance()->GetInventoryCards(); }
		ItemInventory& GetInventoryItems() { return PlayerGlobalData::GetInstance()->GetInventoryItems(); }
		void AddCardToInventory(const std::string& card) { PlayerGlobalData::GetInstance()->AddCardToInventory(card); }
		bool RemoveCardFromInventory(const std::string& card) { return PlayerGlobalData::GetInstance()->RemoveCardFromInventory(card); }
		void ClearInventory() { PlayerGlobalData::GetInstance()->ClearInventory(); }
		int GetEquippedGearID() const { return PlayerGlobalData::GetInstance()->GetEquippedGearID(); }
		const std::vector<int>& GetInventoryGears() const { return PlayerGlobalData::GetInstance()->GetInventoryGears(); }
		void EquipGear(int id) { PlayerGlobalData::GetInstance()->EquipGear(id); }
		void UnequipGear() { PlayerGlobalData::GetInstance()->UnequipGear(); }
		void AddGear(int id) { PlayerGlobalData::GetInstance()->AddGear(id); }
		virtual std::string GetSaveID() const override;
		virtual void GenerateSaveData(nlohmann::json& outData) override;
		virtual void RestoreSaveData(const nlohmann::json& inData) override;

		bool IsWalking() const { return isWalking; }

		void SetFootprintsEnabled(bool enabled);
		void SetSurface(std::string surface);
		void SetBaseSurface(std::string surface) {
			this->baseSurface = surface;
			this->currentSurface = surface;
		}

		bool TakeIndirectDamage(float damage, DamageType type) override;
		void InitGearAnim(std::shared_ptr<DX9GF::Texture> tex);
	};
}