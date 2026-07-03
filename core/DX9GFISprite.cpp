#include "pch.h"
#include "DX9GFISprite.h"

D3DXMATRIX DX9GF::ISprite::GetTransformMatrix()
{
	if (needUpdate) {
		D3DXMATRIX matOffset;
		D3DXMATRIX matScale;
		D3DXMATRIX matRotation;
		D3DXMATRIX matTranslation;
		D3DXMatrixScaling(&matScale, scale.x, scale.y, 1.0f);
		D3DXMatrixRotationZ(&matRotation, rotation);		
		D3DXMatrixTranslation(&matTranslation, pos.x, pos.y, pos.z);
		D3DXMatrixTranslation(&matOffset, -origin.x, -origin.y, 0.0f);
		localTransformMatrix = matOffset * matScale * matRotation * matTranslation;
		needUpdate = false;
	}
	return localTransformMatrix;
}

DX9GF::ISprite::ISprite(GraphicsDevice* graphicsDevice) : graphicsDevice(graphicsDevice) {
	D3DXMatrixIdentity(&localTransformMatrix);
}

void DX9GF::ISprite::Translate(
	float x,
	float y
)
{
	pos.x += x;
	pos.y += y;
	needUpdate = true;
}

void DX9GF::ISprite::SetColor(D3DCOLOR color)
{
	this->color = color;
}

D3DCOLOR DX9GF::ISprite::GetColor() const
{
	return color;
}

void DX9GF::ISprite::SetPosition(float x, float y)
{
	pos.x = x;
	pos.y = y;
	needUpdate = true;
}

void DX9GF::ISprite::SetPosition(float x, float y, float z)
{
	pos.x = x;
	pos.y = y;
	pos.z = z;
	needUpdate = true;
}

void DX9GF::ISprite::SetPositionX(float x)
{
	pos.x = x;
	needUpdate = true;
}

void DX9GF::ISprite::SetPositionY(float y)
{
	pos.y = y;
	needUpdate = true;
}

void DX9GF::ISprite::SetPositionZ(float z)
{
	pos.z = z;
	needUpdate = true;
}

float DX9GF::ISprite::GetPositionX() const
{
	return pos.x;
}

float DX9GF::ISprite::GetPositionY() const
{
	return pos.y;
}

float DX9GF::ISprite::GetPositionZ() const
{
	return pos.z;
}

D3DXVECTOR3 DX9GF::ISprite::GetPosition() const
{
	return pos;
}

//scale
void DX9GF::ISprite::SetScale(float s)
{
	scale.x = s;
	scale.y = s;
	needUpdate = true;
}

void DX9GF::ISprite::SetScale(float sx, float sy)
{
	scale.x = sx;
	scale.y = sy;
	needUpdate = true;
}

void DX9GF::ISprite::SetScaleX(float sx)
{
	scale.x = sx;
	needUpdate = true;
}

void DX9GF::ISprite::SetScaleY(float sy)
{
	scale.y = sy;
	needUpdate = true;
}

D3DXVECTOR2 DX9GF::ISprite::GetScale() const
{
	return scale;
}

//rotate
void DX9GF::ISprite::SetRotation(float radians)
{
	rotation = radians;
	needUpdate = true;
}

void DX9GF::ISprite::Rotate(float radians)
{
	rotation += radians;
	needUpdate = true;
}

float DX9GF::ISprite::GetRotation() const
{
	return rotation;
}
//flip
void DX9GF::ISprite::FlipHorizontal()
{
	scale.x *= -1;
	needUpdate = true;
}

void DX9GF::ISprite::FlipVertical()
{
	scale.y *= -1;
	needUpdate = true;
}
//origin
void DX9GF::ISprite::SetOrigin(float x, float y)
{
	origin.x = x;
	origin.y = y;
	needUpdate = true;
}

DX9GF::GraphicsDevice* DX9GF::ISprite::GetGraphicsDevice()
{
	return graphicsDevice;
}