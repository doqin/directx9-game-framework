#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include <memory>
#include <functional>
namespace Demo
{
	class IButton : public DX9GF::IGameObject
	{
	public:
		//button state to change its appearance, have to set it public for scene to use
		enum class ButtonState { IDLE, HOVER, CLICKED, DISABLED, LISTENING };
	protected:
		ButtonState currentState;

		float width;
		float height;

		//to reuse component as buttons may have different frame counts.
		int frameCount;

		std::shared_ptr<DX9GF::RectangleTrigger> trigger;
		std::function<void(DX9GF::ITrigger*)> callback;
		DX9GF::Camera* uiCamera = nullptr;

		std::string btnHoverSfxName = "btn_hover";
		std::string btnClickSfxName = "btn_click";
	public:

		IButton(std::shared_ptr<DX9GF::TransformManager> tm, float x, float y, float w, float h, int frames = 3)
			: IGameObject(tm, x, y)
		{
			this->width = w;
			this->height = h;
			this->currentState = ButtonState::IDLE;
			this->frameCount = frames;
		}

		virtual void Init(DX9GF::Camera* uiCamera) = 0;
		//abstract function to determine whether the button displays text or an icon
		virtual void Draw(DX9GF::GraphicsDevice* gd, unsigned long long deltaTime) = 0;


		virtual void SetOnReleaseLeft(std::function<void(DX9GF::ITrigger*)> cb);

		//check if the cursor is hovering over the button.
		virtual void Update(unsigned long long deltaTime);

		void SetState(ButtonState state) { this->currentState = state; }
		ButtonState GetState() const { return this->currentState; }
		std::shared_ptr<DX9GF::RectangleTrigger> GetTrigger() { return this->trigger; }

		float GetWidth() const { return this->width; }
		float GetHeight() const { return this->height; }
	};
}
