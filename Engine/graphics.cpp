#include "graphics.h"

Graphics::Graphics()
{
	direct3D_ = 0;
}

Graphics::Graphics(const Graphics& kOther)
{
}

Graphics::~Graphics()
{
}

bool Graphics::Initialize(int screen_width, int screen_height, HWND window)
{
	// Create the Direct3D object.
	direct3D_ = new Direct3D();
	if (!direct3D_)
		return false;

	// Initialize the Direct3D object.
	bool result = direct3D_->Initialize(screen_width, screen_height, VSYNC_ENABLED, window, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(window, L"Failed the initialize Direct 3D", L"Error", MB_OK);
		return false;
	}

	return true;
}

void Graphics::Shutdown()
{
	// Release the Direct3D object.
	if (direct3D_)
	{
		direct3D_->Shutdown();
		delete direct3D_;
		direct3D_ = 0;
	}
}

bool Graphics::Frame()
{
	// Render the graphics scene.
	if (!Render())
		return false;

	return true;
}

bool Graphics::Render()
{
	// Clear the buffers in order to begin the scene.
	direct3D_->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);

	// Present the rendered scene to the screen.
	direct3D_->EndScene();
	return true;
}