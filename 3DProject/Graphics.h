#pragma once

#include "Structs.h"

class Graphics
{
public:

	Graphics(UINT width, UINT height, HWND windowHandle, ID3D11Device*& pDevice, ID3D11DeviceContext*& immediateContext,
		IDXGISwapChain*& pSwapChain, ID3D11RenderTargetView*& renderTargetView, D3D11_VIEWPORT& viewport, ID3D11VertexShader*& VertexShader,
		ID3D11PixelShader*& pixelShader, std::string& vertexShaderByteCode, ID3D11InputLayout*& inputLayout, ID3D11Buffer*& pConstantBuffer,
		ID3D11SamplerState*& sampler, Light& light, ID3D11Buffer*& pPixelConstantBuffer, constantBufferMatrixes matrixes, ID3D11Buffer*& FullScreenVertexBuffer,
		ID3D11VertexShader*& finalPassVertexShader, ID3D11PixelShader*& finalPassPixelShader, std::string& lightPassVertexShaderByteCode,
		ID3D11RasterizerState*& rasState, ID3D11RasterizerState*& rasStateNoCulling, ID3D11GeometryShader*& geomatryShader,
		ID3D11Buffer*& pPerFrameConstantBuffer, PerFrameMatrixes perFrameStruct);

	bool createInterface(UINT width, UINT height, HWND windowHandle, IDXGISwapChain*& pSwapChain, ID3D11Device*& pDevice, ID3D11DeviceContext*& immediateContext);
	bool createRenderTargetView(IDXGISwapChain*& pSwapChain, ID3D11Device*& pDevice, ID3D11RenderTargetView*& renderTargetView);
	bool loadShader(ID3D11Device*& device, ID3D11VertexShader*& VertexShader, ID3D11PixelShader*& pixelShader, std::string& vertexShaderByteCode, ID3D11GeometryShader*& geomatryShader);
	bool createInputLayout(ID3D11Device*& device, ID3D11InputLayout*& inputLayout, const std::string& vertexShaderByteCode);
	void setViewport(D3D11_VIEWPORT& viewport, UINT width, UINT height);
	bool createConstantBuffer(ID3D11Device*& pDevice, ID3D11Buffer*& pConstantBuffer, constantBufferMatrixes matrixes);
	bool createSamplerState(ID3D11Device*& device, ID3D11SamplerState*& sampler);
	bool createPixelConstantBuffer(ID3D11Device*& pDevice, Light& light, ID3D11Buffer*& pPixelConstantBuffer);
	bool fullScreenQuadVertexBuffer(ID3D11Device*& pDevice, ID3D11Buffer*& vertexBuffer);
	bool loadLightPassShaders(ID3D11Device*& device, std::string& lightPassVertexShaderByteCode, ID3D11VertexShader*& finalPassVertexShader, ID3D11PixelShader*& finalPassPixelShader);
	bool loadShaderData(const std::string& filename, std::string& shaderByteCode);
	bool createRasterizerStates(ID3D11Device*& device, ID3D11RasterizerState*& rasState, ID3D11RasterizerState*& rasStateNoCulling, ID3D11DeviceContext*& immediateContext);
	bool createPerFrameBuffer(ID3D11Device*& pDevice, ID3D11Buffer*& pPerFrameConstantBuffer, PerFrameMatrixes perFrameStruct);
};