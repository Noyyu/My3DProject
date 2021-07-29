#include "ShadowMap.h"


ShadowMap::ShadowMap(ID3D11DeviceContext* deviceContext, ID3D11Device* device, unsigned int textureWidth, unsigned int textureHeight,
	ID3D11Buffer*& pShadowConstantBuffer, ID3D11InputLayout*& inputLayoutSM, std::string& vertexShaderByteCode, ID3D11VertexShader*& vertexShader)
	:deviceContext(deviceContext), device(device)
{
	this->textureWidth = textureWidth;
	this->textureHeight = textureHeight;
	this->lightDirection = { 1.0f, -0.5f, 1.0f, 1.0f };

	if (this->CreateShadowMap() == false)
	{
		std::cout << "Failed to create Shadow map." << std::endl;
	}
	if (this->CreateConstantBufferSM(pShadowConstantBuffer) == false)
	{
		std::cout << "Failed to create Shadow Constant Buffer." << std::endl;
	}
	if (this->LoadShadowShaders(device, vertexShader, vertexShaderByteCode) == false)
	{
		std::cout << "Failed to load Shadow Shaders." << std::endl;
	}
	if (this->CreateInputLayoutSM(inputLayoutSM, vertexShaderByteCode) == false)
	{
		std::cout << "Failed to create Input Layout for Shadow map." << std::endl;
	}

}


void ShadowMap::SetProjectionMatrix(Light* light, ID3D11Buffer*& pShadowConstantBuffer)
{
	//Set the projection to ortographic
	float nearZ = 1.0f, farZ = 100.0f;
	float veiwWidth = 30.0f, viewHeight = 20.0f;

	this->lightProjectionMatrix = DirectX::XMMatrixOrthographicOffCenterLH(-veiwWidth, veiwWidth, -viewHeight, viewHeight, nearZ, farZ);

	DirectX::XMVECTOR position = { light->position.x, light->position.y, light->position.z, 1.0f };

	sm::Vector4 lightDirection = light->direction;
	sm::Vector4 lightPosition  = light->position;
	sm::Vector4 target = lightPosition + lightDirection;

	// Set view matrix
	this->lightViewMatrix = DirectX::XMMatrixLookAtLH(position, target, { 0.0f, 1.0f, 0.0f });

	//Set light world view peojection matrix;
	DirectX::XMStoreFloat4x4(&this->lightViewProjectionMatrix, (this->lightViewMatrix * this->lightProjectionMatrix));

	// Update
	this->deviceContext->UpdateSubresource(pShadowConstantBuffer, 0, nullptr, &this->lightViewProjectionMatrix, 0, 0); //Is -not- using a struct constant buffer holder atm. 
}


bool ShadowMap::CreateShadowMap()
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	textureDesc.Width =  this->textureWidth;
	textureDesc.Height = this->textureHeight;

	// Use typeless format because the DSV is going to interpret
	// the bits as DXGI_FORMAT_D24_UNORM_S8_UINT, whereas the SRV is going
	// to interpret the bits as DXGI_FORMAT_R24_UNORM_X8_TYPELESS.
	textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	HRESULT hr = this->device->CreateTexture2D(&textureDesc, nullptr, &this->depthMap.texture);
	if (FAILED(hr))
	{
		std::cout << "Could not create depth Texture2D" << std::endl;
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

	depthStencilDesc.Flags = 0;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Texture2D.MipSlice = 0;

	hr = this->device->CreateDepthStencilView(this->depthMap.texture.Get(), &depthStencilDesc, this->depthMap.depthScentilView.GetAddressOf());

	if (FAILED(hr))
	{
		std::cout << "Could not create DepthStencilView for Shadow map" << std::endl;
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;

	hr = this->device->CreateShaderResourceView(this->depthMap.texture.Get(), &shaderResourceViewDesc, this->depthMap.shaderResourceView.GetAddressOf());

	if (FAILED(hr))
	{
		std::cout << "Could not create ShaderResourceView for Shadow map" << std::endl;
		return false;
	}

	return !FAILED(hr);
}




void ShadowMap::shadowPass(Light* light, ID3D11Buffer*& pShadowConstantBuffer, ID3D11VertexShader* vertexShader, ID3D11InputLayout*& inputLayoutSM)
{
	ID3D11RenderTargetView* nullRTV[] = { nullptr };
	this->deviceContext->OMSetRenderTargets(1, nullRTV, this->depthMap.depthScentilView.Get());
	this->deviceContext->ClearDepthStencilView(this->depthMap.depthScentilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	this->deviceContext->IASetInputLayout(inputLayoutSM);
	this->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	this->SetProjectionMatrix(light, pShadowConstantBuffer);

	this->deviceContext->VSSetConstantBuffers(0, 1, &pShadowConstantBuffer);

	this->deviceContext->VSSetShader(vertexShader, nullptr, 0);
	this->deviceContext->PSSetShader(nullptr, nullptr, 0);
}


bool ShadowMap::CreateInputLayoutSM(ID3D11InputLayout*& inputLayoutSM, std::string& vertexShaderByteCode)
{
	D3D11_INPUT_ELEMENT_DESC inputDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HRESULT hr = this->device->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), vertexShaderByteCode.c_str(), vertexShaderByteCode.length(), &inputLayoutSM);

	if (FAILED(hr))
	{
		std::cout << "Failed to create input layout for Shadow map" << std::endl;
		return false;
	}
	return !FAILED(hr);
}




bool ShadowMap::CreateConstantBufferSM(ID3D11Buffer*& pShadowConstantBuffer)
{
	//Information about D3D11_BUFFER_DESC https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_buffer_desc
	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;//Is updated every frame therefore it needs to be dynamic.. its not tho
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.ByteWidth = sizeof(lightViewProjectionMatrix); //I usually use a struct for this but i dont know if this works or nah.
	constantBufferDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA constantSubresourceData = {};
	constantSubresourceData.pSysMem = &lightViewProjectionMatrix;
	constantBufferDesc.MiscFlags = 0;
	constantSubresourceData.SysMemPitch = 0;
	constantSubresourceData.SysMemSlicePitch = 0;

	HRESULT hr = this->device->CreateBuffer(&constantBufferDesc, &constantSubresourceData, std::addressof(pShadowConstantBuffer));
	return !FAILED(hr);
}




bool ShadowMap::LoadShadowShaders(ID3D11Device* device, ID3D11VertexShader*& VertexShader, std::string& vertexShaderByteCode)
{
	std::string pixelShaderData;

	this->LoadShaderData("ShadowVertexShader", vertexShaderByteCode);

	// Create deferred_geometry_vs.
	if (FAILED(device->CreateVertexShader(vertexShaderByteCode.c_str(), vertexShaderByteCode.length(), nullptr, &VertexShader)))
	{
		std::cout << "Failed to load Shadow Vertex Shader" << std::endl;
		return false;
	}

	return true;
}




bool ShadowMap::LoadShaderData(const std::string& filename, std::string& shaderByteCode)
{
	std::ifstream reader;
	std::string shaderData;
	reader.open("x64/Debug/" + filename + ".cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		std::cout << "Could not open " + filename + ".cso" << std::endl;
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

