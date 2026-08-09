#pragma once
#include "IBlockCard.h"

namespace Demo {
	// The turn's program. Cards placed here execute when Execute is pressed, which also ends the
	// turn, and persistent ones keep executing every turn until the program clears.
	class MainBlockCard : public IBlockCard {
	protected:
		RECT GetBlockFaceRect() const override { return { .left = 0, .top = 272, .right = 80, .bottom = 288 }; }
	public:
	   MainBlockCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), IBlockCard(tm, 160, 32, x, y) {}
	};
}
