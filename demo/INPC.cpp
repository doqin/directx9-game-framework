#include "pch.h"
#include "INPC.h"
#include "SettingsManager.h"

namespace Demo {
	void INPC::Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) {
		player = p;
		this->gd = gd;
		this->worldCamera = camera;
		fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
		this->drawBuffer = drawBuffer;
		MapCharacterVoices(characterVoices);
		if (questMarker) questMarker->Init(gd);
	}

	void INPC::Update(unsigned long long deltaTime) {
		auto pLock = player.lock();
		if (!pLock) return;

		auto [px, py] = pLock->GetWorldPosition();
		auto [sx, sy] = this->GetWorldPosition();

		float distance = std::sqrt((px - sx) * (px - sx) + (py - sy) * (py - sy));
		this->isPlayerNear = (distance <= this->INTERACTION_DISTANCE);

		if (questMarker) {
			questMarker->Update(deltaTime);
		}
	}

	void INPC::Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) {
		DrawPosition(deltaTime, gd, camera);
	}

	void INPC::DrawUI(DX9GF::Camera* uiCamera, unsigned long long deltaTime) {
		if (!fontSprite || !uiCamera || !worldCamera) return;

		auto [worldX, worldY] = GetWorldPosition();
		float zoom = worldCamera->GetZoom();
		float uiX = (worldX - worldCamera->GetPosition().x) * zoom;
		float uiY = (worldY - worldCamera->GetPosition().y) * zoom;
		float scale = 1.0f * zoom;

		// FIX LỘT VIỀN ĐEN: Lấy kích thước Virtual từ World Camera (vì nó không bị dãn khi Resize)
		auto [vw, vh] = worldCamera->GetScreenResolution();
		float halfW = vw / 2.0f;
		float halfH = vh / 2.0f;

		//culling
		if (uiX < -halfW || uiX > halfW ||
			uiY < -halfH || uiY > halfH) {
			return;
		}

		if (questMarker) {
			float markerX = uiX - (markerOffsetX * zoom);
			float markerY = uiY - (markerOffsetY * zoom);
			questMarker->Draw(uiCamera, markerX, markerY, scale);
		}

		if (isPlayerNear) {
			fontSprite->Begin();
			fontSprite->SetText(SettingsManager::GetInstance()->GetKeybindDisplayName("INTERACT"));
			fontSprite->SetScale(scale);
			fontSprite->SetColor(0xFFFFFFFF);

			float textW = fontSprite->GetWidth() * scale;

			fontSprite->SetPosition(uiX - textW / 2.f, uiY - uiOffsetY * zoom);

			fontSprite->SetOutline(true, 0xFF000000);
			fontSprite->Draw(*uiCamera, deltaTime);
			fontSprite->End();
		}
	}
	void INPC::AddLine(std::wstring name, std::wstring content) {
		DialogueLine line;
		line.name = name;
		line.content = content;
		line.voiceClip = characterVoices.count(name) ? std::optional<std::string>(characterVoices[name]) : std::nullopt;
		dialogueLines.push_back(line);
	}
}