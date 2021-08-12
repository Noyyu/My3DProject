#include "Graphics.h"
#pragma comment(lib,"d3d11.lib")
#define STB_IMAGE_IMPLEMENTATION 
#include <vector>
#include "stb_image.h"

Graphics::Graphics(UINT width, UINT height, HWND windowHandle, D3D11_VIEWPORT& viewport, Light& light, constantBufferMatrixes matrixes, PerFrameMatrixes perFrameStruct)
{
	//... SETING UP D3D11 THINGS ...//
	//////////////////////////////

	//... Create interface
	createInterface(width, height, windowHandle);

	//... Create render tager view //
	createRenderTargetView();

	//... Set viewport //
	setViewport(viewport, width, height);


	//... SETING UP PIPELINE ...//
	//////////////////////////////

	//... Create Rasterizer States //
	createRasterizerStates();

	//... Create constant buffer for vertex shader //
	createConstantBuffer(matrixes);

	//... Create constant buffer for pixel shader (Light buffer) // 
	createPixelConstantBuffer(light);

	//... Create  Sampler //
	createSamplerState();

	//... Load pixel, geomatry and vertex shaders for geomatry pass //
	loadShader();

	//... Creating input layer //
	createInputLayout();

	//... Creates the full screen quad //
	fullScreenQuadVertexBuffer();

	//... Load pixel and vertex shaders for light pass //
	loadLightPassShaders();

	//.. Create Per Frame Constant Buffer //
	createPerFrameBuffer(perFrameStruct);
}

Graphics::~Graphics()
{
}

bool Graphics::createInterface(UINT width, UINT height, HWND windowHandle)
{
	//Information about DXGI_SWAP_CHAIN_DESC https://docs.microsoft.com/en-us/windows/win32/api/dxgi/ns-dxgi-dxgi_swap_chain_desc

	// .... Descriptour structure for swap chain //
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferDesc.Width = width; // The width and height is set to 0 to tell the thing "Look at the window, you figure it out." Never mind ima try this instead
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //Pixel layout, RGBA 8 bit
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; //Scaning order (in what way it will scan the screen to get information kind of)

	swapChainDesc.SampleDesc.Count = 1; //anti aliasing (no anti aliasing)
	swapChainDesc.SampleDesc.Quality = 0;//anti aliasing (no anti aliasing)
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Use this buffer as the render taget, pipeline will be rendering all of its stuff to this one. 
	swapChainDesc.BufferCount = 1; // Buffer count, we want one back buffer as we are using double buffering. 
	swapChainDesc.OutputWindow = windowHandle;
	swapChainDesc.Windowed = TRUE; //Windowed
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // Vanilla swap effect // Bytt från DXGI_SWAP_EFFECT_DISCARD should not be used anymore but i cant be bothered geting the swap chain to work. DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL is where its at. 
	swapChainDesc.Flags = 0; //No flags

	//... creates device and swapChain //
	HRESULT hr = D3D11CreateDeviceAndSwapChain // https://docs.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-d3d11createdeviceandswapchain
	(
		nullptr, // Choose the default adapter 
		D3D_DRIVER_TYPE_HARDWARE,// We want a hardwere device
		nullptr, // Handle to the binary for the driver we want to load (which i dont want)
		/*D3D11_CREATE_DEVICE_SINGLETHREADED |*/ D3D11_CREATE_DEVICE_DEBUG, // makes the device single threaded which can inprove performence
		nullptr, // Feature level
		0,
		D3D11_SDK_VERSION,// Use the sdk verion you got on your system
		&swapChainDesc,
		&this->pSwapChain,
		&this->pDevice,
		nullptr,
		&this->immediateContext
	);

	return !(FAILED(hr));
	
}

bool Graphics::createRenderTargetView() //Has something to do with the backbuffer
{
	// Gains access to texture subresource in swap chain (back buffer)
	ID3D11Texture2D* pBackBuffer = nullptr;
	//1: gets index of the buffer we want to get, 0 = backbuffer. 2: what id we want to get back, 3: we give it the pointer
	//This gets the back buffer btw
	this->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	//Creates renderTarget
	HRESULT hr = this->pDevice->CreateRenderTargetView(pBackBuffer, nullptr, this->renderTargetView.GetAddressOf());
	pBackBuffer->Release();

	return !(FAILED(hr));
}

bool Graphics::fullScreenQuadVertexBuffer()
{
	Vertex quad[6] =
	{
		//Position				                             //color		                //UV                      //Normal
		{{DirectX::XMFLOAT3{-1, 1, 0.0 }}, {DirectX::XMFLOAT3{1,0,0}}, {DirectX::XMFLOAT2{ 0.0f, 0.0f }}, {DirectX::XMFLOAT3{ 0.0, 0.0, -1.0 }}, {DirectX::XMFLOAT3{0,0,1}}},  // Top left

		{{DirectX::XMFLOAT3{ 1, 1, 0.0 }}, {DirectX::XMFLOAT3{0,1,0}}, {DirectX::XMFLOAT2{ 1.0f, 0.0f }}, {DirectX::XMFLOAT3{ 0.0, 0.0, -1.0 }}, {DirectX::XMFLOAT3{0,0,1}}}, // Top right

		{{DirectX::XMFLOAT3{-1,-1, 0.0 }}, {DirectX::XMFLOAT3{0,0,1}}, {DirectX::XMFLOAT2{ 0.0f, 1.0f }}, {DirectX::XMFLOAT3{ 0.0, 0.0, -1.0 }}, {DirectX::XMFLOAT3{0,0,1}}}, // Bottom left


		{{DirectX::XMFLOAT3{-1,-1, 0.0 }}, {DirectX::XMFLOAT3{0,0,1}}, {DirectX::XMFLOAT2{ 0.0f, 1.0f }}, {DirectX::XMFLOAT3{ 0.0, 0.0, -1.0 }}, {DirectX::XMFLOAT3{0,0,1}}}, // Bottom left

		{{DirectX::XMFLOAT3{ 1, 1, 0.0 }}, {DirectX::XMFLOAT3{0,1,0}}, {DirectX::XMFLOAT2{ 1.0f, 0.0f }}, {DirectX::XMFLOAT3{ 0.0, 0.0, -1.0 }}, {DirectX::XMFLOAT3{0,0,1}}}, // Top right

		{{DirectX::XMFLOAT3{ 1,-1, 0.0 }}, {DirectX::XMFLOAT3{1,0,0}}, {DirectX::XMFLOAT2{ 1.0f, 1.0f }}, {DirectX::XMFLOAT3{ 0.0, 0.0, -1.0 }}, {DirectX::XMFLOAT3{0,0,1}}}  // Bottom right
	};

	//Create the buffer and configurate it
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Bind a buffer as a vertex buffer to the input-assembler stage.
	bufferDesc.Usage = D3D11_USAGE_DEFAULT; //requiers read and write acces to the GPU
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.ByteWidth = sizeof(quad);
	bufferDesc.StructureByteStride = sizeof(Vertex);
	D3D11_SUBRESOURCE_DATA subresourceData = { 0 };
	subresourceData.pSysMem = quad;
	HRESULT hr = this->pDevice->CreateBuffer(&bufferDesc, &subresourceData, this->fullScreenVertexBuffer.GetAddressOf());
	//pDevice->Release();
	return !FAILED(hr);
}

bool Graphics::loadShader()
{
	///------ GEOMATRY PASS SHADERS ------///


	//std::string vertexShaderData;
	std::string pixelShaderData;
	std::string geomatryShaderData;
	std::string computeShaderData;

	this->loadShaderData("VertexShader", this->vertexShaderByteCode);
	this->loadShaderData("PixelShader", pixelShaderData);
	this->loadShaderData("GeomatryShader", geomatryShaderData);


	if (FAILED(this->pDevice->CreateVertexShader(this->vertexShaderByteCode.c_str(), this->vertexShaderByteCode.length(), nullptr, &this->vertexShader)))
	{
		std::cout << "ERROR::loadGeomatryPassShaders:: Could not create GEOMATRY_PASS_VERTEX_SHADER" << std::endl;
		return false;
	}

	if (FAILED(this->pDevice->CreatePixelShader(pixelShaderData.c_str(), pixelShaderData.length(), nullptr, &this->pixelShader)))
	{
		std::cout << "ERROR::loadGeomatryPassShaders:: Could not create GEOMATRY_PASS_PIXEL_SHADER" << std::endl;
		return false;
	}

	if (FAILED(this->pDevice->CreateGeometryShader(geomatryShaderData.c_str(), geomatryShaderData.length(), nullptr, &this->geomatryShader)))
	{
		std::cout << "ERROR::loadGeomatryPassShaders:: Could not create GEOMATRY_PASS_GEOMATRY_SHADER" << std::endl;
		return false;
	}
}

bool Graphics::loadLightPassShaders()
{
	///------ LIGHT PASS SHADERS ------///

	std::string pixelShaderData = {};

	this->loadShaderData("finalPassVertexShader", this->lightPassVertexShaderByteCode);
	this->loadShaderData("finalPassPixelShader", pixelShaderData);


	// Create deferred_geometry_vs.
	if (FAILED(this->pDevice->CreateVertexShader(this->lightPassVertexShaderByteCode.c_str(), this->lightPassVertexShaderByteCode.length(), nullptr, &this->lightPassVertexShader)))
	{
		std::cout << "ERROR::loadLightPassShaders:: Could not create LIGHT_PASS_VERTEX_SHADER" << std::endl;
		return false;
	}


	// Create deferred_geometry_vs.
	if (FAILED(this->pDevice->CreatePixelShader(pixelShaderData.c_str(), pixelShaderData.length(), nullptr, &this->lightPassPixelShader)))
	{
		std::cout << "ERROR::loadLightPassShaders:: Could not create LIGHT_PASS_PIXEL_SHADER" << std::endl;
		return false;
	}
	//device->Release();
	return true;
}

void Graphics::setViewport(D3D11_VIEWPORT& viewport, UINT width, UINT height)
{
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
}

bool Graphics::createConstantBuffer(constantBufferMatrixes matrixes)
{
	//Information about D3D11_BUFFER_DESC https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_buffer_desc
	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT; //requiers read and write acces to the CPU
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.ByteWidth = sizeof(matrixes);
	constantBufferDesc.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA constantSubresourceData = {};
	constantSubresourceData.pSysMem = &matrixes;

	constantBufferDesc.MiscFlags = 0;
	constantSubresourceData.SysMemPitch = 0;
	constantSubresourceData.SysMemSlicePitch = 0;

	HRESULT hr = this->pDevice->CreateBuffer(&constantBufferDesc, &constantSubresourceData, this->pConstantBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		std::cout << "Failed to create constant buffer" << std::endl;
	}
	//pDevice->Release();
	return !FAILED(hr);
}

bool Graphics::createPixelConstantBuffer(Light& light) //Light buffer
{
	//Information about D3D11_BUFFER_DESC https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_buffer_desc
	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;//Is updated every frame therefore it needs to be dynamic.. its not tho
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.ByteWidth = sizeof(light);
	constantBufferDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA constantSubresourceData = {};
	constantSubresourceData.pSysMem = &light;
	constantBufferDesc.MiscFlags = 0;
	constantSubresourceData.SysMemPitch = 0;
	constantSubresourceData.SysMemSlicePitch = 0;

	HRESULT hr = this->pDevice->CreateBuffer(&constantBufferDesc, &constantSubresourceData, this->pPixelConstantBuffer.GetAddressOf());
	//pDevice->Release();
	return !FAILED(hr);
}

bool Graphics::createSamplerState()
{
	//Information about D3D11_SAMPLER_DESC https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_sampler_desc
	D3D11_SAMPLER_DESC desc = {};
	desc.Filter = D3D11_FILTER_ANISOTROPIC; //	Use anisotropic interpolation for minification, magnification, and mip-level sampling. https://www.3dgep.com/wp-content/uploads/2014/04/Texture-Filtering1.png https://www.3dgep.com/wp-content/uploads/2014/04/Mipmap-Filtering.png
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // Tile the texture at every (u,v) integer junction
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; //dito
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; //dito
	desc.MipLODBias = 0;
	desc.MaxAnisotropy = 16; // Clamping value
	desc.BorderColor[0] = desc.BorderColor[1] = desc.BorderColor[2] = desc.BorderColor[3] = 0;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX; // no upper limit on LOD

	HRESULT hr = this->pDevice->CreateSamplerState(&desc, &this->sampler);
	//device->Release();
	return !FAILED(hr);
}

bool Graphics::createInputLayout()
{
	//I should get some sort of polygon count here

	//Information about D3D11_INPUT_ELEMENT_DESC https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_input_element_desc
	//Dont forget to move the tihngs forward with how many bytes they take.
	D3D11_INPUT_ELEMENT_DESC inputDesc[] =
	{
		//SemanticName index, format, input slot, AlignedByteOffset, inputSlotClass, InstanceDataStepRate
		//SemanticName: The HLSL semantic associated with this element in a shader input-signature.

		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0},// 12 bitar
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},// 12 bitar
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},// 8  bitar
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},// 12 bitar
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	//HRESULT is a data type that basically handles common error codes
	HRESULT hr = this->pDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), this->vertexShaderByteCode.c_str(), this->vertexShaderByteCode.length(), &this->inputLayout);
	//device->Release();
	return !FAILED(hr);
}

bool Graphics::loadShaderData(const std::string& filename, std::string& shaderByteCode)
{
	std::ifstream reader;
	std::string shaderData;
	reader.open("x64/Debug/" + filename + ".cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		std::cout << "ERROR::loadShaderData()::Could not open " + filename + ".cso" << std::endl;
		return false;
	}
	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);
	shaderData.assign((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());
	shaderByteCode = shaderData;
	reader.close();

	return true;
}

bool Graphics::createRasterizerStates()
{
	
	D3D11_RASTERIZER_DESC rasStateDesc;
	ZeroMemory(&rasStateDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasStateDesc.DepthClipEnable = true;
	rasStateDesc.AntialiasedLineEnable = false;
	rasStateDesc.FrontCounterClockwise = false;
	rasStateDesc.MultisampleEnable = false;
	rasStateDesc.ScissorEnable = false;
	rasStateDesc.DepthBias = 0;
	rasStateDesc.DepthBiasClamp = 0.0f;
	rasStateDesc.SlopeScaledDepthBias = 0.0f;
	rasStateDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;

	rasStateDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	HRESULT hr = this->pDevice->CreateRasterizerState(&rasStateDesc, &this->rasStateNoCulling); //No culling state;

	if (FAILED(hr))
	{
		std::cout << "Could not create Rasterizer State (No Culling)" << std::endl;
		return false;
	}

	//Set default state
	this->immediateContext->RSSetState(this->rasStateNoCulling.Get());
	//device->Release();
	return true;
}

bool Graphics::createPerFrameBuffer(PerFrameMatrixes perFrameStruct)
{
	//Information about D3D11_BUFFER_DESC https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_buffer_desc
	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.ByteWidth = sizeof(perFrameStruct);
	constantBufferDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA constantSubresourceData = {};
	constantSubresourceData.pSysMem = &perFrameStruct;
	constantBufferDesc.MiscFlags = 0;
	constantSubresourceData.SysMemPitch = 0;
	constantSubresourceData.SysMemSlicePitch = 0;

	HRESULT hr = this->pDevice->CreateBuffer(&constantBufferDesc, &constantSubresourceData, this->pPerFrameConstantBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		std::cout << "Could not create PerFrameConstantBuffer" << std::endl;
	}
	//pDevice->Release();
	return !FAILED(hr);
}

