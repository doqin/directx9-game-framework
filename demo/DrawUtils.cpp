#include "pch.h"
#include "DrawUtils.h"

#include <algorithm>
#include <cmath>

void Demo::DrawAnimatedDashedLine(
	DX9GF::GraphicsDevice* graphicsDevice,
	float firstX,
	float firstY,
	float lastX,
	float lastY,
	float lineThickness,
	D3DCOLOR lineColor,
	bool drawOutline,
	float outlineThickness,
	D3DCOLOR outlineColor,
	float dashLength,
	float dashDistance,
	float animationSpeed,
	unsigned long long tick)
{
	if (graphicsDevice == nullptr || lineThickness <= 0.0f || dashLength <= 0.0f || dashDistance < 0.0f) {
		return;
	}

	const float dx = lastX - firstX;
	const float dy = lastY - firstY;
	const float lineLength = std::sqrt(dx * dx + dy * dy);
	if (lineLength <= 0.00001f) {
		return;
	}

	const float patternLength = dashLength + dashDistance;
	const float directionX = dx / lineLength;
	const float directionY = dy / lineLength;
	const double traveledDistance = static_cast<double>(tick) * static_cast<double>(animationSpeed) / 1000.0;
	float animationOffset = static_cast<float>(std::fmod(traveledDistance, static_cast<double>(patternLength)));
	if (animationOffset < 0.0f) {
		animationOffset += patternLength;
	}

	for (float dashStart = animationOffset - patternLength; dashStart < lineLength; dashStart += patternLength) {
		const float clippedStart = std::max(0.0f, dashStart);
		const float clippedEnd = std::min(lineLength, dashStart + dashLength);
		if (clippedEnd <= clippedStart) {
			continue;
		}

		const float startX = firstX + directionX * clippedStart;
		const float startY = firstY + directionY * clippedStart;
		const float endX = firstX + directionX * clippedEnd;
		const float endY = firstY + directionY * clippedEnd;

		if (drawOutline && outlineThickness > 0.0f) {
			const float outlineStart = std::max(0.0f, clippedStart - outlineThickness);
			const float outlineEnd = std::min(lineLength, clippedEnd + outlineThickness);
			graphicsDevice->DrawLine(
				firstX + directionX * outlineStart,
				firstY + directionY * outlineStart,
				firstX + directionX * outlineEnd,
				firstY + directionY * outlineEnd,
				outlineColor,
				lineThickness + outlineThickness * 2.0f);
		}

		graphicsDevice->DrawLine(startX, startY, endX, endY, lineColor, lineThickness);
	}
}

void Demo::DrawAnimatedDashedLine(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera& camera, float firstX, float firstY, float lastX, float lastY, float lineThickness, D3DCOLOR lineColor, bool drawOutline, float outlineThickness, D3DCOLOR outlineColor, float dashLength, float dashDistance, float animationSpeed, unsigned long long tick)
{
	auto [screenWidth, screenHeight] = camera.GetScreenResolution();
	float camFirstX = firstX - camera.GetPosition().x + screenWidth / 2.f;
	float camFirstY = firstY - camera.GetPosition().y + screenHeight / 2.f;
	float camLastX = lastX - camera.GetPosition().x + screenWidth / 2.f;
	float camLastY = lastY - camera.GetPosition().y + screenHeight / 2.f;
	DrawAnimatedDashedLine(
		graphicsDevice,
		camFirstX,
		camFirstY,
		camLastX,
		camLastY,
		lineThickness * camera.GetZoom(),
		lineColor,
		drawOutline,
		outlineThickness * camera.GetZoom(),
		outlineColor,
		dashLength * camera.GetZoom(),
		dashDistance * camera.GetZoom(),
		animationSpeed,
		tick
	);
}

void Demo::DrawAnimatedDashedRectangle(DX9GF::GraphicsDevice* graphicsDevice, float x, float y, float width, float height, float lineThickness, D3DCOLOR lineColor, bool drawOutline, float outlineThickness, D3DCOLOR outlineColor, float dashLength, float dashDistance, float animationSpeed, unsigned long long tick)
{
	if (graphicsDevice == nullptr || lineThickness <= 0.0f || dashLength <= 0.0f || dashDistance < 0.0f) {
		return;
	}
	DrawAnimatedDashedLine(graphicsDevice, x, y, x + width, y, lineThickness, lineColor, drawOutline, outlineThickness, outlineColor, dashLength, dashDistance, animationSpeed, tick);
	DrawAnimatedDashedLine(graphicsDevice, x + width, y, x + width, y + height, lineThickness, lineColor, drawOutline, outlineThickness, outlineColor, dashLength, dashDistance, animationSpeed, tick);
	DrawAnimatedDashedLine(graphicsDevice, x + width, y + height, x, y + height, lineThickness, lineColor, drawOutline, outlineThickness, outlineColor, dashLength, dashDistance, animationSpeed, tick);
	DrawAnimatedDashedLine(graphicsDevice, x, y + height, x, y, lineThickness, lineColor, drawOutline, outlineThickness, outlineColor, dashLength, dashDistance, animationSpeed, tick);
}

void Demo::DrawAnimatedDashedRectangle(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera& camera, float x, float y, float width, float height, float lineThickness, D3DCOLOR lineColor, bool drawOutline, float outlineThickness, D3DCOLOR outlineColor, float dashLength, float dashDistance, float animationSpeed, unsigned long long tick)
{
	auto [screenWidth, screenHeight] = camera.GetScreenResolution();
	float camX = x - camera.GetPosition().x + screenWidth / 2.f;
	float camY = y - camera.GetPosition().y + screenHeight / 2.f;
	float camWidth = width * camera.GetZoom();
	float camHeight = height * camera.GetZoom();
	DrawAnimatedDashedRectangle(
		graphicsDevice,
		camX,
		camY,
		camWidth,
		camHeight,
		lineThickness * camera.GetZoom(),
		lineColor,
		drawOutline,
		outlineThickness * camera.GetZoom(),
		outlineColor,
		dashLength * camera.GetZoom(),
		dashDistance * camera.GetZoom(),
		animationSpeed,
		tick
	);
}
