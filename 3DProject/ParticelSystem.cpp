#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::InitializeParticles(ID3D11Device* device, Particle particleList[], DirectX::XMFLOAT4 position)
{

	int xPosition = 0, zPosition = 0;
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		int yPosition = rand() % 50 + 50;
		particleList[i].position = DirectX::XMFLOAT4(position.x + xPosition, yPosition, position.z + zPosition, position.w);
		xPosition++;
		if (xPosition == 100)
		{
			zPosition++;
			xPosition = 0;

		}
	}
	//------------------------ VERTEX BUFFER

	//Create VertexBuffer
	D3D11_BUFFER_DESC descVert;
	ZeroMemory(&descVert, sizeof(descVert));
	descVert.Usage = D3D11_USAGE_DEFAULT;
	//assert(sizeof(Particle) % 16 == 0);				//Ser till att det är 16 bitar
	descVert.ByteWidth = sizeof(Particle) * MAX_PARTICLES;
	descVert.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	descVert.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	descVert.StructureByteStride = sizeof(Particle);

	D3D11_SUBRESOURCE_DATA dataVert;
	ZeroMemory(&dataVert, sizeof(dataVert));
	dataVert.pSysMem = particleList;

	HRESULT hr = device->CreateBuffer(&descVert, &dataVert, this->vertexParticleBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		std::cout << "Could not create Particle System Vertex Buffer" << std::endl;
	}

	//------------------------ SHADER RESOURCE VIEW

	//SRV to Buffer that hold every particle (Lucka för att hämta information från vertex shadern)
	D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
	ZeroMemory(&descSRV, sizeof(descSRV));
	descSRV.Format = DXGI_FORMAT_UNKNOWN;
	descSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	descSRV.BufferEx.NumElements = MAX_PARTICLES;

	hr = device->CreateShaderResourceView(vertexParticleBuffer.Get(), &descSRV, particleSRV.GetAddressOf());
	if (FAILED(hr))
	{
		std::cout << ("Failed to create D3D11_SHADER_RESOURCE_VIEW_ for vertexBuffer") << std::endl;
	}

	//------------------------- CONSTANT BUFFER

	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;//Is updated every frame therefore it needs to be dynamic.. its not tho
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.ByteWidth = sizeof(ParticlePerFrame); //I usually use a struct for this but i dont know if this works or nah.
	constantBufferDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA constantSubresourceData = {};
	constantSubresourceData.pSysMem = &particlesPerFrameMatrixes;
	constantBufferDesc.MiscFlags = 0;
	constantSubresourceData.SysMemPitch = 0;
	constantSubresourceData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&constantBufferDesc, &constantSubresourceData, &vertexParticleConstantBuffer);
	if (FAILED(hr))
	{
		std::cout << "Could not create Particle System Constant Buffer" << std::endl;
	}

	

	//----------------------- DUMMY BUFFER (Används för att vi MÅSTE koppla något till inputassemblern (IA'n))

	D3D11_BUFFER_DESC dumyParticleDesc;
	ZeroMemory(&dumyParticleDesc, sizeof(dumyParticleDesc));
	dumyParticleDesc.Usage = D3D11_USAGE_DEFAULT;
	dumyParticleDesc.ByteWidth = sizeof(Particle);
	dumyParticleDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	hr = device->CreateBuffer(&dumyParticleDesc, nullptr, this->dummyParticleBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		std::cout << ("Failed to create dummy vertex Buffer") << std::endl;
	}

	//---------------------- UNORDERED ACCES VIEW (Lucka för att ge och ta från vertex buffern)
	D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
	descView.Format = DXGI_FORMAT_UNKNOWN;
	descView.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	descView.Buffer.NumElements = MAX_PARTICLES;
	descView.Buffer.FirstElement = 0;
	descView.Buffer.Flags = 0;

	hr = device->CreateUnorderedAccessView(this->vertexParticleBuffer.Get(), &descView, this->particleUAV.GetAddressOf());
	if (FAILED(hr))
	{
		std::cout << ("Failed to create D3D11_UNORDERED_ACCESS_VIEW") << std::endl;
	}

	LoadShader(device);
}

void ParticleSystem::particlePass(ID3D11DeviceContext* deviceContext, Camera* walkingCamera)
{
	static UINT stride = sizeof(Particle);
	static UINT offset = 0;

	ID3D11ShaderResourceView* nullSRV = nullptr;
	ID3D11UnorderedAccessView* nullUAV = nullptr;

	DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(1, 1, 1);
	DirectX::XMMATRIX translate = DirectX::XMMatrixTranslation(0, 0, 0); // X+ = >, Z+ = ^
	DirectX::XMMATRIX world = scale * translate;

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	particlesPerFrameMatrixes.viewMatrix = matrixFunctions.setWorld(DirectX::XMMatrixTranspose(walkingCamera->getCameraView()));
	particlesPerFrameMatrixes.projectionMatrix = matrixFunctions.setWVP((DirectX::XMMatrixTranspose(walkingCamera->getCameraProjection())));
	particlesPerFrameMatrixes.cameraPosition = DirectX::XMFLOAT4(DirectX::XMVectorGetX(walkingCamera->getCameraPos()), DirectX::XMVectorGetY(walkingCamera->getCameraPos()), DirectX::XMVectorGetZ(walkingCamera->getCameraPos()), 1.f);

	deviceContext->UpdateSubresource(vertexParticleConstantBuffer.Get(), 0, NULL, &particlesPerFrameMatrixes, 0, 0);

	deviceContext->GSSetConstantBuffers(0, 1, vertexParticleConstantBuffer.GetAddressOf());
	deviceContext->IASetVertexBuffers(0, 1, dummyParticleBuffer.GetAddressOf(),&stride,&offset); //Is not needed


	deviceContext->VSSetShader(particleVertexShader.Get(), nullptr, 0);
	deviceContext->CSSetShader(particleComputeShader.Get(), nullptr, 0);
	deviceContext->GSSetShader(particleGeomatryShader.Get(), nullptr, 0);
	deviceContext->PSSetShader(particlePixelShader.Get(), nullptr, 0);


	//1: Koppla en unordered acces view for read and write acces.
	//2: Räkna ut hur många gånger compute shadern behöver köras.
	//3: Kör compute shadern.
	//4: Unbind unordered acces to compute shader. 
	deviceContext->CSSetUnorderedAccessViews(0, 1, particleUAV.GetAddressOf(), nullptr);
	const int groupCount = static_cast<int>(ceil(MAX_PARTICLES / 768.f)); //Hur många grupper som körs
	deviceContext->Dispatch(groupCount, 1, 1); //Gör dina beräkningar, Compute shader!
	deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);


	//1: Bind the buffer to the vertex shader.
	//2: Draw.
	deviceContext->VSSetShaderResources(0, 1, particleSRV.GetAddressOf());
	deviceContext->DrawInstanced(1, MAX_PARTICLES, 0, 0);

	//Kill stuff
	deviceContext->VSSetShaderResources(0, 1, &nullSRV);
	deviceContext->GSSetShader(nullptr, nullptr, 0);
	deviceContext->CSSetShader(nullptr, nullptr, 0);

}

void ParticleSystem::LoadShader(ID3D11Device* device)
{
	std::string vertexShaderData;
	std::string computeShaderData;
	std::string geomatryShaderData;
	std::string pixelShaderData;

	this->LoadShaderData("ParticleVertexShader", vertexShaderData);
	this->LoadShaderData("ParticleComputeShader", computeShaderData);
	this->LoadShaderData("ParticleGeomatryShader", geomatryShaderData);
	this->LoadShaderData("ParticlePixelShader", pixelShaderData);

	if (FAILED(device->CreateVertexShader(vertexShaderData.c_str(), vertexShaderData.length(), nullptr, &particleVertexShader)))
	{
		std::cout << "Failed to load Particle Vertex Shader" << std::endl;
	}

	if (FAILED(device->CreateComputeShader(computeShaderData.c_str(), computeShaderData.length(), nullptr, &particleComputeShader)))
	{
		std::cout << "Failed to load Particle Compute Shader" << std::endl;
	}

	if (FAILED(device->CreateGeometryShader(geomatryShaderData.c_str(), geomatryShaderData.length(), nullptr, &particleGeomatryShader)))
	{
		std::cout << "Failed to load Particle geomatry Shader" << std::endl;
	}

	if (FAILED(device->CreatePixelShader(pixelShaderData.c_str(), pixelShaderData.length(), nullptr, &particlePixelShader)))
	{
		std::cout << "Failed to load Particle Pixel Shader" << std::endl;
	}
}

void ParticleSystem::LoadShaderData(const std::string& filename, std::string& shaderByteCode)
{
	std::ifstream reader;
	std::string shaderData;
	reader.open("x64/Debug/" + filename + ".cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		std::cout << "Could not open " + filename + ".cso" << std::endl;
	}
	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);
	shaderData.assign((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());
	shaderByteCode = shaderData;
	reader.close();
}


