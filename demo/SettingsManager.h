#pragma once
#include "Windows.h"
#include "DX9GF.h"

namespace Demo
{
	struct Resolution {
		int width;
		int height;
		std::string label;
		bool isFullScreenMode;
	};

	class SettingsManager
	{
	private:
		static SettingsManager* instance;
		SettingsManager();

		float masterVolume;
		float musicVolume;
		float sfxVolume;

		std::map<std::string, int> keybinds;

		std::string configFilePath;
		bool isFullscreen;

		std::vector<Resolution> supportedResolutions;
		int currentResIndex;
	public:
		static SettingsManager* GetInstance()
		{
			if (!instance)
			{
				instance = new SettingsManager();
				instance->LoadSettings();
			}
			return instance;
		}

		void SetDefaultSettings();

		bool LoadSettings();
		bool SaveSettings();

		void SetMusicVolume(float vol);
		float GetMusicVolume() { return musicVolume; }

		void SetMasterVolume(float vol);
		float GetMasterVolume() { return masterVolume; }

		void SetSfxVolume(float vol);
		float GetSfxVolume() { return sfxVolume; }

		void SetKeybind(std::string actionName, int keyCode);
		int GetKeybind(std::string actionName);

		void SetFullscreen(bool fs);
		bool GetFullscreen() { return isFullscreen; }

		const std::vector<Resolution>& GetSupportedResolutions() const { return supportedResolutions; }
		int GetCurrentResolutionIndex() const { return currentResIndex; }
		void SetResolutionIndex(int index);

		std::wstring GetKeybindDisplayName(std::string actionName);

		// Helper function to apply immediately
		void ApplyResolution();
	};
}
