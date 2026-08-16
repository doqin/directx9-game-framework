#include "pch.h"
#include <Windows.h>
#include "DX9GF.h"
#include "Game.h"

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx9.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
        app->SetAppIcon(L"assets/icon.ico");
		app->OverrideWindowProc(ImGui_ImplWin32_WndProcHandler);
		app->SetOnDeviceResetHandler([]() {
			ImGui_ImplDX9_InvalidateDeviceObjects();
			ImGui_ImplDX9_CreateDeviceObjects();
		});
        app->Init(hInstance, L"Demo", 960, 720, false);
		// Create your game that interfaces with IGame
		Demo::Game game(app->GetHWnd(), app->GetScreenWidth(), app->GetScreenHeight());
		game.Init();
		// Attach your game to the application
		app->AttachGame(&game);
		// Run the application
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		ImGui::StyleColorsDark();
		ImGui_ImplWin32_Init(app->GetHWnd());
		ImGui_ImplDX9_Init(game.GetGraphicsDevice()->GetDevice());
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