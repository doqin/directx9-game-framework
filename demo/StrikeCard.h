#pragma once
#include "IStatementCard.h"
#include "EnemyCard.h"

namespace Demo {
	class StrikeCard : public IStatementCard {
	private:
		std::weak_ptr<EnemyCard> enemyCard;
		bool isDone = false;
		std::shared_ptr<DX9GF::Font> nameFont;
		std::shared_ptr<DX9GF::FontSprite> nameFontSprite;
		std::shared_ptr<DX9GF::Texture> strikeTexture;
		std::shared_ptr<DX9GF::StaticSprite> strikeSprite;
	public:
		StrikeCard(std::weak_ptr<DX9GF::TransformManager> tm, float x = 0, float y = 0)
			: IGameObject(tm, x, y), IStatementCard(tm, 160, 32, x, y) {}
		bool OnDrop(std::shared_ptr<IDraggable> other) override;
		bool Execute() override;
		void ResetExecution() override;
		void Update(unsigned long long deltaTime) override;
		void Draw(unsigned long long deltaTime) override;
		std::wstring GetDescription() const override;
		std::wstring GetInputsDescription() const override;
		size_t GetCost() const override;
		size_t GetWidth() const override;
	};
}
