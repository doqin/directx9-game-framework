#pragma once
#include "DX9GF.h"
#include "QuestManager.h"
#include <string>
#include <memory>
#include <vector>

namespace Demo {
    enum class QuestMarkerRole { Giver, Receiver };

    class QuestMarker {
    private:
        std::string questId;
        QuestMarkerRole role;
        bool conditionMet = false;

        std::shared_ptr<DX9GF::Texture> uiTex;

        std::shared_ptr<DX9GF::AnimatedSprite> sprExclGray;
        std::shared_ptr<DX9GF::AnimatedSprite> sprExclGold;
        std::shared_ptr<DX9GF::AnimatedSprite> sprQuesGray;
        std::shared_ptr<DX9GF::AnimatedSprite> sprQuesCyan;

    public:
        QuestMarker(const std::string& qId, QuestMarkerRole r)
            : questId(qId), role(r) {
        }

        void Init(DX9GF::GraphicsDevice* gd);
        void Draw(DX9GF::Camera* uiCamera, float x, float y, float scale);

        void SetConditionMet(bool met) { conditionMet = met; }
        bool IsConditionMet() const { return conditionMet; }
        std::string GetQuestId() const { return questId; }
    };
}