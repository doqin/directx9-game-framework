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
		bool HasRequiredTargets() const override;
		void ResetExecution() override;
		void Update(unsigned long long deltaTime) override;
		void Draw(unsigned long long deltaTime) override;
		std::wstring GetDescription() const override;
		RECT GetFaceRect() const override { return RECT{ 0, 288, 80, 304 }; }
		std::wstring GetInputsDescription() const override;
		size_t GetCost() const override;
		size_t GetWidth() const override;
		bool CanAcceptEnemyCard() const override { return !enemyCard.lock(); }
		bool AttachEnemyCard(std::shared_ptr<EnemyCard> card) override;
		void ReleaseEnemyCards() override;
		std::tuple<float, float> GetEnemyCardSlotWorldPosition() const override;
	};
}
