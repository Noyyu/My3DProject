#pragma once

#include "Structs.h"

class Graphics
{
public:

	//All the tihngs needed
	
	ComPtr <ID3D11RasterizerState> rasStateNoCulling; // Back face culling with geomatry shader
	ComPtr < ID3D11Device> pDevice;
	ComPtr < IDXGISwapChain> pSwapChain;
	ComPtr < ID3D11DeviceContext> immediateContext;
	ComPtr < ID3D11RenderTargetView> renderTargetView;

	//Shaders
	ComPtr < ID3D11VertexShader> vertexShader; // Geomatry pass
	ComPtr < ID3D11PixelShader> pixelShader; // Geomatry pass
	ComPtr < ID3D11VertexShader> lightPassVertexShader; // Light pass
	ComPtr < ID3D11PixelShader> lightPassPixelShader; // Light pass
	ComPtr < ID3D11GeometryShader> geomatryShader; // Back face culling
	ComPtr < ID3D11VertexShader> ShadowVertexShader; // Shadow shader

	ComPtr < ID3D11InputLayout> inputLayout;
	ComPtr < ID3D11InputLayout> shadowInputLayout;

	//Byte codes
	std::string              lightPassVertexShaderByteCode;
	std::string              vertexShaderByteCode;
	std::string              vertexShadowShaderByteCode;       // Shadow map

	//Buffers
	ComPtr < ID3D11Buffer> pConstantBuffer; // Per Object
	ComPtr < ID3D11Buffer> pPixelConstantBuffer; // Light buffer
	ComPtr < ID3D11Buffer> fullScreenVertexBuffer; // Fullscreen quad
	ComPtr < ID3D11Buffer> pShadowConstantBuffer; // Shadow map
	ComPtr < ID3D11Buffer> pPerFrameConstantBuffer; // Per Frame

	ComPtr < ID3D11SamplerState> sampler; // Using the repeat thing (Shadow has its own sampler for.. not repeat things)



	Graphics(UINT width, UINT height, HWND windowHandle, D3D11_VIEWPORT& viewport, Light& light, constantBufferMatrixes matrixes, PerFrameMatrixes perFrameStruct);
	~Graphics();

private:

	bool createInterface(UINT width, UINT height, HWND windowHandle);
	bool createRenderTargetView();
	bool loadShader();
	bool createInputLayout();
	void setViewport(D3D11_VIEWPORT& viewport, UINT width, UINT height);
	
	bool createSamplerState();
	
	bool fullScreenQuadVertexBuffer();
	bool loadLightPassShaders();
	bool loadShaderData(const std::string& filename, std::string& shaderByteCode);
	bool createRasterizerStates();

	bool createPerFrameBuffer(PerFrameMatrixes perFrameStruct);
	bool createConstantBuffer(constantBufferMatrixes matrixes);
	bool createPixelConstantBuffer(Light& light);
};