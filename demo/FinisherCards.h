#pragma once
#include "IStatementCard.h"
#include "MultiTargetCard.h"

namespace Demo {

	// The 4-5 cost tier. MAX_ENERGY is 3, so none of these can be played off a plain turn - they
	// need a ramp card resolved first (EnergyCard for next turn, Jumpstart or Overclock for this
	// one). Persistence is the balance lever: Terminate and Inferno are installs that keep firing
	// for the rest of the cycle, while the two effects that would be degenerate three turns
	// running are non-persistent and have to be paid for again.

	class TerminateCard : public MultiTargetCard {
	public:
		TerminateCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), MultiTargetCard(tm, 1, L"Terminate", x, y, 160, 32) {
		}

		size_t GetCost() const override { return 3; }
		std::wstring GetDescription() const override { return L"Deal 30 damage to an enemy, or 60 if it is below 40% health."; }
		RECT GetFaceRect() const override { return RECT{ 160, 368, 240, 384 }; }

		bool Execute() override;
		void CollectProjectedSteps(VirtualBattleState& state) override;
		void DrawCardFace(unsigned long long deltaTime) override;
	};

	// Hits every living enemy with no EnemyCard targets attached, and stacks its Burn each turn
	// it re-fires, so installing it early in a cycle is worth much more than late.
	class InfernoCard : public IStatementCard {
	private:
		bool isDone = false;
	public:
		InfernoCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), IStatementCard(tm, 160, 32, x, y) {
		}

		size_t GetCost() const override { return 3; }
		std::wstring GetDescription() const override { return L"Deal 8 damage to all enemies and apply Burn 5 for 3 turns."; }
		RECT GetFaceRect() const override { return RECT{ 0, 384, 80, 400 }; }

		bool Execute() override;
		void CollectProjectedSteps(VirtualBattleState& state) override;
		void ResetExecution() override;
		void DrawCardFace(unsigned long long deltaTime) override;
	};

	class SystemPurgeCard : public IStatementCard {
	private:
		bool isDone = false;
	public:
		SystemPurgeCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), IStatementCard(tm, 192, 32, x, y) {
			SetPersistent(false);
		}

		size_t GetCost() const override { return 4; }
		std::wstring GetDescription() const override { return L"Deal 16 damage to all enemies and stun them for 1 turn."; }
		RECT GetFaceRect() const override { return RECT{ 80, 384, 176, 400 }; }

		bool Execute() override;
		void CollectProjectedSteps(VirtualBattleState& state) override;
		void ResetExecution() override;
		void DrawCardFace(unsigned long long deltaTime) override;
	};

	class OverdriveCard : public IStatementCard {
	private:
		bool isDone = false;
		const float attackBuff = 2.f;
		const float turns = 3.f;
		const float regenBuff = 6.f;
	public:
		OverdriveCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), IStatementCard(tm, 160, 32, x, y) {
			SetPersistent(false);
		}

		size_t GetCost() const override { return 3; }
		std::wstring GetDescription() const override { return std::format(L"Gain {} attack and Regen {} for {} turns.", attackBuff, regenBuff, turns); }
		RECT GetFaceRect() const override { return RECT{ 176, 384, 256, 400 }; }

		bool Execute() override;
		// The attack buff lands mid-program, so every hit queued after this one gains 4. Regen
		// only heals the player, so it does not show up in the enemies' readout.
		void CollectProjectedSteps(VirtualBattleState& state) override;
		void ResetExecution() override;
		void DrawCardFace(unsigned long long deltaTime) override;
	};
}
