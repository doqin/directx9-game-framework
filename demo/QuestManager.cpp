#include "pch.h"
#include "QuestManager.h"
#include "IConversation.h"

namespace {
	constexpr float MARGIN_X = 20.0f;
	constexpr float MARGIN_Y = 20.0f;
	constexpr float ARROW_SIZE = 28.0f;
	constexpr float PANEL_MIN_W = 120.0f;
	constexpr float PANEL_PADDING_X = 24.0f;
	constexpr float PANEL_H = 30.0f;
	constexpr D3DCOLOR PANEL_BG = 0xCC141410;
	constexpr D3DCOLOR TEXT_COLOR = 0xFFFFD700;
}

void Demo::QuestManager::Init(DX9GF::GraphicsDevice* gd, std::shared_ptr<DX9GF::TransformManager> tm,
	DX9GF::Camera* uiCamera, std::shared_ptr<DX9GF::Font> font)
{
	if (!fontSprite || this->font.get() != font.get()) {
		this->font = font;
		fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
	}
	if (!uiTex) {
		uiTex = std::make_shared<DX9GF::Texture>(gd);
		uiTex->LoadTexture(L"assets/ui.png");
	}
	if (!uiTransformManager) {
		uiTransformManager = std::make_shared<DX9GF::TransformManager>();
	}

	if (!btnToggle) {
		btnToggle = std::make_shared<IconButton>(uiTransformManager, 0, 0,
			static_cast<int>(ARROW_SIZE), static_cast<int>(ARROW_SIZE), uiTex, 3);
		btnToggle->SetSpriteCoords(240, 96, 16, 16, 0);
		btnToggle->SetSpriteScale(ARROW_SIZE / 16.0f, ARROW_SIZE / 16.0f);
		btnToggle->SetOnReleaseLeft([this](DX9GF::ITrigger*) {
			isExpanded = !isExpanded;
			});
	}

	SetUICamera(uiCamera);
	uiTransformManager->RebuildHierarchy();
}
void Demo::QuestManager::Update(unsigned long long deltaTime)
{
	if (!isVisible) return;
	float panelX = -virtualWidth / 2.0f + MARGIN_X;
	float panelY = -virtualHeight / 2.0f + MARGIN_Y;
	if (btnToggle) {
		btnToggle->SetLocalPosition(panelX, panelY);
		btnToggle->Update(deltaTime);
	}
	if (uiTransformManager) {
		uiTransformManager->UpdateAll();
	}
}

void Demo::QuestManager::Draw(DX9GF::GraphicsDevice* gd, DX9GF::Camera* uiCamera, unsigned long long deltaTime)
{
	if (!isVisible) return;
	if (btnToggle) btnToggle->Draw(gd, deltaTime);
	if (!isExpanded || questText.empty()) return;

	float panelX = -virtualWidth / 2.0f + MARGIN_X;
	float panelY = -virtualHeight / 2.0f + MARGIN_Y;
	static int qmLogCounter = 0;
	if (qmLogCounter++ % 60 == 0) {
		char buf[256];
		sprintf_s(buf, "[QM DEBUG] virtualW=%.1f virtualH=%.1f panelX=%.1f panelY=%.1f\n",
			virtualWidth, virtualHeight, panelX, panelY);
		OutputDebugStringA(buf);
	}
	float textX = panelX + ARROW_SIZE + 8.0f;

	fontSprite->Begin();
	fontSprite->SetOutline(false);
	fontSprite->SetColor(TEXT_COLOR);
	fontSprite->SetText(std::wstring(questText));

	float textWidth = fontSprite->GetWidth();
	float panelW = std::max(PANEL_MIN_W, textWidth + PANEL_PADDING_X);

	gd->SetAlphaBlending(true);
	gd->DrawRectangle(*uiCamera, textX, panelY, panelW, PANEL_H, PANEL_BG, true);
	gd->SetAlphaBlending(false);

	fontSprite->SetPosition(textX + 6.0f, panelY + (PANEL_H - fontSprite->GetHeight()) / 2.0f);
	fontSprite->Draw(*uiCamera, deltaTime);
	fontSprite->End();
}

//npcs call this function when give quest
void Demo::QuestManager::AcceptQuest(const std::string& questId) {
	if (questStates.find(questId) != questStates.end() && questStates[questId] != QuestState::Locked) return;

	questStates[questId] = QuestState::Active;
	currentTrackedQuest = questId;

	//still hardcode text here
	if (questId == "SecretBoss_Pacman") {
		SetQuest(L"Quest: Find secret boss, defeat it and get rewards: Boss defeated 0/1");
	}
	else if (questId == "Quest_BossWorld") {
		SetQuest(L"Quest: Activate the terminals: 0/4");
	}
	else if (questId == "Quest_Tutorial") {
		SetQuest(L"Quest: First Encounter...?");
	}
	else if (questId == "Quest_ThreadAlley_Start") {
		SetQuest(L"Quest: Find the malware through this alley!");
	}
	// else if (questId == "...") { ... }
}

Demo::QuestEventResult Demo::QuestManager::NotifyEvent(const std::string& eventType, const std::string& targetId, Player* player) {

	if (eventType == "ENTITY_DEAD" && targetId == "SecretBoss_Pacman") {
		if (questStates["SecretBoss_Pacman"] == QuestState::Active) {
			questStates["SecretBoss_Pacman"] = QuestState::Completed;

			SetQuest(L"Quest: Find secret boss, defeat it and get rewards: Boss defeated 1/1");

			player->GetInventoryItems().AddItem(10, 1);

			auto* bp = ItemData::GetInstance()->GetItemBlueprint(10);
			std::wstring msg = L"You found: ";
			if (bp) msg += bp->GetName();

			return { true, msg };
		}
	}

	if (eventType == "TERMINAL_HACKED") {
		if (questStates["Quest_BossWorld"] == QuestState::Active) {
			int step = std::stoi(targetId);
			if (step >= 4) {
				SetQuest(L"Quest: Defeat the Boss!");
				// questStates["Quest_BossWorld"] = QuestState::Completed;
			}
			else {
				SetQuest(L"Quest: Activate the terminals: " + std::to_wstring(step) + L"/4");
			}
		}
	}

	if (eventType == "TROJAN_TALKED") {
		if (questStates.find("Quest_ThreadAlley_Start") != questStates.end() && questStates["Quest_ThreadAlley_Start"] == QuestState::Active) {
			SetQuest(L"Quest: Find an auth token somewhere in the alley");
		}
	}

	if (eventType == "TROJAN_HAS_TOKEN") {
		if (questStates.find("Quest_ThreadAlley_Start") != questStates.end() && questStates["Quest_ThreadAlley_Start"] == QuestState::Active) {
			SetQuest(L"Quest: Bring the auth token back to the stranger");
		}
	}

	if (eventType == "TROJAN_REVEALED") {
		if (questStates.find("Quest_ThreadAlley_Start") != questStates.end() && questStates["Quest_ThreadAlley_Start"] == QuestState::Active) {
			SetQuest(L"Quest: Delete the Trojan!");
		}
	}

	if (eventType == "TROJAN_DEFEATED") {
		if (questStates.find("Quest_ThreadAlley_Start") != questStates.end() && questStates["Quest_ThreadAlley_Start"] == QuestState::Active) {
			questStates["Quest_ThreadAlley_Start"] = QuestState::Completed;
			SetQuest(L"Quest: Trojan deleted. Alley is safe.");
			player->AddGold(50);
			return { true, L"50 Gold - Quest Completed: Defeated the Trojan" };
		}
	}

	if (eventType == "FIRST_ENCOUNTER_DEFEATED") {
		if (targetId == "tutorial_keye_01" && GetQuestState("Quest_Tutorial") == QuestState::Active) {
			questStates["Quest_Tutorial"] = QuestState::Completed;
			SetQuest(L"Quest: First Encounter - Completed!");
			player->AddGold(26);
			return { true, L"26 Gold - Quest Completed: First Encounter" };
		}
	}
	return { false, L"" };
}

void Demo::QuestManager::GenerateSaveData(nlohmann::json& outData) {
	nlohmann::json questsJson;
	for (auto& pair : questStates) {
		questsJson[pair.first] = static_cast<int>(pair.second);
	}
	outData["questStates"] = questsJson;
	outData["trackedQuest"] = currentTrackedQuest;
}

void Demo::QuestManager::RestoreSaveData(const nlohmann::json& inData) {
	if (inData.contains("questStates")) {
		for (auto& item : inData["questStates"].items()) {
			questStates[item.key()] = static_cast<QuestState>(item.value().get<int>());
		}
	}
	if (inData.contains("trackedQuest")) {
		currentTrackedQuest = inData["trackedQuest"].get<std::string>();

		if (currentTrackedQuest == "SecretBoss_Pacman") {
			if (questStates["SecretBoss_Pacman"] == QuestState::Completed) {
				SetQuest(L"Quest: Find secret boss, defeat it and get rewards: Boss defeated 1/1");
			}
			else {
				SetQuest(L"Quest: Find secret boss, defeat it and get rewards: Boss defeated 0/1");
			}
		}
		else if (currentTrackedQuest == "Quest_Tutorial") {
			SetQuest(L"Quest: First Encounter...?");
		}
		else if (currentTrackedQuest == "Quest_BossWorld") {
			if (questStates["Quest_BossWorld"] == QuestState::Completed) {
				SetQuest(L"Quest: Defeat the Boss!");
			}
			else {
				SetQuest(L"Quest: Activate the terminals: ?/4");
			}
		}
		else if (currentTrackedQuest == "Quest_ThreadAlley_Start") {
			if (questStates["Quest_ThreadAlley_Start"] == QuestState::Completed) {
				SetQuest(L"Quest: Trojan deleted. Alley is safe.");
			}
			else {
				//change this later for state saving 
				SetQuest(L"Quest: Find the malware through this alley!");
			}
		}
	}

}