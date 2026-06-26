#include "pch.h"
#include "DX9GFInputManager.h"
#include <stdexcept>
#include <DxErr.h>
#include "DX9GFApplication.h"

DX9GF::InputManager* DX9GF::InputManager::instance = nullptr;

bool DX9GF::InputManager::CheckKeysBuffer(int DIKey) const
{
    return keysBuffer[DIKey] & 0x80;
}

bool DX9GF::InputManager::CheckMouseBuffer(MouseButton button)
{
    return mouseBuffer[button] & 0x80;
}

void DX9GF::InputManager::UpdateMouseBuffer(int buttonIndex)
{
    if (firstMouseCheck && (diMouseState.rgbButtons[buttonIndex] != mouseBuffer[buttonIndex])) {
        lastMouseBufferPressed = lastMouseButtonPressed;
        lastMouseButtonPressed = buttonIndex;
        mouseDelta = 0;
        firstMouseCheck = false;
    }
}

DX9GF::InputManager* DX9GF::InputManager::GetInstance()
{
    if (instance == nullptr) {
        instance = new InputManager();
    }
    return instance;
}

void DX9GF::InputManager::Init(HWND hWnd, HINSTANCE hInstance)
{
    HRESULT result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, NULL);
    if (result != DI_OK) {
        auto error = DXGetErrorDescription(result);
        std::string what = std::string(error, error + wcslen(error));
        throw std::runtime_error(what);
    }
    result = dinput->CreateDevice(GUID_SysKeyboard, &diKeyboard, NULL);
    if (result != DI_OK) {
        auto error = DXGetErrorDescription(result);
        std::string what = std::string(error, error + wcslen(error));
        throw std::runtime_error(what);
    }
    diKeyboard->SetDataFormat(&c_dfDIKeyboard);
    diKeyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    result = diKeyboard->Acquire();
    if (result != DI_OK) {
        auto error = DXGetErrorDescription(result);
        std::string what = std::string(error, error + wcslen(error));
        throw std::runtime_error(what);
    }

    result = dinput->CreateDevice(GUID_SysMouse, &diMouse, NULL);
    if (result != DI_OK) {
        auto error = DXGetErrorDescription(result);
        std::string what = std::string(error, error + wcslen(error));
        throw std::runtime_error(what);
    }
    diMouse->SetDataFormat(&c_dfDIMouse);
    diMouse->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    result = diMouse->Acquire();
    if (result != DI_OK) {
        auto error = DXGetErrorDescription(result);
        std::string what = std::string(error, error + wcslen(error));
        throw std::runtime_error(what);
    }
}

void DX9GF::InputManager::ConsumeKey(int DIKey)
{
    if (DIKey >= 0 && DIKey < 256) {
        consumedKeys[DIKey] = true;
    }
}

void DX9GF::InputManager::ConsumeMouseButton(MouseButton button)
{
    if (button >= 0 && button < 4) {
        consumedMouseButtons[button] = true;
    }
}

void DX9GF::InputManager::ReadKeyboard(unsigned long long deltaTime)
{
    std::memset(consumedKeys, 0, sizeof(consumedKeys));
    keysDelta += deltaTime;
    if (keysDelta > KEYBOARD_LAST_PRESS_TIME) {
        ReleaseLastPressed();
    }
    for (int i = 0; i < 256; i++) {
        keysBuffer[i] = keys[i];
    }
    HRESULT result = diKeyboard->GetDeviceState(sizeof(keys), (LPVOID)&keys);
    while (result != DI_OK && (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED)) {
        result = diKeyboard->Acquire();
    }
    firstKeyCheck = true;
}

bool DX9GF::InputManager::KeyPress(int DIKey)
{
    if (consumedKeys[DIKey]) return false;
    if (keys[DIKey] & 0x80) {
        if (firstKeyCheck && keysBuffer[DIKey] != keys[DIKey]) {
            lastBufferPressed = lastKeyPressed;
            lastKeyPressed = DIKey;
            keysDelta = 0;
            firstKeyCheck = false;
        }
        return true;
    }
    return false;
}

bool DX9GF::InputManager::KeyDown(int DIKey)
{
    if (consumedKeys[DIKey]) return false;
    return KeyPress(DIKey) && !CheckKeysBuffer(DIKey);
}

bool DX9GF::InputManager::KeyUp(int DIKey)
{
    if (consumedKeys[DIKey]) return false;
    return !KeyPress(DIKey) && CheckKeysBuffer(DIKey);
}

void DX9GF::InputManager::ReleaseLastPressed()
{
    keysDelta = 0;
    lastKeyPressed = -1;
    lastBufferPressed = -1;
}

void DX9GF::InputManager::ReadMouse(unsigned long long deltaTime)
{
    std::memset(consumedMouseButtons, 0, sizeof(consumedMouseButtons));
    mouseDelta += deltaTime;
    if (mouseDelta > MOUSE_LAST_PRESS_TIME) {
        mouseDelta = 0;
        lastMouseButtonPressed = -1;
        lastMouseBufferPressed = -1;
    }
    for (int i = 0; i < 4; i++) {
        mouseBuffer[i] = diMouseState.rgbButtons[i];
    }

    // ==========================================
    // BẮT ĐẦU INVERSE TRANSFORM TỌA ĐỘ CHUỘT
    // ==========================================
    auto app = DX9GF::Application::GetInstance();
    HWND hwnd = app->GetHWnd();

    POINT rawMousePos;
    GetCursorPos(&rawMousePos);
    ScreenToClient(hwnd, &rawMousePos);

    // Lấy kích thước vật lý thật của cửa sổ Windows
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    float currentWidth = static_cast<float>(clientRect.right - clientRect.left);
    float currentHeight = static_cast<float>(clientRect.bottom - clientRect.top);

    // Kích thước ảo của game (ví dụ: 960x720)
    float virtualWidth = static_cast<float>(app->GetScreenWidth());
    float virtualHeight = static_cast<float>(app->GetScreenHeight());

    // Tính tỷ lệ Scale và Offset giống hệt Letterboxing
    float scaleX = currentWidth / virtualWidth;
    float scaleY = currentHeight / virtualHeight;
    float scale = (std::min)(scaleX, scaleY);

    float finalW = virtualWidth * scale;
    float finalH = virtualHeight * scale;

    float offsetX = (currentWidth - finalW) / 2.0f;
    float offsetY = (currentHeight - finalH) / 2.0f;

    // Dịch ngược tọa độ: Trừ phần viền đen và chia cho tỷ lệ Scale
    mousePos.x = static_cast<LONG>((rawMousePos.x - offsetX) / scale);
    mousePos.y = static_cast<LONG>((rawMousePos.y - offsetY) / scale);

    // CLAMP: Bức tường tàng hình chặn chuột không cho đi vào dải viền đen
    if (mousePos.x < 0) mousePos.x = 0;
    if (mousePos.x > static_cast<LONG>(virtualWidth)) mousePos.x = static_cast<LONG>(virtualWidth);
    if (mousePos.y < 0) mousePos.y = 0;
    if (mousePos.y > static_cast<LONG>(virtualHeight)) mousePos.y = static_cast<LONG>(virtualHeight);

    // ==========================================
    // KẾT THÚC INVERSE TRANSFORM
    // ==========================================

    if (!hasMousePos) {
        lastMousePos = mousePos;
        relativeMousePos = { 0, 0 };
        hasMousePos = true;
    }
    else {
        relativeMousePos = { mousePos.x - lastMousePos.x, mousePos.y - lastMousePos.y };
        lastMousePos = mousePos;
    }

    HRESULT result = diMouse->GetDeviceState(sizeof(diMouseState), (LPVOID)&diMouseState);
    while (result != DI_OK && (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED)) {
        result = diMouse->Acquire();
    }
    firstMouseCheck = true;
}

bool DX9GF::InputManager::MousePress(MouseButton button)
{
    if (consumedMouseButtons[button]) return false;
    if (diMouseState.rgbButtons[button] & 0x80) {
        UpdateMouseBuffer(button);
        return true;
    }
    return false;
}

bool DX9GF::InputManager::MouseDown(MouseButton button)
{
    if (consumedMouseButtons[button]) return false;
    return MousePress(button) && !CheckMouseBuffer(button);
}

bool DX9GF::InputManager::MouseUp(MouseButton button)
{
    if (consumedMouseButtons[button]) return false;
    return !MousePress(button) && CheckMouseBuffer(button);
}

long DX9GF::InputManager::GetRelativeMouseX() const
{
	return relativeMousePos.x;
}

long DX9GF::InputManager::GetRelativeMouseY() const
{
	return relativeMousePos.y;
}

POINT DX9GF::InputManager::GetRelativeMousePos() const
{
	return relativeMousePos;
}

long DX9GF::InputManager::GetAbsoluteMouseX() const
{
    return mousePos.x;
}

long DX9GF::InputManager::GetAbsoluteMouseY() const
{
    return mousePos.y;
}

POINT DX9GF::InputManager::GetAbsoluteMousePos() const
{
    return mousePos;
}

long DX9GF::InputManager::GetMouseScroll() const
{
    return diMouseState.lZ;
}
//
void DX9GF::InputManager::EnableCustomCursor(bool enable)
{
    //if the state is the same, do nothing to prevent the counter from jamming
    if (this->isCustomCursorActive == enable) return;

    this->isCustomCursorActive = enable;

    if (enable) 
    {
        //hide win default cursor when the counter is positive
        while (ShowCursor(FALSE) >= 0);
    }
    else 
    {
        //show win default cursor when the counter is negative
        while (ShowCursor(TRUE) < 0);
    }
}
void DX9GF::InputManager::AddCursor(CursorType type, DX9GF::GraphicsDevice* gd, const std::wstring& path, float scale, float hX, float hY)
{
    auto tex = std::make_shared<DX9GF::Texture>(gd);
    tex->LoadTexture(path);
    if (tex->GetRawTexture()) {
        auto sprite = std::make_shared<DX9GF::StaticSprite>(tex.get());
        sprite->SetScale(scale, scale);
        this->cursorTextures[type] = tex;
        this->cursorSprites[type] = sprite;
        this->hotspotsX[type] = hX;
        this->hotspotsY[type] = hY;
        this->EnableCustomCursor(true);
    }
}

void DX9GF::InputManager::SwitchCursor(CursorType type)
{
    if (this->cursorSprites.count(type)) {
        this->currentCursorType = type;
    }
}

void DX9GF::InputManager::DrawCursor(DX9GF::Camera* uiCamera, unsigned long long deltaTime)
{
    if (!isCustomCursorActive || !uiCamera || !cursorSprites.count(currentCursorType)) return;

    auto activeSprite = cursorSprites[currentCursorType];
    float hX = hotspotsX[currentCursorType];
    float hY = hotspotsY[currentCursorType];

    auto app = DX9GF::Application::GetInstance();

    // LẤY TỌA ĐỘ ĐÃ ĐƯỢC CHUYỂN ĐỔI CHUẨN XÁC TỪ READ MOUSE
    POINT mappedMousePos = this->GetAbsoluteMousePos();

    float drawX = (float)mappedMousePos.x - (app->GetScreenWidth() / 2.0f) - hX;
    float drawY = (float)mappedMousePos.y - (app->GetScreenHeight() / 2.0f) - hY;

    activeSprite->SetPosition(drawX, drawY);
    activeSprite->Begin();
    activeSprite->Draw(*uiCamera, deltaTime);
    activeSprite->End();

    //auto reset state for next frame
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)//check hold left mouse
    {
        this->currentCursorType = CursorType::GRAB;
    }
    else
    {
        this->currentCursorType = CursorType::CURSOR;
    }
}

void DX9GF::InputManager::Dispose()
{
    if (diKeyboard != nullptr) {
        diKeyboard->Unacquire();
        diKeyboard->Release();
        diKeyboard = nullptr;
    }
    if (diMouse != nullptr) {
        diKeyboard->Unacquire();
        diMouse->Release();
        diMouse = nullptr;
    }
    if (dinput != nullptr) {
        dinput->Release();
        dinput = nullptr;
    }
}
