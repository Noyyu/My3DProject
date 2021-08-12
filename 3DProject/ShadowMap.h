#pragma once
#include "Structs.h"
#include "Window.h"
#include "Camera.h"


class ShadowMap
{
private:
	unsigned int textureWidth;
	unsigned int textureHeight;

	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> deviceContext;

	DirectX::XMFLOAT4X4 lightViewProjectionMatrix;
	DirectX::XMMATRIX lightProjectionMatrix;
	DirectX::XMMATRIX lightViewMatrix;
	DirectX::XMFLOAT4 lightDirection;
	
	bool CreateShadowMap();
	bool CreateInputLayoutSM(ID3D11InputLayout*& inputLayoutSM, std::string& vertexShaderByteCode);
	bool CreateConstantBufferSM(ID3D11Buffer*& pShadowConstantBuffer); //Is to be filled with the lightViewProjectionMatrix. Needs constant buffer.
	bool LoadShadowShaders(ID3D11Device*& device, ID3D11VertexShader*& VertexShader, std::string& vertexShaderByteCode);
	bool LoadShaderData(const std::string& filename, std::string& shaderByteCode);
	void CreateShadowSampler();

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

	ShadowMap(ID3D11DeviceContext* deviceContext, ID3D11Device* device, unsigned int textureWidth, unsigned int textureheighte,
			  ID3D11Buffer*& pShadowConstantBuffer, ID3D11InputLayout*& inputLayoutSM, std::string& vertexShaderByteCode, ID3D11VertexShader*& vertexShader);
	virtual ~ShadowMap() = default;

	void shadowPass(Light* light, ID3D11Buffer*& pShadowConstantBuffer, ID3D11VertexShader* vertexShader, ID3D11InputLayout*& inputLayoutSM); //Needs constant buffer probably.
	void SetProjectionMatrix(Light* light, ID3D11Buffer*& pShadowConstantBuffer); //needs constantbuffer

};