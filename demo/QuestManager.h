#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"
#include "IconButton.h"
#include <string>

namespace Demo {
	class QuestManager {
		std::wstring questText;
		bool isExpanded = true;
		bool isVisible = true;
		std::shared_ptr<DX9GF::Font> font;
		std::shared_ptr<DX9GF::FontSprite> fontSprite;
		std::shared_ptr<DX9GF::Texture> uiTex;
		std::shared_ptr<IconButton> btnToggle;
		QuestManager() = default;
	public:
		static QuestManager* GetInstance() {
			static QuestManager instance;
			return &instance;
		}
		void Init(DX9GF::GraphicsDevice* gd, std::shared_ptr<DX9GF::TransformManager> tm, DX9GF::Camera* uiCamera, std::shared_ptr<DX9GF::Font> font);
		void SetQuest(const std::wstring& text) { questText = text; }
		const std::wstring& GetQuestText() const { return questText; }

		void SetVisible(bool v) { isVisible = v; }  
		bool IsVisible() const { return isVisible; }

		void Update(unsigned long long deltaTime);
		void Draw(DX9GF::GraphicsDevice* gd, DX9GF::Camera* uiCamera, unsigned long long deltaTime);
	};
}