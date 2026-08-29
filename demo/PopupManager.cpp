#include "pch.h"
#include "PopupManager.h"
#include "DX9GFInputManager.h"
#include <algorithm>
#include <cmath>
#include "TextIconButton.h"

namespace Demo {
    // Horizontal breathing room between a popup button's label and its edge. Must match the
    // padding handed to TextIconButton::SetAutoResize so Show()'s width estimate agrees with the
    // size the button settles at.
    static constexpr float BTN_TEXT_PAD_X = 16.0f;

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
        RegisterStyle("stepped_white", { {32, 128, 64, 160},   6, 6, 6, 6, 0xFF000000, 0xFF000000, false, 0x00000000 });
        RegisterStyle("stepped_red", { {64, 128, 96, 160},   6, 6, 6, 6, 0xFFFFFFFF, 0xFFFFFFFF, true,  0xFF000000 });
        RegisterStyle("stepped_blue", { {96, 128, 128, 160},  6, 6, 6, 6, 0xFFFFFFFF, 0xFFFFFFFF, true,  0xFF000000 });
        RegisterStyle("stepped_gold", { {128, 128, 160, 160}, 6, 6, 6, 6, 0xFF000000, 0xFF000000, true,  0xFFFFFFFF });

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
        float baseBtnH = static_cast<float>(btnRectsMed[0].bottom - btnRectsMed[0].top);
        float btnH = baseBtnH * scale;
        // Smallest a button is allowed to be; longer labels widen it past this instead of spilling.
        float minBtnW = static_cast<float>(btnRectsMed[0].right - btnRectsMed[0].left) * scale;

        float spacing = 20.0f;
        // Each button fits its own label (measured with the popup font, same font the button draws).
        std::vector<float> btnWidths;
        float totalBtnW = 0.0f;
        for (const auto& btnData : buttons) {
            fontSprite->SetText(std::wstring(btnData.first));
            float w = std::floor(std::max(fontSprite->GetWidth() + BTN_TEXT_PAD_X * 2.0f, minBtnW) + 0.5f);
            btnWidths.push_back(w);
            totalBtnW += w;
        }
        totalBtnW += std::max(0, (int)buttons.size() - 1) * spacing;

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
        this->backgroundSprite->SetScale(scale);

        for (size_t i = 0; i < buttons.size(); ++i) {
            const auto& btnData = buttons[i];

            auto textBtn = std::make_shared<TextIconButton>(internalTm, 0, 0, (int)btnWidths[i], (int)btnH, uiSheetTex, this->font.get(), btnData.first, 3);
            textBtn->SetSpriteRects(btnRectsMed);
            textBtn->SetSpriteScale(scale, scale);
            // Slice the frame so a widened button stretches its centre instead of smearing its caps.
            textBtn->SetSliceMargins(6, 6);
            textBtn->SetAutoResize(true, BTN_TEXT_PAD_X);
            textBtn->SetMinWidth(minBtnW);
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
        keyboardNavigator.Reset();
    }

    std::vector<KeyboardNavigator::Candidate> PopupManager::CollectKeyboardCandidates() {
        std::vector<KeyboardNavigator::Candidate> candidates;
        for (auto& popupBtn : activeButtons) {
            auto btn = popupBtn->backgroundBtn;
            if (!btn) {
                continue;
            }
            candidates.push_back({
                btn,
                btn->GetWorldX(),
                btn->GetWorldY(),
                (float)btn->GetWidth(),
                (float)btn->GetHeight(),
                [btn]() { btn->Activate(); }
                });
        }
        return candidates;
    }

    void PopupManager::LayoutButtons() {
        if (!this->uiCamera) return;
        auto [sw, sh] = this->uiCamera->GetScreenResolution();

        popupX = -popupWidth / 2.0f;
        popupY = -popupHeight / 2.0f;

        if (activeButtons.empty()) return;

        float btnH = activeButtons[0]->backgroundBtn->GetHeight();
        float spacing = 20.0f;

        // Buttons can each be a different width (they grow to fit their label), so sum the actual
        // widths rather than assuming a uniform size.
        float totalWidth = std::max(0, (int)activeButtons.size() - 1) * spacing;
        for (auto& popupBtn : activeButtons) totalWidth += popupBtn->backgroundBtn->GetWidth();

        float startX = popupX + (popupWidth - totalWidth) / 2.0f;
        float startY = popupY + popupHeight - btnH - 25.0f;

        float x = startX;
        for (auto& popupBtn : activeButtons) {
            popupBtn->backgroundBtn->SetLocalPosition(x, startY);
            x += popupBtn->backgroundBtn->GetWidth() + spacing;
        }
        internalTm->RebuildHierarchy();
    }

    void PopupManager::Close() {
        this->isActive = false;
        keyboardNavigator.Reset();
    }

    void PopupManager::Update(unsigned long long deltaTime, DX9GF::Camera* uiCamera) {
        if (!isActive) return;

        internalTm->UpdateAll();

        for (size_t i = 0; i < activeButtons.size(); ++i) {
            activeButtons[i]->backgroundBtn->Update(deltaTime);

            if (!isActive) break;
        }
        // Re-run the layout each frame: a button only settles on its label-fitted width after its
        // first Update, and its position has to follow.
        if (isActive) LayoutButtons();
        if (isActive) {
            auto inpMan = DX9GF::InputManager::GetInstance();
            for (auto& popupBtn : activeButtons) {
                // Any label carrying a "(Y)" / "(N)" hotkey hint, not just "Yes(Y)" / "No(N)".
                bool isYesKey = (popupBtn->label.find(L"(Y)") != std::wstring::npos && inpMan->KeyPress(DIK_Y));
                bool isNoKey = (popupBtn->label.find(L"(N)") != std::wstring::npos && inpMan->KeyPress(DIK_N));
                if (isYesKey || isNoKey) {
                    if (popupBtn->onClick) popupBtn->onClick();
                    Close();
                    break;
                }
            }
        }
        if (isActive) {
            keyboardNavigator.Update(deltaTime, CollectKeyboardCandidates());
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
        fontSprite->SetOutline(style.hasOutline, style.outlineColor, 2.0f);

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

        keyboardNavigator.Draw(gd, *uiCamera, CollectKeyboardCandidates());
    }
}