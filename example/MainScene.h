#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "ExampleGame.h"
#include "Player.h"
#include "Rectangle.h"

class MainScene : public DX9GF::IScene, public DX9GF::ISaveable
{
private:
	ExampleGame* game;
	DX9GF::InputManager* inputManager = nullptr;
	DX9GF::CommandBuffer commandBuffer;
	std::shared_ptr<DX9GF::Font> fontArial;
	std::shared_ptr<DX9GF::FontSprite> fontSpriteArial;

	std::shared_ptr<DX9GF::Font> fontdetemination;
	std::shared_ptr<DX9GF::FontSprite> fontSpritedetemination;

	std::vector<std::shared_ptr<GO::Player>> players;
	std::shared_ptr<GO::Rectangle> rect;
	std::shared_ptr<DX9GF::TransformManager> transformManager;
	std::shared_ptr<DX9GF::ColliderManager> colliderManager;
	std::shared_ptr<DX9GF::Texture> whiteTexture;
	std::shared_ptr<DX9GF::Texture> dawgTexture;
	std::shared_ptr<DX9GF::StaticSprite> colorRec;
	std::shared_ptr<DX9GF::StaticSprite> textureRec;
	std::shared_ptr<DX9GF::Map> map;
	DX9GF::AudioManager* audioManager;
	float triggerX = 170.0f;
	float triggerY = 170.0f;
	float triggerRadius = 20.0f;
	bool wasInTrigger = false;
public:
	MainScene(ExampleGame* game, int width, int height) : game(game), IScene(width, height) {}
	void Init() override;
	void DrawWorld(unsigned long long deltaTime) override;
	void DrawUI(unsigned long long deltaTime) override;
	void Update(unsigned long long deltaTime) override;
	void Dispose() override;

	// Inherited via ISaveable
	std::string GetSaveID() const override;
	void GenerateSaveData(nlohmann::json& outData) override;
	void RestoreSaveData(const nlohmann::json& inData) override;
};