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
		void CollectProjectedSteps(VirtualBattleState& state) override { CollectHitsOnTargets(state, 16.f, 1); }

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
		void CollectProjectedSteps(VirtualBattleState& state) override {
			CollectHitsOnTargets(state, 3.f, 1);
			CollectHitsOnTargets(state, 3.f, 1);
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
		void CollectProjectedSteps(VirtualBattleState& state) override { CollectHitsOnTargets(state, 7.f); }

		void Draw(unsigned long long deltaTime) override;
	};

	class ChainLightningCard : public MultiTargetCard {
		std::shared_ptr<DX9GF::Texture> strikeTexture;
		std::shared_ptr<DX9GF::StaticSprite> strikeSprite;
		const float baseDamage = 20.f;
		const float damageReductionPerRepeat = 75.f;
	public:
		ChainLightningCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 3, L"Chain L.", x, y, 224, 32) {
		}

		size_t GetCost() const override { return 3; }
		std::wstring GetDescription() const override { return std::format(L"Deal {} damage to up to 3 enemies, but base damage is reduced by {}% \neach time it targets the same enemy.", baseDamage, damageReductionPerRepeat); }
		RECT GetFaceRect() const override { return RECT{ 80, 304, 192, 320 }; }

		bool Execute() override;
		void CollectProjectedSteps(VirtualBattleState& state) override;

		void Draw(unsigned long long deltaTime) override;
	};

	// EFFECT CARDS

	class PoisonCard : public MultiTargetCard {
		std::shared_ptr<DX9GF::Texture> strikeTexture;
		std::shared_ptr<DX9GF::StaticSprite> strikeSprite;
		const int poisonTurns = 2;
	public:
		PoisonCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Poison", x, y, 160, 32) {
			SetPersistent(false);
		}

		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return std::format(L"Apply Poison {} (damage equals remaining turns).", poisonTurns); }
		RECT GetFaceRect() const override { return RECT{ 192, 304, 272, 320 }; }

		bool Execute() override;
		// No value of its own, so its tick is worth however many turns are on it - and AddModifier
		// adds to whatever is already there, so poisoning an already-poisoned enemy ticks harder.
		void CollectProjectedSteps(VirtualBattleState& state) override {
			CollectEffectOnTargets(state, ModifierType::Poison, 0.f, poisonTurns, 1);
		}

		void DrawCardFace(unsigned long long deltaTime) override;
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
		void CollectProjectedSteps(VirtualBattleState& state) override {
			CollectEffectOnTargets(state, ModifierType::Vulnerable, 0.f, 1, 1);
		}

		void DrawCardFace(unsigned long long deltaTime) override;
	};

	class WeaknessCard : public MultiTargetCard {
		std::shared_ptr<DX9GF::Texture> strikeTexture;
		std::shared_ptr<DX9GF::StaticSprite> strikeSprite;
	public:
		WeaknessCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Weakness", x, y, 192, 32) {
			SetPersistent(false);
		}

		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Apply Weak 2 to an enemy."; }
		RECT GetFaceRect() const override { return RECT{ 96, 320, 192, 336 }; }

		bool Execute() override;

		void DrawCardFace(unsigned long long deltaTime) override;
		void CollectProjectedSteps(VirtualBattleState& state) override {
			CollectEffectOnTargets(state, ModifierType::Weak, 0.f, 2, 2);
		}
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
		RECT GetFaceRect() const override { return RECT{ 192, 320, 272, 336 }; }

		bool Execute() override;

		void Draw(unsigned long long deltaTime) override;
		void CollectProjectedSteps(VirtualBattleState& state) override {
			CollectEffectOnTargets(state, ModifierType::Stun, 0.f, 1, 1);
		}
	};

	class IgniteCard : public MultiTargetCard {
	public:
		IgniteCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Ignite", x, y, 160, 32) {
		}

		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Apply Spark 2 for 3 turns."; }

		RECT GetFaceRect() const override { return RECT{ 80, 400, 160, 416 }; }

		bool Execute() override;
		void CollectProjectedSteps(VirtualBattleState& state) override {
			CollectEffectOnTargets(state, ModifierType::Spark, 2.f, 3, 1);
		}
		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
	};

	class FireDetonationCard : public MultiTargetCard {
	public:
		FireDetonationCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Fire Deton.", x, y, 224, 32) {
			SetPersistent(false);
		}

		size_t GetCost() const override { return 2; }
		std::wstring GetDescription() const override { return L"Deal 3 damage. Consumes all Spark on target to deal 5 extra damage per stack."; }
		RECT GetFaceRect() const override { return RECT{ 160, 400, 272, 416 }; }

		bool Execute() override;
		void CollectProjectedSteps(VirtualBattleState& state) override;

		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
	};

	class RagingStrikeCard : public MultiTargetCard {
	private:
		int currentDamage = 4;
		bool hasExecutedThisCycle = false;
	public:
		RagingStrikeCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Raging Strike", x, y, 192, 32) {
		}

		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override {
			return L"Deal " + std::to_wstring(currentDamage) + L" damage. Increases this card's damage by 3 when played. Resets to 4 damage on discard.";
		}
		RECT GetFaceRect() const override { return RECT{ 80, 416, 176, 432 }; }

		bool Execute() override;
		void CollectProjectedSteps(VirtualBattleState& state) override {
			CollectHitsOnTargets(state, static_cast<float>(currentDamage), 1);
		}
		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
		void OnDiscard() override {
			currentDamage = 4;
		}
	};
	class OverloadCard : public MultiTargetCard {
	public:
		OverloadCard(std::weak_ptr<DX9GF::TransformManager> tm)
			: IGameObject(tm, 0, 0), MultiTargetCard(tm, 1, L"Overload", 0, 0, 160, 32) {
			SetPersistent(false);
			//SetMaxUses(1);
		}
		bool Execute() override;
		void CollectProjectedSteps(VirtualBattleState& state) override;
		void DrawCardFace(unsigned long long deltaTime) override;
		size_t GetCost() const override { return 2; }
		std::wstring GetDescription() const override { return L"Deal 5 DMG. Deal +4 DMG for each Persistent card currently in the Block."; }
		RECT GetFaceRect() const override { return RECT{ 0, 416, 80, 432 }; }
	};

	class ChainReactionCard : public MultiTargetCard {
	public:
		ChainReactionCard(std::weak_ptr<DX9GF::TransformManager> tm)
			: IGameObject(tm, 0, 0), MultiTargetCard(tm, 1, L"Chain Reaction", 0, 0, 192, 32) {
			SetPersistent(true);
		}
		bool Execute() override;
		void CollectProjectedSteps(VirtualBattleState& state) override;
		void DrawCardFace(unsigned long long deltaTime) override;
		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Deal 4 DMG. If the previous card killed its target, deal 8 DMG to the lowest HP enemy instead."; }
		RECT GetFaceRect() const override { return RECT{ 176, 416, 272, 432 }; }
	};

	class LethalHarvestCard : public MultiTargetCard {
	public:
		LethalHarvestCard(std::weak_ptr<DX9GF::TransformManager> tm)
			: IGameObject(tm, 0, 0), MultiTargetCard(tm, 1, L"Lethal Harvest", 0, 0, 192, 32) {
			SetPersistent(false);
			SetMaxUses(1);
		}
		bool Execute() override;
		void CollectProjectedSteps(VirtualBattleState& state) override { CollectHitsOnTargets(state, 6.f, 1); }
		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Deal 6 DMG. If fatal, heal 8 HP."; }
		RECT GetFaceRect() const override { return RECT{ 80, 448, 176, 464 }; }
	};

	class ExecuteCard : public MultiTargetCard {
	public:
		ExecuteCard(std::weak_ptr<DX9GF::TransformManager> tm)
			: IGameObject(tm, 0, 0), MultiTargetCard(tm, 1, L"Execute", 0, 0, 160, 32) {
			SetPersistent(false);
		}
		bool Execute() override;
		void CollectProjectedSteps(VirtualBattleState& state) override { CollectHitsOnTargets(state, 15.f, 1); }
		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
		size_t GetCost() const override { return 2; }
		std::wstring GetDescription() const override { return L"Deal 15 DMG. If fatal, gain 1 Energy next turn."; }
		RECT GetFaceRect() const override { return RECT{ 0, 448, 80, 464 }; }
	};

	class ArmorPiercerCard : public MultiTargetCard {
	public:
		ArmorPiercerCard(std::weak_ptr<DX9GF::TransformManager> tm)
			: IGameObject(tm, 0, 0), MultiTargetCard(tm, 1, L"Armor Piercer", 0, 0, 192, 32) {
			SetPersistent(true);
		}
		bool Execute() override;
		void CollectProjectedSteps(VirtualBattleState& state) override;
		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Deal 5 DMG. Deals 2x damage to enemies with Armor. Ignores their defense."; }
		RECT GetFaceRect() const override { return RECT{ 0, 432, 96, 448 }; }
	};

	class CruelStrikeCard : public MultiTargetCard {
	public:
		CruelStrikeCard(std::weak_ptr<DX9GF::TransformManager> tm)
			: IGameObject(tm, 0, 0), MultiTargetCard(tm, 1, L"Cruel Strike", 0, 0, 192, 32) {
			SetPersistent(false);
		}
		bool Execute() override;
		void CollectProjectedSteps(VirtualBattleState& state) override;
		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Deal 8 DMG. Deals 2x damage if target is Weak."; }
		RECT GetFaceRect() const override { return RECT{ 96, 432, 192, 448 }; }
	};

	class ShieldBashCard : public MultiTargetCard {
	public:
		ShieldBashCard(std::weak_ptr<DX9GF::TransformManager> tm)
			: IGameObject(tm, 0, 0), MultiTargetCard(tm, 1, L"Shield Bash", 0, 0, 160, 32) {
			SetPersistent(false);
		}
		bool Execute() override;
		void CollectProjectedSteps(VirtualBattleState& state) override;
		void DrawCardFace(unsigned long long deltaTime) override { DrawSheetFace(deltaTime, GetFaceRect()); }
		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Consume all your Armor. Deal damage equal to 1.5x the consumed amount."; }
		RECT GetFaceRect() const override { return RECT{ 192, 432, 272, 448 }; }
	};
}