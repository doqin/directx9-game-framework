#include "pch.h"
#include "TextIconButton.h"

namespace Demo {
    TextIconButton::TextIconButton(std::shared_ptr<DX9GF::TransformManager> tm, float displayX, float displayY, int imgW, int imgH,
        std::shared_ptr<DX9GF::Texture> uiSheetTex, DX9GF::Font* font, const std::wstring& text, int frames)
        : IconButton(tm, displayX, displayY, imgW, imgH, uiSheetTex, frames), label(text)
    {
        if (font) {
            this->fontSprite = std::make_shared<DX9GF::FontSprite>(font);
        }

        this->baseTextOffsetY = -2.5f;
        this->stateOffsetsY[ButtonState::HOVER] = 1.0f;
        this->stateOffsetsY[ButtonState::CLICKED] = 3.0f;
        this->stateOffsetsY[ButtonState::LISTENING] = 3.0f;
    }

    void TextIconButton::SetSpriteScale(float scaleX, float scaleY)
    {
        IconButton::SetSpriteScale(scaleX, scaleY);

        //auto scale text offset to match the sprite
        this->baseTextOffsetY = -2.5f * scaleY;
        this->stateOffsetsY[ButtonState::HOVER] = 1.0f * scaleY;
        this->stateOffsetsY[ButtonState::CLICKED] = 3.0f * scaleY;
        this->stateOffsetsY[ButtonState::LISTENING] = 3.0f * scaleY;
    }

    void Demo::TextIconButton::Draw(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime)
    {
        IconButton::Draw(gd, deltaTime);

        if (!fontSprite || !this->uiCamera) return;

        std::wstring currentText = this->label;

        if (dynamicLabelGetter) {
            currentText = dynamicLabelGetter();
        }

        if (currentText.empty()) return;

        fontSprite->Begin();
        fontSprite->SetText(std::wstring(currentText));
        fontSprite->SetScale(textScaleX, textScaleY);
        fontSprite->SetColor(textColor);
        fontSprite->SetOutline(hasOutline, outlineColor, 2.0f);

        float tw = fontSprite->GetWidth();
        float th = fontSprite->GetHeight();

        float renderX = GetWorldX();
        float renderY = GetWorldY();

        //look up offset based on current state
        if (stateOffsetsY.find(this->currentState) != stateOffsetsY.end()) {
            renderY += stateOffsetsY[this->currentState];
        }

        float finalX = renderX + (this->width - tw) / 2.0f;
        float finalY = renderY + (this->height - th) / 2.0f + baseTextOffsetY;

        fontSprite->SetPosition(finalX, finalY);
        fontSprite->Draw(*this->uiCamera, deltaTime);
        fontSprite->End();
    }
}