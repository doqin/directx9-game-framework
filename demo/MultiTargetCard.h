#pragma once
#include "IStatementCard.h"
#include "EnemyCard.h"
#include <vector>
#include <string>

namespace Demo {
	class MultiTargetCard : public IStatementCard {
	protected:
		std::vector<std::weak_ptr<EnemyCard>> targets;
		size_t maxTargets;
		std::wstring cardName;
		bool isDone = false;
		std::shared_ptr<DX9GF::Font> nameFont;
		std::shared_ptr<DX9GF::FontSprite> nameFontSprite;
	public:
		MultiTargetCard(std::weak_ptr<DX9GF::TransformManager> tm, size_t maxTargets, std::wstring name, float x = 0, float y = 0, size_t dragAreaWidth = 0, size_t dragAreaHeight = 0);

		bool OnDrop(std::shared_ptr<IDraggable> other) override;
		void Update(unsigned long long deltaTime) override;
		virtual void Draw(unsigned long long deltaTime) override;
		size_t GetWidth() const override;
		void ResetExecution() override;
		std::wstring GetInputsDescription() const override;
	};
}