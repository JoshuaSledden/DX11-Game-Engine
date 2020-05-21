#pragma once
#include "System.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int cmd_show)
{
	// Create the system object.
	System* system = new System();
	if (!system)
		return 0;

	// Initialize and run the system object if it is valid.
	bool result = system->Initialize();
	if (result)
		system->Run();

	// Shutdown and release the system object before exiting.
	system->Shutdown();
	delete system;
	system = 0;

	return 0;
}