#include "pch.h"
#include "DX9GFSceneManager.h"
#include "DX9GFIScene.h"
#include <stdexcept>
#include "DX9GFApplication.h"

DX9GF::SceneManager::~SceneManager()
{
	Dispose();
}

DX9GF::IScene* DX9GF::SceneManager::GetCurrentScene()
{
	return scenes[index];
}

DX9GF::IScene* DX9GF::SceneManager::GetScene(size_t index) {
	return scenes[index];
}

void DX9GF::SceneManager::ChangeScene(IScene* scene)
{
	if (!scenes.empty()) {
		delete scenes[index];
	}
	scenes[index] = scene;
	scene->Init();
}

void DX9GF::SceneManager::PushScene(IScene* scene)
{
	scenes.push_back(scene);
	scene->Init();
}

void DX9GF::SceneManager::InsertScene(size_t index, IScene* scene)
{
	scenes.insert(scenes.begin() + index, scene);
	scene->Init();
}

void DX9GF::SceneManager::PopScene()
{
	auto scene = scenes.back();
	delete scene;
	scenes.pop_back();
}

void DX9GF::SceneManager::RemoveScene(size_t index)
{
	auto scene = scenes[index];
	delete scene;
	scenes.erase(scenes.begin() + index);
}

void DX9GF::SceneManager::Update(unsigned long long deltaTime)
{
	if (scenes.empty()) {
		throw std::runtime_error("No scene to update!");
	}
	scenes[index]->Update(deltaTime);
}

void DX9GF::SceneManager::DrawWorld(unsigned long long deltaTime)
{
	if (scenes.empty()) throw std::runtime_error("No scene to draw!");
	int startIndex = index;
	while (startIndex > 0 && scenes[startIndex]->IsOverlay()) startIndex--;
	for (int i = startIndex; i <= index; ++i) scenes[i]->DrawWorld(deltaTime);
}

void DX9GF::SceneManager::DrawUI(unsigned long long deltaTime)
{
	if (scenes.empty()) throw std::runtime_error("No scene to draw!");
	int startIndex = index;
	while (startIndex > 0 && scenes[startIndex]->IsOverlay()) startIndex--;
	for (int i = startIndex; i <= index; ++i) scenes[i]->DrawUI(deltaTime);
}

void DX9GF::SceneManager::OnResize(int width, int height)
{
	for (auto& scene : scenes) {
		scene->GetUICamera().SetScreenResolution(width, height);
	}
}

void DX9GF::SceneManager::GoToNext()
{
	index++;
	if (index >= scenes.size()) throw std::runtime_error("Attempted to switch to a scene out of bound");
}

void DX9GF::SceneManager::GoToPrevious()
{
	index--;
	if (index < 0) throw std::runtime_error("Attempted to switch to a scene out of bound");
}

void DX9GF::SceneManager::GoToScene(size_t index)
{
	this->index = index;
	if (index < 0 || index >= scenes.size()) throw std::runtime_error("Attempted to switch to a scene out of bound");
}

void DX9GF::SceneManager::Dispose()
{
	while (!scenes.empty()) {
		PopScene();
	}
}

size_t DX9GF::SceneManager::GetSceneCount() const
{
	return scenes.size();
}