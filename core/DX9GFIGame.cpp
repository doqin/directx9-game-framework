#include "pch.h"
#include "DX9GFIGame.h"
#include "DX9GFSceneManager.h"
#include "DX9GFGraphicsDevice.h"
#include "DX9GFTexture.h"
#include "DX9GFSprites.h"
#include <stdexcept>

DX9GF::IGame::~IGame()
{
}

HWND DX9GF::IGame::GetHwnd() const
{
	return hwnd;
}

DX9GF::GraphicsDevice* DX9GF::IGame::GetGraphicsDevice()
{
	return graphicsDevice;
}

DX9GF::SceneManager* DX9GF::IGame::GetSceneManager()
{
	return sceneManager;
}

void DX9GF::IGame::Update(unsigned long long deltaTime)
{
	sceneManager->Update(deltaTime);
}

void DX9GF::IGame::Draw(unsigned long long deltaTime)
{
	if (pendingDeviceReset) {
		if (!TryResetDevice(pendingWidth, pendingHeight)) {
			return;
		}
	}

	auto deviceState = graphicsDevice->IsValid();
	if (deviceState == D3DERR_DEVICELOST) {
		return;
	}
	if (deviceState == D3DERR_DEVICENOTRESET) {
		if (!TryResetDevice(pendingWidth, pendingHeight)) {
			return;
		}
	}

	// ==========================================
	// BƯỚC 1: TRỎ LUỒNG VẼ VÀO TEXTURE ẢO
	// ==========================================
	graphicsDevice->SetRenderTarget(renderTargetTex.get());

	// ÉP VIEWPORT VỀ KÍCH THƯỚC ẢO (960x720)
	graphicsDevice->SetViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f);

	sceneManager->Draw(deltaTime);

	// ==========================================
	// BƯỚC 2: VẼ TEXTURE ĐÓ RA MÀN HÌNH THẬT (LETTERBOXING)
	// ==========================================
	graphicsDevice->RestoreRenderTarget();

	// TRẢ VIEWPORT VỀ LẠI KÍCH THƯỚC VẬT LÝ CỦA MÀN HÌNH THẬT
	graphicsDevice->SetViewport(0, 0, d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, 0.0f, 1.0f);

	graphicsDevice->Clear(0xFF000000);

	if (SUCCEEDED(graphicsDevice->BeginDraw())) {
		float currentWidth = static_cast<float>(d3dpp.BackBufferWidth);
		float currentHeight = static_cast<float>(d3dpp.BackBufferHeight);

		// Tính toán tỷ lệ duy trì khung hình gốc 4:3
		float scaleX = currentWidth / static_cast<float>(SCREEN_WIDTH);
		float scaleY = currentHeight / static_cast<float>(SCREEN_HEIGHT);
		float scale = (std::min)(scaleX, scaleY);

		float finalW = SCREEN_WIDTH * scale;
		float finalH = SCREEN_HEIGHT * scale;

		// Căn giữa màn hình (Tạo viền đen)
		float offsetX = (currentWidth - finalW) / 2.0f;
		float offsetY = (currentHeight - finalH) / 2.0f;

		// --- BÍ QUYẾT TRIỆT TIÊU ĐỘ LỆCH TÂM CỦA CAMERA ---
		// Bằng cách set Position của camera đúng bằng một nửa màn hình, 
		// phép tịnh tiến -Position sẽ triệt tiêu hoàn toàn phép tịnh tiến +ScreenCenter
		defaultCamera.SetScreenResolution(static_cast<int>(currentWidth), static_cast<int>(currentHeight));
		defaultCamera.SetPosition(currentWidth / 2.0f, currentHeight / 2.0f);

		renderTargetSprite->SetScale(scale, scale);
		renderTargetSprite->SetPosition(offsetX, offsetY);

		renderTargetSprite->Begin();
		renderTargetSprite->Draw(defaultCamera, deltaTime);
		renderTargetSprite->End();

		graphicsDevice->EndDraw();
	}

	// ==========================================
	// BƯỚC 3: CHỐT HẠ KHUNG HÌNH (RẤT QUAN TRỌNG)
	// ==========================================
	graphicsDevice->Present();
}

void DX9GF::IGame::Init()
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d == NULL) throw std::runtime_error("Error initializing Direct3D");

	ZeroMemory(&d3dpp, sizeof(d3dpp)); // Xóa mọi thứ về 0 trước khi sử dụng

	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferWidth = SCREEN_WIDTH;
	d3dpp.BackBufferHeight = SCREEN_HEIGHT;
	d3dpp.hDeviceWindow = hwnd;

	graphicsDevice = new GraphicsDevice();
	sceneManager = new SceneManager();
	// Create Direct3D device
	d3d->CreateDevice(
		D3DADAPTER_DEFAULT, // Dùng card màn hình mặc định
		D3DDEVTYPE_HAL, // Vẽ bằng phần cứng (bằng card màn hình thay vì giả lập)
		hwnd, // Cửa sổ ứng dụng
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, // Các tham số thể hiện của thiết bị
		&graphicsDevice->GetDevice() // đối tượng dev được tạo ra
	);

	if (graphicsDevice->GetDevice() == NULL) throw std::runtime_error("Error creating Direct3D device");

	// create pointer to the back buffer
	graphicsDevice->GetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &graphicsDevice->GetBackBuffer());

	// --- BỔ SUNG LETTERBOXING: KHỞI TẠO MÀN HÌNH ẢO ---
	renderTargetTex = std::make_shared<DX9GF::Texture>(graphicsDevice);
	renderTargetTex->CreateRenderTarget(SCREEN_WIDTH, SCREEN_HEIGHT); // Luôn là kích thước chuẩn 960x720
	renderTargetSprite = std::make_shared<DX9GF::StaticSprite>(renderTargetTex.get());
}

void DX9GF::IGame::OnResize(UINT width, UINT height)
{
	if (graphicsDevice == nullptr || graphicsDevice->GetDevice() == nullptr) return;
	if (width == 0 || height == 0) return;

	//sceneManager->OnResize(width, height);

	pendingWidth = width;
	pendingHeight = height;

	if (!TryResetDevice(width, height)) {
		pendingDeviceReset = true;
	}
}

bool DX9GF::IGame::TryResetDevice(UINT width, UINT height)
{
	if (graphicsDevice == nullptr || graphicsDevice->GetDevice() == nullptr) return false;
	if (width == 0 || height == 0) return false;

	if (graphicsDevice->GetBackBuffer() != nullptr) {
		graphicsDevice->GetBackBuffer()->Release();
		graphicsDevice->GetBackBuffer() = nullptr;
	}

	// --- CHỐNG CRASH: Xóa Render Target khỏi VRAM trước khi Reset ---
	if (renderTargetTex != nullptr) {
		renderTargetTex->ReleaseRawTexture();
	}

	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;

	HRESULT resetResult = graphicsDevice->GetDevice()->Reset(&d3dpp);
	if (FAILED(resetResult)) {
		pendingDeviceReset = true;
		return false;
	}

	graphicsDevice->GetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &graphicsDevice->GetBackBuffer());
	graphicsDevice->SetViewport(0, 0, width, height, 0.0f, 1.0f);

	// --- SAU KHI RESET THÀNH CÔNG: Phục hồi lại Render Target ---
	if (renderTargetTex != nullptr) {
		renderTargetTex->CreateRenderTarget(SCREEN_WIDTH, SCREEN_HEIGHT);
	}

	pendingDeviceReset = false;
	return true;
}

void DX9GF::IGame::Dispose()
{
	if (d3d != nullptr) d3d->Release();
	if (graphicsDevice != nullptr) delete graphicsDevice;
	if (sceneManager != nullptr) delete sceneManager;
}
