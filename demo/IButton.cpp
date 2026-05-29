#include "pch.h"
#include "IButton.h"
#include "DX9GFAudioManager.h"
void Demo::IButton::SetOnReleaseLeft(std::function<void(DX9GF::ITrigger*)> cb)
{
	//save the button's feature
	this->callback = cb;

	if (this->trigger && this->callback)
	{
		this->trigger->SetOnReleaseLeft(this->callback);
	}
}

//check if the cursor is hovering over the button.
void Demo::IButton::Update(unsigned long long deltaTime)
{
	if (!this->trigger)
		return;

	//update trigger
	this->trigger->Update(deltaTime);

	ButtonState oldState = this->currentState;

	if (this->currentState == ButtonState::LISTENING || this->currentState == ButtonState::DISABLED)
	{
		return;
	}

	//change button state
	if (this->trigger->IsHeldLeft(deltaTime)) {
		this->currentState = ButtonState::CLICKED;
	}
	else if (this->trigger->IsHovering(deltaTime))
	{
		this->currentState = ButtonState::HOVER;
	}
	else
		this->currentState = ButtonState::IDLE;

	//isolate logic between button's state / cursor's state
	if (this->trigger->IsHovering(deltaTime)) {
		if (this->trigger->IsHeldLeft(deltaTime)) {
			DX9GF::InputManager::GetInstance()->SwitchCursor(DX9GF::InputManager::CursorType::CLICK);
		}
		else {
			DX9GF::InputManager::GetInstance()->SwitchCursor(DX9GF::InputManager::CursorType::POINTER);
		}
	}

	if (oldState == ButtonState::IDLE && this->currentState == ButtonState::HOVER)
	{
		DX9GF::AudioManager::GetInstance()->Play(btnHoverSfxName);
	}

	if (oldState != ButtonState::CLICKED && this->currentState == ButtonState::CLICKED)
	{
		DX9GF::AudioManager::GetInstance()->Play(btnClickSfxName);
	}
}

