#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "ExampleGame.h"
#include "Rectangle.h"
#include "Ellipse.h"

class SubScene : public DX9GF::IScene, public DX9GF::ISaveable {
private:
	ExampleGame* game = nullptr;
	DX9GF::InputManager* inputManager = nullptr;
	std::shared_ptr<DX9GF::TransformManager> transformManager;
	std::shared_ptr<DX9GF::ColliderManager> colliderManager;
	std::vector<std::shared_ptr<GO::Rectangle>> rects;
	std::vector<std::shared_ptr<GO::Ellipse>> ellipses;
	std::shared_ptr<DX9GF::RectangleTrigger> rectTrigger1;
	std::shared_ptr<DX9GF::RectangleTrigger> rectTrigger2;
public:
	SubScene(ExampleGame* game, int width, int height) : game(game), IScene(width, height) {}
	void Init() override;
	void Update(unsigned long long deltaTime) override;
	void DrawWorld(unsigned long long deltaTime) override;
	void DrawUI(unsigned long long deltaTime) override;
	void Dispose() override;

	// Inherited via ISaveable
	std::string GetSaveID() const override;
	void GenerateSaveData(nlohmann::json& outData) override;
	void RestoreSaveData(const nlohmann::json& inData) override;
};