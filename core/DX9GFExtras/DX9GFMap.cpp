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
	auto* device = graphicsDevice != nullptr ? graphicsDevice->GetDevice() : nullptr;
	if (device == nullptr || layers.empty()) return;

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
	MapLayer::ViewBounds viewBounds = {
		(std::min)({ corner00X, corner10X, corner01X, corner11X }),
		(std::min)({ corner00Y, corner10Y, corner01Y, corner11Y }),
		(std::max)({ corner00X, corner10X, corner01X, corner11X }),
		(std::max)({ corner00Y, corner10Y, corner01Y, corner11Y })
	};

	// D3D9 offsets pixel centers from texel centers by half a pixel
	// ("Directly Mapping Texels to Pixels"). Shift the view by -0.5 so
	// point-sampled UVs land inside texels instead of on their boundaries.
	D3DXMATRIX matView;
	D3DXMatrixTranslation(&matView, -0.5f, -0.5f, 0.0f);
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

	device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	for (auto& layer : layers) {
		layer->Draw(camera, viewBounds);
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
