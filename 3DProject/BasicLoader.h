#pragma once
#include "Structs.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"


struct MeshData
{
	std::vector<DirectX::XMFLOAT3> vertices;
	std::vector<DirectX::XMFLOAT3> normals;
	std::vector<DirectX::XMFLOAT2> texCoords;
	std::vector<DirectX::XMFLOAT3> tangents;
	std::vector<DirectX::XMUINT3>  faces;		// x = vertex, y = uv, z = normal
};

struct Meshs
{
	VertexBuffer vb;
	IndexBuffer ib;
};

class BasicLoader
{
public:
	/* Loads an .obj file and store its data in MeshData. */
	static MeshData loadOBJFromFile(const std::string& filename);
	/* Calculates normals and tangents based on MeshData. */
	static void processMeshData(MeshData& meshData);
	/* Creates a Mesh based on MeshData. */
	static Meshs createMesh(ID3D11Device* pDevice, const MeshData & meshData);
	/* Creates a Mesh based on given .obj filename. */
	static Meshs createMeshFromFile(ID3D11Device* pDevice, const std::string& filename);
	/* Load shader data from .hlsl files. */
	static bool loadShaderData(const std::string& filename, std::string& shaderByteCode);

private:
	/* Debug method that prints MeshData to console. */
	static void print(const MeshData & meshData);
};

