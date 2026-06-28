#pragma once
#include "framework.h"
#include <d3d9.h>
#include <memory>
#include "DX9GFCamera.h"
namespace DX9GF {
	class GraphicsDevice;
	class SceneManager;
	class Texture;
	class StaticSprite;

	class IGame
	{
	private:
		LPDIRECT3D9 d3d = nullptr;
		HWND hwnd = NULL;
		D3DPRESENT_PARAMETERS d3dpp;
		bool pendingDeviceReset = false;
		UINT pendingWidth = 0;
		UINT pendingHeight = 0;
		bool TryResetDevice(UINT width, UINT height);

		//LETTERBOXING
		std::shared_ptr<DX9GF::Texture> renderTargetTex;
		std::shared_ptr<DX9GF::StaticSprite> renderTargetSprite;
		DX9GF::Camera defaultCamera;
	protected:
		const UINT SCREEN_WIDTH;
		const UINT SCREEN_HEIGHT;

		GraphicsDevice* graphicsDevice;
		SceneManager* sceneManager;
	public:
		IGame(HWND hwnd, const UINT screenWidth, const UINT screenHeight)
			: hwnd(hwnd), SCREEN_WIDTH(screenWidth), SCREEN_HEIGHT(screenHeight),
			defaultCamera(screenWidth, screenHeight) {
		}
		virtual ~IGame();

		HWND GetHwnd() const;
		GraphicsDevice* GetGraphicsDevice();
		SceneManager* GetSceneManager();

		virtual void Init();
		virtual void OnResize(UINT width, UINT height);
		void Update(unsigned long long deltaTime);
		void Draw(unsigned long long deltaTime);
		virtual void Dispose();
	};
}