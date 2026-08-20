#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"

namespace Demo {
	class IBattleScene;
	class Player;

	class IToken : public DX9GF::IGameObject {
	protected:
		bool isCollected = false;
		std::shared_ptr<DX9GF::RectangleCollider> collider;

	public:
		IToken(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y)
			: DX9GF::IGameObject(tm, x, y) {}
		virtual ~IToken() = default;

		virtual void Update(unsigned long long deltaTime) = 0;
		virtual void Draw(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera, unsigned long long deltaTime) = 0;
		virtual void OnCollect(Player* player, IBattleScene* scene) = 0;
		virtual bool IsDone() const = 0;

		bool IsCollected() const { return isCollected; }
		std::weak_ptr<DX9GF::RectangleCollider> GetCollider() { return collider; }
	};
}