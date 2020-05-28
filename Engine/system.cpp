#include "system.h"

System::System() :
	input_(0),
	graphics_(0)
{
}

System::System(const System& kOther)
{
}

System::~System()
{
}

bool System::Initialize()
{
	int screen_width = 0, screen_height = 0;

	// Initialize the Windows API.
	InitializeWindows(screen_width, screen_height);

	// Create the Input object.
	// The Input object will be used to handle input from the user.
	input_ = new Input();
	if (!input_)
		return false;

	// Initialize the Input object.
	input_->Initialize();

	// Create the Graphics object.
	// The Graphics object will handle rendering all graphics for the application.
	graphics_ = new Graphics();
	if (!graphics_)
		return false;

	// Initialize the Graphics object and return its result.
	return graphics_->Initialize(screen_width, screen_height, window_);
}

void System::Shutdown()
{
	// Shutdown and Release the Graphics object.
	if (graphics_)
	{
		graphics_->Shutdown();
		delete graphics_;
		graphics_ = 0;
	}

	// Release the Input object.
	if (input_)
	{
		delete input_;
		input_ = 0;
	}

	// Shutdown the Windows API.
	ShutdownWindows();
}

void System::Run()
{
	// Initialize the message structure.
	MSG message;
	ZeroMemory(&message, sizeof(MSG));

	// Loop until there is a quit message from the window or the user.
	bool quit = false, result;
	while (!quit)
	{
		// Handle the window messages.
		if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		
		// If Windows signals to end the application then tell the application to quit.
		if (message.message == WM_QUIT)
		{
			quit = true;
		}
		else
		{
			// Do any frame processing.
			result = Frame();

			// If there were any issues during Frame processing we will tell the application to quit.
			if (!result)
				quit = true;
		}
	}
}

bool System::Frame()
{
	// Check if the user pressed the escape key and wants to exit the application.
	if (input_->IsKeyDown(VK_ESCAPE))
		return false;

	// Do Graphics frame processing.
	bool result = graphics_->Frame();

	return result;
}

LRESULT CALLBACK System::MessageHandler(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	// Check for a keyboard key press.
	case WM_KEYDOWN:
		// If a key is pressed - Send it to the Input object so it can record that state.
		input_->KeyDown(static_cast<UINT>(wparam));
		return 0;
	case WM_KEYUP:
		// If a key is released - Send it to the Input object so it can unset the state of the key.
		input_->KeyUp(static_cast<UINT>(wparam));
		return 0;

	// Any other messages send to the default message handler.
	default:
		return DefWindowProc(window, message, wparam, lparam);
	}
}

void System::InitializeWindows(int& screen_width, int& screen_height)
{
	// Get an external pointer to this object.
	ApplicationHandle = this;

	// Get the instance of this application.
	instance_ = GetModuleHandle(0);

	// Give the application a name.
	application_name_ = L"Game Engine";

	// Setup the Windows class with default settings.
	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instance_;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = application_name_;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	// Determine the resolution of the clients desktop screen.
	screen_width = GetSystemMetrics(SM_CXSCREEN);
	screen_height = GetSystemMetrics(SM_CYSCREEN);

	//Setup the screen settings depending on whether it is running full-screen or windowed mode.
	int position_x, position_y;
	DEVMODE screen_settings;
	if (FULL_SCREEN)
	{
		// Set the screen size to the maximum user desktop size and establish 32 bits.
		memset(&screen_settings, 0, sizeof(screen_settings));

		screen_settings.dmSize = sizeof(screen_settings);
		screen_settings.dmPelsWidth = static_cast<ULONG>(screen_width);
		screen_settings.dmPelsHeight = static_cast<ULONG>(screen_height);
		screen_settings.dmBitsPerPel = 32;
		screen_settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&screen_settings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		position_x = position_y = 0;
	}
	else
	{
		// If in windowed mode - set the default resolution.
		screen_width = 800;
		screen_height = 600;

		// Place the window in the centre of the screen.
		position_x = (GetSystemMetrics(SM_CXSCREEN) - screen_width) / 2;
		position_y = (GetSystemMetrics(SM_CYSCREEN) - screen_height) / 2;
	}

	// Create the window using the designated screen settings above then return the handle.
	window_ = CreateWindowEx(
		WS_EX_APPWINDOW, application_name_, application_name_,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP, position_x, position_y, 
		screen_width, screen_height, 0,
		0, instance_, 0
	);

	// Show the window on-screen and set it as the focus.
	ShowWindow(window_, SW_SHOW);
	SetForegroundWindow(window_);
	SetFocus(window_);

	// Hide the mouse cursor
	ShowCursor(false);
}

void System::ShutdownWindows()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
		ChangeDisplaySettings(NULL, 0);

	// Remove the window.
	DestroyWindow(window_);
	window_ = 0;

	// Remove the application instance.
	UnregisterClass(application_name_, instance_);
	instance_ = 0;

	// Release the pointer to this class.
	ApplicationHandle = 0;
}

LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	// Check if the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	// Check if the window is being closed.
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	// All other messages pass to the message handler within the System class.
	default:
		return ApplicationHandle->MessageHandler(window, message, wparam, lparam);
	}
}