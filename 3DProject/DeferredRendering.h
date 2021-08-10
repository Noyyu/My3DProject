#pragma once
#include<d3d11.h>
#include<DirectXMath.h>

//This class keeps all the textures. It's the the g-buffer.

class Deferred
{
private:
	//Deferred rendering textures
	ID3D11Texture2D* normalData = nullptr;
	ID3D11Texture2D* positionData = nullptr;
	ID3D11Texture2D* diffuseData = nullptr;

	ID3D11RenderTargetView* normalTargetView = nullptr;
	ID3D11RenderTargetView* positionTargetView = nullptr;
	ID3D11RenderTargetView* diffuseTargetView = nullptr;

	ID3D11ShaderResourceView* normalResourceView = nullptr;
	ID3D11ShaderResourceView* positionResourceView = nullptr;
	ID3D11ShaderResourceView* diffuseResourceView = nullptr;

	//temp depth things:
	ID3D11Texture2D* depthData = nullptr;
	ID3D11DepthStencilView* depthStencilView = nullptr;

	int textureHeight = 500;
	int	textureWidth = 500;

	D3D11_VIEWPORT viewport = {};

public:

	Deferred();
	~Deferred();

	bool instalize(ID3D11Device* &device, int textureWidth, int textureHight, float screenDepth, float screenNear);
	void shutDownDeferredObjects();
	void setRenderTargets(ID3D11DeviceContext*& deviceContext);
	void clearRenderTargets(ID3D11DeviceContext*& deviceContex);
	ID3D11ShaderResourceView* getShaderResourceView(int index);
	void setShaderResourceView(ID3D11DeviceContext*& deviceContext, ID3D11ShaderResourceView* shadowResourceView);
	void unbindShaderResourceView(ID3D11DeviceContext*& deviceContext);
	void setLightPassRenderTarget(ID3D11RenderTargetView* renderTargetView, ID3D11DeviceContext*& deviceContext);
};
