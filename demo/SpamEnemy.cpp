#include "pch.h"
#include "SpamEnemy.h"
#include "resource.h"
#include "RNG.h"
#include "PopUpMessage.h"
#include <algorithm>
#include <cmath>

namespace {
	constexpr float SPAM_PI = 3.14159265359f;
}

void Demo::SpamEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera)
{
	texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	texture->LoadTexture(L"assets/spam-Sheet.png");
	sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 12), 12);
	sprite->SetOrigin(32, 32);
	sprite->SetScale(2.f);

	projTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
	projTexture->LoadTexture(L"assets/spamprojectile.png");

	SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
	InitCardSpawnTrigger(camera, 128.f, 128.f);
}

void Demo::SpamEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime)
{
	if (sprite) {
		sprite->Begin();
		auto [x, y] = GetWorldPosition();
		sprite->SetPosition(x, y);
		sprite->Draw(*camera, deltaTime);
		sprite->End();
	}
	IEnemy::Draw(graphicsDevice, camera, deltaTime);
}

void Demo::SpamEnemy::OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn)
{
	this->player = player;

	int cycle = (currentTurn - 1) / 3;
	if (currentCycle != cycle) {
		currentCycle = cycle;
		skillTurnThisCycle = (RNG::Range(1, 100) <= 55) ? RNG::Range(1, 3) : -1;
	}

	int turnInCycle = (currentTurn - 1) % 3 + 1;
	if (turnInCycle == skillTurnThisCycle) {
		if (RNG::Range(1, 2) == 1) {
			CastAbility([this]() { this->Heal(12.f); }, popUpMessage, L"Spam multiplies! (+12 HP)");
		}
		else {
			CastAbility([this]() {
				if (auto lock = this->player.lock()) lock->AddModifier(ModifierType::Weak, 2, 0.f, false);
				}, popUpMessage, L"Your best shots get buried in junk!");
		}
	}
}

void Demo::SpamEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn)
{
	(void)enemies;
	(void)popUpMessage;
	(void)graphicsDevice;
	(void)camera;
	(void)currentTurn;
	this->player = player;

	const float baseDamage = 4.f;
	const float finalDamage = CalculateOutgoingDamage(baseDamage);

	int patternId = GetSmartRandomPattern(1, 5);
	if (patternId == 1) PatternInboxFlood(finalDamage);
	else if (patternId == 2) PatternChainLetter(finalDamage);
	else if (patternId == 3) PatternMassMailer(finalDamage);
	else if (patternId == 4) PatternPopupAds(finalDamage);
	else PatternUnsubscribe(finalDamage);
}

// "Inbox Flood" - envelopes rain straight down in columns. Every wave seals off all
// columns but a moving two-wide gap, so the dodge is a steady sidestep that keeps
// chasing the opening.
void Demo::SpamEnemy::PatternInboxFlood(float projDamage)
{
	constexpr int COLUMNS = 10;
	constexpr float SPAN = 240.f;                 // -120 .. 120 across the battle box
	constexpr float STEP = SPAN / (COLUMNS - 1);
	constexpr int WAVES = 14;
	constexpr float SPAWN_Y = -260.f;
	constexpr float BULLET_SPEED = 220.f;
	constexpr float WAVE_DELAY = 0.55f;

	int gap = RNG::Range(0, COLUMNS - 2);
	for (int w = 0; w < WAVES; w++) {
		for (int c = 0; c < COLUMNS; c++) {
			if (c == gap || c == gap + 1) continue;
			float x = -SPAN * 0.5f + c * STEP;
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, x, BULLET_SPEED, SPAWN_Y](std::function<void(void)> markFinished) {
				if (auto lock = this->player.lock()) {
					projectiles.Spawn(
						lock,
						ProjectileDesc(projTexture.get(), 8, 8, 11, 11, x, SPAWN_Y)
						.SetTrajectory(D3DXVECTOR2(0.f, 1.f))
						.SetVelocity(BULLET_SPEED)
						.SetDecayTime(4.f)
						.SetDamage(projDamage)
					);
				}
				markFinished();
				}));
			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.01f));
		}
		// Nudge the gap one or two columns each wave so it slides rather than teleports.
		gap = std::clamp(gap + RNG::Range(-2, 2), 0, COLUMNS - 2);
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(WAVE_DELAY));
	}
}

// "Chain Letter" - a slowly rotating three-armed spiral from the boss. No single spot
// stays safe; the player has to keep orbiting.
void Demo::SpamEnemy::PatternChainLetter(float projDamage)
{
	constexpr int TICKS = 55;
	constexpr int ARMS = 3;
	constexpr float ANGLE_STEP = 0.36f;
	constexpr float BULLET_SPEED = 165.f;
	constexpr float TICK_DELAY = 0.06f;

	auto angle = std::make_shared<float>(RNG::Range(0.f, SPAM_PI * 2.f));
	for (int t = 0; t < TICKS; t++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, angle, ARMS, ANGLE_STEP, BULLET_SPEED](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				for (int a = 0; a < ARMS; a++) {
					float dirAngle = *angle + a * (SPAM_PI * 2.f / ARMS);
					D3DXVECTOR2 dir(std::cos(dirAngle), std::sin(dirAngle));
					projectiles.Spawn(
						lock,
						ProjectileDesc(projTexture.get(), 8, 8, 11, 11, 0.f, 0.f)
						.SetDelay(1.f)
						.SetTrajectory(dir)
						.SetVelocity(BULLET_SPEED)
						.SetDecayTime(5.f)
						.SetDamage(projDamage)
					);
				}
				*angle += ANGLE_STEP;
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(TICK_DELAY));
	}
}

// "Mass Mailer" - CC-bombed volleys from alternating side walls, each a short fan
// aimed at where the player was standing. Reward for reading which wall is next.
void Demo::SpamEnemy::PatternMassMailer(float projDamage)
{
	constexpr int VOLLEYS = 28;
	constexpr int FAN = 5;
	constexpr float SPREAD = 0.25f;               // radians between fan bullets
	constexpr float WALL_X = 320.f;
	constexpr float BULLET_SPEED = 240.f;
	constexpr float VOLLEY_DELAY = 0.5f;

	for (int v = 0; v < VOLLEYS; v++) {
		float sign = (v % 2 == 0) ? -1.f : 1.f;
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, sign, FAN, SPREAD, WALL_X, BULLET_SPEED](std::function<void(void)> markFinished) {
			if (auto lock = this->player.lock()) {
				auto [px, py] = lock->GetWorldPosition();
				float spawnX = sign * WALL_X;
				float spawnY = RNG::Range(-120.f, 120.f);
				float baseAngle = std::atan2(py - spawnY, px - spawnX);
				for (int i = 0; i < FAN; i++) {
					float ang = baseAngle + (i - FAN / 2) * SPREAD;
					D3DXVECTOR2 dir(std::cos(ang), std::sin(ang));
					projectiles.Spawn(
						lock,
						ProjectileDesc(projTexture.get(), 8, 8, 11, 11, spawnX, spawnY)
						.SetTrajectory(dir)
						.SetVelocity(BULLET_SPEED)
						.SetDecayTime(5.f)
						.SetDamage(projDamage)
					);
				}
			}
			markFinished();
			}));
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(VOLLEY_DELAY));
	}
}

// "Pop-up Ads" - clusters blink in around the player, hold for a beat, then crawl
// inward. The telegraph is the whole point: step out of the cluster before it moves.
void Demo::SpamEnemy::PatternPopupAds(float projDamage)
{
	constexpr int WAVES = 9;
	constexpr int PER_WAVE = 4;
	constexpr float RING_RADIUS = 130.f;
	constexpr float TELEGRAPH = 0.4f;
	constexpr float TURN_SPEED = 1.8f;
	constexpr float BULLET_SPEED = 300.f;
	constexpr float WAVE_DELAY = 0.5f;

	for (int w = 0; w < WAVES; w++) {
		for (int i = 0; i < PER_WAVE; i++) {
			float ang = RNG::Range(0.f, SPAM_PI * 2.f);
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, ang, RING_RADIUS, TELEGRAPH, TURN_SPEED, BULLET_SPEED](std::function<void(void)> markFinished) {
				if (auto lock = this->player.lock()) {
					auto [px, py] = lock->GetWorldPosition();
					float spawnX = px + std::cos(ang) * RING_RADIUS;
					float spawnY = py + std::sin(ang) * RING_RADIUS;
					projectiles.Spawn(
						lock,
						ProjectileDesc(projTexture.get(), 8, 8, 11, 11, spawnX, spawnY)
						.SetTrajectory(D3DXVECTOR2(-std::cos(ang), -std::sin(ang)))
						.SetHoming(TURN_SPEED)
						.SetDelay(TELEGRAPH)
						.SetVelocity(BULLET_SPEED)
						.SetDecayTime(3.f)
						.SetDamage(projDamage)
					);
				}
				markFinished();
				}));
			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.04f));
		}
		commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(WAVE_DELAY));
	}
}

// "Unsubscribe" - the spam filter sweeps the arena: telegraphed beams, vertical or
// horizontal, laid on the player's line, with a trickle of envelopes in between so
// there is no full rest between beams.
void Demo::SpamEnemy::PatternUnsubscribe(float projDamage)
{
	constexpr float BEAM_LENGTH = 256.f;
	constexpr float WARN_TIME = 1.3f;
	constexpr float FIRE_TIME = 0.9f;
	constexpr float BEAM_GAP = 1.7f;
	constexpr D3DCOLOR WARN_COLOR = D3DCOLOR_ARGB(150, 255, 170, 70);
	constexpr D3DCOLOR GLOW_COLOR = D3DCOLOR_ARGB(120, 255, 120, 20);
	constexpr D3DCOLOR CORE_COLOR = 0xFFFFFFFF;

	const int beams = RNG::Range(3, 4);
	for (int b = 0; b < beams; b++) {
		commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage, BEAM_LENGTH, WARN_TIME, FIRE_TIME, WARN_COLOR, GLOW_COLOR, CORE_COLOR](std::function<void(void)> markFinished) {
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

		// Filler envelopes drifting down while the beam telegraphs / burns.
		const int filler = 5;
		for (int f = 0; f < filler; f++) {
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, projDamage](std::function<void(void)> markFinished) {
				if (auto lock = this->player.lock()) {
					projectiles.Spawn(
						lock,
						ProjectileDesc(projTexture.get(), 8, 8, 11, 11, RNG::Range(-120.f, 120.f), -260.f)
						.SetTrajectory(D3DXVECTOR2(0.f, 1.f))
						.SetVelocity(150.f)
						.SetDecayTime(4.f)
						.SetDamage(projDamage)
					);
				}
				markFinished();
				}));
			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(BEAM_GAP / filler));
		}
	}
}
