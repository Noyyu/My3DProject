#include "DeferredRendering.h"

Deferred::Deferred()
{
	normalData = 0;
	positionData = 0; 
	diffuseData = 0;
	depthData = 0;

	normalTargetView = 0; 
	positionTargetView = 0;
	diffuseTargetView = 0;
	depthTargetView = 0;

	normalResourceView = 0;
	positionResourceView = 0;
	diffuseResourceView = 0;
	depthResourceView = 0;

	depthData = 0;
	depthTargetView = 0;
	depthResourceView = 0;

	textureHeight = 500;
	textureWidth = 500;
}

Deferred::~Deferred()
{
}

bool Deferred::instalize(ID3D11Device* device, int textureWidth, int textureHeight, float screenDepth, float screenNear)
{
	HRESULT result;
	this->textureHeight = textureHeight;
	this->textureWidth = textureWidth;


	//-------- Set up TEXTURE DESCRIPTION ---------//
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	result = device->CreateTexture2D(&textureDesc, NULL, &normalData);
	if (FAILED(result)) { return false; };
	result = device->CreateTexture2D(&textureDesc, NULL, &positionData);
	if (FAILED(result)) { return false; };
	result = device->CreateTexture2D(&textureDesc, NULL, &diffuseData);
	if (FAILED(result)) { return false; };

	//-------- Set up RENDER TARGET VIEW DESCRIPTION ---------//
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView(normalData, &renderTargetViewDesc, &normalTargetView);
	if (FAILED(result)) { return false; };
	result = device->CreateRenderTargetView(positionData, &renderTargetViewDesc, &positionTargetView);
	if (FAILED(result)) { return false; };
	result = device->CreateRenderTargetView(diffuseData, &renderTargetViewDesc, &diffuseTargetView);
	if (FAILED(result)) { return false; };

	//-------- Set up SHADER RESOURCE VIEW DESCRIPTION ---------//
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(normalData, &shaderResourceViewDesc, &normalResourceView);
	if (FAILED(result)) { return false; };
	result = device->CreateShaderResourceView(positionData, &shaderResourceViewDesc, &positionResourceView);
	if (FAILED(result)) { return false; };
	result = device->CreateShaderResourceView(diffuseData, &shaderResourceViewDesc, &diffuseResourceView);
	if (FAILED(result)) { return false; };


	//DEPTH STECIL
	//-------- Set up DEPTH TEXTURE DESCRIPTION ---------//
	D3D11_TEXTURE2D_DESC depthTextureDesc = {};
	ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
	depthTextureDesc.Width = textureWidth;
	depthTextureDesc.Height = textureHeight;
	depthTextureDesc.MipLevels = 1; // Use 1 for a multisampled texture
	depthTextureDesc.ArraySize = 1; //one buffer
	depthTextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;//Is used to render depth stencil as texture //DXGI_FORMAT_D24_UNORM_S8_UINT;//24 bitar, normalized, 8 bitar, unInt //Denna måste jag förklara också
	depthTextureDesc.SampleDesc.Count = 1;// The number of multisamples per pixel.
	depthTextureDesc.SampleDesc.Quality = 0;//The lowers quality
	depthTextureDesc.Usage = D3D11_USAGE_DEFAULT; //requiers read and write acces to the CPU
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // Bind a texture as a depth-stencil target for the output-merger stage.
	depthTextureDesc.CPUAccessFlags = 0; // read or write.
	depthTextureDesc.MiscFlags = 0;

	result = (device->CreateTexture2D(&depthTextureDesc, nullptr, &depthData));
	if (FAILED(result)) { return false; };


	//-------- Set up DEPTH STENCIL VIEW DESCRIPTION ---------//
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;//kolla upp detta
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = (device->CreateDepthStencilView(depthData, &depthStencilViewDesc, &depthStencilView));
	if (FAILED(result)) { return false; };


	//-------- Set up VIEWPORT ---------//

	// Setup the viewport for rendering.
	viewport.Width = (float)textureWidth;
	viewport.Height = (float)textureHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	return true;
}

void Deferred::shutDownDeferredObjects()
{
	normalData->Release();
	positionData->Release();
	diffuseData->Release();

	normalTargetView->Release();
	positionTargetView->Release();
	diffuseTargetView->Release();

	normalResourceView->Release();
	positionResourceView->Release();
	diffuseResourceView->Release();
	depthResourceView->Release();

	depthData->Release();
	depthTargetView->Release();

	normalData = 0;
	positionData = 0;
	diffuseData = 0;

	normalTargetView = 0;
	positionTargetView = 0;
	diffuseTargetView = 0;

	normalResourceView = 0;
	positionResourceView = 0;
	diffuseResourceView = 0;

	depthData = 0;
	depthTargetView = 0;
	depthResourceView = 0;
}

void Deferred::setRenderTargets(ID3D11DeviceContext* deviceContext)
{
	ID3D11RenderTargetView* targets[3] = { normalTargetView, positionTargetView, diffuseTargetView};

	// Bind the render target view array and depth stencil buffer to the output render pipeline.
	deviceContext->OMSetRenderTargets(3, targets, depthStencilView);

	// Set the viewport.
	deviceContext->RSSetViewports(1, &viewport);

	return;
}

void Deferred::clearRenderTargets(ID3D11DeviceContext* deviceContext)
{
	float color[4];

	// Setup the color to clear the buffer to.
	float clearColor[4] = { 0,0,0,0 };

	deviceContext->ClearRenderTargetView(normalTargetView, clearColor);
	deviceContext->ClearRenderTargetView(positionTargetView, clearColor);
	deviceContext->ClearRenderTargetView(diffuseTargetView, clearColor);
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

ID3D11ShaderResourceView* Deferred::getShaderResourceView(int index)
{
	if (index == 0)
	{
		return normalResourceView;
	}
	if (index == 1)
	{
		return positionResourceView;
	}
	if (index == 2)
	{
		return diffuseResourceView;
	}
}

void Deferred::setShaderResourceView(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* shadowResourceView)
{
	ID3D11ShaderResourceView* targets[4] = { normalResourceView, positionResourceView, diffuseResourceView, shadowResourceView}; //Binds the textures from the geomatry pass to the light pass

	// Bind the render target view array and depth stencil buffer to the output render pipeline.
	deviceContext->PSSetShaderResources(0, 4, targets);
}

void Deferred::unbindShaderResourceView(ID3D11DeviceContext* deviceContext)
{
	ID3D11ShaderResourceView* const kill[4] = { nullptr };
	deviceContext->PSSetShaderResources(0, 4, kill);

}

void Deferred::setLightPassRenderTarget(ID3D11RenderTargetView* renderTargetView, ID3D11DeviceContext* deviceContext)
{
	deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
}
