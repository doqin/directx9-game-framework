#pragma once
#include "DX9GF.h"
#include "IconButton.h"
#include <memory>
#include <vector>

namespace Demo {
	class GearSlotUI {
	private:
		std::shared_ptr<IconButton> button;
		int gearID = -1;
		bool isCore = false;
		float scale = 3.0f;
		float x = 0, y = 0;

		std::shared_ptr<DX9GF::StaticSprite> frameSprite;
		std::shared_ptr<DX9GF::StaticSprite> glowSprite;
		std::shared_ptr<DX9GF::AnimatedSprite> flameAnim;
		std::shared_ptr<DX9GF::StaticSprite> gearSprite;

	public:
	public:
		GearSlotUI(std::shared_ptr<DX9GF::TransformManager> tm, DX9GF::Camera* cam,
			std::shared_ptr<DX9GF::Texture> uiTex,
			std::shared_ptr<DX9GF::Texture> flameTex,
			std::shared_ptr<DX9GF::Texture> gearTex,
			bool coreSlot, float uiScale = 3.0f);

		void SetLocalPosition(float newX, float newY);
		void SetGearID(int id);
		int GetGearID() const { return gearID; }

		void Update(unsigned long long deltaTime);
		void Draw(DX9GF::GraphicsDevice* gd, DX9GF::Camera* cam, unsigned long long deltaTime);

		std::shared_ptr<IconButton> GetButton() { return button; }
	};
}