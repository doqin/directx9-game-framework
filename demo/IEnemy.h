#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Player.h"
#include "ProjectileSystem.h"
#include <functional>
#include <vector>
#include "ICombatant.h"
namespace Demo {
	class PopUpMessage;

	class IEnemy : public ICombatant {
	protected:
		int goldReward = 0;
		struct DamageIndicator {
			std::wstring text;
			float offsetX = 0.f;
			float offsetY = 0.f;
			float vx = 0.f;
			float vy = 0.f;
			unsigned long long elapsed = 0;
			D3DCOLOR textColor = 0xFFFF4444;
		};
		bool isOnStandby = true;
		std::function<void(int)> onRequestLockCard = nullptr;
		// Hands a newly created enemy to the battle scene instead of touching its enemy list
		// directly. Spawns run from deferred commands inside the scene's enemy update loop, so
		// inserting straight into that vector would reallocate it mid-iteration.
		std::function<void(std::shared_ptr<IEnemy>)> onRequestSpawnEnemy = nullptr;
		std::shared_ptr<DX9GF::RectangleTrigger> cardSpawnTrigger;
		std::function<void(std::shared_ptr<IEnemy>)> onRequestEnemyCard = [](std::shared_ptr<IEnemy>) {};
		std::vector<DamageIndicator> damageIndicators;
		std::shared_ptr<DX9GF::Font> font;
		std::shared_ptr<DX9GF::FontSprite> fontSprite;
		ProjectileSystem projectiles;
		DX9GF::CommandBuffer commandBuffer;
		DX9GF::CommandBuffer animationBuffer;
		std::shared_ptr<DX9GF::Texture> uiTexture;
		std::shared_ptr<DX9GF::StaticSprite> uiSprite;
		std::shared_ptr<DX9GF::Texture> hitImpactTexture;
		std::vector<std::shared_ptr<DX9GF::AnimatedSprite>> hitImpactSprites;
		DX9GF::GraphicsDevice* graphicsDevice = nullptr;
		unsigned long long timeSinceStart = 0;
		void SetGoldReward(int reward) { goldReward = reward; }
		int lastPattern = -1;
		int streakCount = 0;
		int GetSmartRandomPattern(const int&& minPattern, const int&& maxPattern);
	public:
		IEnemy(std::weak_ptr<DX9GF::TransformManager> tm, float maxHealth) : ICombatant(tm, maxHealth) {}

		IEnemy(std::weak_ptr<DX9GF::TransformManager> tm, float maxHealth, float x, float y, float rot = 0, float sx = 1, float sy = 1) : ICombatant(tm, maxHealth, x, y, rot, sx, sy) {}

		IEnemy(std::weak_ptr<DX9GF::TransformManager> tm, std::weak_ptr<DX9GF::IGameObject> parent, float maxHealth, float x, float y, float rot = 0, float sx = 1, float sy = 1) : ICombatant(tm, parent, maxHealth, x, y, rot, sx, sy) {}
		void InitCardSpawnTrigger(DX9GF::Camera* camera, float width, float height);
		void SetOnRequestEnemyCard(std::function<void(std::shared_ptr<IEnemy>)> callback);
		virtual void Update(unsigned long long deltaTime);
		virtual void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime);
		virtual bool TakeDamage(float damage);
		virtual void StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies = nullptr, std::shared_ptr<PopUpMessage> popUpMessage = nullptr, DX9GF::GraphicsDevice* graphicsDevice = nullptr, DX9GF::Camera* camera = nullptr, int currentTurn = 1) = 0;
		void SetState(bool isOnStandby);
		bool IsOnStandby() const { return isOnStandby; }
		bool IsDoneAttacking();
		int GetGoldReward() const { return goldReward; }
		// On-screen size of the sprite, used by the battle scene to pack enemies without
		// overlapping them. Subclasses that draw larger art override these; the default matches
		// the common 64px frame at the 2x scale every enemy sets in Init.
		// Deliberately separate from cardSpawnTrigger, which is the click box.
		virtual float GetBodyWidth() const { return 128.f; }
		virtual float GetBodyHeight() const { return 128.f; }
		std::weak_ptr<DX9GF::RectangleTrigger> GetCardSpawnTrigger() const { return cardSpawnTrigger; }
		void SetOnRequestLockCard(std::function<void(int)> callback) { onRequestLockCard = callback; }
		void SetOnRequestSpawnEnemy(std::function<void(std::shared_ptr<IEnemy>)> callback) { onRequestSpawnEnemy = callback; }
		virtual void OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) {}
		virtual void OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn, std::vector<std::shared_ptr<IEnemy>>* enemies, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
			OnTurnBegin(player, popUpMessage, currentTurn);
		}

		bool TakeIndirectDamage(float damage, DamageType type) override;
		void SpawnHealText(float actualHeal) override;
		void CastAbility(std::function<void()> effect, std::shared_ptr<PopUpMessage> popUpMessage, const std::wstring& message);

	protected:
		virtual std::vector<int>& GetPatternOrder() = 0;
		virtual size_t& GetCurrentPatternIndex() = 0;
	};

	template <typename T>
	class EnemyBase : public IEnemy {
	protected:
		std::vector<int>& GetPatternOrder() override {
			static std::vector<int> patternOrder;
			return patternOrder;
		}
		size_t& GetCurrentPatternIndex() override {
			static size_t currentIndex = 0;
			return currentIndex;
		}
	public:
		using IEnemy::IEnemy;
	};
}