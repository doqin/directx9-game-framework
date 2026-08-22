#pragma once
#include "DX9GFExtras.h"
#include "DX9GF.h"
#include <deque>
#include <string>
#include "Game.h"

namespace Demo {
	/// <summary>
	/// The stack of short-lived messages at the top of the screen, plus a scrollable log of
	/// everything that has been shown since the log was last cleared.
	///
	/// The animation is driven by Update() instead of by commands on a CommandBuffer, so several
	/// messages can share the screen - including ones pushed from different buffers, e.g. the
	/// scene's and an enemy's - and clearing a buffer can no longer strand a message on screen.
	/// </summary>
	class PopUpMessage : public DX9GF::IGameObject {
	public:
		struct HistoryEntry {
			unsigned long long stampMs; // milliseconds since the log was last cleared
			int turn;                   // 0 when the owner never reported a turn
			std::wstring text;
			int repeatCount;
		};
	private:
		enum class Phase {
			Entering,
			Holding,
			Leaving
		};
		struct Line {
			std::wstring text;        // as pushed, without the repeat suffix
			std::wstring displayText; // what actually gets drawn
			float width = 0.f;
			float height = 0.f;
			float holdMs = 0.f;  // hold time left once the line has finished entering
			float ageMs = 0.f;   // time spent entering
			float exitMs = 0.f;  // time spent leaving
			float exitStartAlpha = 255.f;
			float y = 0.f;       // current offset from the anchor, eased toward targetY
			float targetY = 0.f;
			float alpha = 0.f;
			int repeatCount = 1;
			Phase phase = Phase::Entering;
			bool positioned = false; // false until the first layout hands it a slot
		};

		// Line animation
		static constexpr float ENTER_MS = 250.f;
		static constexpr float EXIT_MS = 300.f;
		static constexpr float ENTER_SLIDE = 24.f;     // pixels a line drops in from
		static constexpr float EXIT_SLIDE = 24.f;      // pixels a line rises while fading out
		static constexpr float LINE_GAP = 4.f;
		static constexpr float ANCHOR_MARGIN = 20.f;   // gap between the top edge and the first line
		static constexpr float RESHUFFLE_SPEED = 12.f; // higher = the stack collapses faster
		static constexpr D3DCOLOR TEXT_COLOR = 0xFFfffc40;
		static constexpr size_t MAX_VISIBLE_LINES = 5;

		// History log
		static constexpr size_t MAX_HISTORY_ENTRIES = 64;
		static constexpr size_t HISTORY_VISIBLE_ROWS = 10;
		static constexpr float HISTORY_WIDTH = 360.f;
		static constexpr float HISTORY_PADDING = 8.f;
		static constexpr float HISTORY_MARGIN = 12.f;  // gap between the panel and the screen edge

		DX9GF::GraphicsDevice* graphicsDevice = nullptr;
		DX9GF::Camera* camera = nullptr;
		std::shared_ptr<DX9GF::Font> font;
		std::shared_ptr<DX9GF::FontSprite> fontSprite;
		float lineHeight = 0.f;

		std::deque<Line> lines;
		std::deque<HistoryEntry> history;
		unsigned long long elapsedMs = 0;
		int currentTurn = 0;

		bool isHistoryOpen = false;
		size_t scrollOffset = 0;
		bool stickToLatest = true;

		void MeasureLine(Line& line);
		void RelayoutLines();
		float GetAnchorY() const;
		float GetSlideOffset(const Line& line) const;
		size_t GetMaxScrollOffset() const;
		void ScrollToLatest();
		std::wstring FormatEntry(const HistoryEntry& entry) const;
		std::wstring TruncateToWidth(const std::wstring& text, float maxWidth) const;
		Game* game = nullptr;
	public:
		PopUpMessage(std::weak_ptr<DX9GF::TransformManager> tm, Game* game) : IGameObject(tm), game(game) {}
		void Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera);

		/// <summary>
		/// Shows a message right away. Nothing waits on it - use this for feedback that answers
		/// something the player just did.
		/// </summary>
		void ShowMessage(const std::wstring& message, float duration = 1.0f);
		/// <summary>
		/// Shows a message from a command buffer, keeping that buffer busy for the duration so
		/// the surrounding sequence still paces around the message.
		/// </summary>
		void QueueMessage(DX9GF::CommandBuffer* commandBuffer, std::wstring message, float duration = 1.0f);

		void Update(unsigned long long deltaTime);
		void Draw(unsigned long long deltaTime);

		/// <summary>
		/// Clears what is currently on screen. The log is kept.
		/// </summary>
		void Reset();
		void ClearHistory();
		void SetCurrentTurn(int turn) { currentTurn = turn; }

		// History overlay
		void ToggleHistory();
		bool IsHistoryOpen() const { return isHistoryOpen; }
		void UpdateHistory();
		void DrawHistory(DX9GF::Camera& uiCamera, unsigned long long deltaTime);
		const std::deque<HistoryEntry>& GetHistory() const { return history; }
	};
}
