#include "pch.h"
#include "TextInputManager.h"
#include <DX9GFInputManager.h>
#include <dinput.h>

Demo::TextInputManager* Demo::TextInputManager::instance = nullptr;

Demo::TextInputManager::TextInputManager() {}

Demo::TextInputManager* Demo::TextInputManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new TextInputManager();
	}
	return instance;
}

struct KeyMap {
	int dik;
	char lower;
	char upper;
};

static const KeyMap keyMappings[] = {
	{ DIK_A, 'a', 'A' }, { DIK_B, 'b', 'B' }, { DIK_C, 'c', 'C' }, { DIK_D, 'd', 'D' },
	{ DIK_E, 'e', 'E' }, { DIK_F, 'f', 'F' }, { DIK_G, 'g', 'G' }, { DIK_H, 'h', 'H' },
	{ DIK_I, 'i', 'I' }, { DIK_J, 'j', 'J' }, { DIK_K, 'k', 'K' }, { DIK_L, 'l', 'L' },
	{ DIK_M, 'm', 'M' }, { DIK_N, 'n', 'N' }, { DIK_O, 'o', 'O' }, { DIK_P, 'p', 'P' },
	{ DIK_Q, 'q', 'Q' }, { DIK_R, 'r', 'R' }, { DIK_S, 's', 'S' }, { DIK_T, 't', 'T' },
	{ DIK_U, 'u', 'U' }, { DIK_V, 'v', 'V' }, { DIK_W, 'w', 'W' }, { DIK_X, 'x', 'X' },
	{ DIK_Y, 'y', 'Y' }, { DIK_Z, 'z', 'Z' },
	{ DIK_0, '0', ')' }, { DIK_1, '1', '!' }, { DIK_2, '2', '@' }, { DIK_3, '3', '#' },
	{ DIK_4, '4', '$' }, { DIK_5, '5', '%' }, { DIK_6, '6', '^' }, { DIK_7, '7', '&' },
	{ DIK_8, '8', '*' }, { DIK_9, '9', '(' },
	{ DIK_SPACE, ' ', ' ' }, { DIK_BACK, '\b', '\b' },
	{ DIK_MINUS, '-', '_' }, { DIK_EQUALS, '=', '+' },
	{ DIK_LBRACKET, '[', '{' }, { DIK_RBRACKET, ']', '}' },
	{ DIK_SEMICOLON, ';', ':' }, { DIK_APOSTROPHE, '\'', '\"' },
	{ DIK_COMMA, ',', '<' }, { DIK_PERIOD, '.', '>' }, { DIK_SLASH, '/', '?' },
	{ DIK_BACKSLASH, '\\', '|' },
	{ DIK_NUMPAD0, '0', '0' }, { DIK_NUMPAD1, '1', '1' }, { DIK_NUMPAD2, '2', '2' },
	{ DIK_NUMPAD3, '3', '3' }, { DIK_NUMPAD4, '4', '4' }, { DIK_NUMPAD5, '5', '5' },
	{ DIK_NUMPAD6, '6', '6' }, { DIK_NUMPAD7, '7', '7' }, { DIK_NUMPAD8, '8', '8' },
	{ DIK_NUMPAD9, '9', '9' }
};

std::vector<char> Demo::TextInputManager::ReadInput()
{
	std::vector<char> inputChars;
	auto input = DX9GF::InputManager::GetInstance();

	bool isShiftDown = input->KeyPress(DIK_LSHIFT) || input->KeyPress(DIK_RSHIFT);

	for (const auto& key : keyMappings) {
		if (input->KeyDown(key.dik)) {
			inputChars.push_back(isShiftDown ? key.upper : key.lower);

			input->ConsumeKey(key.dik);
		}
	}

	return inputChars;
}