#include "pch.h"
#include "IEnemy.h"
#include <cmath>
#include <algorithm>
#include "DrawUtils.h"
#include "RNG.h"
#include "DX9GFInputManager.h"
#include "PopUpMessage.h"
void Demo::IEnemy::InitCardSpawnTrigger(DX9GF::Camera* camera, float width, float height)
{
	cardSpawnTrigger = std::make_shared<DX9GF::RectangleTrigger>(transformManager, shared_from_this(), width, height);
	cardSpawnTrigger->SetOriginCenter();
	cardSpawnTrigger->Init(camera);
	cardSpawnTrigger->SetOnClickLeft([this](DX9GF::ITrigger* trigger) {
		onRequestEnemyCard(std::dynamic_pointer_cast<IEnemy>(shared_from_this()));
		});
}

void Demo::IEnemy::SetOnRequestEnemyCard(std::function<void(std::shared_ptr<IEnemy>)> callback)
{
	onRequestEnemyCard = callback;
}

void Demo::IEnemy::Update(unsigned long long deltaTime)
{
	if (!isOnStandby) {
		if (cardSpawnTrigger) {
			cardSpawnTrigger->Update(deltaTime);
		}
	}

	timeSinceStart += deltaTime;

	for (auto& indicator : damageIndicators) {
		indicator.elapsed += deltaTime;
		indicator.vy += 800.f * deltaTime / 1000.f; // gravity
		indicator.offsetX += indicator.vx * deltaTime / 1000.f;
		indicator.offsetY += indicator.vy * deltaTime / 1000.f;
	}
	damageIndicators.erase(std::remove_if(damageIndicators.begin(), damageIndicators.end(), [](const DamageIndicator& indicator) {
		return indicator.elapsed >= 700;
		}), damageIndicators.end());
	projectiles.Update(deltaTime);
	commandBuffer.Update(deltaTime);
	animationBuffer.Update(deltaTime);
}

void Demo::IEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime)
{
	if (!graphicsDevice || !camera) {
		return;
	}
	this->graphicsDevice = graphicsDevice;

    if (!font) {
        font = std::make_shared<DX9GF::Font>(graphicsDevice, L"StatusPlz", 16);
        fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
    }
    if (!uiTexture) {
        uiTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
        uiTexture->LoadTexture(L"assets/ui.png");
        uiSprite = std::make_shared<DX9GF::StaticSprite>(uiTexture.get());
        uiSprite->SetScale(2.0f);
    }

	if (!isOnStandby && cardSpawnTrigger) {
		bool isHovered = cardSpawnTrigger->IsHovering(deltaTime);

		const float triggerLeft = cardSpawnTrigger->GetWorldX() - cardSpawnTrigger->GetOriginX();
		const float triggerTop = cardSpawnTrigger->GetWorldY() - cardSpawnTrigger->GetOriginY();
		const float triggerW = cardSpawnTrigger->GetWidth();
		const float triggerH = cardSpawnTrigger->GetHeight();

		// Blink logic (idle state, not hovered)
		float blinkFreq = 1.0f; // blinks per second
		float alphaMult = (sin(timeSinceStart * 0.001f * blinkFreq * 3.14159f) + 1.0f) * 0.5f; // 0 to 1
		int idleAlpha = (int)(64 * alphaMult); // 64 is ~25% of 255

		graphicsDevice->SetAlphaBlending(true);

		if (isHovered) {
			graphicsDevice->DrawRectangle(
				*camera,
				triggerLeft, triggerTop,
				triggerW, triggerH,
				0, 1, 1, 0, 0, D3DCOLOR_ARGB(80, 255, 240, 120), true);

			Demo::DrawAnimatedDashedRectangle(
				graphicsDevice,
				*camera,
				triggerLeft,
				triggerTop,
				triggerW,
				triggerH,
				3.f,
				0xFFFFE678,
				false,
				4.f,
				0xFFFFE678,
				20.f,
				10.f,
				40.f,
				GetTickCount64()
			);

			{
				const float centerX = triggerLeft + triggerW / 2.f;
				const float centerY = triggerTop + triggerH / 2.f;
				const float plusLength = 40.f;
				const float plusThickness = 10.f;

				const float outlinePad = 3.f;
				graphicsDevice->DrawRectangle(
					*camera,
					centerX - plusLength / 2.f - outlinePad, centerY - plusThickness / 2.f - outlinePad,
					plusLength + outlinePad * 2.f, plusThickness + outlinePad * 2.f,
					0xFF000000, true);
				graphicsDevice->DrawRectangle(
					*camera,
					centerX - plusThickness / 2.f - outlinePad, centerY - plusLength / 2.f - outlinePad,
					plusThickness + outlinePad * 2.f, plusLength + outlinePad * 2.f,
					0xFF000000, true);

				graphicsDevice->DrawRectangle(
					*camera,
					centerX - plusLength / 2.f, centerY - plusThickness / 2.f,
					plusLength, plusThickness,
					0xFFFFE678, true);
				graphicsDevice->DrawRectangle(
					*camera,
					centerX - plusThickness / 2.f, centerY - plusLength / 2.f,
					plusThickness, plusLength,
					0xFFFFE678, true);
			}
		}
		else {
			D3DCOLOR color = D3DCOLOR_ARGB(idleAlpha, 128, 128, 128);
			graphicsDevice->DrawRectangle(
				*camera,
				triggerLeft, triggerTop,
				triggerW, triggerH,
				0, 1, 1, 0, 0, color, true);

			if (fontSprite) {
				fontSprite->Begin();
				fontSprite->SetOutline(true, 0xFF000000, 2.f);
				fontSprite->SetColor(D3DCOLOR_ARGB((std::max)(idleAlpha, 140), 255, 230, 120));
				std::wstring hintText = L"Click to get enemy card";
				fontSprite->SetText(std::move(hintText));
				float hintWidth = fontSprite->GetWidth();
				fontSprite->SetPosition(GetWorldX() - hintWidth / 2.f, triggerTop - 24.f);
				fontSprite->Draw(*camera, deltaTime);
				fontSprite->SetOutline(false);
				fontSprite->End();
			}
		}

		graphicsDevice->SetAlphaBlending(false);
	}

	if (cardSpawnTrigger) {
		cardSpawnTrigger->Draw(graphicsDevice, *camera);
	}

	const auto currentHealth = static_cast<int>(std::round(health));
	const auto totalHealth = static_cast<int>(std::round(maxHealth));
	auto healthText = std::to_wstring(currentHealth) + L"/" + std::to_wstring(totalHealth);

	fontSprite->Begin();
	fontSprite->SetOutline(true, 0xFF000000, 2.f);
	fontSprite->SetColor(0xFFFFFFFF);
	fontSprite->SetPosition(GetWorldX(), GetWorldY() - 120.f);
	fontSprite->SetText(std::move(healthText));
	fontSprite->Draw(*camera, deltaTime);

	for (auto it = hitImpactSprites.begin(); it != hitImpactSprites.end(); ) {
		auto& sprite = *it;
		sprite->Begin();
		sprite->Draw(*camera, deltaTime);
		sprite->End();
		if (sprite->IsFinished()) {
			it = hitImpactSprites.erase(it);
		}
		else {
			++it;
		}
	}

	for (size_t i = 0; i < damageIndicators.size(); ++i) {
		auto text = damageIndicators[i].text;
		fontSprite->SetColor(damageIndicators[i].textColor);
		fontSprite->SetOutline(true, 0xFF000000, 2.f);
		fontSprite->SetBold(true);
		fontSprite->SetScale(2.f);
		fontSprite->SetPosition(GetWorldX() + damageIndicators[i].offsetX, GetWorldY() - 30.f + damageIndicators[i].offsetY);
		fontSprite->SetText(std::move(text));
		fontSprite->Draw(*camera, deltaTime);
	}
	fontSprite->SetBold(false);
	fontSprite->SetOutline(false);
	fontSprite->SetScale(1.f);

	float statusOffsetX = 64.f;
	float statusOffsetY = -20.f;

	if (!isOnStandby) {
		auto [screenX, screenY] = DX9GF::InputManager::GetInstance()->GetVirtualAbsoluteMousePos(camera);
		auto [mouseX, mouseY] = DX9GF::Utils::WindowToWorldCoords(*camera, screenX, screenY);

		for (const auto& mod : modifiers) {
			if (mod.type == ModifierType::BuffDefense && mod.value <= 0.f) continue;
			if (mod.duration <= 0) continue;

			std::wstring statusName = L"Unknown";
			std::wstring statusDescription = L"";
			RECT sourceRect = { 0, 0, 0, 0 };

			if (mod.type == ModifierType::Poison) {
				statusName = L"Poison";
				statusDescription = L"Takes damage equal to remaining turns at end of turn.";
				sourceRect = { 128, 240, 144, 256 };
			}
			else if (mod.type == ModifierType::Burn) {
				statusName = L"Burn";
				statusDescription = L"Takes damage equal to its value at end of turn, ignoring block.";
				sourceRect = { 240, 288, 256, 304 };
			}
			else if (mod.type == ModifierType::Regen) {
				statusName = L"Regen";
				statusDescription = L"Heals its value at end of turn.";
				sourceRect = { 272, 288, 288, 304 };
			}
			else if (mod.type == ModifierType::Marked) {
				statusName = L"Marked";
				statusDescription = L"Takes extra damage from every hit.";
				sourceRect = { 128, 256, 144, 272 };
			}
			else if (mod.type == ModifierType::Vulnerable) {
				statusName = L"Vulnerable";
				statusDescription = L"Takes 50% more damage from attacks.";
				sourceRect = { 96, 256, 112, 272 };
			}
			else if (mod.type == ModifierType::Weak) {
				statusName = L"Weak";
				statusDescription = L"Deals 25% less damage with attacks.";
				sourceRect = { 112, 256, 128, 272 };
			}
			else if (mod.type == ModifierType::Stun) {
				statusName = L"Stun";
				statusDescription = L"Cannot take action this turn.";
				sourceRect = { 224, 288, 240, 304 };
			}
			else if (mod.type == ModifierType::BuffDamage) {
				statusName = L"Atk Up";
				statusDescription = L"Increases attack damage.";
				sourceRect = { 112, 240, 128, 256 };
			}
			else if (mod.type == ModifierType::BuffDefense) {
				statusName = L"Def Up";
				statusDescription = L"Blocks incoming damage.";
				sourceRect = { 96, 240, 112, 256 };
			}

			float iconX = GetWorldX() + statusOffsetX;
			float iconY = GetWorldY() + statusOffsetY;

			std::wstring displayValueText = L"";
			if (mod.type == ModifierType::BuffDefense || mod.type == ModifierType::BuffDamage || mod.type == ModifierType::Poison
				|| mod.type == ModifierType::Burn || mod.type == ModifierType::Marked || mod.type == ModifierType::Regen) {
				int displayValue = (mod.type == ModifierType::Poison) ?
					static_cast<int>(std::round((mod.value > 0.f) ? mod.value : static_cast<float>(mod.duration))) :
					static_cast<int>(std::round(mod.value));

				if (mod.type == ModifierType::BuffDamage) displayValueText = L" +" + std::to_wstring(displayValue);
				else displayValueText = L" " + std::to_wstring(displayValue);
			}

			if (sourceRect.right > 0) {
				uiSprite->SetSrcRect(sourceRect);
				uiSprite->SetPosition(iconX, iconY);
				uiSprite->Begin();
				uiSprite->Draw(*camera, deltaTime);
				uiSprite->End();

				std::wstring durationText = std::to_wstring(mod.duration);
				fontSprite->SetColor(0xFFfffc40);
				fontSprite->SetOutline(true, 0xFF000000, 1.f);
				fontSprite->SetPosition(iconX + 36.f, iconY);
				fontSprite->SetText(durationText + displayValueText);
				fontSprite->Draw(*camera, deltaTime);

				bool isHovered = mouseX >= iconX && mouseX <= iconX + 32 && mouseY >= iconY && mouseY <= iconY + 32;
				if (isHovered) {
					std::wstring tooltipText = statusName + L" (" + durationText + L" turns remaining)\n" + statusDescription;
					fontSprite->SetText(std::move(tooltipText));
					float tooltipWidth = fontSprite->GetWidth() + 8.f;
					float tooltipHeight = fontSprite->GetHeight() + 8.f;

					auto [screenW, screenH] = camera->GetScreenResolution();
					float targetScreenX = screenX;
					float targetScreenY = screenY - tooltipHeight;

					if (targetScreenX + tooltipWidth > screenW) {
						targetScreenX = screenW - tooltipWidth;
					}
					if (targetScreenY < 0) {
						targetScreenY = screenY + 32.f;
					}

					auto [worldDrawX, worldDrawY] = DX9GF::Utils::WindowToWorldCoords(*camera, targetScreenX, targetScreenY);

					fontSprite->End();
					graphicsDevice->SetAlphaBlending(true);
					graphicsDevice->DrawRectangle(*camera, worldDrawX, worldDrawY, tooltipWidth, tooltipHeight, 0, 1, 1, 0, 0, D3DCOLOR_ARGB(220, 0, 0, 0), true);
					fontSprite->Begin();

					fontSprite->SetColor(0xFFFFFFFF);
					fontSprite->SetOutline(true, 0xFF000000, 1.f);
					fontSprite->SetPosition(worldDrawX + 4.f, worldDrawY + 4.f);
					fontSprite->Draw(*camera, deltaTime);
				}
			}
			else {
				std::wstring statusText = statusName + displayValueText + L" (" + std::to_wstring(mod.duration) + L")";
				fontSprite->SetColor(0xFFfffc40);
				fontSprite->SetOutline(true, 0xFF000000, 2.f);
				fontSprite->SetPosition(iconX, iconY);
				fontSprite->SetText(std::move(statusText));
				fontSprite->Draw(*camera, deltaTime);
			}

			statusOffsetY += 36.f;
        }
    }

	fontSprite->End();
	projectiles.Draw(graphicsDevice, *camera, deltaTime);
}

bool Demo::IEnemy::TakeDamage(float damage)
{
	float actualDamage = CalculateActualDamage(damage);
	health -= actualDamage;
	if (health < 0) health = 0;

	if (actualDamage > 0) DX9GF::AudioManager::GetInstance()->PlayRandom("take_dmg", 0.8f);

	damageIndicators.push_back(DamageIndicator{
		L"-" + std::to_wstring(static_cast<int>(std::round(actualDamage))),
		0.f,
		0.f,
		RNG::Range(-64.f, 64.f),
		RNG::Range(-200.f, -100.f),
		0,
		0xFFFF4444
		});

	if (!hitImpactTexture && graphicsDevice) {
		hitImpactTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		hitImpactTexture->LoadTexture(L"assets/hitimpact-Sheet.png");
	}
	hitImpactSprites.push_back(std::make_shared<DX9GF::AnimatedSprite>(hitImpactTexture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 32, 32, 4), 24, false));

	hitImpactSprites.back()->SetPosition(GetWorldX() + RNG::Range(-16.f, 16.f), GetWorldY() + RNG::Range(-16.f, 16.f));
	hitImpactSprites.back()->SetScale(RNG::Range(2.f, 3.f));
	hitImpactSprites.back()->SetRotation(RNG::Range(-0.5f, 0.5f));

	float ox = GetWorldX();
	float oy = GetWorldY();

	if (!animationBuffer.IsBusy()) {
		animationBuffer.PushCommand(std::make_shared<DX9GF::GoToCommand>(shared_from_this(), ox - 8.f, oy - 6.f, 0.05f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}));
		animationBuffer.PushCommand(std::make_shared<DX9GF::GoToCommand>(shared_from_this(), ox + 8.f, oy + 6.f, 0.1f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}));
		animationBuffer.PushCommand(std::make_shared<DX9GF::GoToCommand>(shared_from_this(), ox - 4.f, oy - 4.f, 0.05f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}));
		animationBuffer.PushCommand(std::make_shared<DX9GF::GoToCommand>(shared_from_this(), ox + 4.f, oy + 2.f, 0.05f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}));
		animationBuffer.PushCommand(std::make_shared<DX9GF::GoToCommand>(shared_from_this(), ox, oy, 0.05f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}));
	}

	return IsDead();
}

bool Demo::IEnemy::TakeIndirectDamage(float damage, DamageType type) {
	health -= damage;
	if (health < 0) health = 0;

	D3DCOLOR textColor = 0xFFFFFFFF;

	if (type == DamageType::Poison) {
		textColor = 0xFFba4aed; //purple
		//TODO: find audio resource to replace this. Physical - Effect audio should be diff
		DX9GF::AudioManager::GetInstance()->PlayRandom("take_dmg", 0.5f);
	}
	else if (type == DamageType::Burn) {
		textColor = 0xFFff8800; //orange
		DX9GF::AudioManager::GetInstance()->PlayRandom("take_dmg", 0.5f);
	}

	float ox = GetWorldX();
	float oy = GetWorldY();
	if (!animationBuffer.IsBusy()) {
		animationBuffer.PushCommand(std::make_shared<DX9GF::GoToCommand>(shared_from_this(), ox - 2.f, oy, 0.05f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}));
		animationBuffer.PushCommand(std::make_shared<DX9GF::GoToCommand>(shared_from_this(), ox + 2.f, oy, 0.05f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}));
		animationBuffer.PushCommand(std::make_shared<DX9GF::GoToCommand>(shared_from_this(), ox, oy, 0.05f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}));
	}

	damageIndicators.push_back(DamageIndicator{
		L"-" + std::to_wstring(static_cast<int>(std::round(damage))),
		0.f,
		0.f,
		RNG::Range(-32.f, 32.f),
		RNG::Range(-150.f, -100.f),
		0,
		textColor
		});

	return IsDead();
}

void Demo::IEnemy::SetState(bool isOnStandby)
{
	this->isOnStandby = isOnStandby;
}

bool Demo::IEnemy::IsDoneAttacking()
{
	return !commandBuffer.IsBusy() && !animationBuffer.IsBusy() && projectiles.IsEmpty() && hitImpactSprites.empty();
}

int Demo::IEnemy::GetSmartRandomPattern(int minPattern, int maxPattern, int maxStreak, int breakChance)
{
	int patternId = RNG::Range(minPattern, maxPattern);

	if (patternId == lastPattern) {
		streakCount++;
		if (streakCount >= maxStreak) {
			if (RNG::Range(1, 100) <= breakChance) {
				do {
					patternId = RNG::Range(minPattern, maxPattern);
				} while (patternId == lastPattern);
                streakCount = 0; //reset if switched skill
			}
		}
	}
	else {
		streakCount = 0;
	}

	lastPattern = patternId;
	return patternId;
}

void Demo::IEnemy::SpawnHealText(float actualHeal) {
	damageIndicators.push_back(DamageIndicator{
		L"+" + std::to_wstring(static_cast<int>(std::round(actualHeal))),
		0.f,
		0.f,
		RNG::Range(-32.f, 32.f),
		RNG::Range(-150.f, -100.f),
		0,
		0xFF59c135 //green
		});
}

void Demo::IEnemy::CastAbility(std::function<void()> effect, std::shared_ptr<PopUpMessage> popUpMessage, const std::wstring& message) {
	commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([effect](std::function<void(void)> markFinished) {
		if (effect) effect();
		markFinished();
		}));

	if (popUpMessage && !message.empty()) {
		popUpMessage->QueueMessage(&commandBuffer, message, 1.5f);
	}

	commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
}