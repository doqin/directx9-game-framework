#pragma once
#include <d3d9.h>
#include "DX9GFCamera.h"

namespace DX9GF {
	class GraphicsDevice final {
	private:
		IDirect3DDevice9* d3ddev = NULL; // Đại diện cho card đồ họa máy tính
		IDirect3DSurface9* backbuffer = NULL;
		D3DVIEWPORT9 viewport;
	public:
		GraphicsDevice() {};
		GraphicsDevice(IDirect3DDevice9* d3ddev, IDirect3DSurface9* backbuffer);
		~GraphicsDevice();

		/// <summary>
		/// Gets a reference to the Direct3D device pointer.
		/// </summary>
		/// <returns>A reference to the Direct3D device pointer.</returns>
		IDirect3DDevice9*& GetDevice();
		/// <summary>
		/// Retrieves a reference to the back buffer surface.
		/// </summary>
		/// <returns>A reference to a pointer to the Direct3D surface representing the back buffer.</returns>
		IDirect3DSurface9*& GetBackBuffer();
		/// <summary>
		/// Sets the viewport parameters for rendering.
		/// </summary>
		/// <param name="x">The x-coordinate of the upper-left corner of the viewport.</param>
		/// <param name="y">The y-coordinate of the upper-left corner of the viewport.</param>
		/// <param name="width">The width of the viewport in pixels.</param>
		/// <param name="height">The height of the viewport in pixels.</param>
		/// <param name="minZ">The minimum depth value of the viewport range (typically 0.0).</param>
		/// <param name="maxZ">The maximum depth value of the viewport range (typically 1.0).</param>
		HRESULT SetViewport(DWORD x, DWORD y, DWORD width, DWORD height, float minZ, float maxZ);
		HRESULT SetViewport(D3DVIEWPORT9 viewport);
		HRESULT BeginDraw();
		HRESULT Clear(D3DXCOLOR color);
		HRESULT Clear();
		HRESULT EndDraw();
		HRESULT Present();
		HRESULT IsValid();

		void SetAlphaBlending(bool enabled);
		void SetScissorTest(bool enabled);
		void SetScissorRect(const RECT& rect);

		void DrawLine(float x1, float y1, float x2, float y2, D3DCOLOR color, float thickness = 1.0f);
		void DrawLine(const DX9GF::Camera& camera, float x1, float y1, float x2, float y2, D3DCOLOR color, float thickness = 1.0f);
		void DrawRectangle(float x, float y, float width, float height, D3DCOLOR color, bool isFilled);
		void DrawRectangle(const DX9GF::Camera& camera, float x, float y, float width, float height, D3DCOLOR color, bool isFilled);
		void DrawRectangle(float x, float y, float width, float height, float rotation, float scaleX, float scaleY, float offsetX, float offsetY, D3DCOLOR color, bool isFilled);
		void DrawRectangle(const DX9GF::Camera& camera, float x, float y, float width, float height, float rotation, float scaleX, float scaleY, float offsetX, float offsetY, D3DCOLOR color, bool isFilled);
		void DrawEllipse(float centerX, float centerY, float width, float height, D3DCOLOR color, bool isFilled);
		void DrawEllipse(const DX9GF::Camera& camera, float centerX, float centerY, float width, float height, D3DCOLOR color, bool isFilled);
		void DrawEllipse(float x, float y, float width, float height, float rotation, float scaleX, float scaleY, float offsetX, float offsetY, D3DCOLOR color, bool isFilled);
		void DrawEllipse(const DX9GF::Camera& camera, float x, float y, float width, float height, float rotation, float scaleX, float scaleY, float offsetX, float offsetY, D3DCOLOR color, bool isFilled);
	};
};