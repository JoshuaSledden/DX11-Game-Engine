#include "direct3D.h"

Direct3D::Direct3D() :
	swap_chain_(0),
	device_(0), device_context_(0),
	render_target_view_(0),
	depth_stencil_buffer_(0), depth_stencil_state_(0), depth_stencil_view_(0),
	raster_state_(0)
{
}

Direct3D::Direct3D(const Direct3D &kOther)
{
}

Direct3D::~Direct3D()
{
}

bool Direct3D::Initialize(int screen_width, int screen_height, bool vsync, HWND window, bool fullscreen, float screen_depth, float screen_near)
{
	// Store the V-Sync setting.
	vsync_enabled_ = vsync;

	// Create a DirectX graphics interface factory.
	IDXGIFactory* factory = nullptr;
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory)))
		return false;

	// Use the factory to create an adapter for the primary graphics interface.
	IDXGIAdapter* adapter = nullptr;
	if (FAILED(factory->EnumAdapters(0, &adapter)))
		return false;

	// Enumerate the primary adapter output (monitor).
	IDXGIOutput* adapter_output = nullptr;
	if (FAILED(adapter->EnumOutputs(0, &adapter_output)))
		return false;

	// Obtain a list of modes that are avilable for the adapter in the selected format (DXGI_FORMAT_R8G8B8A8_UNORM).
	unsigned int mode_count = 0;
	DXGI_MODE_DESC* display_modes = nullptr;
	if (FAILED(adapter_output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &mode_count, 0)))
		return false;

	// Create a list to hold all possible display modes for the monitor/video card combination.
	display_modes = new DXGI_MODE_DESC[mode_count];
	if (!display_modes)
		return false;

	// Fill the display mode list structures.
	if (FAILED(adapter_output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &mode_count, display_modes)))
	{
		delete[] display_modes;
		display_modes = nullptr;
		return false;
	}

	// Iterate through all display modes within the list and find one that matches the current window resolution.
	DXGI_RATIONAL refresh_rate {0, 0};
	for (unsigned int i = 0; i < mode_count; i++)
	{
		// When a match is found - store the refresh rate.
		if (display_modes[i].Width == screen_width && display_modes[i].Height == screen_height)
			refresh_rate = display_modes[i].RefreshRate;
	}

	// Obtain the user adapter information.
	DXGI_ADAPTER_DESC adapter_desc;
	if (FAILED(adapter->GetDesc(&adapter_desc)))
	{
		delete[] display_modes;
		display_modes = nullptr;
		return false;
	}
	
	// Store the adapter memory (Megabytes).
	video_card_memory_ = static_cast<int>(adapter_desc.DedicatedVideoMemory / 1024 / 1024);

	// Store the adapter name into a character array.
	size_t string_length = 0;
	if (wcstombs_s(&string_length, video_card_description_, 128, adapter_desc.Description, 128) != 0)
	{
		delete[] display_modes;
		display_modes = nullptr;
		return false;
	}

	// Release no-longer needed structures and interfaces that were used to obtain data.
	delete[] display_modes;
	display_modes = nullptr;
	adapter_output->Release();
	adapter_output = nullptr;
	adapter->Release();
	adapter = nullptr;
	factory->Release();
	factory = nullptr;

	// Initialize the swap chain description.
	DXGI_SWAP_CHAIN_DESC swap_chain_desc;
	ZeroMemory(&swap_chain_desc, sizeof(swap_chain_desc));

	// Set to a single back buffer.
	swap_chain_desc.BufferCount = 1;

	// Set thje width and height of the back buffer.
	swap_chain_desc.BufferDesc.Width = screen_width;
	swap_chain_desc.BufferDesc.Height = screen_height;

	// Set 32-bit surface for the back buffer.
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the refresh rate of the back buffer.
	if (vsync_enabled_)
		swap_chain_desc.BufferDesc.RefreshRate = refresh_rate;
	else
		swap_chain_desc.BufferDesc.RefreshRate = { 0, 1 };

	// Set the back buffer usage.
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	swap_chain_desc.OutputWindow = window;

	// Turn multi-sampling off.
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	swap_chain_desc.Windowed = !(fullscreen);

	// Set the scan line ordering and scaling to unspecified.
	swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer content after presenting.
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Do not set advanced flags.
	swap_chain_desc.Flags = 0;

	// Set the feature level to DirectX 11.
	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;

	// Create the swap chain, Direct3D device and device context.
	if (FAILED(D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, &feature_level,
			1, D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain_, &device_, 0, &device_context_)))
		return false;

	// Get the pointer to the back buffer.
	ID3D11Texture2D* back_buffer_ptr = nullptr;
	if (FAILED(swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&back_buffer_ptr)))
		return false;

	// Create the render target view with the back buffer pointer.
	if (FAILED(device_->CreateRenderTargetView(back_buffer_ptr, 0, &render_target_view_)))
		return false;

	// Release the pointer to the back buffer.
	back_buffer_ptr->Release();
	back_buffer_ptr = nullptr;

	// Initialize the description of the depth buffer.
	D3D11_TEXTURE2D_DESC depth_buffer_desc;
	ZeroMemory(&depth_buffer_desc, sizeof(depth_buffer_desc));

	// Set up the description of the depth buffer.
	depth_buffer_desc.Width = screen_width;
	depth_buffer_desc.Height = screen_height;
	depth_buffer_desc.MipLevels = 1;
	depth_buffer_desc.ArraySize = 1;
	depth_buffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_buffer_desc.SampleDesc.Count = 1;
	depth_buffer_desc.SampleDesc.Quality = 0;
	depth_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depth_buffer_desc.CPUAccessFlags = 0;
	depth_buffer_desc.MiscFlags = 0;

	// Create the texture for the depth buffer using the description.
	if (FAILED(device_->CreateTexture2D(&depth_buffer_desc, 0, &depth_stencil_buffer_)))
		return false;

	// Initialize the description of the stencil state.
	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
	ZeroMemory(&depth_stencil_desc, sizeof(depth_stencil_desc));

	// Setup the description of the stencil state.
	depth_stencil_desc.DepthEnable = true;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;

	depth_stencil_desc.StencilEnable = true;
	depth_stencil_desc.StencilReadMask = 0xFF;
	depth_stencil_desc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	if (FAILED(device_->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state_)))
		return false;

	// Set the depth stencil state.
	device_context_->OMSetDepthStencilState(depth_stencil_state_, 1);

	// Initialize the depth stencil view.
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;
	ZeroMemory(&depth_stencil_view_desc, sizeof(depth_stencil_view_desc));

	// Setup the depth stencil view description.
	depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	if (FAILED(device_->CreateDepthStencilView(depth_stencil_buffer_, &depth_stencil_view_desc, &depth_stencil_view_)))
		return false;

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	device_context_->OMSetRenderTargets(1, &render_target_view_, depth_stencil_view_);

	// Setup the raster description which will determine how and what polygons will be drawn.
	D3D11_RASTERIZER_DESC raster_desc;
	raster_desc.AntialiasedLineEnable = false;
	raster_desc.CullMode = D3D11_CULL_BACK;
	raster_desc.DepthBias = 0;
	raster_desc.DepthBiasClamp = 0.0f;
	raster_desc.DepthClipEnable = true;
	raster_desc.FillMode = D3D11_FILL_SOLID;
	raster_desc.FrontCounterClockwise = false;
	raster_desc.MultisampleEnable = false;
	raster_desc.ScissorEnable = false;
	raster_desc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description.
	if (FAILED(device_->CreateRasterizerState(&raster_desc, &raster_state_)))
		return false;

	// Set the rasterizer state.
	device_context_->RSSetState(raster_state_);

	// Setup the viewport so that Direct3D can map clip space co-ordinates to the render target space.
	D3D11_VIEWPORT viewport;
	viewport.Width = static_cast<float>(screen_width);
	viewport.Height = static_cast<float>(screen_height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create the viewport.
	device_context_->RSSetViewports(1, &viewport);

	// Setup the projection matrix.
	float field_of_view = XM_PIDIV4;
	float aspect_ratio = static_cast<float>(screen_width) / static_cast<float>(screen_height);

	// Create the projection matrix.
	projection_matrix_ = XMMatrixPerspectiveFovLH(field_of_view, aspect_ratio, screen_near, screen_depth);

	// Initialize the world matrix to the identity matrix.
	world_matrix_ = XMMatrixIdentity();

	// Create an orphographic projection matrix for 2D rendering.
	ortho_matrix_ = XMMatrixOrthographicLH(static_cast<float>(screen_width), static_cast<float>(screen_height), screen_near, screen_depth);

	return true;
}

void Direct3D::Shutdown()
{
	// Set fullscreen state to false to prevent swap chain throwing exceptions when released.
	if (swap_chain_)
		swap_chain_->SetFullscreenState(false, 0);

	if (raster_state_)
	{
		raster_state_->Release();
		raster_state_ = nullptr;
	}

	if (depth_stencil_view_)
	{
		depth_stencil_view_->Release();
		depth_stencil_view_ = nullptr;
	}

	if (depth_stencil_state_)
	{
		depth_stencil_state_->Release();
		depth_stencil_state_ = nullptr;
	}

	if (depth_stencil_buffer_)
	{
		depth_stencil_buffer_->Release();
		depth_stencil_buffer_ = nullptr;
	}

	if (render_target_view_)
	{
		render_target_view_->Release();
		render_target_view_ = nullptr;
	}

	if (device_context_)
	{
		device_context_->Release();
		device_context_ = nullptr;
	}

	if (device_)
	{
		device_->Release();
		device_ = nullptr;
	}

	if (swap_chain_)
	{
		swap_chain_->Release();
		swap_chain_ = nullptr;
	}
}

void Direct3D::BeginScene(float red, float green, float blue, float alpha)
{
	// Set the colour to clear the buffer to.
	float colour[4] { red, green, blue, alpha };

	// Clear the back buffer.
	device_context_->ClearRenderTargetView(render_target_view_, colour);

	// Clear the depth buffer.
	device_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Direct3D::EndScene()
{
	// Present the back buffer to the screen.
	swap_chain_->Present(static_cast<int>(vsync_enabled_), 0);
}

ID3D11Device * Direct3D::GetDevice()
{
	return device_;
}

ID3D11DeviceContext * Direct3D::GetDeviceContext()
{
	return device_context_;
}

void Direct3D::GetProjectionMatrix(XMMATRIX &matrix)
{
	matrix = projection_matrix_;
}

void Direct3D::GetWorldMatrix(XMMATRIX &matrix)
{
	matrix = world_matrix_;
}

void Direct3D::GetOrthoMatrix(XMMATRIX &matrix)
{
	matrix = ortho_matrix_;
}

void Direct3D::GetVideoCardInfo(char *card_name, int &memory)
{
	strcpy_s(card_name, 128, video_card_description_);
	memory = video_card_memory_;
}
