#include "input.h"

Input::Input()
{
}

Input::Input(const Input& kOther)
{
}

Input::~Input()
{
}

void Input::Initialize()
{
	// Initialize all keys to a released state.
	for (int i = 0; i < 256; i++)
		keys_[i] = false;
}

void Input::KeyDown(unsigned int input)
{
	// If a key is pressed then save that state in the key array.
	keys_[input] = true;
}

void Input::KeyUp(unsigned int input)
{
	// If a key is released then clear the state in the key array.
	keys_[input] = false;
}

bool Input::IsKeyDown(unsigned int key)
{
	// Return the state of a key (pressed or released).
	return keys_[key];
}
