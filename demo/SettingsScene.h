#pragma once
#include "Game.h"
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "IconButton.h"
#include "TextIconButton.h"
#include "KeyboardNavigator.h"
namespace Demo
{
    //UI constants
    constexpr float LABEL_COLUMN_X = -144.0f;
    constexpr float SLIDER_COLUMN_X = 20.0f;
    constexpr float SLIDER_DESIRED_WIDTH = 100.0f;
    constexpr float SLIDER_ORIGINAL_WIDTH = 47.0f;
    constexpr float SLIDER_SCALE_X = SLIDER_DESIRED_WIDTH / SLIDER_ORIGINAL_WIDTH;

    constexpr float SPACING_Y = 30.0f;
    constexpr float BUTTON_OFFSET_X = 25.0f;
    constexpr float ALIGN_OFFSET_Y = 10.0f;

    // Keybind rows share startY/rowSpacing with the volume rows; with 8 keybind
    // rows a tighter step is needed to keep the last row on screen.
    constexpr float KEYBIND_ROW_START = 5.0f;
    constexpr float KEYBIND_ROW_STEP = 1.1f;

    //save offset to center the keybinds into box (just UI)
    constexpr int BTN_VISUAL_SHADOW_OFFSET = -2;

    class SettingsScene : public DX9GF::IScene
    {
    private:
        Game* game;
        std::shared_ptr<DX9GF::TransformManager> transformManager;

        std::shared_ptr<DX9GF::StaticSprite> bgSprite;
        std::shared_ptr<DX9GF::Texture> bgTex;

        std::shared_ptr<DX9GF::Font> font;
        std::shared_ptr<DX9GF::FontSprite> fontSprite;
        std::shared_ptr<DX9GF::Texture> placeholderTex;
        std::shared_ptr<DX9GF::Texture> uiSheetTex;
        std::vector<std::shared_ptr<Demo::IButton>> uiButtons;
        std::shared_ptr<IconButton> backButton;
        std::shared_ptr<Demo::IconButton> btnResPrev;
        std::shared_ptr<Demo::IconButton> btnResNext;

        bool isGoingBack = false;

        int lastScreenWidth;
        int lastScreenHeight;

        //volume UI
        std::shared_ptr<DX9GF::NineSliceSprite> trackMaster, trackMusic, trackSFX;
        std::shared_ptr<DX9GF::NineSliceSprite> trackMasterFill, trackMusicFill, trackSFXFill;

        std::shared_ptr<Demo::IconButton> btnMasterDec, btnMasterInc;
        std::shared_ptr<Demo::IconButton> btnMusicDec, btnMusicInc;
        std::shared_ptr<Demo::IconButton> btnSFXDec, btnSFXInc;

        //keybind UI
        std::shared_ptr<Demo::TextIconButton> btnUp, btnDown, btnLeft, btnRight;
        std::shared_ptr<Demo::TextIconButton> btnAccept, btnOpenInventory, btnInteract, btnSprint;

        bool isListeningUp = false;
        bool isListeningDown = false;
        bool isListeningLeft = false;
        bool isListeningRight = false;
        bool isListeningAccept = false;
        bool isListeningOpenInventory = false;
        bool isListeningInteract = false;
        bool isListeningSprint = false;

        KeyboardNavigator keyboardNavigator;
        std::vector<KeyboardNavigator::Candidate> CollectKeyboardCandidates();
        bool IsAnyKeybindListening() const;

        //helper methods
        void DrawBackground(unsigned long long deltaTime);
        void DrawVolumeTrack(std::shared_ptr<DX9GF::NineSliceSprite> bg, std::shared_ptr<DX9GF::NineSliceSprite> fill, float vol, RECT originalRect, unsigned long long deltaTime);

    public:
        SettingsScene(Game* game, int screenWidth, int screenHeight)
            : IScene(screenWidth, screenHeight),
            game(game) {
        }

        void Init() override;
        void Update(unsigned long long deltaTime) override;
        void DrawWorld(unsigned long long deltaTime) override;
        void DrawUI(unsigned long long deltaTime) override;
        void UpdateLayout(int width, int height);
        void DrawString(std::wstring text, float x, float y, D3DCOLOR color, DWORD format = DT_RIGHT);
        void ResetListening();
    };
}