#include "pch.h"
#include "IStatementCard.h"

void Demo::IStatementCard::Draw(unsigned long long deltaTime) {
	IDraggable::Draw(deltaTime);

	if (this->IsLocked()) {
		auto thisX = trigger->GetWorldX() - trigger->GetOriginX();
		auto thisY = trigger->GetWorldY() - trigger->GetOriginY();

		graphicsDevice->SetAlphaBlending(true);
		graphicsDevice->DrawRectangle(*camera, thisX, thisY, trigger->GetWidth(), trigger->GetHeight(), D3DCOLOR_ARGB(180, 50, 50, 50), true);
		graphicsDevice->SetAlphaBlending(false);

		if (!descFont) {
			descFont = std::make_shared<DX9GF::Font>(graphicsDevice, L"StatusPlz", 16);
			descFontSprite = std::make_shared<DX9GF::FontSprite>(descFont.get());
		}

		descFontSprite->Begin();
		descFontSprite->SetColor(0xFFff4444);
		descFontSprite->SetOutline(true, 0xFF000000, 2.f);
		descFontSprite->SetPosition(thisX + trigger->GetWidth() / 2 - 35, thisY + trigger->GetHeight() / 2 - 10);
		descFontSprite->SetText(L"LOCKED(" + std::to_wstring(this->GetLockedTurns()) + L")");
		descFontSprite->Draw(*camera, deltaTime);
		descFontSprite->End();
	}

	if (trigger && trigger->IsHovering(deltaTime)) {
		if (!descFont) {
			descFont = std::make_shared<DX9GF::Font>(graphicsDevice, L"StatusPlz", 16);
			descFontSprite = std::make_shared<DX9GF::FontSprite>(descFont.get());
			descFontSprite->SetColor(0xFF000000);
		}
		std::wstring desc = GetDescription();
		size_t cost = GetCost();
		std::wstring inputs = GetInputsDescription();

		std::wstring text = desc + L"\nCost: " + std::to_wstring(cost) + L"\nInputs: " + inputs;

		descFontSprite->SetText(std::move(text));

		if (auto manager = GetDraggableManager().lock()) {
			auto cmd = std::make_shared<DX9GF::CustomCommand>([this, deltaTime](std::function<void()> markFinished) {
				float textWidth = static_cast<float>(descFontSprite->GetWidth());
				float textHeight = static_cast<float>(descFontSprite->GetHeight());

				float textX = GetWorldX();
				float textY = GetWorldY() + static_cast<float>(GetHeight()) + 4.f;
				float padding = 4.f;

				graphicsDevice->DrawRectangle(
					*camera,
					textX - padding,
					textY - padding,
					textWidth + padding * 2,
					textHeight + padding * 2,
					0xFFE0E0E0,
					true
				);

				graphicsDevice->DrawRectangle(
					*camera,
					textX - padding,
					textY - padding,
					textWidth + padding * 2,
					textHeight + padding * 2,
					0xFF000000,
					false
				);

				descFontSprite->Begin();
				descFontSprite->SetPosition(textX, textY);
				descFontSprite->Draw(*camera, deltaTime);
				descFontSprite->End();
				markFinished();
				});
			manager->QueueDraw(cmd);
		}
	}
}
