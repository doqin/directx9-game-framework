#include "pch.h"
#include "IContainer.h"
#include "DX9GFInputManager.h"
#include "DX9GFUtils.h"

size_t Demo::IContainer::GetMaxWidthOfChildren()
{
	size_t maxWidth = 0;
	for (auto& child : children) {
		if (auto lock = child.lock()) {
			maxWidth = (std::max)(maxWidth, lock->GetWidth());
		}
	}
	return maxWidth;
}

size_t Demo::IContainer::GetHeightOfChildren()
{
	size_t height = 0;
	for (auto& child : children) {
		if (auto lock = child.lock()) {
			height += lock->GetHeight();
		}
	}
	return height;
}

bool Demo::IContainer::OnHover(std::shared_ptr<IDraggable> other)
{
	auto [thisX, thisY] = this->GetWorldPosition();
	auto [otherWorldX, otherWorldY] = other->GetWorldPosition();
	auto otherX = otherWorldX + other->GetWidth() / 2.0f;
	auto otherY = otherWorldY + other->GetHeight() / 2.0f;

	size_t currentHeightOfChildren = GetHeightOfChildren();
	size_t displayedHeightOfChildren = (maxHeight > 0) ? (std::min)(currentHeightOfChildren, maxHeight) : currentHeightOfChildren;

	if (otherX > thisX
		&& otherX < thisX + (std::max)(dragAreaWidth, GetMaxWidthOfChildren())
		&& otherY > thisY + dragAreaHeight
		&& otherY < thisY + 2 * dragAreaHeight + displayedHeightOfChildren) {
		isHovered = true;
		return true;
	}
	isHovered = false;
	return false;
}

bool Demo::IContainer::OnDrop(std::shared_ptr<IDraggable> other)
{
	auto [thisX, thisY] = this->GetWorldPosition();
	auto [otherWorldX, otherWorldY] = other->GetWorldPosition();
	auto otherX = otherWorldX + other->GetTrigger().lock()->GetWidth() / 2.0f;
	auto otherY = otherWorldY + other->GetHeight() / 2.0f;

	size_t currentHeightOfChildren = GetHeightOfChildren();
	size_t displayedHeightOfChildren = (maxHeight > 0) ? (std::min)(currentHeightOfChildren, maxHeight) : currentHeightOfChildren;

	if (otherX > thisX
		&& otherX < thisX + (std::max)(dragAreaWidth, GetMaxWidthOfChildren())
		&& otherY > thisY + dragAreaHeight
		&& otherY < thisY + 2 * dragAreaHeight + displayedHeightOfChildren) {
		other->SetParent(shared_from_this());
		other->SetLocalPosition(0, (float)dragAreaHeight + currentHeightOfChildren);
		children.push_back(other);
		return true;
	}
	return false;
}

void Demo::IContainer::Update(unsigned long long deltaTime)
{
	IDraggable::Update(deltaTime);

	if (maxHeight > 0 && GetHeightOfChildren() > maxHeight) {
		auto inputManager = DX9GF::InputManager::GetInstance();
		auto mouseX = (float)inputManager->GetAbsoluteMouseX();
		auto mouseY = (float)inputManager->GetAbsoluteMouseY();
		auto [thisX, thisY] = GetWorldPosition();

		auto [winX, winY] = DX9GF::Utils::WorldToWindowCoords(*camera, thisX, thisY);

		auto width = (float)(std::max)(dragAreaWidth, GetMaxWidthOfChildren());
		auto height = (float)(maxHeight + dragAreaHeight);

		width *= camera->GetZoom();
		height *= camera->GetZoom();

		if (mouseX >= winX && mouseX <= winX + width &&
			mouseY >= winY && mouseY <= winY + height) {
			float scroll = (float)inputManager->GetMouseScroll();
			if (scroll != 0) {
				scrollOffset -= scroll * 0.1f;
				float maxScroll = (float)GetHeightOfChildren() - maxHeight;
				if (scrollOffset < 0) scrollOffset = 0;
				if (scrollOffset > maxScroll) scrollOffset = maxScroll;
			}
		}
	}
	else {
		scrollOffset = 0;
	}

	size_t yPos = dragAreaHeight;
	// Update children in case they are dettached from parent
	for (size_t i = 0; i < children.size(); ++i) {
		auto lock = children[i].lock();
		if (!lock) {
			children.erase(children.begin() + i);
			--i;
			continue;
		}
		if (auto parent = lock->GetParent(); !parent.has_value() || parent.value().lock().get() != this) {
			children.erase(children.begin() + i);
			--i;
			continue;
		}
		lock->SetLocalPosition(0, (float)yPos - scrollOffset);
		yPos += lock->GetHeight();
	}
}

void Demo::IContainer::Draw(unsigned long long deltaTime)
{
	IDraggable::Draw(deltaTime);

	size_t currentHeightOfChildren = GetHeightOfChildren();
	size_t displayedHeightOfChildren = (maxHeight > 0) ? (std::min)(currentHeightOfChildren, maxHeight) : currentHeightOfChildren;

	graphicsDevice->SetAlphaBlending(true);
	graphicsDevice->DrawRectangle(
		*camera,
		GetWorldX(), 
		GetWorldY() + dragAreaHeight, 
		(std::max)(dragAreaWidth, GetMaxWidthOfChildren()), 
		isHovered ? 2.0f * dragAreaHeight + displayedHeightOfChildren : dragAreaHeight + displayedHeightOfChildren,
		0xAAE0E0E0,
		true
	);
	graphicsDevice->DrawRectangle(
		*camera,
		GetWorldX(),
		GetWorldY() + dragAreaHeight,
		(std::max)(dragAreaWidth, GetMaxWidthOfChildren()),
		isHovered ? 2.0f * dragAreaHeight + displayedHeightOfChildren : dragAreaHeight + displayedHeightOfChildren,
		0xAA000000,
		false
	);
	graphicsDevice->SetAlphaBlending(false);

	if (maxHeight > 0) {
		auto [worldX, worldY] = GetWorldPosition();
		auto [tlX, tlY] = DX9GF::Utils::WorldToWindowCoords(*camera, worldX, worldY + dragAreaHeight);
		auto [brX, brY] = DX9GF::Utils::WorldToWindowCoords(*camera, worldX + (std::max)(dragAreaWidth, GetMaxWidthOfChildren()), worldY + dragAreaHeight + maxHeight);

		RECT rect;
		rect.left = (long)tlX;
		rect.top = (long)tlY;
		rect.right = (long)brX;
		rect.bottom = (long)brY;

		for (auto& child : children) {
			if (auto lock = child.lock()) {
				lock->SetScissorRect(rect);
			}
		}

		if (GetHeightOfChildren() > maxHeight && scrollOffset < (float)GetHeightOfChildren() - maxHeight) {
			if (auto manager = draggableManager) {
				auto cmd = std::make_shared<DX9GF::CustomCommand>([this, deltaTime](std::function<void()> markFinished) {
					if (debugFontSprite) {
						auto [worldX, worldY] = GetWorldPosition();
						float textX = worldX + 8;
						float textY = worldY + dragAreaHeight + maxHeight + 8;

						debugFontSprite->Begin();
						debugFontSprite->SetPosition(textX, textY);
						debugFontSprite->SetOutline(true, 0xFF000000, 2.0f);
						debugFontSprite->SetColor(0xFFFFFFFF);
						debugFontSprite->SetText(L"More...");
						debugFontSprite->Draw(*camera, deltaTime);
						debugFontSprite->End();
					}
					markFinished();
				});
				manager->QueueDraw(cmd);
			}
		}
	}
}

void Demo::IContainer::AddChildProgrammatically(std::shared_ptr<IDraggable> child)
{
	child->SetParent(shared_from_this());
	children.push_back(child);
}

void Demo::IContainer::ClearChildren()
{
	children.clear();
}