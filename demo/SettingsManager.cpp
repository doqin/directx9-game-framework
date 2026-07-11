#include "pch.h"
#include "SettingsManager.h"
#include <fstream>
#include <cctype> 
#include "DX9GFAudioManager.h"

namespace Demo
{
	SettingsManager* SettingsManager::instance = nullptr;
	SettingsManager::SettingsManager() {
		supportedResolutions = {
			{ 800, 600, "800x600", false },
			{ 960, 720, "960x720 (Native)", false },
			{ 1024, 768, "1024x768", false },
			{ 1280, 960, "1280x960", false },
			{ 1440, 1080, "1440x1080", false },
			{ 0, 0, "Fullscreen", true }
		};
		currentResIndex = 1;
	}

	void SettingsManager::SetDefaultSettings()
	{
		this->SetMusicVolume(1.0f);
		this->SetSfxVolume(1.0f);
		this->SetMasterVolume(1.0f);
		this->SetFullscreen(false);

		keybinds["MOVE_LEFT"] = DIK_A;
		keybinds["MOVE_UP"] = DIK_W;
		keybinds["MOVE_RIGHT"] = DIK_D;
		keybinds["MOVE_DOWN"] = DIK_S;
		keybinds["ACCEPT"] = DIK_RETURN;
		keybinds["OPEN_INVENTORY"] = DIK_ESCAPE;
		keybinds["INTERACT"] = DIK_E;
		keybinds["SPRINT"] = DIK_LSHIFT;
	}

	bool SettingsManager::LoadSettings()
	{
		// Seed defaults first so actions missing from an older config.ini keep
		// their default binds instead of GetKeybind falling through to 0.
		this->SetDefaultSettings();

		std::ifstream fileInput("config.ini");

		if (!fileInput.is_open())
		{
			this->SaveSettings();
			return true;
		}

		std::string line;

		while (std::getline(fileInput, line))
		{
			//skip empty line
			if (line.empty())
				continue;

			//find '=' to split string
			size_t delimiterPos = line.find('=');
			if (delimiterPos == std::string::npos) continue; //skip line without '='

			std::string key = line.substr(0, delimiterPos);
			std::string value = line.substr(delimiterPos + 1);

			//classify data
			if (key == "MASTER_VOL") 
			{
				this->SetMasterVolume(std::stof(value));
			}
			else if (key == "MUSIC_VOL") 
			{
				this->SetMusicVolume(std::stof(value));
			}
			else if (key == "SFX_VOL") 
			{
				this->SetSfxVolume(std::stof(value));
			}
			else if (key == "FULLSCREEN") {
				this->SetFullscreen(std::stoi(value));
			}
			else if (key == "RESOLUTION_INDEX") {
				this->SetResolutionIndex(std::stoi(value));
			}
			else 
			{
				this->keybinds[key] = std::stoi(value);
			}
		}

		fileInput.close();
		return true;
	}
	bool SettingsManager::SaveSettings()
	{
		std::ofstream file("config.ini");
		if (!file.is_open()) return false;

		file << "MASTER_VOL=" << masterVolume << "\n";
		file << "MUSIC_VOL=" << musicVolume << "\n";
		file << "SFX_VOL=" << sfxVolume << "\n";
		file << "FULLSCREEN=" << isFullscreen << "\n";
		file << "RESOLUTION_INDEX=" << currentResIndex << "\n";
		for (const auto& pair : keybinds)
		{
			file << pair.first << "=" << pair.second << "\n";
		}

		file.close();
		return true;
	}

	void SettingsManager::SetMusicVolume(float vol)
	{
		this->musicVolume = vol;
		DX9GF::AudioManager::GetInstance()->SetMusicVolume(vol); //connect to AudioManager and push vol
	}

	void SettingsManager::SetMasterVolume(float vol)
	{
		this->masterVolume = vol;
		DX9GF::AudioManager::GetInstance()->SetMasterVolume(vol);
	}

	void SettingsManager::SetSfxVolume(float vol)
	{
		this->sfxVolume = vol;
		DX9GF::AudioManager::GetInstance()->SetSfxVolume(vol);
	}

	void SettingsManager::SetKeybind(std::string actionName, int keyCode)
	{
		this->keybinds[actionName] = keyCode;
	}
	int SettingsManager::GetKeybind(std::string actionName)
	{
		// find actionName in keybinds map
		auto it = keybinds.find(actionName);

		if (it != keybinds.end()) 
		{
			return it->second;
		}
		return 0;
	}
	void SettingsManager::SetFullscreen(bool fs)
	{
		this->isFullscreen = fs;
		auto app = DX9GF::Application::GetInstance();
		if (app && app->GetHWnd()) {
			app->SetFullscreen(fs);
		}
	}

	void SettingsManager::SetResolutionIndex(int index) {
		if (index >= 0 && index < supportedResolutions.size()) {
			currentResIndex = index;
		}
	}

	void SettingsManager::ApplyResolution() {
		auto app = DX9GF::Application::GetInstance();
		if (!app || !app->GetHWnd()) return;

		auto res = supportedResolutions[currentResIndex];

		if (res.isFullScreenMode) {
			app->SetFullscreen(true);
			this->SetFullscreen(true);
		}
		else {
			// Disable fullscreen to allow Windows to restore the title bar
			app->SetFullscreen(false);
			this->SetFullscreen(false);

			HWND hwnd = app->GetHWnd();
			DWORD style = GetWindowLong(hwnd, GWL_STYLE);

			RECT windowRect = { 0, 0, static_cast<LONG>(res.width), static_cast<LONG>(res.height) };
			AdjustWindowRect(&windowRect, style, FALSE);

			int windowWidth = windowRect.right - windowRect.left;
			int windowHeight = windowRect.bottom - windowRect.top;

			MONITORINFO mi = { sizeof(mi) };
			GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi);

			int workWidth = mi.rcWork.right - mi.rcWork.left;
			int workHeight = mi.rcWork.bottom - mi.rcWork.top;

			int posX = mi.rcWork.left + (workWidth - windowWidth) / 2;
			int idealPosY = mi.rcWork.top + (workHeight - windowHeight) / 3;
			int safePosY = mi.rcWork.top + 10;
			int posY = (std::max)(idealPosY, safePosY);

			SetWindowPos(hwnd, HWND_TOP, posX, posY,
				windowWidth,
				windowHeight,
				SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
}