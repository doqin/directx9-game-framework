#pragma once
#include "IBlockCard.h"

namespace Demo {
	// The setup program. Runs on demand without ending the turn, so its cards resolve while the
	// player is still building the main block - which is what lets a card grant energy or draw
	// that can actually be spent this turn. Anything run from here is discarded afterwards, so
	// nothing placed in the init block ever repeats.
	class InitBlockCard : public IBlockCard {
	protected:
		RECT GetBlockFaceRect() const override { return { .left = 0, .top = 400, .right = 80, .bottom = 416 }; }
	public:
		InitBlockCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), IBlockCard(tm, 160, 32, x, y) {}
	};
}
