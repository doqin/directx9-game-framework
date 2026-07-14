#include "pch.h"
#include "DX9GFTransformManager.h"
#include "pch.h"

void DX9GF::TransformManager::ComputeTransform(int dataIndex) {
	auto& t = transforms[dataIndex];
	if (t.parentSlotIndex == NO_PARENT) {
		t.worldX = t.localX;
		t.worldY = t.localY;
		t.worldRotation = t.localRotation;
		t.worldScaleX = t.localScaleX;
		t.worldScaleY = t.localScaleY;
	}
	else {
		const auto& p = transforms[slots[t.parentSlotIndex].dataIndex];
		// calculate scale
		t.worldScaleX = p.worldScaleX * t.localScaleX;
		t.worldScaleY = p.worldScaleY * t.localScaleY;
		// calculate rotation
		t.worldRotation = p.worldRotation + t.localRotation;
		// calculate position
		float cosR = cosf(p.worldRotation);
		float sinR = sinf(p.worldRotation);
		float rotatedX = (t.localX * cosR) - (t.localY * sinR);
		float rotatedY = (t.localX * sinR) + (t.localY * cosR);
		t.worldX = p.worldX + (rotatedX * p.worldScaleX);
		t.worldY = p.worldY + (rotatedY * p.worldScaleY);
	}
}

void DX9GF::TransformManager::UpdateAll()
{
	for (const auto& level : levels) {
		for (size_t i = level.start; i < level.end; i++) {
			ComputeTransform(slots[i].dataIndex);
		}
	}
}

void DX9GF::TransformManager::RebuildHierarchy()
{
	std::vector<TransformData> sortedTransforms;
	levels.clear();

	// find all roots
	size_t currentStart = 0;
	for (size_t i = 0; i < transforms.size(); ++i) {
		if (transforms[i].parentSlotIndex == NO_PARENT) {
			sortedTransforms.push_back(transforms[i]);
			slots[transforms[i].ownerSlotIndex].dataIndex = sortedTransforms.size() - 1; // update slot's dataIndex
		}
	}
	levels.push_back({ currentStart, sortedTransforms.size() });
	// Iteratively find children for each level
	while (sortedTransforms.size() < transforms.size()) {
		currentStart = sortedTransforms.size();
		size_t previousLevelStart = levels.back().start;
		size_t previousLevelEnd = levels.back().end;
		for (size_t i = previousLevelStart; i < previousLevelEnd; i++) {
			// TODO: optimize by storing the child IDs in the parent
			for (size_t j = 0; j < transforms.size(); ++j) {
				if (transforms[j].parentSlotIndex == sortedTransforms[i].ownerSlotIndex) {
					sortedTransforms.push_back(transforms[j]);
					slots[transforms[j].ownerSlotIndex].dataIndex = sortedTransforms.size() - 1;
				}
			}
		}
		if (sortedTransforms.size() == currentStart) break;
		levels.push_back({ currentStart, sortedTransforms.size() });
	}
	transforms = std::move(sortedTransforms);
}

DX9GF::TransformData& DX9GF::TransformManager::GetTransform(size_t slotIdx)
{
	ValidateSlotIndexing(slotIdx);
	return transforms[slots[slotIdx].dataIndex];
}

void DX9GF::TransformManager::ValidateSlotIndexing(size_t slotIdx)
{
	if (slotIdx >= slots.size()) {
		throw std::invalid_argument("Attempted to index out of bound");
	}
	if (!slots[slotIdx].isAlive) {
		throw std::invalid_argument("Attempted to index a dead transform data");
	}
}

DX9GF::TransformHandle DX9GF::TransformManager::CreateTransform(
	uint32_t parentSlotIdx,
	float localX,
	float localY,
	float localRotation,
	float localScaleX,
	float localScaleY)
{
	if (parentSlotIdx != NO_PARENT) this->ValidateSlotIndexing(parentSlotIdx);
	uint32_t slotIndex;
	if (!freeSlots.empty()) {
		// Reuse an old slot
		slotIndex = freeSlots.back();
		freeSlots.pop_back();
		// index uses the old dataIndex to the transforms array
	}
	else {
		// Grow the arrays
		slotIndex = (uint32_t)slots.size();
		transforms.emplace_back();
		slots.emplace_back();
		slots[slotIndex].dataIndex = slotIndex; // end of the list
		transforms[slotIndex].ownerSlotIndex = slotIndex;
	}
	slots[slotIndex].isAlive = true;
	slots[slotIndex].generation++; // Increment to invalidate old handles
	DX9GF::TransformData& transformData = transforms[slots[slotIndex].dataIndex];
	transformData.parentSlotIndex = parentSlotIdx;
	transformData.localX = localX;
	transformData.localY = localY;
	transformData.localRotation = localRotation;
	transformData.localScaleX = localScaleX;
	transformData.localScaleY = localScaleY;
	this->ComputeTransform(slots[slotIndex].dataIndex);
	return {
		.slotIndex=slotIndex, 
		.generation=slots[slotIndex].generation 
	};
}

void DX9GF::TransformManager::DestroyTransform(const TransformHandle& handle)
{
	ValidateSlotIndexing(handle.slotIndex);
	if (!slots[handle.slotIndex].isAlive) throw std::invalid_argument("Attempted to destroy a dead transform data");
	if (slots[handle.slotIndex].generation != handle.generation) throw std::invalid_argument("Attempted to destroy an expired transform data");
	slots[handle.slotIndex].isAlive = false;
	freeSlots.push_back(handle.slotIndex);
}

void DX9GF::TransformManager::SetParent(const TransformHandle& handle, uint32_t parentSlotIndex)
{
	transforms[slots[handle.slotIndex].dataIndex].parentSlotIndex = parentSlotIndex;
}