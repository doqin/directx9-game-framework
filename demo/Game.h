#pragma once
#include "DX9GF.h"

namespace Demo {
	class Game : public DX9GF::IGame {
	public:
	Game(HWND hwnd, const UINT screenWidth, const UINT screenHeight) : IGame(hwnd, screenWidth, screenHeight) {}
	void Init() override;
	void Update(unsigned long long deltaTime) override;
	UINT GetVirtualWidth() const { return SCREEN_WIDTH; }
	UINT GetVirtualHeight() const { return SCREEN_HEIGHT; }

	private:
		unsigned long long fpsAccumulator = 0;
		int fpsFrames = 0;
	};

}