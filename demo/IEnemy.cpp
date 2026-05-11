#include "pch.h"
#include "IEnemy.h"
#include <cmath>
#include <algorithm>

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
    for (auto& indicator : damageIndicators) {
        indicator.elapsed += deltaTime;
        indicator.vy += 800.f * deltaTime / 1000.f; // gravity
        indicator.offsetX += indicator.vx * deltaTime / 1000.f;
        indicator.offsetY += indicator.vy * deltaTime / 1000.f;
    }
    damageIndicators.erase(std::remove_if(damageIndicators.begin(), damageIndicators.end(), [](const DamageIndicator& indicator) {
        return indicator.elapsed >= 700;
    }), damageIndicators.end());
    // Clear out dead projectiles 
    for (size_t i = 0; i < projectiles.size(); i++) {
        if (projectiles[i]->GetState() == DX9GF::IGameObject::State::Destroyed) {
            projectiles.erase(projectiles.begin() + i);
            --i;
        }
    }
    for (auto& projectile : projectiles) {
        projectile->Update(deltaTime);
    }
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
    if (cardSpawnTrigger) {
        cardSpawnTrigger->Draw(graphicsDevice, *camera);
    }

    const auto currentHealth = static_cast<int>(std::round(health));
    const auto totalHealth = static_cast<int>(std::round(maxHealth));
    auto healthText = std::to_wstring(currentHealth) + L"/" + std::to_wstring(totalHealth);

    fontSprite->Begin();
	fontSprite->SetOutline(false);
    fontSprite->SetColor(0xFF000000);
    fontSprite->SetPosition(GetWorldX(), GetWorldY() - 48.f);
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

    float statusOffsetX = 30.f;
    float statusOffsetY = -20.f;

    for (const auto& status : activeStatuses) {
        std::wstring statusName = L"Unknown";
        if (status.type == StatusType::POISON) statusName = L"Poison";
        else if (status.type == StatusType::VULNERABLE) statusName = L"Vulnerable";
        else if (status.type == StatusType::WEAK) statusName = L"Weak";
        else if (status.type == StatusType::STUN) statusName = L"Stun";

        std::wstring statusText = statusName + L" (" + std::to_wstring(status.duration) + L")";

        fontSprite->SetColor(0xFFFFFF00);
		fontSprite->SetOutline(true, 0xFF000000, 2.f);
        fontSprite->SetPosition(GetWorldX() + statusOffsetX, GetWorldY() + statusOffsetY);

        fontSprite->SetText(std::move(statusText));
        fontSprite->Draw(*camera, deltaTime);

        statusOffsetY += 16.f;
    }

    fontSprite->End();
    for (auto& projectile : projectiles) {
        projectile->Draw(*camera, deltaTime);
    }
}

bool Demo::IEnemy::TakeDamage(float damage)
{
    float finalDamage = damage;

    if (HasStatus(StatusType::VULNERABLE)) {
        finalDamage *= 1.5f;
    }
    health -= finalDamage;
    if (health < 0) health = 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> vxDis(-64.f, 64.f);
    std::uniform_real_distribution<float> vyDis(-200.f, -100.f);

    damageIndicators.push_back(DamageIndicator{
        L"-" + std::to_wstring(static_cast<int>(std::round(finalDamage))),
        0.f,
        0.f,
        vxDis(gen),
        vyDis(gen),
        0
    });
	if (!hitImpactTexture && graphicsDevice) { // graphicsDevice should be not null by now
		hitImpactTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		hitImpactTexture->LoadTexture(L"hitimpact-Sheet.png");
	}
	hitImpactSprites.push_back(std::make_shared<DX9GF::AnimatedSprite>(hitImpactTexture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 32, 32, 4), 24, false));
	std::uniform_real_distribution<float> dis(-16.f, 16.f);
	std::uniform_real_distribution<float> scaleDis(2.f, 3.f);
	std::uniform_real_distribution<float> rotDis(-0.5f, 0.5f);
	hitImpactSprites.back()->SetPosition(GetWorldX() + dis(gen), GetWorldY() + dis(gen));
	hitImpactSprites.back()->SetScale(scaleDis(gen));
	hitImpactSprites.back()->SetRotation(rotDis(gen));

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
    return !commandBuffer.IsBusy() && !animationBuffer.IsBusy() && projectiles.empty() && hitImpactSprites.empty();
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
    float finalDamage = baseDamage;

    if (HasStatus(StatusType::WEAK)) {
        finalDamage *= 0.75f;
    }

    return finalDamage;
}