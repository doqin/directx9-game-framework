#pragma once
#include "MultiTargetCard.h"

namespace Demo {

	// OFFENSIVE CARDS

	class HeavyStrikeCard : public MultiTargetCard {
		std::shared_ptr<DX9GF::Texture> strikeTexture;
		std::shared_ptr<DX9GF::StaticSprite> strikeSprite;
	public:
		HeavyStrikeCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Heavy Strike", x, y, 192, 32) {
		}

		size_t GetCost() const override { return 2; }
		std::wstring GetDescription() const override { return L"Deal 12 damage to an enemy."; }

		bool Execute() override;

		void Draw(unsigned long long deltaTime) override;
	};

	class TwinStrikeCard : public MultiTargetCard {
	private:
		int hits = 0;
		std::shared_ptr<DX9GF::Texture> strikeTexture;
		std::shared_ptr<DX9GF::StaticSprite> strikeSprite;
	public:
		TwinStrikeCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Twin Strike", x, y, 192, 32) {
		}

		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Deal 3 damage to an enemy twice."; }

		bool Execute() override;

		void Draw(unsigned long long deltaTime) override;

		void ResetExecution() override;
	};

	class CleaveCard : public MultiTargetCard {
		std::shared_ptr<DX9GF::Texture> strikeTexture;
		std::shared_ptr<DX9GF::StaticSprite> strikeSprite;
	public:
		CleaveCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 2, L"Cleave", x, y, 160, 32) {
		}

		size_t GetCost() const override { return 2; }
		std::wstring GetDescription() const override { return L"Deal 4 damage to up to 2 enemies."; }

		bool Execute() override;

		void Draw(unsigned long long deltaTime) override;
	};

	class ChainLightningCard : public MultiTargetCard {
		std::shared_ptr<DX9GF::Texture> strikeTexture;
		std::shared_ptr<DX9GF::StaticSprite> strikeSprite;
	public:
		ChainLightningCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 3, L"Chain L.", x, y, 224, 32) {
		}

		size_t GetCost() const override { return 3; }
		std::wstring GetDescription() const override { return L"Deal 6, 4, 2 damage to up to 3 enemies."; }

		bool Execute() override;

		void Draw(unsigned long long deltaTime) override;
	};

	// EFFECT CARDS

	class PoisonCard : public MultiTargetCard {
		std::shared_ptr<DX9GF::Texture> strikeTexture;
		std::shared_ptr<DX9GF::StaticSprite> strikeSprite;
	public:
		PoisonCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Poison", x, y, 160, 32) {
		}

		size_t GetCost() const override { return 1; }
        std::wstring GetDescription() const override { return L"Apply Poison 3 (damage equals remaining turns)."; }

		bool Execute() override;

		void Draw(unsigned long long deltaTime) override;
	};

	class VulnerableCard : public MultiTargetCard {
		std::shared_ptr<DX9GF::Texture> strikeTexture;
		std::shared_ptr<DX9GF::StaticSprite> strikeSprite;
	public:
		VulnerableCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Vulnerable", x, y, 192, 32) {
		}

		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Apply Vulnerable 2 to an enemy."; }

		bool Execute() override;

		void Draw(unsigned long long deltaTime) override;
	};

	class WeaknessCard : public MultiTargetCard {
		std::shared_ptr<DX9GF::Texture> strikeTexture;
		std::shared_ptr<DX9GF::StaticSprite> strikeSprite;
	public:
		WeaknessCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 2, L"Weakness", x, y, 192, 32) {
		}

		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Apply Weak 2 to up to 2 enemies."; }

		bool Execute() override;

		void Draw(unsigned long long deltaTime) override;
	};

	class StunCard : public MultiTargetCard {
		std::shared_ptr<DX9GF::Texture> strikeTexture;
		std::shared_ptr<DX9GF::StaticSprite> strikeSprite;
	public:
		StunCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Stun", x, y, 160, 32) {
		}

		size_t GetCost() const override { return 2; }
		std::wstring GetDescription() const override { return L"Stun an enemy for 1 turn."; }

		bool Execute() override;

		void Draw(unsigned long long deltaTime) override;
	};
}