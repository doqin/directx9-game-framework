#pragma once
#include "DX9GFCamera.h"

namespace DX9GF {

	class IScene {
	protected:
		Camera camera;
		/// <summary>
		/// Releases all resources used by the object.
		/// </summary>
		virtual void Dispose();
	public:
		IScene(int screenWidth, int screenHeight) : camera(screenWidth, screenHeight) {}
		virtual ~IScene();
		Camera& GetCamera();

		virtual bool IsOverlay() const { return false; }
		/// <summary>
		/// Initializes the object.
		/// </summary>
		virtual void Init() = 0;
		/// <summary>
		/// Updates the object's state
		/// </summary>
		/// <param name="deltaTime">The time elapsed since the last frame in milliseconds</param>
		virtual void Update(unsigned long long deltaTime) = 0;

		/// <summary>
		/// Draws the frame
		/// </summary>
		/// <param name="deltaTime">The time elapsed since the last frame in milliseconds</param>
		virtual void Draw(unsigned long long deltaTime) = 0;
	};
};