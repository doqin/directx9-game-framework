#include "pch.h"
#include <Windows.h>
#include "DX9GF.h"
#include "Game.h"

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
)
{
	// Get the DX9GF Application
	DX9GF::Application* app = DX9GF::Application::GetInstance();
	try {
        app->Init(hInstance, L"Demo", 930, 720, false);
		// Create your game that interfaces with IGame
		Demo::Game game(app->GetHWnd(), app->GetScreenWidth(), app->GetScreenHeight());
		// Attach your game to the application
		app->AttachGame(&game);
		// Run the application
		app->Run();
	}
	catch (std::exception e) {
		MessageBox(
			NULL,
			std::wstring(e.what(), e.what() + strlen(e.what())).c_str(),
			L"Error", MB_OK | MB_ICONEXCLAMATION
		);
		return E_FAIL;
	}


	return 0;
}