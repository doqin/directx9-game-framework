#include "pch.h"
#include "PopupManager.h"
#include "DX9GFInputManager.h"
#include <algorithm>
#include "TextIconButton.h"

namespace Demo {
    PopupManager* PopupManager::instance = nullptr;

    PopupManager::PopupManager() {}

    PopupManager* PopupManager::GetInstance() {
        if (!instance) {
            instance = new PopupManager();
        }
        return instance;
    }

    void PopupManager::Init(DX9GF::GraphicsDevice* gd, std::shared_ptr<DX9GF::Texture> borderTex, std::shared_ptr<DX9GF::Texture> uiTex, std::shared_ptr<DX9GF::Font> popupFont) {
        this->gd = gd;
        this->font = popupFont;
        this->fontSprite = std::make_shared<DX9GF::FontSprite>(this->font.get());
        this->internalTm = std::make_shared<DX9GF::TransformManager>();

        this->borderSheetTex = borderTex;
        this->uiSheetTex = uiTex;

        RegisterStyle("basic_blackwhite", { {0, 0, 32, 32},     2, 2, 2, 2, 0xFF000000, 0xFF000000, false, 0x00000000 });
        RegisterStyle("basic_blackgrey", { {32, 0, 64, 32},    2, 2, 2, 2, 0xFFFFFFFF, 0xFFFFFFFF, true,  0xFF000000 });
        RegisterStyle("basic_whitegrey", { {64, 0, 96, 32},    2, 2, 2, 2, 0xFF000000, 0xFF000000, false, 0x00000000 });
        RegisterStyle("basic_ghost", { {96, 0, 127, 32},   2, 2, 2, 2, 0xFF000000, 0xFF000000, true,  0xFFFFFFFF });
        RegisterStyle("basic_blueyellow", { {0, 64, 32, 96},    2, 2, 2, 2, 0xFFFFFFFF, 0xFFFFFFFF, false, 0x00000000 });
        RegisterStyle("basic_bluepurple", { {32, 64, 64, 96},   2, 2, 2, 2, 0xFFFFFFFF, 0xFFFFFFFF, false, 0x00000000 });
        RegisterStyle("basic_contrastblue", { {64, 64, 96, 96},   2, 2, 2, 2, 0xFFFFFFFF, 0xFFFFFFFF, true,  0xFF000000 });
        RegisterStyle("basic_contrastred", { {96, 64, 128, 96},  2, 2, 2, 2, 0xFFFFFFFF, 0xFFFFFFFF, false, 0x00000000 });

        btnRectsSmall = { {0, 0, 16, 16}, {0, 16, 16, 32}, {0, 32, 16, 48} };
        btnRectsMed = { {16, 0, 48, 16}, {16, 16, 48, 32}, {16, 32, 48, 48} };
        btnRectsLarge = { {48, 0, 96, 16}, {48, 16, 96, 32}, {48, 32, 96, 48} };
    }

    void PopupManager::RegisterStyle(const std::string& styleName, const PopupStyle& style) {
        styles[styleName] = style;
    }

    void PopupManager::Show(const std::string& styleName, const std::wstring& title, const std::wstring& message,
        const std::vector<std::pair<std::wstring, std::function<void()>>>& buttons)
    {
        if (styles.find(styleName) == styles.end() || !this->uiCamera) return;

        this->currentStyle = styleName;
        this->currentTitle = title;
        this->currentMessage = message;
        this->isActive = true;
        this->activeButtons.clear();
        this->internalTm = std::make_shared<DX9GF::TransformManager>();

        PopupStyle& style = styles[styleName];

        fontSprite->SetText(std::wstring(currentTitle));
        float titleW = static_cast<float>(fontSprite->GetWidth());
        float titleH = static_cast<float>(fontSprite->GetHeight());

        fontSprite->SetText(std::wstring(currentMessage));
        float msgW = static_cast<float>(fontSprite->GetWidth());
        float msgH = static_cast<float>(fontSprite->GetHeight());

        float scale = 2.0f; //Change this value to adjust the button size
        float baseBtnW = static_cast<float>(btnRectsMed[0].right - btnRectsMed[0].left);
        float baseBtnH = static_cast<float>(btnRectsMed[0].bottom - btnRectsMed[0].top);
        float btnW = baseBtnW * scale;
        float btnH = baseBtnH * scale;

        float spacing = 20.0f;
        float totalBtnW = (buttons.size() * btnW) + (std::max(0, (int)buttons.size() - 1) * spacing);

        float paddingX = 80.0f;
        float maxContentW = std::max({ titleW, msgW, totalBtnW });
        this->popupWidth = std::max(maxContentW + paddingX, 300.0f);

        float paddingTop = 25.0f;
        float paddingMid = 20.0f;
        float paddingBot = 25.0f;

        this->popupHeight = paddingTop + titleH + paddingMid + msgH + paddingMid + btnH + paddingBot;

        this->backgroundSprite = std::make_shared<DX9GF::NineSliceSprite>(
            borderSheetTex.get(), style.bgSrcRect,
            style.marginL, style.marginT, style.marginR, style.marginB
        );
        this->backgroundSprite->SetTargetSize(popupWidth, popupHeight);

        for (const auto& btnData : buttons) {

            auto textBtn = std::make_shared<TextIconButton>(internalTm, 0, 0, (int)btnW, (int)btnH, uiSheetTex, this->font.get(), btnData.first, 3);
            textBtn->SetSpriteRects(btnRectsMed);
            textBtn->SetSpriteScale(scale, scale);
            textBtn->SetTextColor(0xFF111111);          
            textBtn->Init(this->uiCamera);

            auto popupBtn = std::make_shared<PopupDynamicButton>(textBtn, btnData.first, btnData.second);

            auto cb = btnData.second;
            textBtn->SetOnReleaseLeft([this, cb](DX9GF::ITrigger* t) {
                if (cb) cb();
                this->Close();
                });

            activeButtons.push_back(popupBtn);
        }

        LayoutButtons();
    }

    void PopupManager::LayoutButtons() {
        if (!this->uiCamera) return;
        auto [sw, sh] = this->uiCamera->GetScreenResolution();

        popupX = -popupWidth / 2.0f;
        popupY = -popupHeight / 2.0f;

        if (activeButtons.empty()) return;

        float btnW = activeButtons[0]->backgroundBtn->GetWidth();
        float btnH = activeButtons[0]->backgroundBtn->GetHeight();
        float spacing = 20.0f;
        float totalWidth = (activeButtons.size() * btnW) + ((activeButtons.size() - 1) * spacing);

        float startX = popupX + (popupWidth - totalWidth) / 2.0f;
        float startY = popupY + popupHeight - btnH - 25.0f;

        for (size_t i = 0; i < activeButtons.size(); ++i) {
            activeButtons[i]->backgroundBtn->SetLocalPosition(startX + (i * (btnW + spacing)), startY);
        }
        internalTm->RebuildHierarchy();
    }

    void PopupManager::Close() {
        this->isActive = false;
    }

    void PopupManager::Update(unsigned long long deltaTime, DX9GF::Camera* uiCamera) {
        if (!isActive) return;

        internalTm->UpdateAll();

        for (size_t i = 0; i < activeButtons.size(); ++i) {
            activeButtons[i]->backgroundBtn->Update(deltaTime);

            if (!isActive) break;
        }
    }

    void PopupManager::DrawUI(unsigned long long deltaTime, DX9GF::Camera* uiCamera) {
        if (!isActive) return;

        auto [sw, sh] = uiCamera->GetScreenResolution();
        gd->SetAlphaBlending(true);
        gd->DrawRectangle(*uiCamera, -sw / 2.0f, -sh / 2.0f, (float)sw, (float)sh, 0x88000000, true);
        gd->SetAlphaBlending(false);

        backgroundSprite->Begin();
        backgroundSprite->SetPosition(popupX, popupY);
        backgroundSprite->Draw(*uiCamera, deltaTime);
        backgroundSprite->End();

        PopupStyle& style = styles[currentStyle];

        fontSprite->Begin();
        fontSprite->SetText(std::wstring(currentTitle));
        fontSprite->SetColor(style.titleColor);
        fontSprite->SetOutline(style.hasOutline, style.outlineColor, 1.5f);

        float titleW = fontSprite->GetWidth();
        float titleH = fontSprite->GetHeight();
        fontSprite->SetPosition(popupX + (popupWidth - titleW) / 2.0f, popupY + 25.0f);
        fontSprite->Draw(*uiCamera, deltaTime);

        fontSprite->SetText(std::wstring(currentMessage));
        fontSprite->SetColor(style.msgColor);

        float msgW = fontSprite->GetWidth();
        fontSprite->SetPosition(popupX + (popupWidth - msgW) / 2.0f, popupY + 25.0f + titleH + 20.0f);
        fontSprite->Draw(*uiCamera, deltaTime);
        fontSprite->End();

        gd->SetAlphaBlending(true);
        for (auto& btn : activeButtons) {
            btn->backgroundBtn->Draw(gd, deltaTime);
        }
    }
}