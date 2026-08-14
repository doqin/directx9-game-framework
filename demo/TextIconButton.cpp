#include "pch.h"
#include "TextIconButton.h"
#include <algorithm>
#include <cmath>

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

        //never shrink below the size the button was designed at
        this->minWidth = static_cast<float>(imgW);
    }

    void TextIconButton::SetAutoResize(bool enabled, float paddingX)
    {
        this->autoResize = enabled;
        this->autoResizePaddingX = paddingX;
    }

    std::wstring TextIconButton::GetCurrentText() const
    {
        return this->dynamicLabelGetter ? this->dynamicLabelGetter() : this->label;
    }

    float TextIconButton::MeasureTextWidth(const std::wstring& text) const
    {
        if (!this->fontSprite || text.empty()) return 0.0f;

        this->fontSprite->SetText(std::wstring(text));
        return this->fontSprite->GetWidth() * this->textScaleX;
    }

    void TextIconButton::ApplyAutoResize()
    {
        if (!this->autoResize || !this->fontSprite) return;

        float wanted = MeasureTextWidth(GetCurrentText()) + this->autoResizePaddingX * 2.0f;
        wanted = std::max(wanted, this->minWidth);
        //the caps may not overlap, otherwise the sliced sprite folds into itself
        wanted = std::max(wanted, GetSliceCapsWidth());

        //snap to whole pixels so the stretched centre stays crisp
        wanted = std::floor(wanted + 0.5f);

        if (std::fabs(wanted - this->width) > 0.5f) {
            SetSize(wanted, this->height);
        }
    }

    void TextIconButton::Update(unsigned long long deltaTime)
    {
        //resize before the trigger runs so hovering matches what is drawn
        ApplyAutoResize();
        IButton::Update(deltaTime);
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
        //keep the width in sync even on frames where Update did not run
        ApplyAutoResize();

        IconButton::Draw(gd, deltaTime);

        if (!fontSprite || !this->uiCamera) return;

        std::wstring currentText = GetCurrentText();

        if (currentText.empty()) return;

        fontSprite->Begin();
        fontSprite->SetText(std::wstring(currentText));
        fontSprite->SetScale(textScaleX, textScaleY);
        fontSprite->SetColor(textColor);
        fontSprite->SetOutline(hasOutline, outlineColor, 2.0f);

        //the metrics come back unscaled, so apply the text scale before centring
        float tw = fontSprite->GetWidth() * textScaleX;
        float th = fontSprite->GetHeight() * textScaleY;

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