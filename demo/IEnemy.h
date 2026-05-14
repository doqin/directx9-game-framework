#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "Player.h"
#include "IProjectile.h"
#include <functional>
#include <vector>

namespace Demo {
	enum class StatusType { POISON, VULNERABLE, WEAK, STUN };

	struct StatusEffect {
		StatusType type;
		int duration; 
		float value; 
	};

	class IEnemy : public DX9GF::IGameObject {
	private:
		const float maxHealth;
	protected:
     int goldReward = 0;
	   struct DamageIndicator {
			std::wstring text;
			float offsetX = 0.f;
			float offsetY = 0.f;
			float vx = 0.f;
			float vy = 0.f;
			unsigned long long elapsed = 0;
		};
		bool isOnStandby = true;
		float health;
        std::shared_ptr<DX9GF::RectangleTrigger> cardSpawnTrigger;
		std::function<void(std::shared_ptr<IEnemy>)> onRequestEnemyCard = [](std::shared_ptr<IEnemy>) {};
        std::vector<DamageIndicator> damageIndicators;
		std::shared_ptr<DX9GF::Font> font;
		std::shared_ptr<DX9GF::FontSprite> fontSprite;
		std::vector<std::shared_ptr<IProjectile>> projectiles;
		DX9GF::CommandBuffer commandBuffer;
		DX9GF::CommandBuffer animationBuffer;
		std::shared_ptr<DX9GF::Texture> hitImpactTexture;
		std::vector<std::shared_ptr<DX9GF::AnimatedSprite>> hitImpactSprites;
		std::vector<StatusEffect> activeStatuses;
		DX9GF::GraphicsDevice* graphicsDevice = nullptr;
		unsigned long long timeSinceStart = 0;
		void SetGoldReward(int reward) { goldReward = reward; }
	public:
		IEnemy(std::weak_ptr<DX9GF::TransformManager> tm, float maxHealth) : IGameObject(tm), maxHealth(maxHealth), health(maxHealth) {}
		IEnemy(std::weak_ptr<DX9GF::TransformManager> tm, float maxHealth, float x, float y, float rotation = 0, float scaleX = 1, float scaleY = 1) : IGameObject(tm, x, y, rotation, scaleX, scaleY), maxHealth(maxHealth), health(maxHealth) {}
		IEnemy(std::weak_ptr<DX9GF::TransformManager> tm, std::weak_ptr<DX9GF::IGameObject> parent, float maxHealth, float x, float y, float rotation = 0, float scaleX = 1, float scaleY = 1) : IGameObject(tm, parent, x, y, rotation, scaleX, scaleY), maxHealth(maxHealth), health(maxHealth) {}
        void InitCardSpawnTrigger(DX9GF::Camera* camera, float width, float height);
		void SetOnRequestEnemyCard(std::function<void(std::shared_ptr<IEnemy>)> callback);
		bool IsDead() const;
		virtual void Update(unsigned long long deltaTime);
		virtual void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime);
		virtual bool TakeDamage(float damage);
		virtual void StartAttack(std::shared_ptr<Player> player) = 0;
		void SetState(bool isOnStandby);
		bool IsDoneAttacking();
		int GetGoldReward() const { return goldReward; }
		float GetMaxHealth() const { return maxHealth; }

		virtual void ApplyStatus(StatusType type, int duration, float value = 0.0f);
		virtual void TickStatuses();
		bool HasStatus(StatusType type) const;
		float GetOutgoingDamage(float baseDamage) const;
		std::weak_ptr<DX9GF::RectangleTrigger> GetCardSpawnTrigger() const { return cardSpawnTrigger; }
	};
}