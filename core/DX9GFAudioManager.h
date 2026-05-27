#pragma once
#include <windows.h>
#include <xaudio2.h>
#include <mmsystem.h>
#include <vector>
#include <string>
#include <map>

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "winmm.lib")

namespace DX9GF {
	struct SoundBuffer
	{
		//using XAudio2 instead of directsound
		WAVEFORMATEX wfx{}; //sample rate, channels,...
		std::vector <BYTE> audioRawData; //raw data
		XAUDIO2_BUFFER buffer; //audio buffering to avoid disk I/O latency

		SoundBuffer()
		{
			ZeroMemory(&buffer, sizeof(XAUDIO2_BUFFER));
		}
	};

	class VoiceCallback : public IXAudio2VoiceCallback
	{
	public:
		bool isFinished = false; //finish voice flag

		//play last sound byte will trigger XAudio2 to use this function
		void STDMETHODCALLTYPE OnBufferEnd(void* pBufferContext) override
		{
			isFinished = true;
		}

		//Interface must-have function
		void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32) override {}
		void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override {}
		void STDMETHODCALLTYPE OnStreamEnd() override {}
		void STDMETHODCALLTYPE OnBufferStart(void*) override {}
		void STDMETHODCALLTYPE OnLoopEnd(void*) override {}
		void STDMETHODCALLTYPE OnVoiceError(void*, HRESULT) override {}
	};

	enum class AudioType 
	{
		SFX,
		MUSIC
	};

	struct ActiveVoice
	{
		std::string name; //identify
		IXAudio2SourceVoice* pVoice;
		VoiceCallback* pCallback;
		AudioType type;
		float baseVolume; //save the volume level when play
	};

	bool LoadWavFromResource(int resourceID, SoundBuffer& out_audio);

	class AudioManager {
	private:
		IXAudio2* pEngine = nullptr;              //control XAudio2
		IXAudio2MasteringVoice* pMasterVoice = nullptr; //output
		std::map<std::string, SoundBuffer*> cache; //cache will save the loaded file (avoid disk loading latency)
		std::map<std::string, std::vector<std::string>> soundBanks;
		std::vector<ActiveVoice*> activeVoices; //list of playing sound

		//settings manager will push values to these vars
		float currentMasterVolume = 1.0f;
		float currentMusicVolume = 1.0f;
		float currentSfxVolume = 1.0f;

		// Biến cho Cross-Fade BGM
		bool isFading = false;
		float fadeTimer = 0.0f;
		float fadeDuration = 0.0f;

		// Bài đang phát (bị giảm nhỏ tiếng)
		std::string fadingOutSound = "";

		// Bài mới (chờ bật lên)
		std::string fadingInSound = "";
		float fadingInTargetVolume = 1.0f;
		bool fadingInLoop = true;

		AudioManager() {}
		~AudioManager() {}
		static AudioManager* instance;
	public:
		static AudioManager* GetInstance();
		static void DestroyInstance();
		bool Init();

		//load sound from file to cache
		void Load(std::string name, int resID);

		void Play(std::string name, bool loop = false, float volume = 1.0f, AudioType type = AudioType::SFX);
		void Update(unsigned long long deltaTime);
		void Stop(std::string name);
		void StopAll();
		void Shutdown();

		//set game volume
		void SetMasterVolume(float volume);
		void SetMusicVolume(float volume);
		void SetSfxVolume(float volume);
		
		//manage various types of footstep
		void RegisterBank(std::string bankName, std::vector<std::string> soundNames);
		void PlayRandom(std::string bankName, float volume = 1.0f, AudioType type = AudioType::SFX);

		//background music
		void PlayBGM_Fade(std::string name, float targetVolume = 0.5f, float duration = 1.5f);
		void PlayRandomBGM_Fade(std::string bankName, float targetVolume = 0.5f, float duration = 1.5f);
	};
}