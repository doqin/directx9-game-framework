#include "pch.h"
#include "DX9GFIScene.h"

void DX9GF::IScene::Dispose()
{
}

DX9GF::IScene::~IScene() {
	Dispose();
}

DX9GF::Camera& DX9GF::IScene::GetCamera()
{
	return camera;
}

DX9GF::Camera& DX9GF::IScene::GetUICamera()
{
	return uiCamera;
}