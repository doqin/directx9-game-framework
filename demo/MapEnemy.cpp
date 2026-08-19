#include "pch.h"
#include "MapEnemy.h"
#include "MapBattleScene.h"
#include "EncounterGenerator.h"
namespace Demo {
	MapEnemy::MapEnemy(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y, const BattleEncounter& data)
		: IGameObject(tm, x, y), startX(x), startY(y), encounterData(data) {
	}

	MapEnemy::~MapEnemy() {
		if (colliderManager && collider) {
			colliderManager->Remove(collider);
		}
	}

	void MapEnemy::Init(Game* game, DX9GF::GraphicsDevice* gd, DX9GF::ColliderManager* colMan, std::shared_ptr<Player> player) {
		this->game = game;
		this->colliderManager = colMan;
		this->targetPlayer = player;

		static std::map<std::wstring, std::shared_ptr<DX9GF::Texture>> enemyTextureCache;

		if (enemyTextureCache.find(encounterData.mapTexturePath) == enemyTextureCache.end()) {
			auto newTex = std::make_shared<DX9GF::Texture>(gd);
			newTex->LoadTexture(encounterData.mapTexturePath.c_str());
			enemyTextureCache[encounterData.mapTexturePath] = newTex;
		}
		this->texture = enemyTextureCache[encounterData.mapTexturePath];

		sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(),
			DX9GF::Utils::CreateRectsHorizontal(0, 0, encounterData.spriteWidth, encounterData.spriteHeight, encounterData.frameCount), encounterData.frameCount);

		sprite->SetOrigin(encounterData.spriteWidth / 2.f, encounterData.spriteHeight / 2.f);

		this->SetLocalScale(0.8f, 0.8f); //map enemy's size
		sprite->SetScale(0.8f, 0.8f); //map enemy's size

		float hitBoxW = encounterData.hitBoxWidth;
		float originalHitBoxH = encounterData.hitBoxHeight;
		float hitBoxH = originalHitBoxH * 0.25f;

		float offsetX = -hitBoxW / 2.0f;
		float offsetY = (originalHitBoxH / 2.0f) - hitBoxH;

		collider = std::make_shared<DX9GF::RectangleCollider>(
			transformManager,
			shared_from_this(),
			hitBoxW, hitBoxH,
			offsetX, offsetY
		);

		collider->SetOrigin(0.f, 0.f);

		particleTex = std::make_shared<DX9GF::Texture>(gd);
		particleTex->LoadTexture(L"assets/ui.png");

		wealthyAura = std::make_unique<DX9GF::ParticleSystem>(particleTex.get(), 16);
		wealthyAura->SetSrcRect({ 261, 277, 267, 283});
		wealthyAura->SetOrigin(6.f, 6.f);
		wealthyAura->SetEmissionInterval(100.f);
		wealthyAura->SetLifeTime(500.f);
		wealthyAura->SetFadeOutTime(200.f);
		wealthyAura->SetScaleRange(1.f, 0.2f);
		wealthyAura->SetColorRange(0xFFFF00FF, 0xFF8B00FF);
		wealthyAura->SetEnabled(encounterData.eventType != EventType::None);
	}

	void MapEnemy::Update(unsigned long long deltaTime) {

		//respawn
		if (isDefeated) {
			respawnTimer -= deltaTime / 1000.f;
			if (respawnTimer <= 0) {
				isDefeated = false;
				SetLocalPosition(startX, startY);
				currentState = State::Idle;
				
				//avoid spamming gold token on 1 enemy
				SetEventState(EventType::None);

				if (colliderManager) {
					float hitBoxW = encounterData.hitBoxWidth;
					float originalHitBoxH = encounterData.hitBoxHeight;
					float hitBoxH = originalHitBoxH * 0.25f;

					float offsetX = -hitBoxW / 2.0f;
					float offsetY = (originalHitBoxH / 2.0f) - hitBoxH;

					collider = std::make_shared<DX9GF::RectangleCollider>(
						transformManager,
						shared_from_this(),
						hitBoxW, hitBoxH,
						offsetX, offsetY
					);

					collider->SetOrigin(0.f, 0.f);
				}
				if (encounterData.useGlobalPool) {
					encounterData.enemyTypes = EncounterGenerator::GenerateNormalEncounter();
				}
				else if (!encounterData.randomPool.empty()) {
					encounterData.enemyTypes = EncounterGenerator::GenerateFromTypes(encounterData.randomPool);
				}
			}
			return;
		}

		if (auto player = targetPlayer.lock()) {
			if (postBattleCooldown > 0) {
				postBattleCooldown -= deltaTime / 1000.f;
				if (postBattleCooldown > 0) return;
			}

			auto [px, py] = player->GetWorldPosition();
			auto [ex, ey] = GetWorldPosition();

			float dx = px - ex;
			float dy = py - ey;
			float distanceSq = dx * dx + dy * dy;

			auto pCol = std::dynamic_pointer_cast<DX9GF::RectangleCollider>(player->GetCollider().lock());
			if (pCol && this->collider) {
				auto getBounds = [](std::shared_ptr<DX9GF::RectangleCollider> c) {
					float sx = std::abs(c->GetWorldScaleX());
					float sy = std::abs(c->GetWorldScaleY());
					float left = c->GetWorldX() - (c->GetOriginX() * sx);
					float right = left + (c->GetWidth() * sx);
					float top = c->GetWorldY() - (c->GetOriginY() * sy);
					float bottom = top + (c->GetHeight() * sy);
					return std::make_tuple(left, right, top, bottom);
					};

				auto [pLeft, pRight, pTop, pBottom] = getBounds(pCol);
				auto [eLeft, eRight, eTop, eBottom] = getBounds(this->collider);

				float epsilon = 2.0f;
				if (pLeft <= eRight + epsilon && pRight >= eLeft - epsilon &&
					pTop <= eBottom + epsilon && pBottom >= eTop - epsilon) {

					currentState = State::Idle;
					postBattleCooldown = 2.0f;
					colliderManager->Remove(collider);

					if (onEncounterTriggered) {
						onEncounterTriggered(std::dynamic_pointer_cast<MapEnemy>(shared_from_this()));
					}
					return;
				}
			}

			//state handling and footprint recording
			if (currentState == State::Idle || currentState == State::Patrol) {
				if (distanceSq < aggroRadius * aggroRadius) {
					currentState = State::Chase;
					chasePath.clear();
					returnPath.clear();
					lastPlayerPos = { px, py };
					lastEnemyPos = { ex, ey };
				}
				else {
					float homeDx = startX - ex;
					float homeDy = startY - ey;
					if (homeDx * homeDx + homeDy * homeDy > 400.f) {
						currentState = State::Return;
					}
				}
			}
			else if (currentState == State::Chase) {
				float homeDx = startX - ex;
				float homeDy = startY - ey;
				float homeDistSq = homeDx * homeDx + homeDy * homeDy;

				//tethering
				if (distanceSq > returnRadius * returnRadius || homeDistSq > tetherRadius * tetherRadius) {
					currentState = State::Return;
				}
				else {
					float pDistX = px - lastPlayerPos.x;
					float pDistY = py - lastPlayerPos.y;
					if (pDistX * pDistX + pDistY * pDistY > 1024.f) {
						chasePath.push_back({ px, py });
						lastPlayerPos = { px, py };
						if (chasePath.size() > 20) chasePath.pop_front();
					}

					float eDistX = ex - lastEnemyPos.x;
					float eDistY = ey - lastEnemyPos.y;
					if (eDistX * eDistX + eDistY * eDistY > 1024.f) {
						returnPath.push_back({ ex, ey });
						lastEnemyPos = { ex, ey };
					}
				}
			}
			else if (currentState == State::Return) {
				float homeDx = startX - ex;
				float homeDy = startY - ey;
				float homeDistSq = homeDx * homeDx + homeDy * homeDy;

				// Force enemies to return within the anchor point (less than 10 pixels) before switching to Idle state
				if (homeDistSq < 100.f) {
					currentState = State::Idle;
					returnPath.clear();
				}
			}

			// Calculate movement direction
			D3DXVECTOR2 dir{ 0, 0 };

			if (currentState == State::Chase) {
				float targetX = px;
				float targetY = py;

				losTimer += (deltaTime / 1000.f);
				if (losTimer >= 0.5f) {
					lastLosResult = CheckLineOfSight(ex, ey, px, py);
					losTimer = 0.f;
				}

				if (lastLosResult) {
					chasePath.clear();
				}
				else {
					if (!chasePath.empty()) {
						targetX = chasePath.front().x;
						targetY = chasePath.front().y;

						float tdx = targetX - ex;
						float tdy = targetY - ey;
						if (tdx * tdx + tdy * tdy < 100.f) {
							chasePath.pop_front();
						}
					}
				}

				D3DXVECTOR2 targetDir(targetX - ex, targetY - ey);
				D3DXVec2Normalize(&dir, &targetDir);
			}
			else if (currentState == State::Return) {
				float targetX = startX;
				float targetY = startY;

				if (!returnPath.empty()) {
					targetX = returnPath.back().x;
					targetY = returnPath.back().y;

					float tdx = targetX - ex;
					float tdy = targetY - ey;
					if (tdx * tdx + tdy * tdy < 100.f) {
						returnPath.pop_back();
					}
				}

				D3DXVECTOR2 targetDir(targetX - ex, targetY - ey);
				D3DXVec2Normalize(&dir, &targetDir);
			}

			if (dir.x != 0 || dir.y != 0) {
				float moveX = dir.x * speed * (deltaTime / 1000.f);
				float moveY = dir.y * speed * (deltaTime / 1000.f);
				auto [finalDX, finalDY] = colliderManager->GetSlidingDeltas(collider, moveX, moveY);
				SetLocalPosition(ex + finalDX, ey + finalDY);

				float baseScaleX = std::abs(this->GetLocalScaleX());
				float baseScaleY = std::abs(this->GetLocalScaleY());

				sprite->SetScale(dir.x > 0 ? -baseScaleX : baseScaleX, baseScaleY);
			}
		}
		if (!isDefeated && encounterData.eventType != EventType::None && wealthyAura) {
			auto [ex, ey] = GetWorldPosition();

			//aura particle
			float randomOffsetX = Demo::RNG::Range(-24.f, 24.f);
			float randomOffsetY = Demo::RNG::Range(-24.f, 24.f);

			wealthyAura->Update(deltaTime, ex + randomOffsetX, ey + randomOffsetY, 0.f, 1.f, 1.f, 0xFFFFFFFF, true);
		}
	}

	void MapEnemy::Draw(DX9GF::Camera* camera, unsigned long long deltaTime) {
		if (isDefeated) return;
		if (postBattleCooldown > 0 && static_cast<int>(postBattleCooldown * 10) % 2 == 0) return;

		auto [x, y] = GetWorldPosition();

		if (game && game->GetGraphicsDevice()) {
			auto gd = game->GetGraphicsDevice();
			gd->SetAlphaBlending(true);

			float scaleX = std::abs(this->GetLocalScaleX());
			float scaleY = std::abs(this->GetLocalScaleY());
			float shadowWidth = encounterData.spriteWidth * 0.6f * scaleX;
			float shadowHeight = shadowWidth * 0.4f;

			float shadowY = y + (encounterData.spriteHeight * 0.45f * scaleY);

			gd->DrawEllipse(*camera, x, shadowY, shadowWidth, shadowHeight, 0x64000000, true);
			gd->SetAlphaBlending(false);
		}
		if (!isDefeated && encounterData.eventType != EventType::None && wealthyAura) {
			wealthyAura->Draw(*camera, deltaTime);
		}
		sprite->Begin();
		sprite->SetPosition(x, y);
		sprite->Draw(*camera, deltaTime);
		sprite->End();
	}

	bool MapEnemy::CheckLineOfSight(float startX, float startY, float targetX, float targetY) {
		float dx = targetX - startX;
		float dy = targetY - startY;
		float dist = std::sqrt(dx * dx + dy * dy);

		int steps = static_cast<int>(dist / 16.f);
		if (steps <= 0) return true;

		auto allColliders = colliderManager->GetAllColliders();

		for (int i = 1; i < steps; i++) {
			float tx = startX + (dx * i / steps);
			float ty = startY + (dy * i / steps);

			for (const auto& c : allColliders) {
				if (c == this->collider) continue;
				if (auto player = targetPlayer.lock()) {
					if (c == player->GetCollider().lock()) continue;
				}

				if (auto rect = std::dynamic_pointer_cast<DX9GF::RectangleCollider>(c)) {
					float sx = std::abs(rect->GetWorldScaleX());
					float sy = std::abs(rect->GetWorldScaleY());

					float rx = rect->GetWorldX() - (rect->GetOriginX() * sx);
					float ry = rect->GetWorldY() - (rect->GetOriginY() * sy);

					float rw = rect->GetWidth() * sx;
					float rh = rect->GetHeight() * sy;

					if (tx >= rx && tx <= rx + rw && ty >= ry && ty <= ry + rh) {
						return false;
					}
				}
			}
		}
		return true;
	}
	void MapEnemy::SetEventState(EventType type) {
		encounterData.eventType = type;
		if (wealthyAura) wealthyAura->SetEnabled(type != EventType::None);
	}
}