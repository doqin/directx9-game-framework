#pragma once
#include <string>
#include <optional>
#include <memory>
#include <DX9GFISprite.h>

namespace Demo {
	struct DialogueLine {
		std::optional<std::shared_ptr<DX9GF::ISprite>> left;
		std::optional<std::shared_ptr<DX9GF::ISprite>> right;
		std::wstring name;
		std::wstring content;
		std::optional<std::string> voiceClip; // Optional voice clip associated with the line
	};
}
