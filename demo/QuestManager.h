#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "IconButton.h"
#include "Player.h"
#include <nlohmann/json.hpp>
#include <string>
#include <map>

namespace Demo {
	enum class QuestState { Locked, Active, Completed };

	struct QuestEventResult {
		bool hasReward = false;
		std::wstring rewardMessage = L"";
	};

	class QuestManager {
		std::wstring questText;
		bool isExpanded = true;
		bool isVisible = true;
		std::shared_ptr<DX9GF::Font> font;
		std::shared_ptr<DX9GF::FontSprite> fontSprite;
		std::shared_ptr<DX9GF::Texture> uiTex;
		std::shared_ptr<IconButton> btnToggle;

		DX9GF::Camera* cachedUICamera = nullptr;
		float virtualWidth = 0.0f;
		float virtualHeight = 0.0f;
		std::shared_ptr<DX9GF::TransformManager> uiTransformManager;

		std::map<std::string, QuestState> questStates;
		std::string currentTrackedQuest;

		QuestManager() = default;

	public:
		static QuestManager* GetInstance() {
			static QuestManager instance;
			return &instance;
		}

		void Reset() {
			questStates.clear();
			currentTrackedQuest = "";
			questText = L"Quest: ???";
		}

		void Init(DX9GF::GraphicsDevice* gd, std::shared_ptr<DX9GF::TransformManager> tm, DX9GF::Camera* uiCamera, std::shared_ptr<DX9GF::Font> font);

		//TODO: change this to private
		void SetQuest(const std::wstring& text) { questText = text; }
		const std::wstring& GetQuestText() const { return questText; }

		void SetVisible(bool v) { isVisible = v; }
		bool IsVisible() const { return isVisible; }

		void Update(unsigned long long deltaTime);
		void Draw(DX9GF::GraphicsDevice* gd, DX9GF::Camera* uiCamera, unsigned long long deltaTime);

		void SetVirtualResolution(float w, float h) {
			virtualWidth = w;
			virtualHeight = h;
		}
		void SetUICamera(DX9GF::Camera* cam) {
			if (cachedUICamera == cam) return;
			cachedUICamera = cam;
			if (btnToggle) btnToggle->Init(cam);
		}

		void AcceptQuest(const std::string& questId);
		QuestEventResult NotifyEvent(const std::string& eventType, const std::string& targetId, Player* player);

		void GenerateSaveData(nlohmann::json& outData);
		void RestoreSaveData(const nlohmann::json& inData);

		bool IsQuestCompleted(const std::string& questId) {
			return questStates.find(questId) != questStates.end() && questStates[questId] == QuestState::Completed;
		}

	};
}