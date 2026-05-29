#include "pch.h"
#include "DX9GFAudioManager.h"

// chunk structure of WAV files
#pragma pack(push, 1)
struct ChunkHeader {
	char id[4];
	DWORD size;
};
#pragma pack(pop)

bool DX9GF::LoadWavFromResource(int resourceID, DX9GF::SoundBuffer& out_audio)
{
	//find embedded resource in this project by its ID
	HRSRC hResInfo = FindResource(NULL, MAKEINTRESOURCE(resourceID), L"WAVE");
	if (hResInfo == NULL)
		return false;

	//load resource
	HGLOBAL hResData = LoadResource(NULL, hResInfo);
	if (hResData == NULL)
		return false;

	//lock the resource to point the pointer into resource data
	void* pResourceData = LockResource(hResData);
	DWORD resourceSize = SizeofResource(NULL, hResInfo);
	if (pResourceData == NULL)
		return false;

	//embedded resource can't use mmio, gotta use this technique (just use for pcm wav)

	BYTE* pData = (BYTE*)pResourceData;
	DWORD offset = 12; //skip "RIFF", size, "WAVE"

	bool foundFmt = false;
	bool foundData = false;

	while (offset < resourceSize) {
		ChunkHeader* header = (ChunkHeader*)(pData + offset);
		offset += sizeof(ChunkHeader);

		if (strncmp(header->id, "fmt ", 4) == 0) {
			//take rate and channels,... from file
			memcpy(&out_audio.wfx, pData + offset, (header->size < sizeof(WAVEFORMATEX)) ? header->size : sizeof(WAVEFORMATEX)); //avoid include lib
			foundFmt = true;
		}
		else if (strncmp(header->id, "data", 4) == 0) {
			//take raw audio data from file
			out_audio.audioRawData.resize(header->size);
			memcpy(out_audio.audioRawData.data(), pData + offset, header->size);

			//set the buffer on
			out_audio.buffer.AudioBytes = header->size;
			out_audio.buffer.pAudioData = out_audio.audioRawData.data();
			out_audio.buffer.Flags = XAUDIO2_END_OF_STREAM;

			foundData = true;
			break;
		}

		offset += header->size;
	}

	return foundFmt && foundData;
}

DX9GF::AudioManager* DX9GF::AudioManager::instance = nullptr;

DX9GF::AudioManager* DX9GF::AudioManager::GetInstance()
{
	if (!instance) {
		instance = new AudioManager();
	}
	return instance;
}

void DX9GF::AudioManager::DestroyInstance()
{
	if (instance) {
		instance->Shutdown();
		delete instance;
		instance = nullptr;
	}
}

bool DX9GF::AudioManager::Init()
{
	//turn on COM of Windows
	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
		return false;

	if (FAILED(XAudio2Create(&pEngine, 0, XAUDIO2_DEFAULT_PROCESSOR)))
		return false;

	if (FAILED(pEngine->CreateMasteringVoice(&pMasterVoice)))
		return false;

	return true;
}

void DX9GF::AudioManager::Load(std::string name, int resID)
{
	if (cache.count(name)) return;
	SoundBuffer* ad = new SoundBuffer();
	if (LoadWavFromResource(resID, *ad))
	{
		cache[name] = ad;
	}
	else
	{
		delete ad;
	}
}

void DX9GF::AudioManager::Play(std::string name, bool loop, float volume, AudioType type)
{
	//set a limit voice count to protect the engine 
	if (activeVoices.size() > 64) {
		return;
	}

	//can't find sound name from cache
	if (!cache.count(name)) return;

	SoundBuffer* data = cache[name];

	//create a callback for this turn
	DX9GF::VoiceCallback* cb = new VoiceCallback();
	IXAudio2SourceVoice* pVoice = nullptr;

	if (FAILED(pEngine->CreateSourceVoice(&pVoice, &data->wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, cb, NULL, NULL)))
	{
		delete cb;
		return;
	}

	//set the loop
	if (loop == true)
	{
		//for background music to loop(?)
		data->buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}
	else
	{
		//for normal sound
		data->buffer.LoopCount = 0;
	}

	float typeVol = (type == AudioType::MUSIC) ? currentMusicVolume : currentSfxVolume;
	pVoice->SetVolume(volume * typeVol * currentMasterVolume);
	//play the sound
	pVoice->SubmitSourceBuffer(&data->buffer);
	pVoice->Start(0);

	//save into list to control it easily
	activeVoices.push_back(new ActiveVoice{ name, pVoice, cb, type, volume });
}

void DX9GF::AudioManager::Update(unsigned long long deltaTime) {

	if (isFading) {
		fadeTimer += (deltaTime / 1000.0f);

		float progress = fadeTimer / fadeDuration;
		if (progress > 1.0f) progress = 1.0f;

		for (auto av : activeVoices) {
			if (av->type == AudioType::MUSIC && !av->pCallback->isFinished) {

				if (av->name == fadingOutSound) {
					float newVol = av->baseVolume * (1.0f - progress);
					av->pVoice->SetVolume(newVol * currentMusicVolume * currentMasterVolume);
				}

				if (av->name == fadingInSound) {
					float newVol = fadingInTargetVolume * progress;
					av->pVoice->SetVolume(newVol * currentMusicVolume * currentMasterVolume);
					av->baseVolume = newVol;
				}
			}
		}

		if (progress >= 1.0f) {
			isFading = false;
			if (fadingOutSound != "") {
				Stop(fadingOutSound);
				Play(fadingInSound, true, fadingInTargetVolume, AudioType::MUSIC);
			}
		}
	}

	for (auto it = activeVoices.begin(); it != activeVoices.end(); )
	{
		if ((*it)->pCallback->isFinished) //finished sound should be destroyed
		{
			(*it)->pVoice->DestroyVoice();
			delete (*it)->pCallback;
			delete (*it);
			it = activeVoices.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void DX9GF::AudioManager::Stop(std::string name)
{
	for (auto av : activeVoices)
	{
		if (av->name == name && !av->pCallback->isFinished)
		{
			av->pVoice->Stop();
			av->pCallback->isFinished = true;
		}
	}
}

void DX9GF::AudioManager::StopAll()
{
	for (auto av : activeVoices)
	{
		if (!av->pCallback->isFinished)
		{
			av->pVoice->Stop();
			av->pCallback->isFinished = true;
		}
	}
}

void DX9GF::AudioManager::Shutdown() {
	//destroy playing sound
	for (auto av : activeVoices)
	{
		av->pVoice->Stop();
		av->pVoice->DestroyVoice();
		delete av->pCallback;
		delete av;
	}
	activeVoices.clear();
	soundBanks.clear();

	//clear the cache
	for (auto& pair : cache) delete pair.second;
	cache.clear();

	//free what it need to be freed
	if (pMasterVoice)
		pMasterVoice->DestroyVoice();
	if (pEngine)
		pEngine->Release();
	CoUninitialize();
}

void DX9GF::AudioManager::SetMasterVolume(float volume)
{
	currentMasterVolume = volume;
	for (auto av : activeVoices)
	{
		float typeVol = (av->type == AudioType::MUSIC) ? currentMusicVolume : currentSfxVolume;
		av->pVoice->SetVolume(av->baseVolume * typeVol * currentMasterVolume);
	}
}

void DX9GF::AudioManager::SetMusicVolume(float volume)
{
	currentMusicVolume = volume;
	for (auto av : activeVoices)
	{
		if (av->type == AudioType::MUSIC)
		{
			av->pVoice->SetVolume(av->baseVolume * currentMusicVolume * currentMasterVolume);
		}
	}
}

void DX9GF::AudioManager::SetSfxVolume(float volume)
{
	currentSfxVolume = volume;
	for (auto av : activeVoices)
	{
		if (av->type == AudioType::SFX)
		{
			av->pVoice->SetVolume(av->baseVolume * currentSfxVolume * currentMasterVolume);
		}
	}
}

void DX9GF::AudioManager::RegisterBank(std::string bankName, std::vector<std::string> soundNames)
{
	soundBanks[bankName] = soundNames;
}

void DX9GF::AudioManager::PlayRandom(std::string bankName, float volume, AudioType type)
{
	if (soundBanks.find(bankName) == soundBanks.end() || soundBanks[bankName].empty())
		return;

	int index = rand() % soundBanks[bankName].size();
	std::string selectedSound = soundBanks[bankName][index];

	Play(selectedSound, false, volume, type);
}

void DX9GF::AudioManager::PlayBGM_Fade(std::string name, float targetVolume, float duration)
{
	if (!activeVoices.empty()) {
		bool isAlreadyPlaying = false;
		for (auto av : activeVoices) {
			if (av->name == name && av->type == AudioType::MUSIC && !av->pCallback->isFinished) {
				isAlreadyPlaying = true;
				break;
			}
		}
		if (isAlreadyPlaying) return;
	}

	isFading = true;
	fadeTimer = 0.0f;
	fadeDuration = duration;
	fadingInSound = name;
	fadingInTargetVolume = targetVolume;

	fadingOutSound = "";
	for (auto av : activeVoices) {
		if (av->type == AudioType::MUSIC && !av->pCallback->isFinished) {
			fadingOutSound = av->name;
			break;
		}
	}

	if (fadingOutSound == "") {
		Play(fadingInSound, true, 0.01f, AudioType::MUSIC);
	}
}

void DX9GF::AudioManager::PlayRandomBGM_Fade(std::string bankName, float targetVolume, float duration)
{
	if (soundBanks.find(bankName) == soundBanks.end() || soundBanks[bankName].empty())
		return;

	int index = rand() % soundBanks[bankName].size();
	std::string selectedSound = soundBanks[bankName][index];

	PlayBGM_Fade(selectedSound, targetVolume, duration);
}