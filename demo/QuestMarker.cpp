#include "pch.h"
#include "QuestMarker.h"

namespace Demo {
    void QuestMarker::Init(DX9GF::GraphicsDevice* gd) {
        uiTex = std::make_shared<DX9GF::Texture>(gd);
        uiTex->LoadTexture(L"assets/ui.png");

        std::vector<RECT> r1 = { {240, 128, 258, 144} };
        sprExclGray = std::make_shared<DX9GF::AnimatedSprite>(uiTex.get(), r1, 1, false);
        sprExclGray->SetOrigin(0.f, 0.f);

        std::vector<RECT> r2 = { {240, 144, 258, 160} };
        sprExclGold = std::make_shared<DX9GF::AnimatedSprite>(uiTex.get(), r2, 1, false);
        sprExclGold->SetOrigin(0.f, 0.f);

        std::vector<RECT> r3 = { {258, 128, 276, 144} };
        sprQuesGray = std::make_shared<DX9GF::AnimatedSprite>(uiTex.get(), r3, 1, false);
        sprQuesGray->SetOrigin(0.f, 0.f);

        std::vector<RECT> r4 = { {258, 144, 276, 160} };
        sprQuesCyan = std::make_shared<DX9GF::AnimatedSprite>(uiTex.get(), r4, 1, false);
        sprQuesCyan->SetOrigin(0.f, 0.f);
    }

    void QuestMarker::Update(unsigned long long deltaTime) {
        timeAccumulator += deltaTime * 0.001f;

        //6.0f speed, 1.0f pixel
        bobbingOffset = std::sin(timeAccumulator * 6.0f) * 1.0f;
    }

    void QuestMarker::Draw(DX9GF::Camera* uiCamera, float x, float y, float scale) {
        QuestState state = QuestManager::GetInstance()->GetQuestState(questId);
        std::shared_ptr<DX9GF::AnimatedSprite> activeSprite = nullptr;

        if (role == QuestMarkerRole::Giver) {
            if (state == QuestState::Locked) {
                activeSprite = sprExclGold;
            }
            else if (state == QuestState::Active) {
                activeSprite = sprExclGray;
            }
            //completed quests dont need markers
        }
        else if (role == QuestMarkerRole::Receiver) {
            if (state == QuestState::Active) {
                activeSprite = conditionMet ? sprQuesCyan : sprQuesGray;
            }
        }

        if (activeSprite) {
            activeSprite->SetScale(scale, scale);
            activeSprite->SetPosition(x, y + (bobbingOffset * scale));
            activeSprite->Begin();
            activeSprite->Draw(*uiCamera, 0);
            activeSprite->End();
        }
    }
}