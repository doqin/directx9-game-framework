#include "pch.h"
#include "DX9GFApplication.h"
#include <stdexcept>
#include "DX9GFFont.h"
#include "DX9GFInputManager.h"
#include "DX9GFAudioManager.h"
DX9GF::Application* DX9GF::Application::instance = nullptr;
DX9GF::IGame* p_game = nullptr;

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SIZE:
		if (p_game != nullptr) {
			p_game->OnResize(LOWORD(lParam), HIWORD(lParam));
		}
		if (DX9GF::Application::GetInstance() != nullptr) {
			DX9GF::Application::GetInstance()->OnResize(LOWORD(lParam), HIWORD(lParam));
		}
		return 0;
	case WM_DESTROY:
		p_game->Dispose();
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void DX9GF::Application::Init(HINSTANCE hInstance, std::wstring appTitle, UINT screenWidth, UINT screenHeight, bool resizable)
{
	this->hInstance = hInstance;
	this->appTitle = appTitle;
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;

	// Ensure the current working directory is the executable directory so relative paths work
	wchar_t exePath[MAX_PATH]{};
	DWORD exePathLen = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
	if (exePathLen > 0 && exePathLen < MAX_PATH) {
		std::wstring exePathStr(exePath, exePathLen);
		size_t lastSlash = exePathStr.find_last_of(L"\\/");
		if (lastSlash != std::wstring::npos) {
			std::wstring exeDir = exePathStr.substr(0, lastSlash);
			SetCurrentDirectoryW(exeDir.c_str());
		}
	}

	// Very important =)))
	AppRegisterClass();

    DWORD windowStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
	if (!resizable) {
		windowStyle &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
	}

	RECT windowRect = { 0, 0, static_cast<LONG>(screenWidth), static_cast<LONG>(screenHeight) };
	AdjustWindowRect(&windowRect, windowStyle, FALSE);

	// T?o m?t c?a s?
    hwnd = CreateWindow(
		appTitle.c_str(),
		appTitle.c_str(),
       windowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left, // chi?u r?ng
		windowRect.bottom - windowRect.top, // chi?u cao
		NULL, // c?a s? cha
		NULL, // menu
		hInstance, // instance
		NULL // Các tham s? c?a s?
	);

	if (!hwnd) {
		throw std::runtime_error("Error creating window");
	}

	DX9GF::InputManager::GetInstance()->Init(GetHWnd(), hInstance);
}

void DX9GF::Application::SetFramerate(int fps)
{
	frameRate = fps;
}

HWND DX9GF::Application::GetHWnd() const
{
	return hwnd;
}

DX9GF::Application* DX9GF::Application::GetInstance()
{
	if (instance == nullptr) {
		instance = new Application();
	}
	return instance;
}

unsigned int DX9GF::Application::GetScreenWidth() const
{
	return screenWidth;
}

unsigned int DX9GF::Application::GetScreenHeight() const
{
	return screenHeight;
}

void DX9GF::Application::OnResize(UINT width, UINT height)
{
	this->screenWidth = width;
	this->screenHeight = height;
}

void DX9GF::Application::AttachGame(IGame* game)
{
	p_game = game;
}

void DX9GF::Application::Run()
{
	p_game->Init();

	//test audio
	auto audioManager = AudioManager::GetInstance();
	audioManager->Init();

	MSG msg;
	int done = 0;
	unsigned long long start = GetTickCount64();
	while (!done) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) done = 1;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			if (frameRate == -1 || GetTickCount64() - start >= 1000 / frameRate) {
				unsigned long long deltaTime = GetTickCount64() - start;
				start = GetTickCount64();
				p_game->Update(deltaTime);
				audioManager->Update();
				p_game->Draw(deltaTime);
			}
		}
	}
	// Cleanup any fonts added at runtime
	DX9GF::Font::RemoveAllFonts();
	audioManager->Shutdown();
}

ATOM DX9GF::Application::AppRegisterClass()
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = DX9GF::Application::appTitle.c_str();
	wc.hIconSm = NULL;

	return RegisterClassEx(&wc);
}
