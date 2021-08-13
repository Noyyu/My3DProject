#pragma once
#include"pch.h"
#include<d3d11.h>
#include<DirectXMath.h>

//This class keeps all the textures. It's the the g-buffer.

class Deferred
{
private:
	//Deferred rendering textures
	ComPtr <ID3D11Texture2D> normalData;
	ComPtr <ID3D11Texture2D> positionData;
	ComPtr <ID3D11Texture2D> diffuseData;

	ComPtr <ID3D11RenderTargetView> normalTargetView;
	ComPtr <ID3D11RenderTargetView> positionTargetView;
	ComPtr <ID3D11RenderTargetView> diffuseTargetViewr;

	ComPtr <ID3D11ShaderResourceView> normalResourceView;
	ComPtr <ID3D11ShaderResourceView> positionResourceView;
	ComPtr <ID3D11ShaderResourceView> diffuseResourceView;

	ComPtr <ID3D11Texture2D> depthData;
	ComPtr <ID3D11DepthStencilView> depthStencilView;

	int textureHeight = 500;
	int	textureWidth = 500;

	D3D11_VIEWPORT viewport = {};

public:

	Deferred();
	~Deferred();

	bool instalize(ID3D11Device* device, int textureWidth, int textureHight, float screenDepth, float screenNear);
	void setRenderTargets(ID3D11DeviceContext* deviceContext);
	void clearRenderTargets(ID3D11DeviceContext* deviceContex);
	ID3D11ShaderResourceView* getShaderResourceView(int index);
	void setShaderResourceView(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* shadowResourceView);
	void unbindShaderResourceView(ID3D11DeviceContext* deviceContext);
	void setLightPassRenderTarget(ID3D11RenderTargetView* renderTargetView, ID3D11DeviceContext* deviceContext);
};
