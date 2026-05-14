#include "pch.h"
#include "DX9GFGraphicsDevice.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <d3dx9math.h>
#include "DX9GFApplication.h"
#include "DX9GFUtils.h"

struct Vertex {
	float x, y, z, rhw; // rhw is reciprocal of homogenous w
	DWORD color;
};

static D3DXVECTOR2 TransformPoint(const D3DXMATRIX& matrix, float x, float y)
{
	D3DXVECTOR4 v(x, y, 0.0f, 1.0f);
	D3DXVec4Transform(&v, &v, &matrix);
	if (v.w != 0.0f) {
		v.x /= v.w;
		v.y /= v.w;
	}
	return D3DXVECTOR2(v.x, v.y);
}

static D3DXVECTOR2 TransformRectanglePoint(
	float x,
	float y,
	float localX,
	float localY,
	float rotation,
	float scaleX,
	float scaleY,
	float offsetX,
	float offsetY
)
{
	float px = (localX - offsetX) * scaleX;
	float py = (localY - offsetY) * scaleY;
	float c = std::cos(rotation);
	float s = std::sin(rotation);
	float rx = px * c - py * s;
	float ry = px * s + py * c;
	return D3DXVECTOR2(x + rx, y + ry);
}

DX9GF::GraphicsDevice::GraphicsDevice(IDirect3DDevice9* d3ddev, IDirect3DSurface9* backbuffer) : d3ddev(d3ddev), backbuffer(backbuffer)
{
	d3ddev->GetViewport(&viewport);
}

DX9GF::GraphicsDevice::~GraphicsDevice() {
	if (d3ddev != NULL) d3ddev->Release();
}

IDirect3DDevice9*& DX9GF::GraphicsDevice::GetDevice() {
	return d3ddev;
}

LPDIRECT3DSURFACE9& DX9GF::GraphicsDevice::GetBackBuffer() {
	return backbuffer;
}

HRESULT DX9GF::GraphicsDevice::SetViewport(D3DVIEWPORT9 viewport)
{
	return d3ddev->SetViewport(&viewport);
}

HRESULT DX9GF::GraphicsDevice::SetViewport(DWORD x, DWORD y, DWORD width, DWORD height, float minZ, float maxZ)
{
	D3DVIEWPORT9 vp = { x, y, width, height, minZ, maxZ };
	return d3ddev->SetViewport(&vp);
}

HRESULT DX9GF::GraphicsDevice::BeginDraw()
{
	return d3ddev->BeginScene();
}

HRESULT DX9GF::GraphicsDevice::Clear(D3DXCOLOR color)
{
	return d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, color, 1.0f, 0);
}

HRESULT DX9GF::GraphicsDevice::Clear()
{
	return d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
}

HRESULT DX9GF::GraphicsDevice::EndDraw()
{
	return d3ddev->EndScene();
}

HRESULT DX9GF::GraphicsDevice::Present()
{
	return d3ddev->Present(NULL, NULL, NULL, NULL);
}

HRESULT DX9GF::GraphicsDevice::IsValid()
{
	if (d3ddev == NULL)
		return E_FAIL;

	return d3ddev->TestCooperativeLevel();
}

void DX9GF::GraphicsDevice::SetAlphaBlending(bool enabled)
{
	if (enabled) {
		d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
	else {
		d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}
}

void DX9GF::GraphicsDevice::SetScissorTest(bool enabled)
{
	d3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, enabled);
}

void DX9GF::GraphicsDevice::SetScissorRect(const RECT& rect)
{
	d3ddev->SetScissorRect(&rect);
}

void DX9GF::GraphicsDevice::DrawLine(float x1, float y1, float x2, float y2, D3DCOLOR color, float thickness)
{
	if (thickness <= 1.0f) {
		Vertex vertices[] = {
			{ .x=x1, .y=y1, .z=0.0f, .rhw=1.0f, .color=color },
			{ .x=x2, .y=y2, .z=0.0f, .rhw=1.0f, .color=color }
		};
		d3ddev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
		d3ddev->DrawPrimitiveUP(D3DPT_LINELIST, 1, vertices, sizeof(Vertex));
	}
	else {
		float dx = x2 - x1;
		float dy = y2 - y1;
		float len = std::sqrt(dx * dx + dy * dy);
		if (len < 0.00001f) return;

		float nx = -dy / len;
		float ny = dx / len;

		float halfThickness = thickness * 0.5f;

		Vertex vertices[] = {
			{ .x = x1 + nx * halfThickness, .y = y1 + ny * halfThickness, .z = 0.0f, .rhw = 1.0f, .color = color },
			{ .x = x2 + nx * halfThickness, .y = y2 + ny * halfThickness, .z = 0.0f, .rhw = 1.0f, .color = color },
			{ .x = x1 - nx * halfThickness, .y = y1 - ny * halfThickness, .z = 0.0f, .rhw = 1.0f, .color = color },
			{ .x = x2 - nx * halfThickness, .y = y2 - ny * halfThickness, .z = 0.0f, .rhw = 1.0f, .color = color }
		};

		DWORD prevCullMode;
		d3ddev->GetRenderState(D3DRS_CULLMODE, &prevCullMode);
		d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		d3ddev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
		d3ddev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(Vertex));

		d3ddev->SetRenderState(D3DRS_CULLMODE, prevCullMode);
	}
}

void DX9GF::GraphicsDevice::DrawLine(const DX9GF::Camera& camera, float x1, float y1, float x2, float y2, D3DCOLOR color, float thickness)
{
	auto matCamera = camera.GetTransformMatrix();
	auto p1 = TransformPoint(matCamera, x1, y1);
	auto p2 = TransformPoint(matCamera, x2, y2);
	DrawLine(p1.x, p1.y, p2.x, p2.y, color, thickness);
}

void DX9GF::GraphicsDevice::DrawRectangle(float x, float y, float width, float height, D3DCOLOR color, bool isFilled)
{
	DrawRectangle(x, y, width, height, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, color, isFilled);
}

void DX9GF::GraphicsDevice::DrawRectangle(const DX9GF::Camera& camera, float x, float y, float width, float height, D3DCOLOR color, bool isFilled)
{
	DrawRectangle(camera, x, y, width, height, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, color, isFilled);
}

void DX9GF::GraphicsDevice::DrawRectangle(float x, float y, float width, float height, float rotation, float scaleX, float scaleY, float offsetX, float offsetY, D3DCOLOR color, bool isFilled)
{
	auto p0 = TransformRectanglePoint(x, y, 0.0f, 0.0f, rotation, scaleX, scaleY, offsetX, offsetY);
	auto p1 = TransformRectanglePoint(x, y, width, 0.0f, rotation, scaleX, scaleY, offsetX, offsetY);
	auto p2 = TransformRectanglePoint(x, y, width, height, rotation, scaleX, scaleY, offsetX, offsetY);
	auto p3 = TransformRectanglePoint(x, y, 0.0f, height, rotation, scaleX, scaleY, offsetX, offsetY);

	std::vector<Vertex> vertices = {
		{.x = p0.x, .y = p0.y, .z = 0.0f, .rhw = 1.0f, .color = color },
		{.x = p1.x, .y = p1.y, .z = 0.0f, .rhw = 1.0f, .color = color },
		{.x = p2.x, .y = p2.y, .z = 0.0f, .rhw = 1.0f, .color = color },
		{.x = p3.x, .y = p3.y, .z = 0.0f, .rhw = 1.0f, .color = color }
	};

	if (!isFilled) {
		vertices.push_back(vertices[0]);
	}

	d3ddev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	D3DPRIMITIVETYPE primitiveType = isFilled ? D3DPT_TRIANGLEFAN : D3DPT_LINESTRIP;
	d3ddev->DrawPrimitiveUP(primitiveType, isFilled ? 2 : 4, vertices.data(), sizeof(Vertex));
}

void DX9GF::GraphicsDevice::DrawRectangle(const DX9GF::Camera& camera, float x, float y, float width, float height, float rotation, float scaleX, float scaleY, float offsetX, float offsetY, D3DCOLOR color, bool isFilled)
{
	auto matCamera = camera.GetTransformMatrix();

	auto wp0 = TransformRectanglePoint(x, y, 0.0f, 0.0f, rotation, scaleX, scaleY, offsetX, offsetY);
	auto wp1 = TransformRectanglePoint(x, y, width, 0.0f, rotation, scaleX, scaleY, offsetX, offsetY);
	auto wp2 = TransformRectanglePoint(x, y, width, height, rotation, scaleX, scaleY, offsetX, offsetY);
	auto wp3 = TransformRectanglePoint(x, y, 0.0f, height, rotation, scaleX, scaleY, offsetX, offsetY);

	auto p0 = TransformPoint(matCamera, wp0.x, wp0.y);
	auto p1 = TransformPoint(matCamera, wp1.x, wp1.y);
	auto p2 = TransformPoint(matCamera, wp2.x, wp2.y);
	auto p3 = TransformPoint(matCamera, wp3.x, wp3.y);

	std::vector<Vertex> vertices = {
		{.x = p0.x, .y = p0.y, .z = 0.0f, .rhw = 1.0f, .color = color },
		{.x = p1.x, .y = p1.y, .z = 0.0f, .rhw = 1.0f, .color = color },
		{.x = p2.x, .y = p2.y, .z = 0.0f, .rhw = 1.0f, .color = color },
		{.x = p3.x, .y = p3.y, .z = 0.0f, .rhw = 1.0f, .color = color }
	};

	if (!isFilled) {
		vertices.push_back(vertices[0]);
	}
	D3DPRIMITIVETYPE primitiveType = isFilled ? D3DPT_TRIANGLEFAN : D3DPT_LINESTRIP;
	d3ddev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	d3ddev->DrawPrimitiveUP(primitiveType, isFilled ? 2 : 4, vertices.data(), sizeof(Vertex));
}

void DX9GF::GraphicsDevice::DrawEllipse(float centerX, float centerY, float width, float height, D3DCOLOR color, bool isFilled)
{
	DrawEllipse(centerX, centerY, width, height, 0.0f, 1.0f, 1.0f, width * 0.5f, height * 0.5f, color, isFilled);
}

void DX9GF::GraphicsDevice::DrawEllipse(const DX9GF::Camera& camera, float centerX, float centerY, float width, float height, D3DCOLOR color, bool isFilled)
{
	DrawEllipse(camera, centerX, centerY, width, height, 0.0f, 1.0f, 1.0f, width * 0.5f, height * 0.5f, color, isFilled);
}

void DX9GF::GraphicsDevice::DrawEllipse(float x, float y, float width, float height, float rotation, float scaleX, float scaleY, float offsetX, float offsetY, D3DCOLOR color, bool isFilled)
{
	const int SAMPLES = 36;
	std::vector<Vertex> vertices;

	float rx = width * 0.5f;
	float ry = height * 0.5f;
	float cx = rx;
	float cy = ry;

	if (isFilled) {
		auto c = TransformRectanglePoint(x, y, cx, cy, rotation, scaleX, scaleY, offsetX, offsetY);
		vertices.push_back({ .x = c.x, .y = c.y, .z = 0.0f, .rhw = 1.0f, .color = color });
	}
	for (int i = 0; i <= SAMPLES; ++i) {
		float theta = i * (2.0f * D3DX_PI / SAMPLES);
		float lx = cx + std::cos(theta) * rx;
		float ly = cy + std::sin(theta) * ry;
		auto p = TransformRectanglePoint(x, y, lx, ly, rotation, scaleX, scaleY, offsetX, offsetY);
		vertices.push_back({
			.x = p.x,
			.y = p.y,
			.z = 0.0f,
			.rhw = 1.0f,
			.color = color
		});
	}
	d3ddev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	D3DPRIMITIVETYPE primitiveType = isFilled ? D3DPT_TRIANGLEFAN : D3DPT_LINESTRIP;
	d3ddev->DrawPrimitiveUP(primitiveType, SAMPLES, vertices.data(), sizeof(Vertex));
}

void DX9GF::GraphicsDevice::DrawEllipse(const DX9GF::Camera& camera, float x, float y, float width, float height, float rotation, float scaleX, float scaleY, float offsetX, float offsetY, D3DCOLOR color, bool isFilled)
{
	const int SAMPLES = 36;
	auto matCamera = camera.GetTransformMatrix();

	std::vector<Vertex> vertices;

	float rx = width * 0.5f;
	float ry = height * 0.5f;
	float cx = rx;
	float cy = ry;

	if (isFilled) {
		auto wc = TransformRectanglePoint(x, y, cx, cy, rotation, scaleX, scaleY, offsetX, offsetY);
		auto c = TransformPoint(matCamera, wc.x, wc.y);
		vertices.push_back({ .x = c.x, .y = c.y, .z = 0.0f, .rhw = 1.0f, .color = color });
	}
	for (int i = 0; i <= SAMPLES; ++i) {
		float theta = i * (2.0f * D3DX_PI / SAMPLES);
		float lx = cx + std::cos(theta) * rx;
		float ly = cy + std::sin(theta) * ry;
		auto wp = TransformRectanglePoint(x, y, lx, ly, rotation, scaleX, scaleY, offsetX, offsetY);
		auto p = TransformPoint(matCamera, wp.x, wp.y);
		vertices.push_back({
			.x = p.x,
			.y = p.y,
			.z = 0.0f,
			.rhw = 1.0f,
			.color = color
		});
	}

	d3ddev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	D3DPRIMITIVETYPE primitiveType = isFilled ? D3DPT_TRIANGLEFAN : D3DPT_LINESTRIP;
	d3ddev->DrawPrimitiveUP(primitiveType, SAMPLES, vertices.data(), sizeof(Vertex));
}
