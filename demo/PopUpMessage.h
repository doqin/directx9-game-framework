#pragma once
#include "DX9GFExtras.h"
#include "DX9GF.h"

namespace Demo {
	class PopUpMessage : public DX9GF::IGameObject {
		DX9GF::GraphicsDevice* graphicsDevice = nullptr;
		DX9GF::Camera* camera = nullptr;
		std::shared_ptr<DX9GF::Font> font;
		std::shared_ptr<DX9GF::FontSprite> fontSprite;
		bool isDrawing = false;
	public:
		PopUpMessage(std::weak_ptr<DX9GF::TransformManager> tm) : IGameObject(tm) {}
		void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera);
		void QueueMessage(DX9GF::CommandBuffer* commandBuffer, std::wstring message, float duration = 1.0f);
		void Draw(unsigned long long deltaTime);
		void Reset();
	};
}