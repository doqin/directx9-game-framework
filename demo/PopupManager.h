#pragma once
#include "DX9GF.h"
#include "DX9GFSprites.h"
#include "DX9GFFont.h"
#include "DX9GFTexture.h"
#include "IconButton.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <map>

namespace Demo {
    class PopupDynamicButton {
    public:
        std::shared_ptr<IconButton> backgroundBtn;
        std::wstring label;
        std::function<void()> onClick;

        PopupDynamicButton(std::shared_ptr<IconButton> btn, const std::wstring& text, std::function<void()> callback)
            : backgroundBtn(btn), label(text), onClick(callback) {
        }
    };

    struct PopupStyle {
        RECT bgSrcRect;
        int marginL, marginT, marginR, marginB;
        D3DCOLOR titleColor;
        D3DCOLOR msgColor;
        bool hasOutline;
        D3DCOLOR outlineColor;
    };

    class PopupManager {
    private:
        static PopupManager* instance;

        DX9GF::GraphicsDevice* gd = nullptr;
        DX9GF::Camera* uiCamera = nullptr;

        std::shared_ptr<DX9GF::Texture> uiSheetTex;
        std::shared_ptr<DX9GF::Texture> borderSheetTex;
        std::shared_ptr<DX9GF::Font> font;
        std::shared_ptr<DX9GF::FontSprite> fontSprite;
        std::shared_ptr<DX9GF::NineSliceSprite> backgroundSprite;
        std::shared_ptr<DX9GF::TransformManager> internalTm;

        bool isActive = false;
        std::wstring currentTitle;
        std::wstring currentMessage;
        std::vector<std::shared_ptr<PopupDynamicButton>> activeButtons;

        std::vector<RECT> btnRectsSmall;
        std::vector<RECT> btnRectsMed;
        std::vector<RECT> btnRectsLarge;

        float popupWidth = 400.0f;
        float popupHeight = 200.0f;
        float popupX = 0.0f;
        float popupY = 0.0f;

        PopupManager();
        ~PopupManager() = default;

        void LayoutButtons();

    public:
        std::map<std::string, PopupStyle> styles;
        std::string currentStyle;

        static PopupManager* GetInstance();

        void Init(DX9GF::GraphicsDevice* gd, std::shared_ptr<DX9GF::Texture> borderTex, std::shared_ptr<DX9GF::Texture> uiTex, std::shared_ptr<DX9GF::Font> popupFont);

        void SetUICamera(DX9GF::Camera* camera) { this->uiCamera = camera; }

        void RegisterStyle(const std::string& styleName, const PopupStyle& style);

        void Show(const std::string& styleName, const std::wstring& title, const std::wstring& message,
            const std::vector<std::pair<std::wstring, std::function<void()>>>& buttons);

        void Close();
        bool IsActive() const { return isActive; }

        void Update(unsigned long long deltaTime, DX9GF::Camera* uiCamera);
        void DrawUI(unsigned long long deltaTime, DX9GF::Camera* uiCamera);
    };
}