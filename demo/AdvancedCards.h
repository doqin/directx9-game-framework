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
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override { CollectHitsOnTargets(out, 16.f, 1); }

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
		// Two passes over the same target - Execute lands 3 damage twice.
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override {
			CollectHitsOnTargets(out, 3.f, 1);
			CollectHitsOnTargets(out, 3.f, 1);
		}

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
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override { CollectHitsOnTargets(out, 7.f); }

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
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override { CollectHitsOnTargets(out, 10.f, 3); }

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
		// No value of its own, so its tick is worth however many turns are on it - and AddModifier
		// adds to whatever is already there, so poisoning an already-poisoned enemy ticks harder.
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override {
			CollectEffectOnTargets(out, ModifierType::Poison, 0.f, 3, 1);
		}

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
		// Vulnerable has no value of its own - it is a flat 1.5x on everything queued after it.
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override {
			CollectEffectOnTargets(out, ModifierType::Vulnerable, 0.f, 1, 1);
		}

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

	class IgniteCard : public MultiTargetCard {
	public:
		IgniteCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			// TODO: adjust drag area
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Ignite", x, y, 160, 32) {
		}

		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Apply Burn 2 for 3 turns."; }

		// TODO: adjust asset's rect
		RECT GetFaceRect() const override { return RECT{ 192, 304, 272, 320 }; }

		bool Execute() override;
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override {
			CollectEffectOnTargets(out, ModifierType::Spark, 2.f, 3, 1);
		}
		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
	};

	class FireDetonationCard : public MultiTargetCard {
	public:
		FireDetonationCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			// TODO: adjust drag area
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Fire Deton.", x, y, 192, 32) {
			SetPersistent(false);
		}

		size_t GetCost() const override { return 2; }
		std::wstring GetDescription() const override { return L"Deal 3 damage. Consumes all Burn on target to deal 5 extra damage per stack."; }
		// TODO: adjust asset's rect
		RECT GetFaceRect() const override { return RECT{ 128, 272, 224, 288 }; }

		bool Execute() override;
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override {
			CollectHitsOnTargets(out, 3.f, 1);
		}
		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
	};

	class RagingStrikeCard : public MultiTargetCard {
	private:
		int currentDamage = 4;
		bool hasExecutedThisCycle = false;
	public:
		RagingStrikeCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			// TODO: adjust drag area
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Raging Strike", x, y, 192, 32) {
		}

		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override {
			return L"Deal " + std::to_wstring(currentDamage) + L" damage. Increases this card's damage by 3 when played.";
		}
		// TODO: adjust asset's rect
		RECT GetFaceRect() const override { return RECT{ 0, 288, 80, 304 }; }

		bool Execute() override;
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override {
			CollectHitsOnTargets(out, static_cast<float>(currentDamage), 1);
		}
		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
		void OnDiscard() override {
			currentDamage = 4;
		}
	};
	class OverloadCard : public MultiTargetCard {
	public:
		OverloadCard(std::weak_ptr<DX9GF::TransformManager> tm)
			// TODO: adjust drag area
			: IGameObject(tm, 0, 0), MultiTargetCard(tm, 1, L"Overload", 0, 0, 160, 32) {
			SetPersistent(false);
			SetMaxUses(1);
		}
		bool Execute() override;
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override;
		void DrawCardFace(unsigned long long deltaTime) override;
		size_t GetCost() const override { return 2; }
		std::wstring GetDescription() const override { return L"Deal 5 DMG. Deal +4 DMG for each Persistent card currently in the Block."; }
		// TODO: adjust asset's rect
		RECT GetFaceRect() const override { return RECT{ 128, 272, 224, 288 }; }
	};

	class ChainReactionCard : public MultiTargetCard {
	public:
		ChainReactionCard(std::weak_ptr<DX9GF::TransformManager> tm)
			// TODO: adjust drag area
			: IGameObject(tm, 0, 0), MultiTargetCard(tm, 1, L"Chain Reaction", 0, 0, 160, 32) {
			SetPersistent(true);
		}
		bool Execute() override;
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override;
		void DrawCardFace(unsigned long long deltaTime) override;
		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Deal 4 DMG. If the previous card killed its target, deal 8 DMG to the lowest HP enemy instead."; }
		// TODO: adjust asset's rect
		RECT GetFaceRect() const override { return RECT{ 0, 304, 80, 320 }; }
	};

	class LethalHarvestCard : public MultiTargetCard {
	public:
		LethalHarvestCard(std::weak_ptr<DX9GF::TransformManager> tm)
			// TODO: adjust drag area
			: IGameObject(tm, 0, 0), MultiTargetCard(tm, 1, L"Lethal Harvest", 0, 0, 160, 32) {
			SetPersistent(false);
			SetMaxUses(1);
		}
		bool Execute() override;
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override { CollectHitsOnTargets(out, 6.f, 1); }
		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Deal 6 DMG. If fatal, heal 8 HP."; }
		// TODO: adjust asset's rect
		RECT GetFaceRect() const override { return RECT{ 128, 288, 224, 304 }; }
	};

	class ExecuteCard : public MultiTargetCard {
	public:
		ExecuteCard(std::weak_ptr<DX9GF::TransformManager> tm)
			// TODO: adjust drag area
			: IGameObject(tm, 0, 0), MultiTargetCard(tm, 1, L"Execute", 0, 0, 160, 32) {
			SetPersistent(false);
		}
		bool Execute() override;
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override { CollectHitsOnTargets(out, 15.f, 1); }
		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
		size_t GetCost() const override { return 2; }
		std::wstring GetDescription() const override { return L"Deal 15 DMG. If fatal, gain 1 Energy next turn."; }
		// TODO: adjust asset's rect
		RECT GetFaceRect() const override { return RECT{ 128, 272, 224, 288 }; }
	};

	class ArmorPiercerCard : public MultiTargetCard {
	public:
		ArmorPiercerCard(std::weak_ptr<DX9GF::TransformManager> tm)
			// TODO: adjust drag area
			: IGameObject(tm, 0, 0), MultiTargetCard(tm, 1, L"Armor Piercer", 0, 0, 160, 32) {
			SetPersistent(true);
		}
		bool Execute() override;
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override;
		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Deal 5 DMG. Deals 2x damage to enemies with Armor."; }
		// TODO: adjust asset's rect
		RECT GetFaceRect() const override { return RECT{ 0, 304, 80, 320 }; }
	};

	class CruelStrikeCard : public MultiTargetCard {
	public:
		CruelStrikeCard(std::weak_ptr<DX9GF::TransformManager> tm)
			// TODO: adjust drag area
			: IGameObject(tm, 0, 0), MultiTargetCard(tm, 1, L"Cruel Strike", 0, 0, 160, 32) {
			SetPersistent(false);
		}
		bool Execute() override;
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override;
		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Deal 8 DMG. Deals 2x damage if target is Weak."; }
		// TODO: adjust asset's rect
		RECT GetFaceRect() const override { return RECT{ 96, 320, 192, 336 }; }
	};

	class ShieldBashCard : public MultiTargetCard {
	public:
		ShieldBashCard(std::weak_ptr<DX9GF::TransformManager> tm)
			// TODO: adjust drag area
			: IGameObject(tm, 0, 0), MultiTargetCard(tm, 1, L"Shield Bash", 0, 0, 160, 32) {
			SetPersistent(false);
		}
		bool Execute() override;
		void CollectProjectedSteps(std::vector<ProjectedStep>& out) override;
		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Consume all your Armor. Deal damage equal to 1.5x the consumed amount."; }
		// TODO: adjust asset's rect
		RECT GetFaceRect() const override { return RECT{ 96, 240, 112, 256 }; }
	};
}