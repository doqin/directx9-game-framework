#include "pch.h"
#include "GearSlotUI.h"
#include "GameItems.h"
namespace Demo {
	GearSlotUI::GearSlotUI(std::shared_ptr<DX9GF::TransformManager> tm, DX9GF::Camera* cam,
		std::shared_ptr<DX9GF::Texture> uiTex,
		std::shared_ptr<DX9GF::Texture> flameTex,
		std::shared_ptr<DX9GF::Texture> gearTex,
		bool coreSlot, float uiScale)
	{
		this->isCore = coreSlot;
		this->scale = uiScale;
		float size = 16.0f * scale;

		button = std::make_shared<IconButton>(tm, 0.0f, 0.0f, static_cast<int>(size), static_cast<int>(size), uiTex);
		button->SetSpriteRects({ {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} });
		button->Init(cam);

		frameSprite = std::make_shared<DX9GF::StaticSprite>(uiTex.get());
		if (isCore) frameSprite->SetSrcRect({ 192, 144, 208, 160 });
		else frameSprite->SetSrcRect({ 208, 144, 224, 160 });
		frameSprite->SetScale(scale, scale);

		glowSprite = std::make_shared<DX9GF::StaticSprite>(uiTex.get());
		glowSprite->SetSrcRect({ 224, 144, 240, 160 });
		glowSprite->SetScale(scale, scale);

		gearSprite = std::make_shared<DX9GF::StaticSprite>(gearTex.get());
		gearSprite->SetSrcRect({ 7, 7, 19, 19 });
		gearSprite->SetScale(scale, scale);

		std::vector<RECT> flameFrames = {
			{0, 0, 16, 16}, {16, 0, 32, 16}, {32, 0, 48, 16}, {48, 0, 64, 16}
		};
		flameAnim = std::make_shared<DX9GF::AnimatedSprite>(flameTex.get(), flameFrames);
		flameAnim->SetFrameRate(10);
		flameAnim->SetScale(scale, scale);
	}

	void GearSlotUI::SetLocalPosition(float newX, float newY) {
		x = newX;
		y = newY;
		button->SetLocalPosition(x, y);
	}

	void GearSlotUI::SetGearID(int id) {
		gearID = id;
		if (gearID != -1) {
			auto blueprint = Demo::ItemData::GetInstance()->GetGearBlueprint(gearID);
			if (blueprint && !blueprint->frames.empty()) {
				gearSprite->SetSrcRect(blueprint->frames[0]);
			}
		}
	}

	void GearSlotUI::Update(unsigned long long deltaTime) {
		button->Update(deltaTime);
	}

	void GearSlotUI::Draw(DX9GF::GraphicsDevice* gd, DX9GF::Camera* cam, unsigned long long deltaTime) {
		button->Draw(gd, deltaTime);

		if (gearID == -1) {
			frameSprite->SetPosition(x, y);
			frameSprite->Begin(); frameSprite->Draw(*cam, deltaTime); frameSprite->End();
		}
		else {
			Demo::IButton::ButtonState state = button->GetState();

			if (isCore || state == Demo::IButton::ButtonState::HOVER) {
				flameAnim->SetPosition(x, y);
				flameAnim->Begin(); flameAnim->Draw(*cam, deltaTime); flameAnim->End();
			}
			else {
				glowSprite->SetPosition(x, y);
				glowSprite->Begin(); glowSprite->Draw(*cam, deltaTime); glowSprite->End();
			}

			float offset = 2.0f * scale;
			gearSprite->SetPosition(x + offset, y + offset);
			gearSprite->Begin(); gearSprite->Draw(*cam, deltaTime); gearSprite->End();
		}
	}
}