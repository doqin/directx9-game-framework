#pragma once
#include "dinput.h"
//
#include "DX9GFSprites.h"
#include "DX9GFCamera.h"
#include <memory>
//
#define KEYBOARD_BUFFER_SIZE 1024
#define KEYBOARD_LAST_PRESS_TIME 125
#define MOUSE_LAST_PRESS_TIME 125




namespace DX9GF {
	class InputManager final {
	public:
		enum MouseButton {
			Left = 0,
			Right = 1,
			Middle = 2
		};

		enum class CursorType //i use enum class becuz it's more secure, i guess
		{
			CURSOR,
			POINTER,
			CLICK,
			GRAB,
			TEXTSELECT,
		};

	private:
		static InputManager* instance;

		LPDIRECTINPUT8 dinput = nullptr;
		LPDIRECTINPUTDEVICE8 diKeyboard = nullptr;
		LPDIRECTINPUTDEVICE8 diMouse = nullptr;
		DIMOUSESTATE diMouseState;
		BYTE mouseBuffer[4];
		POINT mousePos = {0, 0};
		POINT lastMousePos = {0, 0};
		POINT relativeMousePos = {0, 0};
		bool hasMousePos = false;

		//CUSTOM CURSOR VARS
		std::map<CursorType, std::shared_ptr<DX9GF::Texture>> cursorTextures;
		std::map<CursorType, std::shared_ptr<DX9GF::StaticSprite>> cursorSprites;
		std::map<CursorType, float> hotspotsX; // = 
		std::map<CursorType, float> hotspotsY;
		CursorType currentCursorType = CursorType::CURSOR; //save state, cursor default
		bool isCustomCursorActive = false;

		InputManager() {}
		~InputManager() {}

		char keys[256];
		char keysBuffer[256];
		bool consumedKeys[256] = { false };
		bool consumedMouseButtons[4] = { false };
		int lastKeyPressed = -1;
		int lastBufferPressed = -1;
		int lastMouseButtonPressed = -1;
		int lastMouseBufferPressed = -1;
		bool firstKeyCheck = true;
		bool firstMouseCheck = true;
		unsigned long long keysDelta = 0;
		unsigned long long mouseDelta = 0;

		bool CheckKeysBuffer(int DIKey) const;
		bool CheckMouseBuffer(MouseButton button);
		void UpdateMouseBuffer(int buttonIndex);
	public:
		static InputManager* GetInstance();
		void Init(HWND, HINSTANCE);
		void ConsumeKey(int DIKey);
		void ConsumeMouseButton(MouseButton button);
		void ReadKeyboard(unsigned long long deltaTime);
		bool KeyPress(int DIKey);
		bool KeyDown(int DIKey);
		bool KeyUp(int DIKey);
		void ReleaseLastPressed();
		void ReadMouse(unsigned long long deltaTime);
		bool MousePress(MouseButton button);
		bool MouseDown(MouseButton button);
		bool MouseUp(MouseButton button);
		long GetRelativeMouseX() const;
		long GetRelativeMouseY() const;
		POINT GetRelativeMousePos() const;
		long GetAbsoluteMouseX() const;
		long GetAbsoluteMouseY() const;
		POINT GetAbsoluteMousePos() const;
		long GetMouseScroll() const;
		std::tuple<float, float> GetVirtualAbsoluteMousePos(const DX9GF::Camera* camera) const;
		std::tuple<float, float> GetVirtualRelativeMousePos(const DX9GF::Camera* camera) const;
		void AddCursor(CursorType type, DX9GF::GraphicsDevice* gd, const std::wstring& path, float scale, float hX, float hY);
		void SwitchCursor(CursorType type);
		void EnableCustomCursor(bool enable);
		void DrawCursor(DX9GF::Camera* uiCamera, unsigned long long deltaTime);
		void Dispose();

	};
}