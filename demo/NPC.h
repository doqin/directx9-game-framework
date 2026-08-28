#pragma once
#include "pch.h"
#include "INPC.h"
#include <functional>

namespace Demo {

	//npc info
	struct NPCConfig {
		std::wstring texturePath;
		int frameW, frameH, frameCount,
		animSpeed; //frames per second
		float colW, colH, colOffsetY;
	};

	class NPC : public INPC {
	public:
		using InteractLogicFunc = std::function<std::function<void()>(NPC*)>;

	private:
		NPCConfig config;
		InteractLogicFunc interactLogic;

	protected:
		void MapCharacterVoices(std::unordered_map<std::wstring, std::string>& voiceMap) override {}

	public:
		NPC(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y, const NPCConfig& config);

		void Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) override;
		void Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) override;
		bool CanInteract() const override { return isPlayerNear; }

		void RegisterVoice(const std::wstring& name, const std::string& voiceId) {
			characterVoices[name] = voiceId;
		}

		void SetInteractLogic(InteractLogicFunc logic) { interactLogic = logic; }

		std::function<void()> TriggerInteract() {
			if (interactLogic) {
				return interactLogic(this);
			}
			return nullptr;
		}
	};
}