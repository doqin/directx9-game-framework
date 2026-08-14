#include "pch.h"
#include "KernelEnemy.h"
#include "PopUpMessage.h"
#include "resource.h"
#include "RNG.h"

const float PI = 3.14159265359f;

namespace Demo {

	void KernelEnemy::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
		texture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		texture->LoadTexture(L"assets/kernel.png");
		sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(), DX9GF::Utils::CreateRectsHorizontal(0, 0, 64, 64, 11), 12);
		sprite->SetOrigin(32, 32);
		sprite->SetScale(2.f);

		mineTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		mineTexture->LoadTexture(L"assets/kernelprojectile.png");

		bulletTexture = std::make_shared<DX9GF::Texture>(graphicsDevice);
		bulletTexture->LoadTexture(L"assets/kernelprojectile.png");
		SetGoldReward(static_cast<int>(std::round(GetMaxHealth())));
		InitCardSpawnTrigger(camera, 128.f, 128.f);
	}

	void KernelEnemy::Update(unsigned long long deltaTime) {
		IEnemy::Update(deltaTime);

		if (laserState == 2) {
			if (auto p = player.lock()) {
				auto [px, py] = p->GetWorldPosition();

				float pw = 10.f;
				float ph = 10.f;
				float lw = 2.f;

				bool hit = false;
				for (float v : laserVerticals) {
					if (px + pw > v - lw && px - pw < v + lw && py + ph > -128.f && py - ph < 128.f) hit = true;
				}
				for (float h : laserHorizontals) {
					if (px + pw > -128.f && px - pw < 128.f && py + ph > h - lw && py - ph < h + lw) hit = true;
				}

				if (hit) {
					p->TakeDamage(storedProjDamage);
				}
			}
		}
	}

	void KernelEnemy::Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) {
		if (laserState > 0) {
			graphicsDevice->SetAlphaBlending(true);

			if (laserState == 1) {
				float alphaMult = sin(timeSinceStart * 0.02f) * 0.5f + 0.5f;
				int alpha = static_cast<int>(150 * alphaMult);
				D3DCOLOR warnColor = D3DCOLOR_ARGB(alpha, 255, 100, 100);

				for (float v : laserVerticals) graphicsDevice->DrawRectangle(*camera, v - 1.f, -128.f, 2.f, 256.f, warnColor, true);
				for (float h : laserHorizontals) graphicsDevice->DrawRectangle(*camera, -128.f, h - 1.f, 256.f, 2.f, warnColor, true);

				if (auto p = player.lock()) {
					auto [px, py] = p->GetWorldPosition();
					float minX = -128.f, maxX = 128.f;
					float minY = -128.f, maxY = 128.f;

					for (float v : laserVerticals) {
						if (px > v) minX = (std::max)(minX, v);
						if (px < v) maxX = (std::min)(maxX, v);
					}
					for (float h : laserHorizontals) {
						if (py > h) minY = (std::max)(minY, h);
						if (py < h) maxY = (std::min)(maxY, h);
					}

					float centerX = (minX + maxX) * 0.5f;
					float centerY = (minY + maxY) * 0.5f;

					float targetAlphaMult = sin(timeSinceStart * 0.04f) * 0.5f + 0.5f;
					D3DCOLOR targetColor = D3DCOLOR_ARGB(static_cast<int>(200 * targetAlphaMult), 255, 50, 50);

					graphicsDevice->DrawRectangle(*camera, centerX - 8.f, centerY - 8.f, 16.f, 16.f, targetColor, true);
					graphicsDevice->DrawRectangle(*camera, centerX - 16.f, centerY - 16.f, 32.f, 32.f, targetColor, false);
				}
			}
			else if (laserState == 2) {
				D3DCOLOR glowColor = D3DCOLOR_ARGB(120, 255, 20, 147);
				D3DCOLOR coreColor = 0xFFFFFFFF;

				for (float v : laserVerticals) {
					graphicsDevice->DrawRectangle(*camera, v - 8.f, -128.f, 16.f, 256.f, glowColor, true);
					graphicsDevice->DrawRectangle(*camera, v - 2.f, -128.f, 4.f, 256.f, coreColor, true);
				}
				for (float h : laserHorizontals) {
					graphicsDevice->DrawRectangle(*camera, -128.f, h - 8.f, 256.f, 16.f, glowColor, true);
					graphicsDevice->DrawRectangle(*camera, -128.f, h - 2.f, 256.f, 4.f, coreColor, true);
				}
			}
			graphicsDevice->SetAlphaBlending(false);
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

		int patternId = GetSmartRandomPattern(1, 3);
		if (patternId == 1) PatternDefrag(baseDamage);
		else if (patternId == 2) PatternPing999(baseDamage);
		else PatternBadSector(baseDamage);
	}

	void KernelEnemy::PatternDefrag(float baseDamage) {
		struct MineData { float x, y; bool isCross; };
		std::vector<MineData> mines;
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
							.SetTrajectory(dir).SetVelocity(250.f).SetDamage(finalDamage).SetDecayTime(3.f));
					}
				}
			}
			markFinished();
			}));
	}

	void KernelEnemy::PatternPing999(float baseDamage) {
		const int WAVE_COUNT = 6;

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
							.SetTrajectory(D3DXVECTOR2(0, 1))
							.SetVelocity(420.f)
							.SetReturnAcceleration(290.f)
							.SetDamage(finalDamage)
							.SetDecayTime(4.0f));
					}
				}
				markFinished();
				}));
			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(1.5f));
		}
	}

	void KernelEnemy::PatternBadSector(float baseDamage) {
		const int WAVES = RNG::Range(2, 4);

		for (int w = 0; w < WAVES; ++w) {
			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this](auto markFinished) {
				this->laserVerticals.clear();
				this->laserHorizontals.clear();
				this->laserVerticals.push_back(RNG::Range(-80.f, 80.f));
				this->laserHorizontals.push_back(RNG::Range(-80.f, 80.f));

				this->laserState = 1; //warning line
				markFinished();
				}));

			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(1.5f));

			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this, baseDamage](auto markFinished) {
				this->laserState = 2;

				this->storedProjDamage = this->CalculateOutgoingDamage(baseDamage);
				float finalDamage = this->storedProjDamage;

				if (auto p = this->player.lock()) {
					auto [px, py] = p->GetWorldPosition();

					float minX = -128.f, maxX = 128.f;
					float minY = -128.f, maxY = 128.f;

					for (float v : this->laserVerticals) {
						if (px > v) minX = (std::max)(minX, v);
						if (px < v) maxX = (std::min)(maxX, v);
					}
					for (float h : this->laserHorizontals) {
						if (py > h) minY = (std::max)(minY, h);
						if (py < h) maxY = (std::min)(maxY, h);
					}

					float centerX = (minX + maxX) * 0.5f;
					float centerY = (minY + maxY) * 0.5f;
					float holdTime = 4.0f;

					projectiles.Spawn(p, ProjectileDesc(bulletTexture.get(), 8, 8, 16, 16, centerX, centerY)
						.SetVelocity(0.f)
						.SetDamage(finalDamage)
						.SetDecayTime(holdTime));

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
								.SetDecayTime(holdTime));
						}
					}
				}
				markFinished();
				}));

			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(4.0f));

			commandBuffer.PushCommand(std::make_shared<DX9GF::CustomCommand>([this](auto markFinished) {
				this->laserState = 0;
				markFinished();
				}));

			commandBuffer.PushCommand(std::make_shared<DX9GF::DelayCommand>(0.4f));
		}
	}
}