#pragma once
#include "IStatementCard.h"

namespace Demo {
	// Single-use card that takes no targets: spend 1 energy now to start the next turn with
	// one extra energy. Once executed it is depleted and moves to the battle's nullified pile.
	class EnergyCard : public IStatementCard {
	private:
		bool isDone = false;
		std::shared_ptr<DX9GF::Texture> cardTexture;
		std::shared_ptr<DX9GF::StaticSprite> cardSprite;
	public:
		EnergyCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), IStatementCard(tm, 160, 32, x, y) {
			//SetMaxUses(1);
		}

		size_t GetCost() const override { return 1; }
		std::wstring GetDescription() const override { return L"Gain 1 extra energy next turn."; }

		bool Execute() override;
		void ResetExecution() override;
		void DrawCardFace(unsigned long long deltaTime) override;
	};
}
