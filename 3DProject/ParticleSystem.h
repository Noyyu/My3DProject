#pragma once
#include "pch.h"
#include "Structs.h"
#include "Camera.h"

#define MAX_PARTICLES 20000

struct Particle
{
	DirectX::XMFLOAT4 position;
};

struct ParticlePerFrame
{
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
	DirectX::XMFLOAT4 cameraPosition;
};

class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();
	void InitializeParticles(ID3D11Device*& device, Particle particleList[], DirectX::XMFLOAT4 position);
	void particlePass(ID3D11DeviceContext*& deviceContext, Camera*& walkingCamera);

private:

	ParticlePerFrame particlesPerFrameMatrixes;
	MatrixFunctions matrixFunctions;

	ComPtr<ID3D11VertexShader> particleVertexShader;
	ComPtr<ID3D11ComputeShader> particleComputeShader;
	ComPtr<ID3D11GeometryShader> particleGeomatryShader;
	ComPtr<ID3D11PixelShader> particlePixelShader;

	ID3D11Buffer* vertexParticleConstantBuffer;
	ComPtr <ID3D11Buffer> dummyParticleBuffer;
	ComPtr <ID3D11Buffer> vertexParticleBuffer;
	ComPtr <ID3D11ShaderResourceView> particleSRV;
	ComPtr <ID3D11UnorderedAccessView> particleUAV;

	void LoadShader(ID3D11Device*& device);
	void LoadShaderData(const std::string& filename, std::string& shaderByteCode);

	//Temp
	void shutDown();
	
};
