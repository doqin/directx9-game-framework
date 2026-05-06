#include "pch.h"
#include "DX9GFMap.h"
#include "../DX9GFUtils.h"
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace {
	bool EqualsIgnoreCase(const std::string& a, const std::string& b)
	{
		return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](unsigned char left, unsigned char right) {
			return std::tolower(left) == std::tolower(right);
		});
	}
}

void DX9GF::Map::Create(std::weak_ptr<TransformManager> transformManager, std::weak_ptr<ColliderManager> colliderManager, std::string pathToTmx)
{
	if (!std::filesystem::exists(pathToTmx)) {
		throw std::invalid_argument("Could not find map file: " + pathToTmx);
	}
	if (!map.load(pathToTmx)) {
		throw std::invalid_argument("Could not load map file: " + pathToTmx);
	}
	// load the textures
	const auto& tileSets = map.getTilesets();
	assert(!tileSets.empty());
	for (const auto& ts : tileSets)
	{
		std::string imgPathStr = ts.getImagePath();
		if (!std::filesystem::exists(imgPathStr)) {
			// Skip or throw? It's better to throw or log, let's throw.
			throw std::invalid_argument("Could not find texture file: " + imgPathStr);
		}
		textures.emplace_back(std::make_shared<Texture>(graphicsDevice));
		std::wstring imgPath{ std::from_range, imgPathStr };
		textures.back()->LoadTexture(imgPath);
	}
	// load the layers
	const auto& mapLayers = map.getLayers();
	for (auto i = 0u; i < mapLayers.size(); ++i)
	{
		if (mapLayers[i]->getType() == tmx::Layer::Type::Tile)
		{
			layers.emplace_back(std::make_shared<MapLayer>(graphicsDevice));
			layers.back()->Create(this, i); //just cos we're using C++14
		}
        if (mapLayers[i]->getType() == tmx::Layer::Type::Object)
		{
			const auto& layer = mapLayers[i]->getLayerAs<tmx::ObjectGroup>();
           const auto& layerName = layer.getName();
			const bool isCollisionLayer = EqualsIgnoreCase(layerName, "collision");
			const auto& objects = layer.getObjects();
			for (auto& object : objects) {
				if (object.getShape() == tmx::Object::Shape::Rectangle) {
					auto rect = object.getAABB();
                  if (isCollisionLayer) {
						std::shared_ptr<RectangleCollider> collider = std::make_shared<RectangleCollider>(transformManager, rect.width, rect.height, rect.left, rect.top);
						colliders.push_back(collider);
						colliderManager.lock()->Add(collider);
					}
                  else {
						objectAreasByLayer[layerName].push_back({ rect.left, rect.top, rect.width, rect.height });
					}
				}
			}
		}
	}
}

void DX9GF::Map::Draw(const Camera& camera)
{
	for (auto& layer : layers) {
		layer->Draw(camera);
	}
}

void DX9GF::Map::UpdateAreas(float pointX, float pointY)
{
   for (const auto& [layerName, areas] : objectAreasByLayer) {
		auto handlerIt = areaUpdateHandlers.find(layerName);
		if (handlerIt == areaUpdateHandlers.end()) {
			continue;
		}
		const auto& handler = handlerIt->second;
		if (!handler) {
			continue;
		}
		for (const auto& area : areas) {
			if (Utils::IsWithinRectangle(pointX, pointY, area.x, area.y, area.width, area.height)) {
				handler(area);
			}
		}
	}
}

void DX9GF::Map::SetAreaUpdateHandler(const std::string& layerName, std::function<void(const ObjectArea&)> handler)
{
 areaUpdateHandlers[layerName] = std::move(handler);
}

std::vector<std::shared_ptr<DX9GF::Texture>>& DX9GF::Map::GetTextures()
{
	return textures;
}

std::vector<std::shared_ptr<DX9GF::MapLayer>>& DX9GF::Map::GetLayers()
{
	return layers;
}

std::vector<std::shared_ptr<DX9GF::ICollider>>& DX9GF::Map::GetColliders()
{
	return colliders;
}
