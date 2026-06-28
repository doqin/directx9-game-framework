#include "pch.h"
#include "DX9GFDebug.h"
#include "DX9GFUtils.h"
#include <algorithm>
#include <cmath>

DX9GF::Font* DX9GF::Debug::debugFont = nullptr;
DX9GF::FontSprite* DX9GF::Debug::debugSprite = nullptr;
std::map<int, std::wstring> DX9GF::Debug::coordinateCache;

bool DX9GF::Debug::drawGrid = false;
bool DX9GF::Debug::drawAxis = false;

void DX9GF::Debug::Init(GraphicsDevice* graphicsDevice) {
	if (debugFont) return;

	debugFont = new Font(graphicsDevice, L"Consolas", 14);

	debugSprite = new FontSprite(debugFont);

	for (int i = -2048; i <= 2048; i += 16) {
		coordinateCache[i] = std::to_wstring(i);
	}
}

void DX9GF::Debug::Release() {
	if (debugSprite) {
		delete debugSprite;
		debugSprite = nullptr;
	}
	if (debugFont) {
		delete debugFont;
		debugFont = nullptr;
	}
	coordinateCache.clear();
}

void DX9GF::Debug::DrawAxis(const Camera& camera, int range, int step, D3DCOLOR color) {

		
	if (!debugSprite) return;

	debugSprite->Begin();
	for (int i = -range + 24; i <= range; i += step) {
		if (coordinateCache.find(i) == coordinateCache.end()) {
			coordinateCache[i] = std::to_wstring(i);
		}

		debugSprite->SetPosition((float)i, 0.0f);
		debugSprite->SetText(coordinateCache[i].c_str());
		debugSprite->Draw(camera, color);

		if (i != 0) {
			debugSprite->SetPosition(0.0f, (float)-i);
			debugSprite->SetText(coordinateCache[i].c_str());
			debugSprite->Draw(camera, color);
		}
	}
	debugSprite->End();
}
//void DX9GF::Debug::DrawAxis(
//	GraphicsDevice* graphicsDevice,
//	const Camera& camera,
//	Application* app,
//	int offsetX,
//	int offsetY,
//	int sreenWidth,
//	int screenHeight,
//	int spacingX,
//	int spacingY,
//	int range, int step,
//	D3DCOLOR color)
//{
//	if (spacingX <= 0 || spacingY <= 0) return;
//
//	auto matCamera = camera.GetTransformMatrix();
//	D3DXMATRIX matInv;
//	if (D3DXMatrixInverse(&matInv, nullptr, &matCamera) == nullptr) return;
//
//	auto TransformPoint = [&matInv](float x, float y) {
//		D3DXVECTOR4 v(x, y, 0.0f, 1.0f);
//		D3DXVec4Transform(&v, &v, &matInv);
//		if (v.w != 0.0f) {
//			v.x /= v.w;
//			v.y /= v.w;
//		}
//		return D3DXVECTOR2(v.x, v.y);
//		};
//
//	// Convert screen rect to a world-space AABB (works fine even with camera rotation; the AABB just becomes a conservative bound).
//	auto w0 = TransformPoint(0.0f, 0.0f);
//	auto w1 = TransformPoint((float)app->GetScreenWidth(), 0.0f);
//	auto w2 = TransformPoint(0.0f, (float)screenHeight);
//	auto w3 = TransformPoint((float)app->GetScreenWidth(), (float)screenHeight);
//
//	float minX = (std::min)((std::min)(w0.x, w1.x), (std::min)(w2.x, w3.x));
//	float maxX = (std::max)((std::max)(w0.x, w1.x), (std::max)(w2.x, w3.x));
//	float minY = (std::min)((std::min)(w0.y, w1.y), (std::min)(w2.y, w3.y));
//	float maxY = (std::max)((std::max)(w0.y, w1.y), (std::max)(w2.y, w3.y));
//
//	// Expand a bit so lines cover the full screen when the view is rotated.
//	minX -= spacingX;
//	maxX += spacingX;
//	minY -= spacingY;
//	maxY += spacingY;
//
//	float startX = std::floor((minX - offsetX) / (float)spacingX) * spacingX + offsetX;
//	float startY = std::floor((minY - offsetY) / (float)spacingY) * spacingY + offsetY;
//
//	if (!debugSprite) return;
//
//	debugSprite->Begin();
//
//	// X axis (y = 0)
//	for (float x = startX; x <= maxX; x += spacingX) {
//		int ix = (int)x;
//
//		auto& text = coordinateCache[ix];
//		if (text.empty()) {
//			text = std::to_wstring(ix);
//		}
//
//		debugSprite->SetPosition(x, 0.0f);
//		debugSprite->SetText(text.c_str());
//		debugSprite->Draw(camera, color);
//	}
//
//	// Y axis (x = 0)
//	for (float y = startY; y <= maxY; y += spacingY) {
//		int iy = (int)y;
//
//		if (iy == 0) continue;
//
//		auto& text = coordinateCache[iy];
//		if (text.empty()) {
//			text = std::to_wstring(iy);
//		}
//
//		debugSprite->SetPosition(0.0f, y);
//		debugSprite->SetText(text.c_str());
//		debugSprite->Draw(camera, color);
//	}
//
//	debugSprite->End();

	//if (!debugSprite) return;

	//debugSprite->Begin();
	//for (int i = -range + 24; i <= range; i += step) {
	//	if (coordinateCache.find(i) == coordinateCache.end()) {
	//		coordinateCache[i] = std::to_wstring(i);
	//	}

	//	debugSprite->SetPosition((float)i, 0.0f);
	//	debugSprite->SetText(coordinateCache[i].c_str());
	//	debugSprite->Draw(camera, color);

	//	if (i != 0) {
	//		debugSprite->SetPosition(0.0f, (float)-i);
	//		debugSprite->SetText(coordinateCache[i].c_str());
	//		debugSprite->Draw(camera, color);
	//	}
	//}
	//debugSprite->End();
//}

void DX9GF::Debug::DrawGrid(GraphicsDevice* graphicsDevice, int offsetX, int offsetY, int screenWidth, int screenHeight, int spacingX, int spacingY,
	D3DCOLOR lineColor) {
		// Horizontal lines
		for (int i = offsetY % spacingY - spacingY; i < screenHeight; i += spacingY) {
			graphicsDevice->DrawLine(offsetX % spacingX - spacingX, i, screenWidth, i, lineColor);
		}
		// Vertical lines
		for (int i = offsetX % spacingX - spacingX; i < screenWidth; i += spacingX) {
			graphicsDevice->DrawLine(i, offsetY % spacingY - spacingY, i, screenHeight, lineColor);
		}
}

void DX9GF::Debug::DrawGrid(GraphicsDevice* graphicsDevice, const Camera& camera, int offsetX, int offsetY, int screenWidth, int screenHeight, int spacingX, int spacingY, D3DCOLOR lineColor)
{
		if (spacingX <= 0 || spacingY <= 0) return;

		auto matCamera = camera.GetTransformMatrix();
		D3DXMATRIX matInv;
		if (D3DXMatrixInverse(&matInv, nullptr, &matCamera) == nullptr) return;

		auto TransformPoint = [&matInv](float x, float y) {
			D3DXVECTOR4 v(x, y, 0.0f, 1.0f);
			D3DXVec4Transform(&v, &v, &matInv);
			if (v.w != 0.0f) {
				v.x /= v.w;
				v.y /= v.w;
			}
			return D3DXVECTOR2(v.x, v.y);
			};

		// Convert screen rect to a world-space AABB (works fine even with camera rotation; the AABB just becomes a conservative bound).
		auto w0 = TransformPoint(0.0f, 0.0f);
		auto w1 = TransformPoint((float)screenWidth, 0.0f);
		auto w2 = TransformPoint(0.0f, (float)screenHeight);
		auto w3 = TransformPoint((float)screenWidth, (float)screenHeight);

		float minX = (std::min)((std::min)(w0.x, w1.x), (std::min)(w2.x, w3.x));
		float maxX = (std::max)((std::max)(w0.x, w1.x), (std::max)(w2.x, w3.x));
		float minY = (std::min)((std::min)(w0.y, w1.y), (std::min)(w2.y, w3.y));
		float maxY = (std::max)((std::max)(w0.y, w1.y), (std::max)(w2.y, w3.y));

		// Expand a bit so lines cover the full screen when the view is rotated.
		minX -= spacingX;
		maxX += spacingX;
		minY -= spacingY;
		maxY += spacingY;

		float startX = std::floor((minX - offsetX) / (float)spacingX) * spacingX + offsetX;
		float startY = std::floor((minY - offsetY) / (float)spacingY) * spacingY + offsetY;

		for (float y = startY; y <= maxY; y += spacingY) {
			graphicsDevice->DrawLine(camera, minX, y, maxX, y, lineColor);
		}
		for (float x = startX; x <= maxX; x += spacingX) {
			graphicsDevice->DrawLine(camera, x, minY, x, maxY, lineColor);
		}
}
