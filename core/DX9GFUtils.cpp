#include "pch.h"
#include "DX9GFUtils.h"
#include "DX9GFApplication.h"

std::vector<RECT> DX9GF::Utils::CreateFrames(UINT sheetWidth, UINT sheetHeight, UINT frameWidth, UINT frameHeight, UINT frameCount, UINT offset)
{
	std::vector<RECT> frames;
	frames.reserve(frameCount);

	UINT framesPerRow = sheetWidth / frameWidth;

	for (UINT i = 0; i < frameCount; ++i)
	{
		UINT frameIndex = offset + i;
		UINT row = frameIndex / framesPerRow;
		UINT col = frameIndex % framesPerRow;

		RECT rect;
		rect.left = col * frameWidth;
		rect.top = row * frameHeight;
		rect.right = rect.left + frameWidth;
		rect.bottom = rect.top + frameHeight;

		frames.push_back(rect);
	}

	return frames;
}

std::vector<RECT> DX9GF::Utils::CreateRectsVertical(long x, long y, long width, long height, long count)
{
	std::vector<RECT> rects(count);
	for (int i = 0; i < count; i++) {
		rects[i] = {
			.left = x,
			.top = y + i * height,
			.right = x + width,
			.bottom = y + (i + 1) * height
		};
	}
	return rects;
}

std::vector<RECT> DX9GF::Utils::CreateRectsHorizontal(long x, long y, long width, long height, long count)
{
	std::vector<RECT> rects(count);
	for (int i = 0; i < count; i++) {
		rects[i] = {
			.left = x + i * width,
			.top = y,
			.right = x + (i + 1) * width,
			.bottom = y + height
		};
	}
	return rects;
}

bool DX9GF::Utils::IsWithinRectangle(float pointX, float pointY, float rectX, float rectY, float rectWidth, float rectHeight)
{
	return pointX > rectX 
		&& pointX < rectX + rectWidth
		&& pointY > rectY
		&& pointY < rectY + rectHeight;
}

void DX9GF::Utils::SetMousePos(int x, int y)
{
	POINT p;
	p.x = x;
	p.y = y;
	if (ClientToScreen(DX9GF::Application::GetInstance()->GetHWnd(), &p)) {
		SetCursorPos(p.x, p.y);
	}
}

std::tuple<float, float> DX9GF::Utils::WindowToWorldCoords(const DX9GF::Camera& camera, float windowX, float windowY)
{
	D3DXMATRIX cameraTransform = camera.GetTransformMatrix();
	D3DXMATRIX invTransform;
	D3DXMatrixInverse(&invTransform, nullptr, &cameraTransform);

	D3DXVECTOR3 windowPos(windowX, windowY, 0.0f);
	D3DXVECTOR3 worldPos;
	D3DXVec3TransformCoord(&worldPos, &windowPos, &invTransform);
	return std::make_tuple(worldPos.x, worldPos.y);
}

std::tuple<float, float> DX9GF::Utils::WorldToWindowCoords(const DX9GF::Camera& camera, float worldX, float worldY)
{
	D3DXMATRIX cameraTransform = camera.GetTransformMatrix();
	D3DXVECTOR3 worldPos(worldX, worldY, 0.0f);
	D3DXVECTOR3 windowPos;
	D3DXVec3TransformCoord(&windowPos, &worldPos, &cameraTransform);
	return std::make_tuple(windowPos.x, windowPos.y);
}
