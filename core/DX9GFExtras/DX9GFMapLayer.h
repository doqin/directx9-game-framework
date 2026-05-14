#pragma once
#include "../DX9GFCamera.h"
#include "../DX9GFGraphicsDevice.h"
#include "../DX9GFTexture.h"
#include "tmxlite/Tileset.hpp"
#include <memory>
#include <vector>

namespace DX9GF {
	class Map;
	class MapLayer {
	private:
		// info about the vertex for the graphics device
		const DWORD D3DFVF_TILEVERTEX = (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		struct TileVertex {
			float x, y, z;
			DWORD color;
			float u, v;
		};

		struct Subset {
         struct ChunkGeometry {
				std::vector<TileVertex> vertexData;
				float x;
				float y;
				float width;
				float height;
			};
			std::vector<ChunkGeometry> chunks;
			Texture* texture;
		};
		
		std::vector<Subset> subsets;

		bool IsTileIDInTileSet(
			unsigned int idx, 
			const std::vector<std::uint32_t>& tileIDs, 
			const tmx::Tileset& tileSet
		);
		GraphicsDevice* graphicsDevice;
	public:
		MapLayer(GraphicsDevice* graphicsDevice) : graphicsDevice(graphicsDevice) {}
		void Create(Map* map, std::uint32_t layerIndex);
		void Draw(const Camera& camera);
	};
}