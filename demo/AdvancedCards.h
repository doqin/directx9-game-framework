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
		std::wstring GetDescription() const override { return L"Deal 16 damage to an enemy."; }
		RECT GetFaceRect() const override { return RECT{ 128, 272, 224, 288 }; }

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
		RECT GetFaceRect() const override { return RECT{ 128, 288, 224, 304 }; }

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
		std::wstring GetDescription() const override { return L"Deal 7 damage to up to 2 enemies."; }
		RECT GetFaceRect() const override { return RECT{ 0, 304, 80, 320 }; }

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
		std::wstring GetDescription() const override { return L"Deal 10 damage to up to 3 enemies."; }
		RECT GetFaceRect() const override { return RECT{ 80, 304, 192, 320 }; }

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
		RECT GetFaceRect() const override { return RECT{ 192, 304, 272, 320 }; }

		bool Execute() override;

		void Draw(unsigned long long deltaTime) override;
	};

	class VulnerableCard : public MultiTargetCard {
		std::shared_ptr<DX9GF::Texture> strikeTexture;
		std::shared_ptr<DX9GF::StaticSprite> strikeSprite;
	public:
		VulnerableCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Vulnerable", x, y, 192, 32) {
			SetPersistent(false);
		}

		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Apply Vulnerable 1 to an enemy."; }
		RECT GetFaceRect() const override { return RECT{ 0, 320, 96, 336 }; }

		bool Execute() override;

		void DrawCardFace(unsigned long long deltaTime) override;
	};

	class WeaknessCard : public MultiTargetCard {
		std::shared_ptr<DX9GF::Texture> strikeTexture;
		std::shared_ptr<DX9GF::StaticSprite> strikeSprite;
	public:
		WeaknessCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 2, L"Weakness", x, y, 192, 32) {
			SetPersistent(false);
		}

		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Apply Weak 2 to up to 2 enemies."; }
		RECT GetFaceRect() const override { return RECT{ 96, 320, 192, 336 }; }

		bool Execute() override;

		void DrawCardFace(unsigned long long deltaTime) override;
	};

	class StunCard : public MultiTargetCard {
		std::shared_ptr<DX9GF::Texture> strikeTexture;
		std::shared_ptr<DX9GF::StaticSprite> strikeSprite;
	public:
		StunCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Stun", x, y, 160, 32) {
		}

		size_t GetCost() const override { return 3; }
		std::wstring GetDescription() const override { return L"Stun an enemy for 1 turn."; }
		RECT GetFaceRect() const override { return RECT{ 192, 320, 272, 352 }; }

		bool Execute() override;

		void Draw(unsigned long long deltaTime) override;
	};
}