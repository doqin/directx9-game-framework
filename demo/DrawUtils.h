#pragma once

#include "DX9GFGraphicsDevice.h"

namespace Demo {
	/// <summary>
	/// Draws a dashed line whose dashes move from the first position toward the last position.
	/// </summary>
	/// <param name="tick">Elapsed animation time in milliseconds.</param>
	/// <param name="animationSpeed">Dash movement speed in pixels per second.</param>
	void DrawAnimatedDashedLine(
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
		unsigned long long tick);

	void DrawAnimatedDashedLine(
		DX9GF::GraphicsDevice* graphicsDevice,
		DX9GF::Camera& camera,
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
		unsigned long long tick);

	void DrawAnimatedDashedRectangle(
		DX9GF::GraphicsDevice* graphicsDevice,
		float x,
		float y,
		float width,
		float height,
		float lineThickness,
		D3DCOLOR lineColor,
		bool drawOutline,
		float outlineThickness,
		D3DCOLOR outlineColor,
		float dashLength,
		float dashDistance,
		float animationSpeed,
		unsigned long long tick);

	void DrawAnimatedDashedRectangle(
		DX9GF::GraphicsDevice* graphicsDevice,
		DX9GF::Camera& camera,
		float x,
		float y,
		float width,
		float height,
		float lineThickness,
		D3DCOLOR lineColor,
		bool drawOutline,
		float outlineThickness,
		D3DCOLOR outlineColor,
		float dashLength,
		float dashDistance,
		float animationSpeed,
		unsigned long long tick);
}
