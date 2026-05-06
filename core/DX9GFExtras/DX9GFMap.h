#pragma once
#include <string>
#include <tmxlite/Map.hpp>
#include <vector>
#include <memory>
#include <functional>
#include <utility>
#include <unordered_map>
#include "../DX9GFCamera.h"
#include "../DX9GFTexture.h"
#include "DX9GFMapLayer.h"
#include "DX9GFICollider.h"
#include "DX9GFColliderManager.h"
#include "DX9GFRectangleCollider.h"

namespace DX9GF {
	class Map {
	public:
		struct ObjectArea {
			float x;
			float y;
			float width;
			float height;
		};
	private:
		GraphicsDevice* graphicsDevice;
		tmx::Map map;
		std::vector<std::shared_ptr<Texture>> textures;
		std::vector<std::shared_ptr<MapLayer>> layers;
		std::vector<std::shared_ptr<ICollider>> colliders;
        
		std::unordered_map<std::string, std::vector<ObjectArea>> objectAreasByLayer;
		std::unordered_map<std::string, std::function<void(const ObjectArea&)>> areaUpdateHandlers;
	public:
		Map(GraphicsDevice* graphicsDevice) : graphicsDevice(graphicsDevice) {}
		void Create(std::weak_ptr<TransformManager> transformManager, std::weak_ptr<ColliderManager> colliderManager, std::string pathToTmx);
		void Draw(const Camera& camera);
        void UpdateAreas(float pointX, float pointY);
		void SetAreaUpdateHandler(const std::string& layerName, std::function<void(const ObjectArea&)> handler);
		std::vector<std::shared_ptr<Texture>>& GetTextures();
		std::vector<std::shared_ptr<MapLayer>>& GetLayers();
		std::vector<std::shared_ptr<ICollider>>& GetColliders();
		friend class MapLayer;
	};
}