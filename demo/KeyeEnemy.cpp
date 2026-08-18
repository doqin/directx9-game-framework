#include "pch.h"
#include "KeyeEnemy.h"
#include "resource.h"
#include "RNG.h"
#include "PopUpMessage.h"

void Demo::KeyeEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"assets/minion-Sheet.png");
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 12);
	sprite->SetOrigin(32, 32);
	sprite->SetScale(2.f);

	projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	projTexture->LoadTexture(L"assets/minionprojectile-Sheet.png");
	projFrames = DX9GF::Utils::CreateRectsHorizontal(0, 0, 32, 32, 5);
	SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
	InitCardSpawnTrigger(camera, 128.f, 128.f);
}

void Demo::KeyeEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) {
	if (sprite) {
		sprite->Begin();
		auto [x, y] = GetWorldPosition();
		sprite->SetPosition(x, y);
		sprite->Draw(*camera, deltaTime);
		sprite->End();
	}
	IEnemy::Draw(graphicsDevice, camera, deltaTime);
}

int Demo::KeyeEnemy::GetRandomPattern() {
	return RNG::Range(1, 2);
}

void Demo::KeyeEnemy::OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) {
	this->player = player; //saved player's ref

	int cycle = (currentTurn - 1) / 3;

	if (currentCycle != cycle) {
		currentCycle = cycle;

		if (RNG::Range(1, 100) <= 50) {
			skillTurnThisCycle = RNG::Range(1, 3);
		}
		else {
			skillTurnThisCycle = -1;
		}
	}

	int turnInCycle = (currentTurn - 1) % 3 + 1;

	if (turnInCycle == skillTurnThisCycle) {
		if (RNG::Range(1, 2) == 1) {
			CastAbility([this]() { this->AddModifier(ModifierType::BuffDamage, 2, 5.0f, true); }, popUpMessage, L"Keye focuses its gaze!");
		}
		else {
			CastAbility([this]() {
				if (auto lock = this->player.lock()) lock->AddModifier(ModifierType::Weak, 2, 0.f, false);
				}, popUpMessage, L"Keye finds your blind spot!");
		}
	}
}

void Demo::KeyeEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) {
	this->player = player;

	float baseDamage = 2.f;
	float finalDamage = CalculateOutgoingDamage(baseDamage);

	int patternId = GetSmartRandomPattern(1, 2);
	if (patternId == 1) PatternRoundCircle(finalDamage);
	else PatternSightline(finalDamage);
}

void Demo::KeyeEnemy::PatternRoundCircle(float projDamage) {
	for (int i = 0; i < 25; i++) {
		float randX = RNG::Range(-120.f, 120.f);
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, randX](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				projectiles.Spawn(
					lock,
					ProjectileDesc(projTexture.get(), projFrames, 12, 16, 16, 16, 16, randX, -220.f)
					.SetTargetPosition(lock->GetCollider().lock()->GetWorldX(), lock->GetCollider().lock()->GetWorldY())
					.SetVelocity(220.f)
					.SetDecayTime(6.f)
					.SetDamage(projDamage)
				);
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
	}
}

// Keye sights down the player and burns a beam along the line it caught them on -
// vertical or horizontal, decided per shot. The beam is laid down where the player
// stands when it locks on, so standing still is what gets punished; the dodge is a
// step off the line, and the shots come close enough together that there is no
// settling back onto it.
void Demo::KeyeEnemy::PatternSightline(float projDamage) {
	// The battle box is 256px square around the origin, so a beam through the middle
	// spans it end to end whichever way it is turned.
	constexpr float BEAM_LENGTH = 256.f;
	// Long enough to read the line and walk off it, short enough that the player cannot
	// wander back before it fires.
	constexpr float WARN_TIME = 0.9f;
	constexpr float FIRE_TIME = 0.5f;
	// Shorter than WARN_TIME + FIRE_TIME, so the next line is already telegraphing while
	// the current one still burns and the player is reading two beams at once.
	constexpr float SHOT_GAP = 1.1f;
	// Keye's own cold blue gaze, so its beams do not read as Kernel's.
	constexpr D3DCOLOR WARN_COLOR = D3DCOLOR_ARGB(150, 120, 190, 255);
	constexpr D3DCOLOR GLOW_COLOR = D3DCOLOR_ARGB(120, 60, 130, 255);
	constexpr D3DCOLOR CORE_COLOR = 0xFFFFFFFF;

	const int shots = RNG::Range(10, 13);
	for (int i = 0; i < shots; i++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [px, py] = lock->GetWorldPosition();

				LaserDesc beam = (RNG::Range(1, 2) == 1)
					? LaserDesc::Vertical(px, 0.f, BEAM_LENGTH)
					: LaserDesc::Horizontal(py, 0.f, BEAM_LENGTH);

				projectiles.Spawn(
					lock,
					beam.SetWarnTime(WARN_TIME)
					.SetFireTime(FIRE_TIME)
					.SetDamage(projDamage)
					.SetColors(WARN_COLOR, GLOW_COLOR, CORE_COLOR)
				);
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(SHOT_GAP));
	}
}
