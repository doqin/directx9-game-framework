#include "pch.h"
#include "KernelEnemy.h"
#include "PopUpMessage.h"
#include "resource.h"
#include "RNG.h"

const float PI = 3.14159265359f;
// Half-width of the bullet-hell arena the patterns play out in.
const float ARENA_HALF = 128.f;

namespace Demo {

	void KernelEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
		texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		texture->LoadTexture(L"assets/kernel.png");
		sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 11), 12);
		sprite->SetOrigin(32, 32);
		sprite->SetScale(2.f);

		mineTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		mineTexture->LoadTexture(L"assets/mine.png");

		bulletTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		bulletTexture->LoadTexture(L"assets/kernelprojectile.png");
		SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
		InitCardSpawnTrigger(camera, 128.f, 128.f);
	}

	void KernelEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) {
		// Marks the cell the beams are about to box the player into, which is also where
		// the spiral will erupt from once they fire.
		if (telegraphingCell) {
			if (auto p = player.lock()) {
				auto [px, py] = p->GetWorldPosition();
				auto [centerX, centerY] = GetSafeCellCenter(px, py);

				float targetAlphaMult = sin(timeSinceStart * 0.04f) * 0.5f + 0.5f;
				D3DCOLOR targetColor = D3DCOLOR_ARGB(static_cast<int>(200 * targetAlphaMult), 255, 50, 50);

				graphicsDevice->SetAlphaBlending(true);
				graphicsDevice->DrawRectangle(*camera, centerX - 8.f, centerY - 8.f, 16.f, 16.f, targetColor, true);
				graphicsDevice->DrawRectangle(*camera, centerX - 16.f, centerY - 16.f, 32.f, 32.f, targetColor, false);
				graphicsDevice->SetAlphaBlending(false);
			}
		}

		if (sprite) {
			sprite->Begin();
			auto [x, y] = GetWorldPosition();
			sprite->SetPosition(x, y);
			sprite->Draw(*camera, deltaTime);
			sprite->End();
		}
		IEnemy::Draw(graphicsDevice, camera, deltaTime);
	}

	void Demo::KernelEnemy::OnTurnBegin(std::shared_ptr<Player> player, std::shared_ptr<PopUpMessage> popUpMessage, int currentTurn) {
		this->player = player;

		int cycle = (currentTurn - 1) / 3;

		if (currentCycle != cycle) {
			currentCycle = cycle;

			if (RNG::Range(1, 100) <= 55) {
				skillTurnThisCycle = RNG::Range(1, 3);
			}
			else {
				skillTurnThisCycle = -1;
			}
		}

		int turnInCycle = (currentTurn - 1) % 3 + 1;

		if (turnInCycle == skillTurnThisCycle) {
			if (RNG::Range(1, 2) == 1) {
				CastAbility([this]() { this->Heal(20.f); }, popUpMessage, L"Kernel runs recovery protocol! (+20 HP)");
			}
			else {
				CastAbility([this]() { this->AddModifier(ModifierType::BuffDamage, 2, 2.0f, true); }, popUpMessage, L"Kernel elevates privileges!");
			}
		}
	}

	void KernelEnemy::StartAttack(std::shared_ptr<Player> player, std::vector<std::shared_ptr<IEnemy>>* enemies, std::shared_ptr<PopUpMessage> popUpMessage, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, int currentTurn) {
		(void)enemies; (void)popUpMessage; (void)graphicsDevice; (void)camera;

		this->player = player;
		float baseDamage = 5.f;

		int patternId = GetSmartRandomPattern(1, 4);
		if (patternId == 1) PatternDefrag(baseDamage);
		else if (patternId == 2) PatternPing999(baseDamage);
		else if (patternId == 3) PatternZigzag(baseDamage);
		else PatternBadSector(baseDamage);
	}

	void KernelEnemy::PatternDefrag(float baseDamage) {
		struct MineData { float x, y; bool isCross; };
		std::vector<MineData> mines;
		constexpr int WAVE_COUNT = 5;

		for (int wave = 0; wave < WAVE_COUNT; ++wave) {
			const int MINE_COUNT = RNG::Range(3, 5);

			for (int i = 0; i < MINE_COUNT; ++i) {
				mines.push_back({ RNG::Range(-100.f, 100.f), RNG::Range(-100.f, 100.f), RNG::Range(0, 1) == 0 });
			}

			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, mines](auto markFinished) {
				if (auto p = this->player.lock()) {
					for (auto& m : mines) {
						projectiles.Spawn(p, ProjectileDesc(mineTexture.get(), 8, 8, 0, 0, m.x, m.y).SetVelocity(0.f).SetDecayTime(1.5f));
					}
				}
				markFinished();
				}));

			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(1.5f));

			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, mines, baseDamage](auto markFinished) {
				if (auto p = this->player.lock()) {
					float finalDamage = this->CalculateOutgoingDamage(baseDamage);

					for (auto& m : mines) {
						float angles[4];
						if (m.isCross) { angles[0] = 0; angles[1] = PI / 2; angles[2] = PI; angles[3] = 3 * PI / 2; }
						else { angles[0] = PI / 4; angles[1] = 3 * PI / 4; angles[2] = 5 * PI / 4; angles[3] = 7 * PI / 4; }

						for (int i = 0; i < 4; ++i) {
							D3DXVECTOR2 dir(cos(angles[i]), sin(angles[i]));
							projectiles.Spawn(p, ProjectileDesc(bulletTexture.get(), 8, 8, 16, 16, m.x, m.y)
								.SetTrajectory(dir).SetVelocity(100.f).SetDamage(finalDamage).SetDecayTime(5.f));
						}
					}
				}
				markFinished();
				}));
			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(.7f));
		}
	}

	void KernelEnemy::PatternPing999(float baseDamage) {
		const int WAVE_COUNT = 20;

		for (int w = 0; w < WAVE_COUNT; ++w) {
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, baseDamage](auto markFinished) {
				if (auto p = this->player.lock()) {
					float finalDamage = this->CalculateOutgoingDamage(baseDamage);

					const int BULLET_COUNT = 15;
					int safeHole = RNG::Range(1, BULLET_COUNT - 4);
					float totalWidth = 500.f;
					float startX = -250.f;
					float baseSpacing = totalWidth / (BULLET_COUNT - 1);

					for (int i = 0; i < BULLET_COUNT; ++i) {
						if (i >= safeHole && i <= safeHole + 2) continue;

						float jitter = RNG::Range(-baseSpacing * 0.4f, baseSpacing * 0.4f);
						float bx = startX + (i * baseSpacing) + jitter;

						projectiles.Spawn(p, ProjectileDesc(bulletTexture.get(), 8, 8, 16, 16, bx, -150.f)
							.SetDelay(1.f)
							.SetTrajectory(D3DXVECTOR2(0.f, -1.f))
							.SetVelocity(420.f)
							.SetReturnAcceleration(800.f)
							.SetDamage(finalDamage)
							.SetDecayTime(4.0f));
					}
				}
				markFinished();
				}));
			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(.5f));
		}
	}

	// Glitch sweep sends rows of zigzagging shots from alternating sides.
	//
	void KernelEnemy::PatternZigzag(float baseDamage) {
		const int WAVE_COUNT = 6;
		const int BULLETS_PER_WAVE = 10;
		const int STEP_Y = 64;
		const float START_Y = -(BULLETS_PER_WAVE - 1) * STEP_Y * 0.5f;
		std::shared_ptr<int> offsetY = std::make_shared<int>(0);
		const int OFFSET_STEP = 8;

		for (int wave = 0; wave < WAVE_COUNT; ++wave) {
			const bool fromLeft = (wave % 2 == 0);
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, baseDamage, fromLeft, START_Y, STEP_Y, BULLETS_PER_WAVE, offsetY, OFFSET_STEP](auto markFinished) {
				if (auto p = this->player.lock()) {
					float finalDamage = this->CalculateOutgoingDamage(baseDamage);
					float spawnX = fromLeft ? -ARENA_HALF * 4.f : ARENA_HALF * 4.f;
					D3DXVECTOR2 dir = fromLeft ? D3DXVECTOR2(1, 0) : D3DXVECTOR2(-1, 0);

					for (int i = 0; i < BULLETS_PER_WAVE; ++i) {
						float spawnY = START_Y + STEP_Y * static_cast<float>(i) + *offsetY;
						float zigzagAmplitude = 100.f;
						float zigzagFrequency = 0.3f;

						projectiles.Spawn(p, ProjectileDesc(bulletTexture.get(), 8, 8, 16, 16, spawnX, spawnY)
							.SetTrajectory(dir)
							.SetVelocity(120.f)
							.SetWave(zigzagAmplitude, zigzagFrequency, true)
							.SetDecayTime(8.5f)
							.SetDamage(finalDamage));
					}
					*offsetY += OFFSET_STEP;
					*offsetY %= STEP_Y;
				}
				markFinished();
			}));
			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.5f));
		}
	}

	std::pair<float, float> KernelEnemy::GetSafeCellCenter(float playerX, float playerY) const {
		float minX = -ARENA_HALF, maxX = ARENA_HALF;
		float minY = -ARENA_HALF, maxY = ARENA_HALF;

		for (float v : laserVerticals) {
			if (playerX > v) minX = (std::max)(minX, v);
			if (playerX < v) maxX = (std::min)(maxX, v);
		}
		for (float h : laserHorizontals) {
			if (playerY > h) minY = (std::max)(minY, h);
			if (playerY < h) maxY = (std::min)(maxY, h);
		}

		return { (minX + maxX) * 0.5f, (minY + maxY) * 0.5f };
	}

	void KernelEnemy::PatternBadSector(float baseDamage) {
		const int WAVES = RNG::Range(2, 4);
		const float WARN_TIME = 1.5f;
		const float HOLD_TIME = 4.0f;

		for (int w = 0; w < WAVES; ++w) {
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, baseDamage, WARN_TIME, HOLD_TIME](auto markFinished) {
				this->laserVerticals.clear();
				this->laserHorizontals.clear();
				this->laserVerticals.push_back(RNG::Range(-80.f, 80.f));
				this->laserHorizontals.push_back(RNG::Range(-80.f, 80.f));
				this->telegraphingCell = true;

				if (auto p = this->player.lock()) {
					float finalDamage = this->CalculateOutgoingDamage(baseDamage);

					// The beams telegraph themselves and start burning on their own, so the
					// command buffer only has to line the spiral up with the moment they fire.
					for (float v : this->laserVerticals) {
						projectiles.Spawn(p, LaserDesc::Vertical(v, 0.f, ARENA_HALF * 2.f)
							.SetWarnTime(WARN_TIME)
							.SetFireTime(HOLD_TIME)
							.SetDamage(finalDamage));
					}
					for (float h : this->laserHorizontals) {
						projectiles.Spawn(p, LaserDesc::Horizontal(h, 0.f, ARENA_HALF * 2.f)
							.SetWarnTime(WARN_TIME)
							.SetFireTime(HOLD_TIME)
							.SetDamage(finalDamage));
					}
				}
				markFinished();
				}));

			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(WARN_TIME));

			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, baseDamage, HOLD_TIME](auto markFinished) {
				this->telegraphingCell = false;

				if (auto p = this->player.lock()) {
					float finalDamage = this->CalculateOutgoingDamage(baseDamage);
					auto [px, py] = p->GetWorldPosition();
					auto [centerX, centerY] = this->GetSafeCellCenter(px, py);

					projectiles.Spawn(p, ProjectileDesc(bulletTexture.get(), 8, 8, 16, 16, centerX, centerY)
						.SetVelocity(0.f)
						.SetDamage(finalDamage)
						.SetDecayTime(HOLD_TIME));

					int numArms = RNG::Range(1, 4);
					float direction = (RNG::Range(0, 1) == 0) ? 1.0f : -1.0f;
					const int BULLETS_PER_ARM = 12;

					for (int arm = 0; arm < numArms; ++arm) {
						float startAngle = arm * (2.0f * PI / numArms);

						for (int i = 1; i <= BULLETS_PER_ARM; ++i) {
							float radialSpeed = i * 12.f;
							float angularSpeed = (PI * 0.7f - (i * 0.03f)) * direction;

							projectiles.Spawn(p, ProjectileDesc(bulletTexture.get(), 8, 8, 16, 16, centerX, centerY)
								.SetSpiralParams(startAngle, radialSpeed, angularSpeed)
								.SetDamage(finalDamage)
								.SetDecayTime(HOLD_TIME));
						}
					}
				}
				markFinished();
				}));

			// The beams burn for HOLD_TIME, then a short breather before the next wave.
			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(HOLD_TIME + 0.4f));
		}
	}
}