#include "pch.h"
#include "MapEnemy.h"
#include "MapBattleScene.h"

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

		texture = std::make_shared<DX9GF::Texture>(gd);
		texture->LoadTexture(encounterData.mapTexturePath.c_str());

		sprite = std::make_shared<DX9GF::AnimatedSprite>(texture.get(),
			DX9GF::Utils::CreateRectsHorizontal(0, 0, encounterData.spriteWidth, encounterData.spriteHeight, encounterData.frameCount), encounterData.frameCount);

		sprite->SetOrigin(encounterData.spriteWidth / 2.f, encounterData.spriteHeight / 2.f);

		this->SetLocalScale(0.4f, 0.4f);
		sprite->SetScale(0.4f, 0.4f);

		collider = std::make_shared<DX9GF::RectangleCollider>(
			transformManager,
			shared_from_this(),
			encounterData.spriteWidth,
			encounterData.spriteHeight,
			0, 0
		);
		collider->SetOriginCenter();
	}

	void MapEnemy::Update(unsigned long long deltaTime) {
		if (isDefeated) {
			respawnTimer -= deltaTime / 1000.f;
			if (respawnTimer <= 0) {
				isDefeated = false;
				SetLocalPosition(startX, startY);
				currentState = State::Idle;
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

					auto app = DX9GF::Application::GetInstance();
					auto sceMan = game->GetSceneManager();

					currentState = State::Idle;
					postBattleCooldown = 2.0f;

					colliderManager->Remove(collider);

					auto battleScene = new MapBattleScene(game, player, app->GetScreenWidth(), app->GetScreenHeight(), encounterData);

					battleScene->SetOnVictoryCallback([this]() {
						this->isDefeated = true;
						this->respawnTimer = 180.f;
						this->postBattleCooldown = 0.f;
						});

					sceMan->InsertScene(sceMan->GetIndex() + 1, battleScene);
					sceMan->GoToNext();
					return;
				}
			}

			// === XỬ LÝ TRẠNG THÁI AI VÀ GHI HÌNH VẾT CHÂN ===
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
				// THÊM: Tính toán khoảng cách giữa quái và nhà của nó
				float homeDx = startX - ex;
				float homeDy = startY - ey;
				float homeDistSq = homeDx * homeDx + homeDy * homeDy;

				// AI TETHERING: Bỏ cuộc nếu Player chạy quá xa HOẶC quái bị dắt ra khỏi vùng xích cổ
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

				// Ép quái phải về sát tận tâm điểm neo (< 10 pixel) mới được quay lại trạng thái Idle
				if (homeDistSq < 100.f) {
					currentState = State::Idle;
					returnPath.clear();
				}
				// ĐÃ XÓA TOÀN BỘ LOGIC CHO PHÉP RE-AGGRO Ở ĐÂY.
				// LUẬT THÉP: Một khi đã quay đầu là đi thẳng về nhà, cấm rượt bậy dọc đường!
			}

			// === TÍNH TOÁN HƯỚNG DI CHUYỂN (HYBRID AI) ===
			D3DXVECTOR2 dir{ 0, 0 };

			if (currentState == State::Chase) {
				float targetX = px;
				float targetY = py;

				if (CheckLineOfSight(ex, ey, px, py)) {
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
	}

	void MapEnemy::Draw(DX9GF::Camera* camera, unsigned long long deltaTime) {
		if (isDefeated) return;

		if (postBattleCooldown > 0) {
			if (static_cast<int>(postBattleCooldown * 10) % 2 == 0) return;
		}

		sprite->Begin();
		auto [x, y] = GetWorldPosition();
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
					float rx = rect->GetWorldX() - rect->GetOriginX();
					float ry = rect->GetWorldY() - rect->GetOriginY();
					float rw = rect->GetWidth() * std::abs(rect->GetWorldScaleX());
					float rh = rect->GetHeight() * std::abs(rect->GetWorldScaleY());

					if (tx >= rx && tx <= rx + rw && ty >= ry && ty <= ry + rh) {
						return false;
					}
				}
			}
		}
		return true;
	}
}