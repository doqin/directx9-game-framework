#include "pch.h"
#include "IStatementCard.h"

int Demo::IStatementCard::GetStatusCoverPanelWidth() const
{
	int contentWidth = 0;
	if (!IsPersistent()) {
		contentWidth += NON_PERSISTENT_BADGE_WIDTH;
	}
	if (HasLimitedUses() && GetMaxUses() > 0) {
		if (contentWidth > 0) {
			contentWidth += USES_PIP_GAP;
		}
		const int uses = GetMaxUses();
		contentWidth += uses * USES_PIP_SIZE + (uses - 1) * USES_PIP_GAP;
	}
	if (contentWidth <= 0) {
		return 0;
	}
	return contentWidth + USES_COVER_PAD_LEFT + USES_COVER_PAD_RIGHT + USES_COVER_CAP_WIDTH;
}

void Demo::IStatementCard::Init(std::shared_ptr<DraggableManager> manager, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera)
{
	IDraggable::Init(manager, graphicsDevice, camera);
	trigger->SetWidth(GetWidth());
}

size_t Demo::IStatementCard::GetStatusCoverWidth() const
{
	const int panelWidth = GetStatusCoverPanelWidth();
	if (panelWidth <= 0) {
		return 0;
	}
	// The overlapped part sits on top of the card, so it adds no width to the row.
	return static_cast<size_t>((panelWidth - USES_COVER_OVERLAP) * USES_COVER_SCALE);
}

size_t Demo::IStatementCard::GetWidth() const
{
	return IDraggable::GetWidth() + GetStatusCoverWidth();
}

void Demo::IStatementCard::DrawStatusCover(unsigned long long deltaTime)
{
	const size_t coverWidth = GetStatusCoverWidth();
	if (coverWidth == 0 || !trigger) {
		return;
	}

	if (!usesTexture) {
		usesTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		usesTexture->LoadTexture(L"assets/ui.png");
		// The panel body is a single uniform column (sheet columns 224-252 are identical), so
		// one slice stretched to any width reproduces it. Sampling from the middle keeps
		// filtering from bleeding in the transparent pixels beside the panel.
		usesCoverBody = std::make_shared<DX9GF::StaticSprite>(usesTexture.get());
		usesCoverBody->SetSrcRect({ .left = 236, .top = 272, .right = 237, .bottom = 288 });
		usesCoverCap = std::make_shared<DX9GF::StaticSprite>(usesTexture.get());
		usesCoverCap->SetSrcRect({ .left = 253, .top = 272, .right = 256, .bottom = 288 });
		usesPipFull = std::make_shared<DX9GF::StaticSprite>(usesTexture.get());
		usesPipFull->SetSrcRect({ .left = 277, .top = 277, .right = 283, .bottom = 283 });
		usesPipSpent = std::make_shared<DX9GF::StaticSprite>(usesTexture.get());
		usesPipSpent->SetSrcRect({ .left = 261, .top = 277, .right = 267, .bottom = 283 });
		nonPersistentBadge = std::make_shared<DX9GF::StaticSprite>(usesTexture.get());
		nonPersistentBadge->SetSrcRect({ .left = 259, .top = 290, .right = 269, .bottom = 302 });
	}

	const float scale = static_cast<float>(USES_COVER_SCALE);
	const float bodyWidth = static_cast<float>(GetStatusCoverPanelWidth() - USES_COVER_CAP_WIDTH);

	const int uses = GetMaxUses();
	const int remaining = GetRemainingUses();

	// Starts past everything else on the row - so attached enemy cards push it right rather
	// than being covered by it - then backs up over the card's right outline.
	const float panelX = trigger->GetWorldX() - trigger->GetOriginX()
		+ GetWidth() - coverWidth - USES_COVER_OVERLAP * scale;
	const float panelY = trigger->GetWorldY() - trigger->GetOriginY();

	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	graphicsDevice->SetAlphaBlending(true);

	usesCoverBody->Begin();
	usesCoverBody->SetPosition(panelX, panelY);
	usesCoverBody->SetScale(bodyWidth * scale, scale);
	usesCoverBody->Draw(*camera, deltaTime);
	usesCoverBody->End();

	usesCoverCap->Begin();
	usesCoverCap->SetPosition(panelX + bodyWidth * scale, panelY);
	usesCoverCap->SetScale(scale, scale);
	usesCoverCap->Draw(*camera, deltaTime);
	usesCoverCap->End();

	// Contents run left to right: the non-persistent badge first, then one pip per use.
	int contentOffset = USES_COVER_PAD_LEFT;
	if (!IsPersistent()) {
		nonPersistentBadge->Begin();
		nonPersistentBadge->SetPosition(panelX + contentOffset * scale, panelY + NON_PERSISTENT_BADGE_TOP * scale);
		nonPersistentBadge->SetScale(scale, scale);
		nonPersistentBadge->Draw(*camera, deltaTime);
		nonPersistentBadge->End();
		contentOffset += NON_PERSISTENT_BADGE_WIDTH + USES_PIP_GAP;
	}

	const float pipY = panelY + USES_PIP_TOP * scale;
	for (int i = 0; i < uses; ++i) {
		auto& pip = (i < remaining) ? usesPipFull : usesPipSpent;
		pip->Begin();
		pip->SetPosition(panelX + (contentOffset + i * (USES_PIP_SIZE + USES_PIP_GAP)) * scale, pipY);
		pip->SetScale(scale, scale);
		pip->Draw(*camera, deltaTime);
		pip->End();
	}

	graphicsDevice->SetAlphaBlending(false);
	if (isCropped) {
		graphicsDevice->SetScissorTest(false);
	}
}

void Demo::IStatementCard::DrawSheetFace(unsigned long long deltaTime, const RECT& srcRect)
{
	if (isCropped) {
		graphicsDevice->SetScissorRect(scissorRect);
		graphicsDevice->SetScissorTest(true);
	}
	if (!faceTexture) {
		faceTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		faceTexture->LoadTexture(L"assets/ui.png");
		faceSprite = std::make_shared<DX9GF::StaticSprite>(faceTexture.get());
		faceSprite->SetSrcRect(srcRect);
	}
	if (faceSprite) {
		faceSprite->Begin();
		faceSprite->SetPosition(GetWorldX(), GetWorldY());
		faceSprite->SetScale(2.f, 2.f);
		faceSprite->Draw(*camera, deltaTime);
		faceSprite->End();
	}
	if (isCropped) {
		graphicsDevice->SetScissorTest(false);
	}
}

void Demo::IStatementCard::Draw(unsigned long long deltaTime) {
	DrawStatusCover(deltaTime);
	DrawCardFace(deltaTime);

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
		}
		// The lock overlay shares this sprite and leaves its own colour and outline behind, so
		// re-apply the tooltip's styling every frame instead of only on first creation.
		descFontSprite->SetColor(0xFF000000);
		descFontSprite->SetOutline(false);
		std::wstring desc = GetDescription();
		size_t cost = GetCost();
		std::wstring inputs = GetInputsDescription();

		std::wstring text = desc + L"\nCost: " + std::to_wstring(cost) + L"\nInputs: " + inputs;
		if (HasLimitedUses()) {
			text += L"\nUses: " + std::to_wstring(GetRemainingUses()) + L"/" + std::to_wstring(GetMaxUses());
		}
		if (!IsPersistent()) {
			text += L"\nNon-persistent: leaves the program at end of turn.";
		}

		descFontSprite->SetText(std::move(text));

		if (auto manager = GetDraggableManager().lock()) {
			// The command is queued now and executed when the manager flushes its draw buffer.
			// If the card dies in between - pile-view cards are destroyed when that menu closes -
			// a captured raw 'this' would dangle, so gate the body on a weak reference.
			std::weak_ptr<DX9GF::IGameObject> self = weak_from_this();
			auto cmd = std::make_shared<DX9GF::CustomCommand>([this, self, deltaTime](std::function<void()> markFinished) {
				if (self.expired()) {
					markFinished();
					return;
				}
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
