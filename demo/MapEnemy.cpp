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
			DX9GF::Utils::CreateRectsHorizontal(0, 0, encounterData.spriteWidth, encounterData.spriteHeight, 12), 12);

		sprite->SetOrigin(encounterData.spriteWidth / 2.f, encounterData.spriteHeight / 2.f);

		this->SetLocalScale(0.4f, 0.4f);
		sprite->SetScale(0.4f, 0.4f);

		// Khởi tạo Collider to bằng đúng khung hình ảnh gốc
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
		// 1. Logic hồi sinh
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
			// 2. Logic Cooldown (Ghost Mode)
			if (postBattleCooldown > 0) {
				postBattleCooldown -= deltaTime / 1000.f;
				// Nếu đang choáng thì ngưng luôn, Player đi xuyên qua thoải mái
				if (postBattleCooldown > 0) return;
			}

			auto [px, py] = player->GetWorldPosition();
			auto [ex, ey] = GetWorldPosition();

			float dx = px - ex;
			float dy = py - ey;
			float distanceSq = dx * dx + dy * dy;

			// TÍNH TOÁN TRIGGER BẰNG TỌA ĐỘ TUYỆT ĐỐI CỦA COLLIDER (CLEAN 100%)
			auto pCol = std::dynamic_pointer_cast<DX9GF::RectangleCollider>(player->GetCollider().lock());
			if (pCol && this->collider) {
				// Hàm lambda bóc tách 4 cạnh (Trái, Phải, Trên, Dưới) chính xác của hộp vật lý
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

				// Nới rộng vùng kiểm tra thêm 2.0f pixel (Epsilon) 
				// Đảm bảo kích hoạt ngay sát mép trước khi hệ thống vật lý kịp chặn lại
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
					// TỰ ĐỘNG TÌM ĐƯỜNG VỀ: Đang Idle mà thấy lạc xa nhà quá 20 pixel (400.f) thì tự đi về
					float homeDx = startX - ex;
					float homeDy = startY - ey;
					if (homeDx * homeDx + homeDy * homeDy > 400.f) {
						currentState = State::Return;
					}
				}
			}
			else if (currentState == State::Chase) {
				if (distanceSq > returnRadius * returnRadius) {
					currentState = State::Return;
				}
				else {
					// Lén ghi lại vết chân của Player mỗi 32 pixel để xài lúc "mù"
					float pDistX = px - lastPlayerPos.x;
					float pDistY = py - lastPlayerPos.y;
					if (pDistX * pDistX + pDistY * pDistY > 1024.f) {
						chasePath.push_back({ px, py });
						lastPlayerPos = { px, py };
						if (chasePath.size() > 20) chasePath.pop_front();
					}

					// Ghi lại vết chân của chính Enemy để tìm đường lùi về nhà an toàn
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

				if (homeDx * homeDx + homeDy * homeDy < 400.f) {
					currentState = State::Idle;
					returnPath.clear();
				}
				else if (distanceSq < aggroRadius * aggroRadius) {
					currentState = State::Chase;
					chasePath.clear();
					lastPlayerPos = { px, py };
				}
			}

			// === TÍNH TOÁN HƯỚNG DI CHUYỂN (HYBRID AI) ===
			D3DXVECTOR2 dir{ 0, 0 };

			if (currentState == State::Chase) {
				float targetX = px;
				float targetY = py;

				// KIỂM TRA HYBRID: Mắt có nhìn thấy người chơi không?
				if (CheckLineOfSight(ex, ey, px, py)) {
					// Sáng mắt: Chạy thẳng tắp, không thèm nhìn vết chân
					chasePath.clear();
				}
				else {
					// Mù (bị tường che): Cúi xuống dò theo mảng vết bánh mì
					if (!chasePath.empty()) {
						targetX = chasePath.front().x;
						targetY = chasePath.front().y;

						float tdx = targetX - ex;
						float tdy = targetY - ey;
						// Điểm neo này đã đến nơi, nhả nó ra để dò điểm tiếp theo
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

				// Thu cuộn dây an toàn để đi lùi về nhà né tường
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

			// Trượt tường và lật hình
			if (dir.x != 0 || dir.y != 0) {
				float moveX = dir.x * speed * (deltaTime / 1000.f);
				float moveY = dir.y * speed * (deltaTime / 1000.f);
				auto [finalDX, finalDY] = colliderManager->GetSlidingDeltas(collider, moveX, moveY);
				SetLocalPosition(ex + finalDX, ey + finalDY);

				// KHÔNG DÙNG HARDCODE: Lấy scale tĩnh từ IGameObject ra để lật
				float baseScaleX = std::abs(this->GetLocalScaleX());
				float baseScaleY = std::abs(this->GetLocalScaleY());

				// Lật ngang (đảo dấu trục X), bắt buộc giữ nguyên trục Y (baseScaleY) để không bị lộn ngược
				sprite->SetScale(dir.x > 0 ? -baseScaleX : baseScaleX, baseScaleY);
			}
		}
	}

	void MapEnemy::Draw(DX9GF::Camera* camera, unsigned long long deltaTime) {
		if (isDefeated) return;

		// Hiệu ứng nhấp nháy nếu đang bị "choáng" sau khi Flee
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

		// Bắn các điểm quét dọc theo đường thẳng, mỗi điểm cách nhau 16 pixel
		int steps = static_cast<int>(dist / 16.f);
		if (steps <= 0) return true;

		auto allColliders = colliderManager->GetAllColliders();

		for (int i = 1; i < steps; i++) {
			float tx = startX + (dx * i / steps);
			float ty = startY + (dy * i / steps);

			// Kiểm tra xem điểm (tx, ty) có lọt vào hộp va chạm của bức tường nào không
			for (const auto& c : allColliders) {
				if (c == this->collider) continue;
				if (auto player = targetPlayer.lock()) {
					if (c == player->GetCollider().lock()) continue;
				}

				if (auto rect = std::dynamic_pointer_cast<DX9GF::RectangleCollider>(c)) {
					// Tính ranh giới của tường
					float rx = rect->GetWorldX() - rect->GetOriginX();
					float ry = rect->GetWorldY() - rect->GetOriginY();
					float rw = rect->GetWidth() * std::abs(rect->GetWorldScaleX());
					float rh = rect->GetHeight() * std::abs(rect->GetWorldScaleY());

					if (tx >= rx && tx <= rx + rw && ty >= ry && ty <= ry + rh) {
						return false; // Mù: Bị tường che khuất
					}
				}
			}
		}
		return true; // Sáng mắt: Đường thẳng tắp không có vật cản
	}
}