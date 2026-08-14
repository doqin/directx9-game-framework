#pragma once
#include "IconButton.h"
#include "DX9GFFont.h"
#include "DX9GFSprites.h"
#include <string>
#include <unordered_map>

namespace Demo {
    class TextIconButton : public IconButton {
    private:
        std::wstring label;
        std::shared_ptr<DX9GF::FontSprite> fontSprite;
        float textScaleX = 1.0f;
        float textScaleY = 1.0f;
        D3DCOLOR textColor = 0xFF111111;
        bool hasOutline = false;
        D3DCOLOR outlineColor = 0xFF000000;

        float baseTextOffsetY = 0.0f;
        //Maps each state to its corresponding text offset
        std::unordered_map<ButtonState, float> stateOffsetsY;

        std::function<std::wstring()> dynamicLabelGetter;

        //dynamic width, the button grows to the right so it always fits its label
        bool autoResize = false;
        float autoResizePaddingX = 12.0f;
        float minWidth = 0.0f;

        std::wstring GetCurrentText() const;
        float MeasureTextWidth(const std::wstring& text) const;
        void ApplyAutoResize();
    public:
        TextIconButton(std::shared_ptr<DX9GF::TransformManager> tm, float displayX, float displayY, int imgW, int imgH,
            std::shared_ptr<DX9GF::Texture> uiSheetTex, DX9GF::Font* font, const std::wstring& text, int frames = 3);

        void SetText(const std::wstring& text) { this->label = text; }
        void SetTextColor(D3DCOLOR color) { this->textColor = color; }
        void SetTextScale(float scaleX, float scaleY) { this->textScaleX = scaleX; this->textScaleY = scaleY; }
        void SetTextOutline(bool outline, D3DCOLOR color = 0xFF000000) { this->hasOutline = outline; this->outlineColor = color; }
        void SetBaseTextOffsetY(float offset) { this->baseTextOffsetY = offset; }
        void SetStateOffsetY(ButtonState state, float offsetY) { this->stateOffsetsY[state] = offsetY; }
        void SetDynamicTextGetter(std::function<std::wstring()> getter) {
            this->dynamicLabelGetter = getter;
        }
        //Resizes the button every frame to fit its label plus paddingX on both sides.
        //Pair it with IconButton::SetSliceMargins so the sprite stretches instead of smearing.
        void SetAutoResize(bool enabled, float paddingX = 12.0f);
        //Width the button never shrinks below, defaults to the size it was created with.
        void SetMinWidth(float width) { this->minWidth = width; }

        void SetSpriteScale(float scaleX, float scaleY);
        void Update(unsigned long long deltaTime) override;
        void Draw(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) override;
    };
}