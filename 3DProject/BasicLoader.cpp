#include "pch.h"
#include "BasicLoader.h"

//#include "ConsoleLogger.h"





//--------------------------------------------------------------------------------------
MeshData BasicLoader::loadOBJFromFile(const std::string& filename)
{
	/*
	* Note: the .obj file needs an extra 'enter' in the end of the file to read correctly.
	*/

	// Try to open the file.
	std::ifstream inputFile(filename.c_str());
	if (!inputFile.is_open()) 
	{
		// throw std::runtime_error("Could not open " + filename);
		std::cout << "Could not open " + filename << std::endl;
		return MeshData();
	}

	// Initialize new MeshData.
	MeshData meshData = {};
	meshData.vertices.reserve(5000);
	meshData.normals.reserve(5000);
	meshData.texCoords.reserve(5000);
	meshData.tangents.reserve(5000);
	meshData.faces.reserve(5000);

	// Some useful variables.
	std::stringstream ss = {};
	std::string line = {};
	std::string prefix = {};
	std::string segment = {};

	// Read from .obj file.
	while (std::getline(inputFile, line) && !inputFile.eof())
	{
		// Split the content of the line at spaces, use a stringstream directly.
		ss.clear();
		ss.str(line);
		ss >> prefix;

		// Check what the current segment is and store data.
		if (prefix == "#") {}
		else if (prefix == "mtllib"){}
		else if (prefix == "o") {}
		else if (prefix == "s") {}
		else if (prefix == "v")
		{
			DirectX::XMFLOAT3 v{ 0.f, 0.f, 0.f };
			ss >> v.x >> v.y >> v.z;
			meshData.vertices.emplace_back(v);
		}
		else if (prefix == "vt")
		{
			DirectX::XMFLOAT2 uv{ 0.f, 0.f };
			ss >> uv.x >> uv.y;
			uv.y = -uv.y;	// invert Y-axis due to Blender being stupid.
			meshData.texCoords.emplace_back(uv);
		}
		else if (prefix == "vn")
		{
			DirectX::XMFLOAT3 n{ 0.f, 0.f, 0.f };
			ss >> n.x >> n.y >> n.z;
			meshData.normals.emplace_back(n);
		}
		else if (prefix == "f")
		{
			while (ss >> segment)
			{
				std::istringstream ref(segment);
				std::string vStr, vtStr, vnStr;
				std::getline(ref, vStr, '/');
				std::getline(ref, vtStr, '/');
				std::getline(ref, vnStr, '/');

				// Begin counting from 0 and not 1, hence add -1.
				int v = atoi(vStr.c_str()) - 1;
				int vt = atoi(vtStr.c_str()) - 1;
				int vn = atoi(vnStr.c_str()) - 1;
				meshData.faces.emplace_back(DirectX::XMUINT3(v, vt, vn));
			}
		}
	}

	// Close file.
	inputFile.close();

	meshData.vertices.shrink_to_fit();
	meshData.normals.shrink_to_fit();
	meshData.texCoords.shrink_to_fit();
	meshData.tangents.shrink_to_fit();
	meshData.faces.shrink_to_fit();

	// Recalculate normals and tangents.
	BasicLoader::processMeshData(meshData);

	return meshData;
}





//--------------------------------------------------------------------------------------
void BasicLoader::processMeshData(MeshData &meshData)
{
	// Check if required data exists.
	assert(meshData.faces.size() * meshData.vertices.size() * meshData.texCoords.size());

	// Variables that temporarily holds the tangent & normal calculations,
	// which later will be user to calculate the weighted sum.
	struct WeightedSum { DirectX::XMVECTOR sum; unsigned int users; };
	std::vector<WeightedSum> normals;
	std::vector<WeightedSum> tangents;

	// Initialize variables.
	for (size_t i = 0; i < meshData.vertices.size(); i++) {
		normals.emplace_back();
		tangents.emplace_back();
	}
	meshData.tangents.resize(tangents.size());


	//	Loop through all our faces and calculate the tangent and normal for each face of a triangle.
	for (size_t i = 0; i < meshData.faces.size(); i += 3)
	{
		// Get the indices for three vertices in the triangle.
		const uint32_t i0 = meshData.faces[i].x;
		const uint32_t i1 = meshData.faces[i + 1].x;
		const uint32_t i2 = meshData.faces[i + 2].x;

		const DirectX::XMVECTOR v0 = DirectX::XMLoadFloat3(&meshData.vertices[i0]);
		const DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&meshData.vertices[i1]);
		const DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&meshData.vertices[i2]);

		const DirectX::XMVECTOR e0 = DirectX::XMVectorSubtract(v1, v0);
		const DirectX::XMVECTOR e1 = DirectX::XMVectorSubtract(v2, v0);
		const DirectX::XMVECTOR n = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(e0, e1));

		normals[i0].sum = DirectX::XMVectorAdd(normals[i0].sum, n);
		normals[i1].sum = DirectX::XMVectorAdd(normals[i1].sum, n);
		normals[i2].sum = DirectX::XMVectorAdd(normals[i2].sum, n);
		
		normals[i0].users++;
		normals[i1].users++;
		normals[i2].users++;

		// Get the indices for the texture coordinates belonging to the vertices.
		uint32_t t0 = meshData.faces[i].y;		
		uint32_t t1 = meshData.faces[i + 1].y;	
		uint32_t t2 = meshData.faces[i + 2].y;	

		const DirectX::XMVECTOR uv0 = DirectX::XMLoadFloat2(&meshData.texCoords[t0]);
		const DirectX::XMVECTOR uv1 = DirectX::XMLoadFloat2(&meshData.texCoords[t1]);
		const DirectX::XMVECTOR uv2 = DirectX::XMLoadFloat2(&meshData.texCoords[t2]);

		const DirectX::XMVECTOR deltaUV1 = DirectX::XMVectorSubtract(uv1, uv0);
		const DirectX::XMVECTOR deltaUV2 = DirectX::XMVectorSubtract(uv2, uv0);
		
		// Calculate the denominator of the tangent/binormal equation.
		float r = 1 / (deltaUV1.m128_f32[0] * deltaUV2.m128_f32[1] - deltaUV1.m128_f32[1] * deltaUV2.m128_f32[0]);

		// Calculate the cross products and multiply by the coefficient to get the tangent.
		const DirectX::XMVECTOR t = {
			r * ((e0.m128_f32[0] * deltaUV2.m128_f32[1]) - (e1.m128_f32[0] * deltaUV1.m128_f32[1])),
			r * ((e0.m128_f32[1] * deltaUV2.m128_f32[1]) - (e1.m128_f32[1] * deltaUV1.m128_f32[1])),
			r * ((e0.m128_f32[2] * deltaUV2.m128_f32[1]) - (e1.m128_f32[2] * deltaUV1.m128_f32[1]))
		};

		tangents[i0].sum = DirectX::XMVectorAdd(tangents[i0].sum, t);
		tangents[i1].sum = DirectX::XMVectorAdd(tangents[i1].sum, t);
		tangents[i2].sum = DirectX::XMVectorAdd(tangents[i2].sum, t);

		tangents[i0].users++;
		tangents[i1].users++;
		tangents[i2].users++;	
	}

	// Process normals (weighted sum).
	for (size_t i = 0; i < normals.size(); i++)
	{
		//using namespace DirectX;
		DirectX::XMVECTOR n = DirectX::operator/(normals[i].sum, normals[i].users);
		//DirectX::XMVECTOR n = DirectX::XMVectorMultiply(normals[i].sum, DirectX::XMVectorReplicate(1.0f / normals[i].users));
		DirectX::XMStoreFloat3(&meshData.normals[i], n);
	}
	
	// Process tangents (weighted sum).
	for (size_t i = 0; i < tangents.size(); i++)
	{
		DirectX::XMVECTOR t = DirectX::operator/(tangents[i].sum, tangents[i].users);
		//DirectX::XMVECTOR t = DirectX::XMVectorMultiply(tangents[i].sum, DirectX::XMVectorReplicate(1.0f / tangents[i].users));
		DirectX::XMStoreFloat3(&meshData.tangents[i], t);
	}		
}





//--------------------------------------------------------------------------------------
void BasicLoader::print(const MeshData& meshData)
{
	std::cout << "\n--- Vertices ---" << std::endl;
	for (const auto& it : meshData.vertices)
		std::cout << std::to_string(it.x) + " " + std::to_string(it.y) + " " + std::to_string(it.z) << std::endl;

	std::cout << "\n--- Normals ---" << std::endl;
	for (const auto& it : meshData.normals)
		std::cout << std::to_string(it.x) + " " + std::to_string(it.y) + " " + std::to_string(it.z) << std::endl;

	std::cout << "\n--- Tangents ---" << std::endl;
	for (const auto& it : meshData.tangents)
		std::cout << std::to_string(it.x) + " " + std::to_string(it.y) + " " + std::to_string(it.z) << std::endl;

	std::cout << "\n--- TexCoords ---" << std::endl;
	for (const auto& it : meshData.texCoords)
		std::cout << std::to_string(it.x) + " " + std::to_string(it.y) << std::endl;
}





//--------------------------------------------------------------------------------------
Meshs BasicLoader::createMesh(ID3D11Device* pDevice, const MeshData& meshData)
{
	std::vector<Vertex> vertexData;
	std::vector<UINT> indexData;
	
	indexData.resize(static_cast<UINT>(meshData.faces.size()));
	vertexData.resize(meshData.faces.size());

	for (UINT i = 0; i < meshData.faces.size(); i++)
	{
		uint32_t vIndex = meshData.faces[i].x;	// also used for tangent.
		uint32_t vtIndex = meshData.faces[i].y;
		uint32_t vnIndex = meshData.faces[i].z;

		vertexData[i] = Vertex(meshData.vertices[vIndex], DirectX::XMFLOAT3{ 1,1,1 },  meshData.texCoords[vtIndex], meshData.normals[vnIndex], meshData.tangents[vIndex]);
		indexData[i] = i;
	}

	Meshs mesh;
	mesh.vb.setDevice(pDevice);
	mesh.ib.setDevice(pDevice);
	
	mesh.vb.createVertexBuffer(vertexData.data(), vertexData.size());
	mesh.ib.createIndexBuffer(indexData.data(), indexData.size());
	
	return mesh;
}





//--------------------------------------------------------------------------------------
Meshs BasicLoader::createMeshFromFile(ID3D11Device* pDevice, const std::string& filename)
{
	const MeshData meshData = loadOBJFromFile(filename);
	const Meshs mesh = createMesh(pDevice, meshData);
	return mesh;
}





//--------------------------------------------------------------------------------------
bool BasicLoader::loadShaderData(const std::string& filename, std::string& shaderByteCode)
{
	std::ifstream reader;
	std::string shaderData;
	reader.open(filename + ".hlsl", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		return false;
	}
	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);
	shaderData.assign((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());
	shaderByteCode = shaderData;
	reader.close();

	return true;
}