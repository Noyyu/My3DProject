#pragma once
#include "Structs.h"
#include "Window.h"
#include "Camera.h"


class ShadowMap
{
private:
	unsigned int textureWidth;
	unsigned int textureHeight;
	
	ComPtr<ID3D11InputLayout> inputLayoutSM;
	ComPtr<ID3D11VertexShader> vertexShader;
	std::string vertexShaderByteCode;

	DirectX::XMFLOAT4X4 lightViewProjectionMatrix;
	DirectX::XMMATRIX lightProjectionMatrix;
	DirectX::XMMATRIX lightViewMatrix;
	DirectX::XMFLOAT4 lightDirection;
	
	bool CreateShadowMap(ID3D11Device* device);
	bool CreateInputLayoutSM(ID3D11Device* device);
	bool CreateConstantBufferSM(ID3D11Device* device); //Is to be filled with the lightViewProjectionMatrix. Needs constant buffer.
	bool LoadShadowShaders(ID3D11Device* device);
	bool LoadShaderData(const std::string& filename, std::string& shaderByteCode);
	void CreateShadowSampler(ID3D11Device* device);

public:

	struct DepthMap
	{
		ComPtr <ID3D11Texture2D> texture;
		ComPtr <ID3D11ShaderResourceView> shaderResourceView;
		ComPtr <ID3D11DepthStencilView> depthScentilView;
		ComPtr <ID3D11SamplerState> samplerState;
	};

	DepthMap depthMap;
	ShadowConstantBuffer shadowConstantBufferStruct;
	ComPtr<ID3D11InputLayout> *shadowInputLayout;
	ComPtr<ID3D11Buffer> shadowConstantBuffer;

	ShadowMap(ID3D11DeviceContext* deviceContext, ID3D11Device* device, unsigned int textureWidth, unsigned int textureheighte);
	virtual ~ShadowMap() = default;

	void shadowPass(Light* light, ID3D11DeviceContext* deviceContext); //Needs constant buffer probably.
	void SetProjectionMatrix(Light* light, ID3D11DeviceContext* deviceContext); //needs constantbuffer

};