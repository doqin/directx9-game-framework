#pragma once
#include "pch.h"
#include "DX9GF.h"      
#include "DX9GFExtras.h"  
#include "Player.h"
#include "DialogueLine.h"
#include "Debug.h"
#include "QuestMarker.h"
#include <functional>

namespace Demo {
    class INPC : public DX9GF::IGameObject, virtual public Pointable {
    protected:
        bool isConversationOpen = false;
        int currentLineIndex = 0;
        DX9GF::GraphicsDevice* gd = nullptr;
        DX9GF::Camera* worldCamera;
        std::weak_ptr<DX9GF::TransformManager> transformManager;
        std::weak_ptr<Player> player;

        std::shared_ptr<DX9GF::AnimatedSprite> sprite;
        std::shared_ptr<DX9GF::Texture> spritesheet;
        std::shared_ptr<DX9GF::RectangleCollider> collider;
        std::shared_ptr<DX9GF::FontSprite> fontSprite;
        std::weak_ptr<DX9GF::CommandBuffer> drawBuffer;

        std::unordered_map<std::wstring, std::string> characterVoices = { { L"Player", "bleep8" } }; // Maps character names to voice clip identifiers
        std::vector<DialogueLine> dialogueLines;
        bool isPlayerNear = false;
        const float INTERACTION_DISTANCE = 40.0f;
        virtual void MapCharacterVoices(std::unordered_map<std::wstring, std::string>& voiceMap) = 0;
        std::shared_ptr<QuestMarker> questMarker = nullptr;
        std::function<void()> onDialogueEnd = nullptr;

        float uiOffsetY = 40.0f;
        float markerOffsetX = 10.0f;
        float markerOffsetY = 26.0f;

    public:
        INPC(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y) : DX9GF::IGameObject(tm, x, y), transformManager(tm) {}
        virtual ~INPC() = default;

		std::pair<float, float> GetPoint() const override {
			return { GetWorldX(), GetWorldY() };
		}
        virtual void Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer);
        virtual void AddLine(std::wstring name, std::wstring content);
        void ClearLines() {
            dialogueLines.clear();
            currentLineIndex = 0;
        }
        virtual void Update(unsigned long long deltaTime);
        virtual void Draw(const DX9GF::Camera& camera, unsigned long long deltaTime);
        virtual void DrawUI(DX9GF::Camera* uiCamera, unsigned long long deltaTime);
        virtual std::vector<DialogueLine> GetDialogueLines() { return dialogueLines; }
        virtual bool CanInteract() const = 0;

        void SetOnDialogueEnd(std::function<void()> callback) { onDialogueEnd = callback; }
        std::function<void()> GetOnDialogueEnd() const { return onDialogueEnd; }

        void AttachQuestMarker(const std::string& questId, QuestMarkerRole role) {
            questMarker = std::make_shared<QuestMarker>(questId, role);
        }
        std::shared_ptr<QuestMarker> GetQuestMarker() const { return questMarker; }
    };
}
