#pragma once
#include <memory>
#include "Player.h"
#include <DX9GF.h>
#include <DX9GFExtras.h>

namespace Demo {
	class Pointable {
		static std::shared_ptr<DX9GF::Font> font;
		static std::shared_ptr<DX9GF::FontSprite> fontSprite;
	public:
		static bool isDrawing;
		virtual std::pair<float, float> GetPoint() const = 0;
		void DrawPosition(unsigned long long deltaTime, DX9GF::GraphicsDevice* gd, const DX9GF::Camera& camera) const;
	};

	void CreateImGuiDebugFrame(std::shared_ptr<Player> player);
}
