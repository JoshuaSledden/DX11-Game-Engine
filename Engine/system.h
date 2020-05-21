#pragma once
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include "input.h"
#include "graphics.h"

class System
{
public:
	System();
	System(const System&);
	~System();

	bool Initialize();
	void Shutdown();
	void Run();

	// Message handler to handle incoming windows system messages.
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

private:
	LPCWSTR application_name_;
	HINSTANCE instance_;
	HWND window_;

	Input* input_;
	Graphics* graphics_;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static System* ApplicationHandle = 0;