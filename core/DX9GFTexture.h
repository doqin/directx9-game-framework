#pragma once
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx9.h"
#include "DX9GFGraphicsDevice.h"
#include <string>
namespace DX9GF {
	class Texture {
	private:
		GraphicsDevice* graphicsDevice;
		IDirect3DTexture9* texture = nullptr;
		UINT width = 0, height = 0;
	public:
		Texture(GraphicsDevice* graphicsDevice);
		~Texture();
		void CreatePlainTexture(D3DCOLOR color, UINT width, UINT height);
		void SetColor(D3DCOLOR color);
		void LoadTexture(std::wstring filePath, UINT width = D3DX_DEFAULT_NONPOW2, UINT height = D3DX_DEFAULT_NONPOW2);
		void LoadTexture(int resourceId, UINT width = D3DX_DEFAULT_NONPOW2, UINT height = D3DX_DEFAULT_NONPOW2);
		void CaptureCurrentBackBuffer();
		IDirect3DTexture9* GetRawTexture();
		UINT GetWidth() const;
		UINT GetHeight() const;
		std::tuple<UINT, UINT> GetSize() const;
		GraphicsDevice* GetGraphicsDevice();
	};
}