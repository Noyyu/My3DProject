#pragma once
#pragma once
#include "pch.h"
#include "DeferredRendering.h"

#define ALIGN16 __declspec(align(16))
#define STB_IMAGE_IMPLEMENTATION 

//Holds the properties of a subset surface
struct SurfaceMaterial
{
	std::wstring materialName = {}; //Name of the material, come from the starting line "newmtl" in the .mtl file
	//Every time a "newmtl" is found in a mtl file we create a new material´abd oush it back into the surface vector

	//Copntains the rgba for the surface. this value comes from "ks" and "ka" for the solid colors. the useless transparecy comes from the "Tr" or "d"
	DirectX::XMFLOAT4 color = {};
	DirectX::XMFLOAT3 specular = {};
	DirectX::XMFLOAT3 ambient = {};

	//"After that we have an integer, texArrayIndex, containing the index value of the texture stored in our meshSRV vector that this material's surface will use." Im not sure yet
	int textureArrayIndex = {};
	int normalMapArrayIndex = {};
	

	//checks if the material uses a texture. This vill over write the XMFLOAT4 color and thats done in the pixel shader. 
	int hasTexture = {}; // we will know if the material has color if the line "map_kd" or "map_ka" is precent in the mtl file.
	int hasNormalMap = {};

	//another for the moment useless variable that checks if the material is transparent or nah. I tihnk i can use this later tho.
	bool transparent = {};
};

struct objMatrixes
{
	DirectX::XMMATRIX objRotation = DirectX::XMMatrixRotationY(3.14f);
	DirectX::XMMATRIX objScale = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
	DirectX::XMMATRIX objTranslation = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	DirectX::XMMATRIX cameraPosition = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX cameraProjection = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX cameraView = DirectX::XMMatrixIdentity();

	DirectX::XMMATRIX objMeshWorldMatrix = objRotation * objScale * objTranslation;

	void setCameraPosition(DirectX::XMMATRIX cameraPosition)
	{
		this->cameraPosition = cameraPosition;
	}
	void setCameraView(DirectX::XMMATRIX cameraView)
	{
		this->cameraView = cameraView;
	}
	void setCameraProjection(DirectX::XMMATRIX cameraProjection)
	{
		this->cameraProjection = cameraProjection;
	}
};

struct Vertex
{
	DirectX::XMFLOAT3 pos = {0,0,0};
	DirectX::XMFLOAT3 color = {1,1,1};
	DirectX::XMFLOAT2 textureCoordenates = { 0,0 };
	DirectX::XMFLOAT3 normal = {1,1,1};
	DirectX::XMFLOAT3 tangents = {};

	Vertex()
	{

	}

	Vertex(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 vertexColor, DirectX::XMFLOAT2 textureCoordinates, DirectX::XMFLOAT3 normal/*, bool hasNormalMap*/, DirectX::XMFLOAT3 tangent)
		:pos(position), color(vertexColor), textureCoordenates(textureCoordinates), normal(normal)/*, hasNormalMap(hasNormalMap)*/, tangents(tangent)
	{
	}
};

ALIGN16
struct Light // https://www.braynzarsoft.net/viewtutorial/q16390-17-point-lights
{

	DirectX::XMFLOAT4 position = {};
	DirectX::XMFLOAT4 attenuation = {};
	DirectX::XMFLOAT4 ambient = {};
	DirectX::XMFLOAT4 diffuse = {};
	DirectX::XMFLOAT4 cameraPosition = {};
	DirectX::XMFLOAT4 direction = {};
	float range = 0;


	Light()
	{
		//Fills a block of memory with zeros.
		ZeroMemory(this, sizeof(Light));
	}
	void updateCameraPosition(DirectX::XMFLOAT4 cameraPosition)
	{
		this->cameraPosition = cameraPosition;
	}
};

ALIGN16
struct constantBufferMatrixes
{
	DirectX::XMFLOAT4X4 WorldViewProjection = {};
	DirectX::XMFLOAT4X4 World = {};
	float time = 0;
	int hasTexture = 1;
	int hasNormal  = 1;
};

struct matrixFunctions
{
	DirectX::XMFLOAT4X4 setWVP(DirectX::XMMATRIX WVP)
	{
		DirectX::XMFLOAT4X4 floatWVP;
		DirectX::XMStoreFloat4x4(&floatWVP, XMMatrixTranspose(WVP));
		return floatWVP;
	}
	DirectX::XMFLOAT4X4 setWorld(DirectX::XMMATRIX world)
	{
		DirectX::XMFLOAT4X4 floatWorld;
		DirectX::XMStoreFloat4x4(&floatWorld, XMMatrixTranspose(world));
		return floatWorld;
	}
};

struct ShadowConstantBuffer
{
	DirectX::XMFLOAT4X4 LightViewProjectionMatrix;
};


