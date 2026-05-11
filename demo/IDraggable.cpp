#include "pch.h"
#include "IDraggable.h"
#include <DX9GF.h>
#include <algorithm>
#include <random>

// Define static member variables
std::shared_ptr<DX9GF::Font> Demo::IDraggable::debugFont = nullptr;
std::shared_ptr<DX9GF::FontSprite> Demo::IDraggable::debugFontSprite = nullptr;

void Demo::DraggableManager::RebuildHierarchy()
{
	std::vector<std::weak_ptr<IDraggable>> newHierarchy;
	std::vector<LevelBatch> newLevels;
	size_t currentStart = 0;
	for (auto& [key, val] : objectMap) {
		// Remove dead objects
		if (!val) {
			objectMap.erase(key);
			continue;
		}
		if (auto parent = val->GetParent(); !parent.has_value()) {
			newHierarchy.push_back(val);
		}
	}
	newLevels.push_back({currentStart, newHierarchy.size()});
	while (newHierarchy.size() < objectMap.size()) {
		auto lastLevel = newLevels.back();
		currentStart = newHierarchy.size();
		size_t sizeBeforeIteration = newHierarchy.size();
		
		for (size_t i = lastLevel.startIdx; i < lastLevel.endIdx; ++i) {
			for (auto& [key, val] : objectMap) {
				// Remove dead objects
				if (!val) {
					objectMap.erase(key);
					continue;
				}
				if (auto parent = val->GetParent(); parent.has_value()) {
					auto lock = parent.value().lock();
					// Skip if parent is not an IDraggable, that is up to TransformManager to care
					if (auto parentDraggable = dynamic_pointer_cast<IDraggable>(lock)) {
						if (parentDraggable->GetID() == newHierarchy[i].lock()->GetID()) {
							newHierarchy.push_back(val);
						}
					}
				}
			}
		}
		
		// Break if no new objects were added to prevent infinite loop
		if (newHierarchy.size() == sizeBeforeIteration) {
			break;
		}
		
		newLevels.push_back({ currentStart, newHierarchy.size()});
	}
	hierarchy = std::move(newHierarchy);
	levels = std::move(newLevels);
}

void Demo::DraggableManager::Add(std::shared_ptr<IDraggable> obj)
{
	objectMap[obj->GetID()] = obj;
	this->RebuildHierarchy();
}

void Demo::DraggableManager::Remove(std::shared_ptr<IDraggable> obj)
{
	if (auto idx = objectMap.erase(obj->GetID()); idx) {
		this->RebuildHierarchy();
	}
}

void Demo::DraggableManager::HoverDroppable(std::shared_ptr<IDraggable> obj)
{
	for (auto& [_, val] : objectMap) {
		// skip current object
		if (val.get() == obj.get()) continue;
		// skip children
		bool isChildren = false;
		for (auto parent = val->GetParent(); parent.has_value(); parent = parent.value().lock()->GetParent()) {
			if (auto lock = parent.value().lock(); lock.get() == obj.get()) {
				isChildren = true;
				break;
			}
		}
		if (isChildren) continue;
		if (val->OnHover(obj)) break;
	}
}

void Demo::DraggableManager::AttachDroppable(std::shared_ptr<IDraggable> obj)
{
	for (auto& [_, val] : objectMap) {
		// skip current object
		if (val.get() == obj.get()) continue;
		// skip children
		bool isChildren = false;
		for (auto parent = val->GetParent(); parent.has_value(); parent = parent.value().lock()->GetParent()) {
			if (auto lock = parent.value().lock(); lock.get() == obj.get()) {
				isChildren = true;
				break;
			}
		}
		if (isChildren) continue;
		if (val->OnDrop(obj)) break;
	}
}

void Demo::DraggableManager::Update(unsigned long long deltaTime)
{
   // Snapshot to avoid iterator/index invalidation if an Update() rebuilds hierarchy.
	auto levelsSnapshot = levels;
	auto hierarchySnapshot = hierarchy;

	// Updates from the top level to the bottom level
	for (size_t i = levelsSnapshot.size(); i-- > 0;) {
		const auto startIdx = std::min(levelsSnapshot[i].startIdx, hierarchySnapshot.size());
		const auto endIdx = std::min(levelsSnapshot[i].endIdx, hierarchySnapshot.size());
		for (size_t j = startIdx; j < endIdx; ++j) {
			if (auto lock = hierarchySnapshot[j].lock()) {
				lock->Update(deltaTime);
			}
		}
	}
}

void Demo::DraggableManager::Draw(unsigned long long deltaTime)
{
	std::vector<std::shared_ptr<IDraggable>> isDraggingDraggables;
	for (size_t i = 0; i < levels.size(); ++i) {
		for (size_t j = levels[i].startIdx; j < levels[i].endIdx; ++j) {
			if (auto lock = hierarchy[j].lock()) {
				if (lock->IsDragging()) {
					isDraggingDraggables.push_back(lock);
					continue;
				}
				bool isFollowingDragged = false;
				for (auto parent = lock->GetParent(); parent.has_value(); parent = parent.value().lock()->GetParent()) {
					if (auto lock = parent.value().lock()) {
						if (auto draggableParent = dynamic_pointer_cast<IDraggable>(lock)) {
							if (draggableParent->IsDragging()) {
								isFollowingDragged = true;
								break;
							}
						}
					}
				}
				if (isFollowingDragged) {
					isDraggingDraggables.push_back(lock);
					continue;
				}
				lock->Draw(deltaTime);
			}
		}
	}
	for (auto& draggable : isDraggingDraggables) {
		draggable->Draw(deltaTime);
	}
	while(drawBuffer.IsBusy()) {
		drawBuffer.Update(deltaTime);
	}
}

void Demo::DraggableManager::QueueDraw(std::shared_ptr<DX9GF::ICommand> cmd)
{
	drawBuffer.PushCommand(std::move(cmd));
}

void Demo::IDraggable::Init(std::shared_ptr<DraggableManager> manager, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera)
{
	this->draggableManager = manager;
	this->camera = camera;
	this->graphicsDevice = graphicsDevice;
	trigger = std::make_shared<DX9GF::RectangleTrigger>(
		this->transformManager, 
		shared_from_this(), 
		dragAreaWidth, 
		dragAreaHeight,
		0,
		0,
		0,
		1,
		1
	);
	trigger->Init(camera);
	//trigger->SetOriginCenter();
	trigger->SetOnHeldLeft([&](DX9GF::ITrigger* thisObj) {
		auto parent = dynamic_pointer_cast<IDraggable>(thisObj->GetParent().value().lock());
		if (parent->GetParent().has_value()) {
			parent->DetachParent();
		}
		auto inpMan = DX9GF::InputManager::GetInstance();
		auto dX = inpMan->GetRelativeMouseX();
		auto dY = inpMan->GetRelativeMouseY();
		auto currentX = parent->GetWorldX();
		auto currentY = parent->GetWorldY();
		parent->SetLocalPosition(currentX + dX / this->camera->GetZoom(), currentY + dY / this->camera->GetZoom());
		parent->GetTransformManager().lock()->RebuildHierarchy();
		isDragging = true;
	});
	trigger->SetOnReleaseLeft([&](DX9GF::ITrigger* thisObj) {
		auto parent = dynamic_pointer_cast<IDraggable>(thisObj->GetParent().value().lock());
		parent->GetDraggableManager().lock()->AttachDroppable(parent);
		parent->GetTransformManager().lock()->RebuildHierarchy();
		isDragging = false;
	});
	manager->Add(dynamic_pointer_cast<IDraggable>(shared_from_this()));
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(0, 1);
	color = D3DXCOLOR(dist(gen), dist(gen), dist(gen), 1);
	if (debugFont.get() == nullptr) {
		IDraggable::debugFont = std::make_shared<DX9GF::Font>(graphicsDevice, L"StatusPlz", 16);
	}
	if (debugFontSprite.get() == nullptr) {
		IDraggable::debugFontSprite = std::make_shared<DX9GF::FontSprite>(debugFont.get());
		debugFontSprite->SetColor(0xFF000000);
	}
}

void Demo::IDraggable::SetParent(std::weak_ptr<IGameObject> parent)
{
	this->parent = parent;
	auto [parentX, parentY] = parent.lock()->GetWorldPosition();
	auto [objX, objY] = this->GetWorldPosition();
	this->SetLocalPosition(objX - parentX, objY - parentY);
	draggableManager->RebuildHierarchy();
	if (auto lock = GetTransformManager().lock()) {
		lock->SetParent(transformHandle, parent.lock()->GetTransformHandle().slotIndex);
		lock->RebuildHierarchy();
	}
}

void Demo::IDraggable::DetachParent()
{
	this->parent.reset();
	draggableManager->RebuildHierarchy();
	if (auto lock = GetTransformManager().lock()) {
		lock->SetParent(transformHandle, -1);
		lock->RebuildHierarchy();
	}
}

void Demo::IDraggable::Update(unsigned long long deltaTime)
{
	this->trigger->Update(deltaTime);
}

void Demo::IDraggable::Draw(unsigned long long deltaTime)
{
	trigger->Draw(graphicsDevice, *camera);
	auto thisX = trigger->GetWorldX() - trigger->GetOriginX();
	auto thisY = trigger->GetWorldY() - trigger->GetOriginY();
	//graphicsDevice->DrawLine(*camera, thisX, thisY, thisX + 1, thisY + 1, 0xFFFF0000);
	if (debug) {
		debugFontSprite->Begin();
		auto [x, y] = GetWorldPosition();
		debugFontSprite->SetPosition(x + dragAreaWidth / static_cast<float>(2), y + dragAreaHeight / static_cast<float>(2));
		std::wstring ws(id.begin(), id.end());
		debugFontSprite->SetText(std::move(ws));
		debugFontSprite->Draw(*camera, deltaTime);
		if (parent.has_value()) {
			if (auto draggableParent = dynamic_pointer_cast<IDraggable>(parent.value().lock())) {
				auto id = draggableParent->GetID();
				std::wstring parentIDWs(id.begin(), id.end());
				debugFontSprite->SetText(L"Parent: " + parentIDWs);
				debugFontSprite->SetPosition(x + dragAreaWidth / static_cast<float>(2), y + dragAreaHeight / static_cast<float>(2) + 30);
				debugFontSprite->Draw(*camera, deltaTime);
			}
		}
		debugFontSprite->End();
	}
}

bool Demo::IDraggable::OnDrop(std::shared_ptr<IDraggable> other)
{
	auto thisX = trigger->GetWorldX() - trigger->GetOriginX();
	auto thisY = trigger->GetWorldY() - trigger->GetOriginY();
	auto otherX = other->trigger->GetWorldX() - other->trigger->GetOriginX() + other->trigger->GetWidth() / 2.0f;
	auto otherY = other->trigger->GetWorldY() - other->trigger->GetOriginY() + other->trigger->GetHeight() / 2.0f;
	if (otherX > thisX
	   && otherX < thisX + trigger->GetWidth()
		&& otherY > thisY
       && otherY < thisY + trigger->GetHeight()) {
		other->SetParent(shared_from_this());
		return true;
	}
	return false;
}

bool Demo::IDraggable::OnHover(std::shared_ptr<IDraggable> other)
{
	auto thisX = trigger->GetWorldX() - trigger->GetOriginX();
	auto thisY = trigger->GetWorldY() - trigger->GetOriginY();
	auto otherX = other->trigger->GetWorldX() - other->trigger->GetOriginX() + other->trigger->GetWidth() / 2.0f;
	auto otherY = other->trigger->GetWorldY() - other->trigger->GetOriginY() + other->trigger->GetHeight() / 2.0f;
	if (otherX > thisX
	   && otherX < thisX + trigger->GetWidth()
		&& otherY > thisY
       && otherY < thisY + trigger->GetHeight()) {
		return true;
	}
	return false;
}

std::string Demo::IDraggable::GetID() const
{
	return this->id;
}

size_t Demo::IDraggable::GetWidth() const
{
	return dragAreaWidth;
}

size_t Demo::IDraggable::GetHeight() const
{
	return dragAreaHeight;
}

std::weak_ptr<Demo::DraggableManager> Demo::IDraggable::GetDraggableManager()
{
	return draggableManager;
}

std::weak_ptr<DX9GF::RectangleTrigger> Demo::IDraggable::GetTrigger()
{
	return trigger;
}

bool Demo::IDraggable::IsDragging() const
{
	return isDragging;
}
