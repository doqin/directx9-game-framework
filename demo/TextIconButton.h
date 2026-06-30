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
        void SetSpriteScale(float scaleX, float scaleY);
        void Draw(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) override;
    };
}