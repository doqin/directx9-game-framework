#include "pch.h"
#include "QuestManager.h"
#include "IConversation.h"
#include <algorithm>

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

//init quest here
void Demo::QuestManager::InitQuestDatabase() {
	questDatabase.clear();

	questDatabase["SecretBoss_Pacman"] = {
		"SecretBoss_Pacman",
		L"The Hidden Malware",
		L"There's a hidden boss somewhere in this maze. Defeat it for a secret reward!",
		L"Secret Boss: 0/1",
		L"Rusty Key"
	};

	questDatabase["Quest_BossWorld"] = {
		"Quest_BossWorld",
		L"System Override",
		L"Activate all four terminals to unlock the gate to the core.",
		L"Activate the terminals: 0/4",
		L"Access to the Core"
	};

	questDatabase["Quest_Tutorial"] = {
		"Quest_Tutorial",
		L"First Encounter",
		L"Learn the basics of survival by defeating your first enemy.",
		L"First Encounter...?",
		L"26 Gold"
	};

	questDatabase["Quest_ThreadAlley_Start"] = {
		"Quest_ThreadAlley_Start",
		L"Thread Alley Cleanup",
		L"A stranger needs an Auth Token to escape. Help them clear the alley.",
		L"Find the malware!",
		L"50 Gold"
	};
}

std::vector<Demo::QuestInfo> Demo::QuestManager::GetActiveQuests() const {
	std::vector<QuestInfo> result;
	for (const auto& [id, state] : questStates) {
		if (state == QuestState::Active && questDatabase.count(id)) {
			result.push_back(questDatabase.at(id));
		}
	}
	return result;
}

std::vector<Demo::QuestInfo> Demo::QuestManager::GetCompletedQuests() const {
	std::vector<QuestInfo> result;
	for (const auto& [id, state] : questStates) {
		if (state == QuestState::Completed && questDatabase.count(id)) {
			result.push_back(questDatabase.at(id));
		}
	}
	return result;
}

Demo::QuestInfo* Demo::QuestManager::GetQuestInfo(const std::string& questId) {
	if (questDatabase.count(questId)) {
		return &questDatabase[questId];
	}
	return nullptr;
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
		btnToggle->SetSpriteCoords(240, 96, 16, 16, 0); //TODO: Change this button
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

	//slide anim
	float targetAnim = isExpanded ? 1.0f : 0.0f;
	if (animProgress != targetAnim) {
		float speed = (deltaTime / 1000.0f) * 4.0f; //slide in 0.25s
		if (animProgress < targetAnim) {
			animProgress = (std::min)(animProgress + speed, targetAnim);
		}
		else {
			animProgress = (std::max)(animProgress - speed, targetAnim);
		}
	}

	//marquee when questText's too long
	if (animProgress == 1.0f && !questText.empty()) {
		fontSprite->SetText(questText);
		float textW = fontSprite->GetWidth();
		if (textW > MAX_PANEL_W - PANEL_PADDING_X) {
			float overWidth = textW - (MAX_PANEL_W - PANEL_PADDING_X);
			textScrollWaitTimer += deltaTime;

			if (textScrollWaitTimer > 1500.0f) {
				textScrollOffset += (30.0f * deltaTime / 1000.0f);
				if (textScrollOffset > overWidth + 20.0f) {
					textScrollOffset = 0.0f;
					textScrollWaitTimer = 0.0f;
				}
			}
		}
		else {
			textScrollOffset = 0.0f;
			textScrollWaitTimer = 0.0f;
		}
	}
	else {
		textScrollOffset = 0.0f;
		textScrollWaitTimer = 0.0f;
	}

	if (uiTransformManager) {
		uiTransformManager->UpdateAll();
	}
}

void Demo::QuestManager::Draw(DX9GF::GraphicsDevice* gd, DX9GF::Camera* uiCamera, unsigned long long deltaTime)
{
	if (!isVisible) return;
	if (btnToggle) btnToggle->Draw(gd, deltaTime);

	if (animProgress <= 0.0f || questText.empty()) return;

	float panelX = -virtualWidth / 2.0f + MARGIN_X;
	float panelY = -virtualHeight / 2.0f + MARGIN_Y;
	float textX = panelX + ARROW_SIZE + 8.0f;

	fontSprite->SetText(questText);
	float textWidth = fontSprite->GetWidth();

	//limit panel width
	float targetPanelW = (std::min)(textWidth + PANEL_PADDING_X, MAX_PANEL_W);
	targetPanelW = (std::max)(targetPanelW, PANEL_MIN_W);

	//Ease Out Cubic for smoother slide
	float t = animProgress - 1.0f;
	float easeOut = (t * t * t + 1.0f);
	float currentWidth = easeOut * targetPanelW;

	gd->SetAlphaBlending(true);
	gd->DrawRectangle(*uiCamera, textX, panelY, currentWidth, PANEL_H, PANEL_BG, true);
	gd->SetAlphaBlending(false);

	//scissor text
	D3DXMATRIX matCamera = uiCamera->GetTransformMatrix();
	D3DXVECTOR4 topLeft(textX, panelY, 0.0f, 1.0f);
	D3DXVECTOR4 bottomRight(textX + currentWidth, panelY + PANEL_H, 0.0f, 1.0f);

	D3DXVec4Transform(&topLeft, &topLeft, &matCamera);
	D3DXVec4Transform(&bottomRight, &bottomRight, &matCamera);

	RECT scissorRect;
	scissorRect.left = static_cast<LONG>(topLeft.x / topLeft.w);
	scissorRect.top = static_cast<LONG>(topLeft.y / topLeft.w);
	scissorRect.right = static_cast<LONG>(bottomRight.x / bottomRight.w);
	scissorRect.bottom = static_cast<LONG>(bottomRight.y / bottomRight.w);

	gd->SetScissorTest(true);
	gd->SetScissorRect(scissorRect);

	fontSprite->Begin();
	fontSprite->SetOutline(false);
	fontSprite->SetColor(TEXT_COLOR);

	float drawTextX = textX + 6.0f - textScrollOffset;
	fontSprite->SetPosition(drawTextX, panelY + (PANEL_H - fontSprite->GetHeight()) / 2.0f);
	fontSprite->Draw(*uiCamera, deltaTime);

	fontSprite->End();

	gd->SetScissorTest(false);
}

void Demo::QuestManager::AcceptQuest(const std::string& questId) {
	if (questStates.find(questId) != questStates.end() && questStates[questId] != QuestState::Locked) return;

	questStates[questId] = QuestState::Active;
	currentTrackedQuest = questId;

	if (questDatabase.count(questId)) {
		SetQuest(L"Quest: " + questDatabase[questId].currentObjective);
	}
	DX9GF::AudioManager::GetInstance()->Play("quest_active", false, 0.05f);
}

Demo::QuestEventResult Demo::QuestManager::NotifyEvent(const std::string& eventType, const std::string& targetId, Player* player) {

	if (eventType == "ENTITY_DEAD" && targetId == "SecretBoss_Pacman") {
		if (questStates["SecretBoss_Pacman"] == QuestState::Active) {
			questStates["SecretBoss_Pacman"] = QuestState::Completed;

			if (questDatabase.count("SecretBoss_Pacman")) {
				questDatabase["SecretBoss_Pacman"].currentObjective = L"Secret Boss: 1/1";
				SetQuest(L"Quest: " + questDatabase["SecretBoss_Pacman"].currentObjective);
			}

			player->GetInventoryItems().AddItem(10, 1);
			auto* bp = ItemData::GetInstance()->GetItemBlueprint(10);
			std::wstring msg = L"You found: ";
			if (bp) msg += bp->GetName();
			DX9GF::AudioManager::GetInstance()->Play("quest_completed", false, 0.8f);
			return { true, msg };
		}
	}

	if (eventType == "TERMINAL_HACKED") {
		if (questStates["Quest_BossWorld"] == QuestState::Active) {
			int step = std::stoi(targetId);
			if (questDatabase.count("Quest_BossWorld")) {
				if (step >= 4) {
					questStates["Quest_BossWorld"] = QuestState::Completed;
					questDatabase["Quest_BossWorld"].currentObjective = L"Terminals Hacked! The Gate is open.";
					SetQuest(L"Quest: " + questDatabase["Quest_BossWorld"].currentObjective);
					DX9GF::AudioManager::GetInstance()->Play("quest_completed", false, 0.8f);
					return { true, L"Quest Completed: System Override" };
				}
				else {
					questDatabase["Quest_BossWorld"].currentObjective = L"Activate terminals: " + std::to_wstring(step) + L"/4";
					SetQuest(L"Quest: " + questDatabase["Quest_BossWorld"].currentObjective);
				}
			}
		}
	}

	if (eventType == "TROJAN_TALKED") {
		if (questStates["Quest_ThreadAlley_Start"] == QuestState::Active && questDatabase.count("Quest_ThreadAlley_Start")) {
			questDatabase["Quest_ThreadAlley_Start"].currentObjective = L"Find an auth token";
			SetQuest(L"Quest: " + questDatabase["Quest_ThreadAlley_Start"].currentObjective);
		}
	}

	if (eventType == "TROJAN_HAS_TOKEN") {
		if (questStates["Quest_ThreadAlley_Start"] == QuestState::Active && questDatabase.count("Quest_ThreadAlley_Start")) {
			questDatabase["Quest_ThreadAlley_Start"].currentObjective = L"Bring the auth token back";
			SetQuest(L"Quest: " + questDatabase["Quest_ThreadAlley_Start"].currentObjective);
		}
	}

	if (eventType == "TROJAN_REVEALED") {
		if (questStates["Quest_ThreadAlley_Start"] == QuestState::Active && questDatabase.count("Quest_ThreadAlley_Start")) {
			questDatabase["Quest_ThreadAlley_Start"].currentObjective = L"Delete the Trojan!";
			SetQuest(L"Quest: " + questDatabase["Quest_ThreadAlley_Start"].currentObjective);
		}
	}

	if (eventType == "TROJAN_DEFEATED") {
		if (questStates["Quest_ThreadAlley_Start"] == QuestState::Active) {
			questStates["Quest_ThreadAlley_Start"] = QuestState::Completed;

			if (questDatabase.count("Quest_ThreadAlley_Start")) {
				questDatabase["Quest_ThreadAlley_Start"].currentObjective = L"Alley is safe.";
				SetQuest(L"Quest: " + questDatabase["Quest_ThreadAlley_Start"].currentObjective);
			}
			DX9GF::AudioManager::GetInstance()->Play("quest_completed", false, 0.8f);
			return { true, L"50 Gold - Quest Completed: Defeated the Trojan" };
		}
	}

	if (eventType == "FIRST_ENCOUNTER_DEFEATED") {
		if (GetQuestState("Quest_Tutorial") == QuestState::Active) {
			questStates["Quest_Tutorial"] = QuestState::Completed;

			if (questDatabase.count("Quest_Tutorial")) {
				questDatabase["Quest_Tutorial"].currentObjective = L"First Encounter - Completed!";
				SetQuest(L"Quest: " + questDatabase["Quest_Tutorial"].currentObjective);
			}
			DX9GF::AudioManager::GetInstance()->Play("quest_completed", false, 0.8f);
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

		if (questDatabase.count(currentTrackedQuest)) {
			if (currentTrackedQuest == "SecretBoss_Pacman" && questStates["SecretBoss_Pacman"] == QuestState::Completed) {
				questDatabase["SecretBoss_Pacman"].currentObjective = L"Secret Boss: 1/1";
			}
			else if (currentTrackedQuest == "Quest_BossWorld" && questStates["Quest_BossWorld"] == QuestState::Completed) {
				questDatabase["Quest_BossWorld"].currentObjective = L"Defeat the Boss!";
			}
			else if (currentTrackedQuest == "Quest_ThreadAlley_Start" && questStates["Quest_ThreadAlley_Start"] == QuestState::Completed) {
				questDatabase["Quest_ThreadAlley_Start"].currentObjective = L"Alley is safe.";
			}

			SetQuest(L"Quest: " + questDatabase[currentTrackedQuest].currentObjective);
		}
	}
}