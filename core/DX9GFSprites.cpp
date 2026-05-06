#include "pch.h"
#include "DX9GFSprites.h"
#include "DX9GFGraphicsDevice.h"
#include <DxErr.h>
#include <d3dx9.h>
#include <stdexcept>

static void ApplyPixelArtSamplerState(IDirect3DDevice9* device)
{
	device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
}

DX9GF::StaticSprite::StaticSprite(Texture* texture) : ISprite(texture->GetGraphicsDevice()), texture(texture)
{
	HRESULT result = D3DXCreateSprite(graphicsDevice->GetDevice(), &p_sprite);
	if (result != D3D_OK) {
		auto error = DXGetErrorDescription(result);
		std::string what = std::string(error, error + wcslen(error));
		throw std::invalid_argument(what);
	}
}

DX9GF::StaticSprite::~StaticSprite()
{
	if (p_sprite != nullptr) p_sprite->Release();
	if (p_src != nullptr) delete p_src;
}

void DX9GF::StaticSprite::Begin()
{
	p_sprite->OnLostDevice();
	p_sprite->OnResetDevice();
	if (auto result = p_sprite->Begin(D3DXSPRITE_ALPHABLEND); FAILED(result)) {
		auto error = DXGetErrorDescription(result);
		std::string what = std::string(error, error + wcslen(error));
		throw std::runtime_error(what);
	}
   ApplyPixelArtSamplerState(graphicsDevice->GetDevice());
}

void DX9GF::StaticSprite::Draw(const Camera& camera, unsigned long long deltaTime)
{
	auto matWorld = GetTransformMatrix();
	auto matCamera = camera.GetTransformMatrix();
	auto matFinal = matWorld * matCamera;
	D3DXVECTOR3 zeroPos(0.0f, 0.0f, 0.0f);
	p_sprite->SetTransform(&matFinal);
	p_sprite->Draw(
		texture->GetRawTexture(),
		p_src,
		nullptr,
		&zeroPos,
		color
	);
}

void DX9GF::StaticSprite::End()
{
	p_sprite->End();
}

void DX9GF::StaticSprite::SetSrcRect(RECT srcRect)
{
	if (this->p_src != nullptr) delete p_src;
	this->p_src = new RECT(srcRect);
}

DX9GF::AnimatedSprite::AnimatedSprite(
	Texture* spritesheet, 
	std::vector<RECT> frames, 
	UINT frameRate, 
	bool isLooped
) : ISprite(spritesheet->GetGraphicsDevice()), 
	frames(frames), 
	frameRate(frameRate), 
	texture(spritesheet), 
	isLooped(isLooped) 
{
	HRESULT result = D3DXCreateSprite(graphicsDevice->GetDevice(), &p_sprite);

	if (result != D3D_OK) {
		auto error = DXGetErrorDescription(result);
		std::string what = std::string(error, error + wcslen(error));
		throw std::runtime_error(what);
	}
}

DX9GF::AnimatedSprite::~AnimatedSprite()
{
	if (p_sprite != nullptr) p_sprite->Release();
}

void DX9GF::AnimatedSprite::SetFrame(unsigned int frameIndex)
{
	this->frame_index = frameIndex;
}

void DX9GF::AnimatedSprite::SetFrameRate(unsigned int frameRate)
{
	this->frameRate = frameRate;
}

void DX9GF::AnimatedSprite::SetLooped(bool isLooped)
{
	this->isLooped = isLooped;
}

bool DX9GF::AnimatedSprite::IsFinished() const
{
	if (isLooped) return false;
	return frame_index >= frames.size() - 1;
}

void DX9GF::AnimatedSprite::Begin()
{
	p_sprite->OnLostDevice();
	p_sprite->OnResetDevice();
	if (auto result = p_sprite->Begin(D3DXSPRITE_ALPHABLEND); FAILED(result)) {
		auto error = DXGetErrorDescription(result);
		std::string what = std::string(error, error + wcslen(error));
		throw std::runtime_error(what);
	}
   ApplyPixelArtSamplerState(graphicsDevice->GetDevice());
}

void DX9GF::AnimatedSprite::Draw(const Camera& camera, unsigned long long deltaTime)
{
	if (frames.size() == 0) throw std::runtime_error("Sprite frames are empty!");
	delta += deltaTime;
	if (delta > 1000 / frameRate) {
		frame_index += delta / (1000 / frameRate); // this will handle skipping more than 1 frame if delta is worth 2+ frame time
		delta = 0;
	}
	if (!isLooped && frame_index >= frames.size()) {
		frame_index = frames.size() - 1;
	}
	frame_index %= frames.size();
	auto p_src = &frames.at(frame_index);
	auto matWorld = GetTransformMatrix();
	auto matCamera = camera.GetTransformMatrix();
	auto matFinal = matWorld * matCamera;
	D3DXVECTOR3 zeroPos(0.0f, 0.0f, 0.0f);
	p_sprite->SetTransform(&matFinal);
	p_sprite->Draw(
		texture->GetRawTexture(),
		p_src,
		nullptr,
		&zeroPos,
		color
	);
}

void DX9GF::AnimatedSprite::End()
{
	p_sprite->End();
}

DX9GF::FontSprite::FontSprite(Font* font) : ISprite(font->GetGraphicsDevice()), font(font)
{
	HRESULT result = D3DXCreateSprite(graphicsDevice->GetDevice(), &p_sprite);
	if (result != D3D_OK) {
		auto error = DXGetErrorDescription(result);
		std::string what = std::string(error, error + wcslen(error));
		throw std::runtime_error(what);
	}
}

DX9GF::FontSprite::~FontSprite()
{
	if (p_src != nullptr) delete p_src;
	if (p_sprite != nullptr) p_sprite->Release();
}

void DX9GF::FontSprite::SetText(std::wstring&& text)
{
	this->text = text;
}

void DX9GF::FontSprite::SetColor(D3DCOLOR color)
{
	this->color = color;
}

void DX9GF::FontSprite::SetBold(bool bold)
{
	this->isBold = bold;
}

void DX9GF::FontSprite::SetOutline(bool outline, D3DCOLOR color, float thickness)
{
	this->isOutline = outline;
	this->outlineColor = color;
	this->outlineThickness = thickness;
}

LONG DX9GF::FontSprite::GetWidth() const
{
	RECT rect{};
	DWORD format = DT_CALCRECT;
	if (p_src != nullptr) {
		rect = *p_src;
		format |= DT_TOP | DT_LEFT;
	}
	else {
		format |= DT_NOCLIP;
	}
	font->GetRawFont()->DrawText(nullptr, text.c_str(), -1, &rect, format, color);
	return rect.right - rect.left;
}

LONG DX9GF::FontSprite::GetHeight() const
{
	RECT rect{};
	DWORD format = DT_CALCRECT;
	if (p_src != nullptr) {
		rect = *p_src;
		format |= DT_TOP | DT_LEFT;
	}
	else {
		format |= DT_NOCLIP;
	}
	font->GetRawFont()->DrawText(nullptr, text.c_str(), -1, &rect, format, color);
	return rect.bottom - rect.top;
}

void DX9GF::FontSprite::Begin()
{
	p_sprite->OnLostDevice();
	p_sprite->OnResetDevice();
	font->GetRawFont()->OnLostDevice();
	font->GetRawFont()->OnResetDevice();
	if (auto result = p_sprite->Begin(D3DXSPRITE_ALPHABLEND); FAILED(result)) {
		auto error = DXGetErrorDescription(result);
		std::string what = std::string(error, error + wcslen(error));
		throw std::runtime_error(what);
	}
   ApplyPixelArtSamplerState(graphicsDevice->GetDevice());
}

void DX9GF::FontSprite::Draw(const Camera& camera, unsigned long long deltaTime)
{
	auto matWorld = GetTransformMatrix();
	auto matCamera = camera.GetTransformMatrix();
	auto matFinal = matWorld * matCamera;
	RECT rect{};
	DWORD format = 0;
	if (p_src == nullptr) {
		rect.left = 0;
		rect.top = 0;
		rect.right = 0;
		rect.bottom = 0;
		format = DT_NOCLIP;
	}
	else {
		rect = *p_src;
		format = DT_TOP | DT_LEFT;
	}

	if (isOutline) {
		for (float x = -outlineThickness; x <= outlineThickness; x += 1.0f) {
			for (float y = -outlineThickness; y <= outlineThickness; y += 1.0f) {
				// Ignore points outside the circle radius for rounded outline
				if (x * x + y * y > outlineThickness * outlineThickness) continue;

				auto matOffsetWorld = matWorld;
				matOffsetWorld._41 += x;
				matOffsetWorld._42 += y;
				auto matOffsetFinal = matOffsetWorld * matCamera;
				p_sprite->SetTransform(&matOffsetFinal);
				font->GetRawFont()->DrawText(p_sprite, text.c_str(), -1, &rect, format, outlineColor);

				if (isBold) {
					matOffsetWorld._41 += 1.0f;
					matOffsetFinal = matOffsetWorld * matCamera;
					p_sprite->SetTransform(&matOffsetFinal);
					font->GetRawFont()->DrawText(p_sprite, text.c_str(), -1, &rect, format, outlineColor);
				}
			}
		}
	}

	p_sprite->SetTransform(&matFinal);
	font->GetRawFont()->DrawText(p_sprite, text.c_str(), -1, &rect, format, color);

	if (isBold) {
		auto matOffsetWorld = matWorld;
		matOffsetWorld._41 += 1.0f; // Offset X by 1
		auto matOffsetFinal = matOffsetWorld * matCamera;
		p_sprite->SetTransform(&matOffsetFinal);
		font->GetRawFont()->DrawText(p_sprite, text.c_str(), -1, &rect, format, color);
	}
}

void DX9GF::FontSprite::End()
{
	p_sprite->End();
}
