#include "pch.h"
#include "IEnemy.h"
#include <cmath>
#include <algorithm>
#include "DrawUtils.h"
#include "RNG.h"
#include "DX9GFInputManager.h"
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

bool Demo::IEnemy::IsDead() const
{
    return health < 1;
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
        fontSprite->SetColor(0xFFFF4444);
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

        for (const auto& status : activeStatuses) {
            std::wstring statusName = L"Unknown";
            std::wstring statusDescription = L"";
            RECT sourceRect = {0, 0, 0, 0};
            if (status.type == StatusType::POISON) {
                statusName = L"Poison";
                statusDescription = L"Takes damage equal to remaining turns at end of turn.";
                sourceRect = {128, 240, 144, 256};
            }
            else if (status.type == StatusType::VULNERABLE) {
                statusName = L"Vulnerable";
                statusDescription = L"Takes 50% more damage from attacks.";
                sourceRect = {96, 256, 112, 272};
            }
            else if (status.type == StatusType::WEAK) {
                statusName = L"Weak";
                statusDescription = L"Deals 25% less damage with attacks.";
                sourceRect = {112, 256, 128, 272};
            }
            else if (status.type == StatusType::STUN) {
                statusName = L"Stun";
                statusDescription = L"Cannot take action this turn.";
            }

            float iconX = GetWorldX() + statusOffsetX;
            float iconY = GetWorldY() + statusOffsetY;

            if (sourceRect.right > 0) {
                uiSprite->SetSrcRect(sourceRect);
                uiSprite->SetPosition(iconX, iconY);
                uiSprite->Begin();
                uiSprite->Draw(*camera, deltaTime);
                uiSprite->End();
                
                std::wstring durationText = std::to_wstring(status.duration);
                fontSprite->SetColor(0xFFfffc40);
                fontSprite->SetOutline(true, 0xFF000000, 1.f);
                fontSprite->SetPosition(iconX + 36.f, iconY);
                fontSprite->SetText(std::wstring(durationText));
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
            } else {
                std::wstring statusText = statusName + L" (" + std::to_wstring(status.duration) + L")";
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
    float finalDamage = damage;

    if (HasStatus(StatusType::VULNERABLE)) {
        finalDamage *= 1.5f;
    }
    health -= finalDamage;
    if (health < 0) health = 0;
    if (finalDamage > 0) {
        DX9GF::AudioManager::GetInstance()->PlayRandom("take_dmg", 0.8f);
    }    
    damageIndicators.push_back(DamageIndicator{
        L"-" + std::to_wstring(static_cast<int>(std::round(finalDamage))),
        0.f,
        0.f,
        RNG::Range(-64.f, 64.f),
        RNG::Range(-200.f, -100.f),
        0
    });
	if (!hitImpactTexture && graphicsDevice) { // graphicsDevice should be not null by now
		hitImpactTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		hitImpactTexture->LoadTexture(L"assets/hitimpact-Sheet.png");
	}
	hitImpactSprites.push_back(std::make_shared<DX9GF::AnimatedSprite>(hitImpactTexture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 32, 32, 4), 24, false));

	hitImpactSprites.back()->SetPosition(GetWorldX() + RNG::Range(-16.f, 16.f), GetWorldY() + RNG::Range(-16.f, 16.f));
	hitImpactSprites.back()->SetScale(RNG::Range(2.f, 3.f));
	hitImpactSprites.back()->SetRotation(RNG::Range(-0.5f, 0.5f));

	// Queue shake animation
	float ox = GetWorldX();
	float oy = GetWorldY();

	// Only queue if animationBuffer is not busy to prevent drifting from rapid hits
	if (!animationBuffer.IsBusy()) {
		animationBuffer.PushCommand(std::make_shared<DX9GF::GoToCommand>(shared_from_this(), ox - 8.f, oy - 6.f, 0.05f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}));
		animationBuffer.PushCommand(std::make_shared<DX9GF::GoToCommand>(shared_from_this(), ox + 8.f, oy + 6.f, 0.1f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}));
		animationBuffer.PushCommand(std::make_shared<DX9GF::GoToCommand>(shared_from_this(), ox - 4.f, oy - 4.f, 0.05f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}));
		animationBuffer.PushCommand(std::make_shared<DX9GF::GoToCommand>(shared_from_this(), ox + 4.f, oy + 2.f, 0.05f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}));
		animationBuffer.PushCommand(std::make_shared<DX9GF::GoToCommand>(shared_from_this(), ox, oy, 0.05f, DX9GF::TimeTag{}, DX9GF::EaseInOutTag{}));
	}

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

void Demo::IEnemy::ApplyStatus(StatusType type, int duration, float value) {
    for (auto& status : activeStatuses) {
        if (status.type == type) {
            status.duration += duration;
            status.value = std::max(status.value, value);
            return;
        }
    }
    activeStatuses.push_back({ type, duration, value });
}

bool Demo::IEnemy::HasStatus(StatusType type) const {
    for (const auto& status : activeStatuses) {
        if (status.type == type && status.duration > 0) return true;
    }
    return false;
}

void Demo::IEnemy::TickStatuses() {
    for (auto it = activeStatuses.begin(); it != activeStatuses.end(); ) {
        if (it->type == StatusType::POISON && it->duration > 0) {
            const float poisonDamage = (it->value > 0.f) ? it->value : static_cast<float>(it->duration);
            this->TakeDamage(poisonDamage);
        }

        it->duration--;

        if (it->duration <= 0) {
            it = activeStatuses.erase(it);
        }
        else {
            ++it;
        }
    }
}

float Demo::IEnemy::GetOutgoingDamage(float baseDamage) const
{
 float finalDamage = baseDamage * 2.f;

    if (HasStatus(StatusType::WEAK)) {
        finalDamage *= 0.75f;
    }

    return finalDamage;
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