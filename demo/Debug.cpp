#include "pch.h"
#include "Debug.h"
#include "MapEnemy.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <string>
#include "backends/imgui_impl_dx9.h"
#include "backends/imgui_impl_win32.h"

bool Demo::Pointable::isDrawing = false;
std::shared_ptr<DX9GF::Font> Demo::Pointable::font = nullptr;
std::shared_ptr<DX9GF::FontSprite> Demo::Pointable::fontSprite = nullptr;

void Demo::CreateImGuiDebugFrame(std::shared_ptr<Player> player)
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
#ifdef _DEBUG
	ImGui::NewFrame();
	ImGui::Begin("Debug Info");
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	auto [playerCurrentX, playerCurrentY] = player->GetLocalPosition();
	ImGui::Text("Player Position: (%.1f, %.1f)", playerCurrentX, playerCurrentY);
	ImGui::End();
	ImGui::Begin("Cheats");
	ImGui::Text("Set Player Position");
	static float playerX = playerCurrentX, playerY = playerCurrentY;
	ImGui::InputFloat("X", &playerX);
	ImGui::InputFloat("Y", &playerY);
	if (ImGui::Button("Set Position")) {
		player->SetLocalPosition(playerX, playerY);
	}
	ImGui::Text("Set Player Velocity");
	static float playerVelocity = player->GetVelocity();
	ImGui::InputFloat("Velocity", &playerVelocity);
	if (ImGui::Button("Set Velocity")) {
		player->SetVelocity(playerVelocity);
	}
	ImGui::Text("Set Player Health");
	static float playerHealth = player->GetHealth();
	ImGui::InputFloat("Health", &playerHealth);
	if (ImGui::Button("Set Health")) {
		player->SetHealth(playerHealth);
	}
	ImGui::Text("Set Player Gold");
	static int playerGold = player->GetGold();
	ImGui::InputInt("Gold", &playerGold);
	if (ImGui::Button("Set Gold")) {
		player->SetGold(playerGold);
	}
	ImGui::Text("Give Player Card");
	static std::string cardToGive = "StrikeCard";
	ImGui::InputText("Card ID", &cardToGive);
	if (ImGui::Button("Give Card")) {
		player->AddCardToDeck(cardToGive);
	}
	ImGui::Text("Give Player Item");
	static int itemToGive = 0;
	ImGui::InputInt("Item ID", &itemToGive);
	static int itemQuantity = 1;
	ImGui::InputInt("Quantity", &itemQuantity);
	if (ImGui::Button("Give Item")) {
		player->GetInventoryItems().AddItem(itemToGive, itemQuantity);
	}
	ImGui::Text("Toggleables");
	if (ImGui::Button("Enable All")) {
		MapEnemy::isDisabled = true;
		player->ignoreCollisions = true;
		Demo::Pointable::isDrawing = true;
	}
	if (ImGui::Button("Disable All")) {
		MapEnemy::isDisabled = false;
		player->ignoreCollisions = false;
		Demo::Pointable::isDrawing = false;
	}
	ImGui::Checkbox("Enemy AI disabled", &MapEnemy::isDisabled);
	ImGui::Checkbox("Ignore Collision", &player->ignoreCollisions);
	ImGui::Checkbox("Draw World Position", &Demo::Pointable::isDrawing);
	ImGui::End();
#endif
	ImGui::EndFrame();
}

void Demo::Pointable::DrawPosition(unsigned long long deltaTime, DX9GF::GraphicsDevice* gd, const DX9GF::Camera& camera) const
{
	if (!isDrawing) return;
	if (!font) {
		font = std::make_shared<DX9GF::Font>(gd, L"StatusPlz", 16);
	}
	if (!fontSprite) {
		fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
	}
	auto [worldX, worldY] = GetPoint();
	fontSprite->SetPosition(worldX, worldY);
	fontSprite->SetText(std::format(L"({:.1f}, {:.1f})", worldX, worldY));
	fontSprite->SetScale(0.5f, 0.5f);
	fontSprite->SetOutline(true, 0xFF000000, 1.0f);
	fontSprite->Begin();
	fontSprite->Draw(camera, deltaTime);
	fontSprite->End();
}
