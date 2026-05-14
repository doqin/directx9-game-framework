#include "pch.h"
#include "DX9GFMapLayer.h"
#include "DX9GFMap.h"
#include "tmxlite/TileLayer.hpp"
#include <ranges>
#include <algorithm>
#include "../DX9GFUtils.h"

bool DX9GF::MapLayer::IsTileIDInTileSet(unsigned int idx, const std::vector<std::uint32_t>& tileIDs, const tmx::Tileset& tileSet)
{
	return idx < tileIDs.size() && tileIDs[idx] >= tileSet.getFirstGID() && tileIDs[idx] < (tileSet.getFirstGID() + tileSet.getTileCount());
}

void DX9GF::MapLayer::Create(Map* map, std::uint32_t layerIndex)
{
	auto& tmxMap = map->map;
	const auto& layers = tmxMap.getLayers();
	const auto& textures = map->textures;
	assert(layers[layerIndex]->getType() == tmx::Layer::Type::Tile);
	const auto& layer = layers[layerIndex]->getLayerAs<tmx::TileLayer>();
	const auto gridSize = tmxMap.getTileSize(); // returns the size of a grid
	const auto& tileSets = tmxMap.getTilesets();
	const auto tintColour = layer.getTintColour();
	D3DXCOLOR vertexColour(
		static_cast<float>(tintColour.r) / 0xFF, 
		static_cast<float>(tintColour.g) / 0xFF,
		static_cast<float>(tintColour.b) / 0xFF,
		static_cast<float>(tintColour.a) / 0xFF
	);

	std::vector<tmx::TileLayer::Chunk> chunks;
	if (tmxMap.isInfinite()) {
		chunks = layer.getChunks();
	} else {
		tmx::TileLayer::Chunk chunk;
		chunk.position = { 0, 0 };
		auto mapSize = tmxMap.getTileCount();
		chunk.size = { static_cast<int>(mapSize.x), static_cast<int>(mapSize.y) };
		chunk.tiles = layer.getTiles();
		chunks.push_back(chunk);
	}

   for (auto i = 0u; i < tileSets.size(); ++i) {
		const auto& tileSet = tileSets[i];
		const auto& [textureSizeX, textureSizeY] = textures[i]->GetSize();
		const auto& tileSetTileSize = tileSet.getTileSize();
		const auto tileCountX = tileSet.getColumnCount();
		const auto tileSpacing = tileSet.getSpacing();
		const auto tileMargin = tileSet.getMargin();
		const float uNorm = static_cast<float>(tileSetTileSize.x) / textureSizeX;
		const float vNorm = static_cast<float>(tileSetTileSize.y) / textureSizeY;
		Subset subset;
		subset.texture = textures[i].get();
		for (const auto& chunk : chunks) {
			const auto tilesView = chunk.tiles | std::views::transform([](const tmx::TileLayer::Tile& tile) { return tile.ID; });
			const std::vector<std::uint32_t> tileIDs(tilesView.begin(), tilesView.end());
			std::vector<TileVertex> vertices;
			for (int y = 0; y < chunk.size.y; ++y) {
				for (int x = 0; x < chunk.size.x; ++x) { // Fixed: was ++y, should be ++x
					const auto idx = y * chunk.size.x + x;
					if (!IsTileIDInTileSet(idx, tileIDs, tileSet)) continue;
					// texture coords
					auto idIndex = (tileIDs[idx] - tileSet.getFirstGID());
					const auto tileX = idIndex % tileCountX;
					const auto tileY = idIndex / tileCountX;
					float u = static_cast<float>(tileMargin + tileX * (tileSetTileSize.x + tileSpacing)) / textureSizeX;
					float v = static_cast<float>(tileMargin + tileY * (tileSetTileSize.y + tileSpacing)) / textureSizeY;
					// vertex pos
					const float tilePosX = static_cast<float>(x + chunk.position.x) * gridSize.x;
					const float tilePosY = (static_cast<float>(y + chunk.position.y) * gridSize.y);
					// first triangle
					// top left, top right, bottom left
					vertices.push_back({ .x = tilePosX, .y = tilePosY, .z = .0f, .color = vertexColour, .u = u, .v = v });
					vertices.push_back({ .x = tilePosX + tileSetTileSize.x, .y = tilePosY, .z = .0f, .color = vertexColour, .u = u + uNorm, .v = v });
					vertices.push_back({ .x = tilePosX, .y = tilePosY + tileSetTileSize.y, .z = .0f, .color = vertexColour, .u = u, .v = v + vNorm });

					// second triangle
					// top right, bottom right, bottom left
					vertices.push_back({ .x = tilePosX + tileSetTileSize.x, .y = tilePosY, .z = .0f, .color = vertexColour, .u = u + uNorm, .v = v });
					vertices.push_back({ .x = tilePosX + tileSetTileSize.x, .y = tilePosY + tileSetTileSize.y, .z = .0f, .color = vertexColour, .u = u + uNorm, .v = v + vNorm });
					vertices.push_back({ .x = tilePosX, .y = tilePosY + tileSetTileSize.y, .z = .0f, .color = vertexColour, .u = u, .v = v + vNorm });
				}
			}

			if (!vertices.empty()) {
				Subset::ChunkGeometry chunkGeometry;
				chunkGeometry.vertexData.swap(vertices);
				chunkGeometry.x = static_cast<float>(chunk.position.x) * gridSize.x;
				chunkGeometry.y = static_cast<float>(chunk.position.y) * gridSize.y;
				chunkGeometry.width = static_cast<float>(chunk.size.x) * gridSize.x;
				chunkGeometry.height = static_cast<float>(chunk.size.y) * gridSize.y;
				subset.chunks.emplace_back(std::move(chunkGeometry));
			}
		}

		if (!subset.chunks.empty()) {
			subsets.emplace_back(std::move(subset));
		}
	}
}

void DX9GF::MapLayer::Draw(const Camera& camera)
{
	auto* device = graphicsDevice != nullptr ? graphicsDevice->GetDevice() : nullptr;
	if (device == nullptr || subsets.empty()) return;

	DWORD oldFVF = 0;
	device->GetFVF(&oldFVF);

	D3DXMATRIX oldWorld, oldView, oldProj;
	device->GetTransform(D3DTS_WORLD, &oldWorld);
	device->GetTransform(D3DTS_VIEW, &oldView);
	device->GetTransform(D3DTS_PROJECTION, &oldProj);

	IDirect3DBaseTexture9* oldTexture0 = nullptr;
	device->GetTexture(0, &oldTexture0);

	DWORD oldAlphaBlendEnable = FALSE;
	DWORD oldSrcBlend = D3DBLEND_ONE;
	DWORD oldDestBlend = D3DBLEND_ZERO;
	DWORD oldZEnable = D3DZB_FALSE;
	DWORD oldLighting = FALSE;
	DWORD oldCullMode = D3DCULL_CCW;
	device->GetRenderState(D3DRS_ALPHABLENDENABLE, &oldAlphaBlendEnable);
	device->GetRenderState(D3DRS_SRCBLEND, &oldSrcBlend);
	device->GetRenderState(D3DRS_DESTBLEND, &oldDestBlend);
	device->GetRenderState(D3DRS_ZENABLE, &oldZEnable);
	device->GetRenderState(D3DRS_LIGHTING, &oldLighting);
	device->GetRenderState(D3DRS_CULLMODE, &oldCullMode);

	DWORD oldColorOp = D3DTOP_MODULATE;
	DWORD oldColorArg1 = D3DTA_TEXTURE;
	DWORD oldColorArg2 = D3DTA_DIFFUSE;
	DWORD oldAlphaOp = D3DTOP_MODULATE;
	DWORD oldAlphaArg1 = D3DTA_TEXTURE;
	DWORD oldAlphaArg2 = D3DTA_DIFFUSE;
	device->GetTextureStageState(0, D3DTSS_COLOROP, &oldColorOp);
	device->GetTextureStageState(0, D3DTSS_COLORARG1, &oldColorArg1);
	device->GetTextureStageState(0, D3DTSS_COLORARG2, &oldColorArg2);
	device->GetTextureStageState(0, D3DTSS_ALPHAOP, &oldAlphaOp);
	device->GetTextureStageState(0, D3DTSS_ALPHAARG1, &oldAlphaArg1);
	device->GetTextureStageState(0, D3DTSS_ALPHAARG2, &oldAlphaArg2);

	DWORD oldMinFilter = D3DTEXF_LINEAR;
	DWORD oldMagFilter = D3DTEXF_LINEAR;
	DWORD oldMipFilter = D3DTEXF_NONE;
	DWORD oldAddressU = D3DTADDRESS_WRAP;
	DWORD oldAddressV = D3DTADDRESS_WRAP;
	device->GetSamplerState(0, D3DSAMP_MINFILTER, &oldMinFilter);
	device->GetSamplerState(0, D3DSAMP_MAGFILTER, &oldMagFilter);
	device->GetSamplerState(0, D3DSAMP_MIPFILTER, &oldMipFilter);
	device->GetSamplerState(0, D3DSAMP_ADDRESSU, &oldAddressU);
	device->GetSamplerState(0, D3DSAMP_ADDRESSV, &oldAddressV);

  const auto [screenWidth, screenHeight] = camera.GetScreenResolution();
	const auto [corner00X, corner00Y] = Utils::WindowToWorldCoords(camera, 0.0f, 0.0f);
	const auto [corner10X, corner10Y] = Utils::WindowToWorldCoords(camera, static_cast<float>(screenWidth), 0.0f);
	const auto [corner01X, corner01Y] = Utils::WindowToWorldCoords(camera, 0.0f, static_cast<float>(screenHeight));
	const auto [corner11X, corner11Y] = Utils::WindowToWorldCoords(camera, static_cast<float>(screenWidth), static_cast<float>(screenHeight));
	const float viewMinX = std::min({ corner00X, corner10X, corner01X, corner11X });
	const float viewMaxX = std::max({ corner00X, corner10X, corner01X, corner11X });
	const float viewMinY = std::min({ corner00Y, corner10Y, corner01Y, corner11Y });
	const float viewMaxY = std::max({ corner00Y, corner10Y, corner01Y, corner11Y });
	const float overlapEpsilon = 0.01f;
	const auto intersectsView = [&](float x, float y, float width, float height) {
		const float rectMaxX = x + width;
		const float rectMaxY = y + height;
		return rectMaxX > viewMinX + overlapEpsilon
			&& x < viewMaxX - overlapEpsilon
			&& rectMaxY > viewMinY + overlapEpsilon
			&& y < viewMaxY - overlapEpsilon;
	};
	D3DXMATRIX matView;
	D3DXMatrixIdentity(&matView);
	D3DXMATRIX matProj;
	D3DXMatrixOrthoOffCenterLH(
		&matProj,
		0.0f,
		static_cast<float>(screenWidth),
		static_cast<float>(screenHeight),
		0.0f,
		0.0f,
		1.0f
	);
	auto matCamera = camera.GetTransformMatrix();
	device->SetTransform(D3DTS_WORLD, &matCamera);
	device->SetTransform(D3DTS_VIEW, &matView);
	device->SetTransform(D3DTS_PROJECTION, &matProj);

	device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	device->SetRenderState(D3DRS_LIGHTING, FALSE);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	device->SetFVF(D3DFVF_TILEVERTEX);
	for (const auto& subset : subsets) {
        if (subset.chunks.empty()) continue;

		if (auto texture = subset.texture; texture != nullptr) {
			device->SetTexture(0, texture->GetRawTexture());
		}
		else {
			device->SetTexture(0, nullptr);
		}

       for (const auto& chunk : subset.chunks) {
			if (chunk.vertexData.empty()) continue;
          if (!intersectsView(chunk.x, chunk.y, chunk.width, chunk.height)) continue;
			const UINT primitiveCount = static_cast<UINT>(chunk.vertexData.size() / 3);
			if (primitiveCount == 0) continue;
			device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, primitiveCount, chunk.vertexData.data(), sizeof(TileVertex));
		}
	}

	device->SetFVF(oldFVF);
	device->SetTransform(D3DTS_WORLD, &oldWorld);
	device->SetTransform(D3DTS_VIEW, &oldView);
	device->SetTransform(D3DTS_PROJECTION, &oldProj);

	device->SetRenderState(D3DRS_ALPHABLENDENABLE, oldAlphaBlendEnable);
	device->SetRenderState(D3DRS_SRCBLEND, oldSrcBlend);
	device->SetRenderState(D3DRS_DESTBLEND, oldDestBlend);
	device->SetRenderState(D3DRS_ZENABLE, oldZEnable);
	device->SetRenderState(D3DRS_LIGHTING, oldLighting);
	device->SetRenderState(D3DRS_CULLMODE, oldCullMode);

	device->SetTextureStageState(0, D3DTSS_COLOROP, oldColorOp);
	device->SetTextureStageState(0, D3DTSS_COLORARG1, oldColorArg1);
	device->SetTextureStageState(0, D3DTSS_COLORARG2, oldColorArg2);
	device->SetTextureStageState(0, D3DTSS_ALPHAOP, oldAlphaOp);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG1, oldAlphaArg1);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG2, oldAlphaArg2);

	device->SetSamplerState(0, D3DSAMP_MINFILTER, oldMinFilter);
	device->SetSamplerState(0, D3DSAMP_MAGFILTER, oldMagFilter);
	device->SetSamplerState(0, D3DSAMP_MIPFILTER, oldMipFilter);
	device->SetSamplerState(0, D3DSAMP_ADDRESSU, oldAddressU);
	device->SetSamplerState(0, D3DSAMP_ADDRESSV, oldAddressV);

	device->SetTexture(0, oldTexture0);
	if (oldTexture0 != nullptr) oldTexture0->Release();
}
