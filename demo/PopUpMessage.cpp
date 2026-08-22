#include "pch.h"
#include "PopUpMessage.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace {
	std::wstring FormatTwoDigits(unsigned long long value)
	{
		std::wstring text = std::to_wstring(value);
		if (text.size() < 2) text.insert(text.begin(), L'0');
		return text;
	}

	float EaseOutCubic(float t)
	{
		const float inverted = 1.f - t;
		return 1.f - inverted * inverted * inverted;
	}
}

void Demo::PopUpMessage::Init(DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera)
{
	this->graphicsDevice = graphicsDevice;
	this->camera = camera;
	font = std::make_shared<DX9GF::Font>(graphicsDevice, L"StatusPlz", 16);
	fontSprite = std::make_shared<DX9GF::FontSprite>(font.get());
	fontSprite->SetColor(0xFF000000);
	// Measured once on a sample carrying both an ascender and a descender: every slot in the
	// stack has to be the same height whatever glyphs happen to land in it.
	fontSprite->SetText(L"Ay");
	lineHeight = static_cast<float>(fontSprite->GetHeight());
}

void Demo::PopUpMessage::ShowMessage(const std::wstring& message, float duration)
{
	if (message.empty() || !fontSprite) return;

	const float holdMs = (std::max)(0.f, duration) * 1000.f;

	// Repeats of the newest line fold into it rather than filling the stack - hammering a button
	// that answers "Not enough energy" should read as one line counting up.
	if (!lines.empty() && lines.back().phase != Phase::Leaving && lines.back().text == message) {
		auto& line = lines.back();
		++line.repeatCount;
		line.holdMs = holdMs;
		line.displayText = message + L" x" + std::to_wstring(line.repeatCount);
		MeasureLine(line);

		if (!history.empty() && history.back().text == message) {
			history.back().repeatCount = line.repeatCount;
			history.back().stampMs = elapsedMs;
		}
		if (stickToLatest) ScrollToLatest();
		return;
	}

	Line line;
	line.text = message;
	line.displayText = message;
	line.holdMs = holdMs;
	MeasureLine(line);
	lines.push_back(std::move(line));

	// Keep the stack shallow: the oldest line starts leaving early rather than the newest one
	// arriving below the visible area.
	size_t visible = 0;
	for (auto& stacked : lines) {
		if (stacked.phase != Phase::Leaving) ++visible;
	}
	for (auto& stacked : lines) {
		if (visible <= MAX_VISIBLE_LINES) break;
		if (stacked.phase == Phase::Leaving) continue;
		stacked.phase = Phase::Leaving;
		stacked.exitMs = 0.f;
		stacked.exitStartAlpha = stacked.alpha;
		--visible;
	}

	history.push_back(HistoryEntry{ elapsedMs, currentTurn, message, 1 });
	while (history.size() > MAX_HISTORY_ENTRIES) {
		history.pop_front();
		if (scrollOffset > 0) --scrollOffset;
	}
	if (stickToLatest) ScrollToLatest();

	RelayoutLines();
}

void Demo::PopUpMessage::QueueMessage(DX9GF::CommandBuffer* commandBuffer, std::wstring message, float duration)
{
	if (commandBuffer == nullptr) {
		ShowMessage(message, duration);
		return;
	}

	auto commands = std::make_shared<DX9GF::MultiCommand>(std::vector<std::shared_ptr<DX9GF::ICommand>>{
		std::make_shared<DX9GF::CustomCommand>([this, message, duration](std::function<void(void)> markFinished) {
			ShowMessage(message, duration);
			markFinished();
		}),
		// The line animates on its own now, so this delay is purely about pacing: a sequence
		// that used to wait for the pop-up still waits about as long.
		std::make_shared<DX9GF::DelayCommand>(duration)
	});
	commandBuffer->PushCommand(commands);
}

void Demo::PopUpMessage::Update(unsigned long long deltaTime)
{
	elapsedMs += deltaTime;

	const float dt = static_cast<float>(deltaTime);
	for (auto& line : lines) {
		switch (line.phase) {
		case Phase::Entering:
			line.ageMs += dt;
			if (line.ageMs >= ENTER_MS) {
				line.ageMs = ENTER_MS;
				line.phase = Phase::Holding;
			}
			break;
		case Phase::Holding:
			line.holdMs -= dt;
			if (line.holdMs <= 0.f) {
				line.phase = Phase::Leaving;
				line.exitMs = 0.f;
				line.exitStartAlpha = line.alpha;
			}
			break;
		case Phase::Leaving:
			line.exitMs += dt;
			break;
		}
	}

	lines.erase(std::remove_if(lines.begin(), lines.end(), [](const Line& line) {
		return line.phase == Phase::Leaving && line.exitMs >= EXIT_MS;
		}), lines.end());

	RelayoutLines();

	const float smoothing = 1.f - std::exp(-RESHUFFLE_SPEED * dt / 1000.f);
	for (auto& line : lines) {
		line.y += (line.targetY - line.y) * smoothing;

		switch (line.phase) {
		case Phase::Entering:
			line.alpha = 255.f * (line.ageMs / ENTER_MS);
			break;
		case Phase::Holding:
			line.alpha = 255.f;
			break;
		case Phase::Leaving:
			line.alpha = line.exitStartAlpha * (1.f - (std::min)(1.f, line.exitMs / EXIT_MS));
			break;
		}
	}
}

void Demo::PopUpMessage::Draw(unsigned long long deltaTime)
{
	if (lines.empty() || !fontSprite || camera == nullptr) return;

	const float worldX = GetWorldX();
	const float worldY = GetWorldY();

	fontSprite->Begin();
	for (auto& line : lines) {
		const int alpha = static_cast<int>((std::max)(0.f, (std::min)(255.f, line.alpha)));
		if (alpha <= 0) continue;

		const D3DCOLOR alphaBits = static_cast<D3DCOLOR>(alpha) << 24;
		fontSprite->SetText(line.displayText);
		fontSprite->SetColor(alphaBits | (TEXT_COLOR & 0x00FFFFFF));
		fontSprite->SetOutline(true, alphaBits, 2.f);
		fontSprite->SetPosition(
			worldX - line.width / 2.f,
			worldY + line.y + GetSlideOffset(line) - line.height / 2.f);
		fontSprite->Draw(*camera, deltaTime);
	}
	fontSprite->End();
	fontSprite->SetOutline(false);
}

void Demo::PopUpMessage::Reset()
{
	lines.clear();
}

void Demo::PopUpMessage::ClearHistory()
{
	history.clear();
	elapsedMs = 0;
	scrollOffset = 0;
	stickToLatest = true;
}

void Demo::PopUpMessage::ToggleHistory()
{
	isHistoryOpen = !isHistoryOpen;
	if (isHistoryOpen) ScrollToLatest();
}

void Demo::PopUpMessage::UpdateHistory()
{
	if (!isHistoryOpen) return;

	const long scroll = DX9GF::InputManager::GetInstance()->GetMouseScroll();
	if (scroll != 0) {
		// One wheel notch is 120 units; walk a row per notch, up toward the older entries.
		size_t rows = static_cast<size_t>(std::abs(scroll) / 120);
		if (rows == 0) rows = 1;

		if (scroll > 0) {
			scrollOffset = scrollOffset > rows ? scrollOffset - rows : 0;
		}
		else {
			scrollOffset = (std::min)(GetMaxScrollOffset(), scrollOffset + rows);
		}
		stickToLatest = scrollOffset >= GetMaxScrollOffset();
	}

	// The log keeps growing while the panel is open.
	const size_t maxOffset = GetMaxScrollOffset();
	if (stickToLatest || scrollOffset > maxOffset) scrollOffset = maxOffset;
}

void Demo::PopUpMessage::DrawHistory(DX9GF::Camera& uiCamera, unsigned long long deltaTime)
{
	if (!isHistoryOpen || !fontSprite || graphicsDevice == nullptr) return;
	const float screenWidth = static_cast<float>(game->GetVirtualWidth());
	const float screenHeight = static_cast<float>(game->GetVirtualHeight());

	const float rowHeight = lineHeight + 2.f;
	const size_t rowCount = (std::max)(size_t{ 1 }, (std::min)(HISTORY_VISIBLE_ROWS, history.size()));
	const float panelWidth = HISTORY_WIDTH;
	const float panelHeight = HISTORY_PADDING * 2.f + rowHeight * static_cast<float>(rowCount + 1); // +1 for the title row
	const float panelX = screenWidth / 2.f - panelWidth - HISTORY_MARGIN;
	const float panelY = -screenHeight / 2.f + HISTORY_MARGIN;

	graphicsDevice->SetAlphaBlending(true);
	graphicsDevice->DrawRectangle(uiCamera, panelX, panelY, panelWidth, panelHeight, 0xDD1a1a2e, true);
	graphicsDevice->DrawRectangle(uiCamera, panelX, panelY, panelWidth, panelHeight, 0xFFdae0ea, false);
	graphicsDevice->DrawRectangle(uiCamera, panelX - 1.f, panelY - 1.f, panelWidth + 2.f, panelHeight + 2.f, 0xFF000000, false);
	graphicsDevice->SetAlphaBlending(false);

	const float textX = panelX + HISTORY_PADDING;
	const float maxTextWidth = panelWidth - HISTORY_PADDING * 2.f;
	float textY = panelY + HISTORY_PADDING;

	fontSprite->Begin();
	fontSprite->SetOutline(true, 0xFF000000, 2.f);

	std::wstring title = L"BATTLE LOG";
	if (history.size() > rowCount) {
		title += L"  " + std::to_wstring(scrollOffset + rowCount) + L"/" + std::to_wstring(history.size());
	}
	fontSprite->SetColor(TEXT_COLOR);
	fontSprite->SetText(TruncateToWidth(title, maxTextWidth));
	fontSprite->SetPosition(textX, textY);
	fontSprite->Draw(uiCamera, deltaTime);
	textY += rowHeight;

	if (history.empty()) {
		fontSprite->SetColor(0xFFb0b6c4);
		fontSprite->SetText(L"(nothing yet)");
		fontSprite->SetPosition(textX, textY);
		fontSprite->Draw(uiCamera, deltaTime);
	}
	else {
		fontSprite->SetColor(0xFFFFFFFF);
		for (size_t i = 0; i < rowCount; ++i) {
			const size_t index = scrollOffset + i;
			if (index >= history.size()) break;
			fontSprite->SetText(TruncateToWidth(FormatEntry(history[index]), maxTextWidth));
			fontSprite->SetPosition(textX, textY);
			fontSprite->Draw(uiCamera, deltaTime);
			textY += rowHeight;
		}
	}

	fontSprite->End();
	fontSprite->SetOutline(false);
	fontSprite->SetColor(0xFFFFFFFF);
}

void Demo::PopUpMessage::MeasureLine(Line& line)
{
	fontSprite->SetText(line.displayText);
	line.width = static_cast<float>(fontSprite->GetWidth());
	line.height = static_cast<float>(fontSprite->GetHeight());
}

void Demo::PopUpMessage::RelayoutLines()
{
	const float anchorY = GetAnchorY();
	size_t slot = 0;
	for (auto& line : lines) {
		// A leaving line keeps the position it had, so the lines under it slide up through it
		// instead of waiting for the fade to finish.
		if (line.phase != Phase::Leaving) {
			line.targetY = anchorY + static_cast<float>(slot) * (lineHeight + LINE_GAP);
			++slot;
		}
		if (!line.positioned) {
			line.y = line.targetY;
			line.positioned = true;
		}
	}
}

float Demo::PopUpMessage::GetAnchorY() const
{
	float screenHeight = 0.f;
	if (camera != nullptr) {
		auto [resWidth, resHeight] = camera->GetScreenResolution();
		screenHeight = static_cast<float>(resHeight);
	}
	else if (auto app = DX9GF::Application::GetInstance(); app != nullptr) {
		screenHeight = static_cast<float>(app->GetScreenHeight());
	}
	return -screenHeight / 2.f + lineHeight + ANCHOR_MARGIN;
}

float Demo::PopUpMessage::GetSlideOffset(const Line& line) const
{
	switch (line.phase) {
	case Phase::Entering: {
		const float t = (std::min)(1.f, line.ageMs / ENTER_MS);
		return -ENTER_SLIDE * (1.f - EaseOutCubic(t));
	}
	case Phase::Leaving: {
		const float t = (std::min)(1.f, line.exitMs / EXIT_MS);
		return -EXIT_SLIDE * EaseOutCubic(t);
	}
	default:
		return 0.f;
	}
}

size_t Demo::PopUpMessage::GetMaxScrollOffset() const
{
	return history.size() > HISTORY_VISIBLE_ROWS ? history.size() - HISTORY_VISIBLE_ROWS : 0;
}

void Demo::PopUpMessage::ScrollToLatest()
{
	scrollOffset = GetMaxScrollOffset();
	stickToLatest = true;
}

std::wstring Demo::PopUpMessage::FormatEntry(const HistoryEntry& entry) const
{
	const unsigned long long totalSeconds = entry.stampMs / 1000ull;
	std::wstring text = L"[" + FormatTwoDigits(totalSeconds / 60ull) + L":" + FormatTwoDigits(totalSeconds % 60ull) + L"] ";
	if (entry.turn > 0) {
		text += L"T" + std::to_wstring(entry.turn) + L" ";
	}
	text += entry.text;
	if (entry.repeatCount > 1) {
		text += L" x" + std::to_wstring(entry.repeatCount);
	}
	return text;
}

std::wstring Demo::PopUpMessage::TruncateToWidth(const std::wstring& text, float maxWidth) const
{
	fontSprite->SetText(text);
	const float width = static_cast<float>(fontSprite->GetWidth());
	if (text.empty() || width <= maxWidth) return text;

	// One proportional guess, then trim a character at a time. Measuring is a DrawText call, so
	// it is not something to run once per character over a long line.
	size_t keep = static_cast<size_t>(static_cast<float>(text.size()) * (maxWidth / width));
	keep = keep > 3 ? keep - 3 : 1;

	std::wstring trimmed = text.substr(0, keep) + L"...";
	fontSprite->SetText(trimmed);
	while (keep > 1 && static_cast<float>(fontSprite->GetWidth()) > maxWidth) {
		--keep;
		trimmed = text.substr(0, keep) + L"...";
		fontSprite->SetText(trimmed);
	}
	return trimmed;
}
