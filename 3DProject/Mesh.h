#include "Structs.h"
#include "Camera.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"

//-------Ignore these
struct SubMesh
{
	std::vector<Vertex> vertexData;
	std::vector<UINT> indexData;
};
struct MeshBuffers
{
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;
};
struct Model
{
	MeshBuffers* mesh;
	//Material* material;
	//TextureResources* textureResources;
};
struct MaterialData
{
	std::string name;		    // Name of mtl file
	DirectX::XMFLOAT4 ambient;	// Ka
	DirectX::XMFLOAT4 diffuse;	// Kd
	DirectX::XMFLOAT4 emissive;	// Ke
	DirectX::XMFLOAT4 specular;	// Ks
	float Ns;				    // Specular exponent
	std::string ambientName;    // Ka name
	std::string diffuseName;	// Kd name
	std::string emissiveName;	// Ke name
	std::string specularName;	// Ks name
	std::string normalName;    	// Normal map name

	BOOL hasAmbientMap;
	BOOL hasDiffuseMap;
	BOOL hasEmissiveMap;
	BOOL hasSpecularMap;
	BOOL hasNormalMap;
};
struct Material
{
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 emissive;
	DirectX::XMFLOAT4 specular;
	BOOL hasAmbientMap;
	BOOL hasDiffuseMap;
	BOOL hasEmissiveMap;
	BOOL hasSpecularMap;
	BOOL hasNormalMap;
};
struct TextureResources
{
	ID3D11ShaderResourceView* ambientRSV;
	ID3D11ShaderResourceView* diffuseRSV;
	ID3D11ShaderResourceView* emissiveRSV;
	ID3D11ShaderResourceView* specularRSV;
	ID3D11ShaderResourceView* normalRSV;
};
//-------


class Mesh
{
private:

	std::wstring filePath;

	float time = 0;

	int totalVertices = 0;
	int meshSubsets = 0;
	bool hasNormalMap = false;
	int animation = 0;

	constantBufferMatrixes objMats;
	matrixFunctions matrixFunction;
	objMatrixes tempMatrixes = {};
	Light lightMatrixes;

	std::vector<int> meshSubsetIndexStart = {};
	std::vector<int> meshSubsetTexture = {};
	std::vector<int> meshSubsetMaterialArray = {};

	DirectX::XMMATRIX meshWorldMatrix = {};
	ID3D11Texture2D* mtlTexture = {};
	ID3D11Texture2D* mtlNormalTexture = {};
	ID3D11ShaderResourceView* mtlShaderResourceView = {};
	ID3D11ShaderResourceView* mtlNormalShaderResourceView = {};
	ID3D11RenderTargetView* mtlRenderTargetView = {};

	ID3D11Buffer* meshVertexBuffer = {};
	ID3D11Buffer* meshIndexBuffer = {};
	ID3D11Buffer* objConstantBuffer = {};

	std::vector<SurfaceMaterial> material = {};
	std::vector<ID3D11ShaderResourceView*> meshShaderResourceView = {};
	std::vector<std::wstring> textureNameArray = {}; //https://www.cplusplus.com/reference/string/wstring/



	//and these
	Model *model;
	std::unique_ptr<ConstantBuffer> perObjectConstantBuffer;
	Camera *camera;

	std::vector<DirectX::XMFLOAT3> vertexPosition;

public:

	Mesh(ID3D11Device* pDevice);

	std::vector<DirectX::XMFLOAT3> getModelVertecies();

	void setFilePath(std::wstring filePath);
	bool loadObjModel(ID3D11Device* device, std::wstring fileName, bool isRightHandCoordSystem, bool computeNormals);
	bool drawObjModel(ID3D11DeviceContext* immediateContext, ID3D11Buffer*& pConstantBuffer, Deferred deferred,
		ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader, ID3D11SamplerState* sampler, ID3D11Buffer*& pPixelConstantBuffer, Camera* camera);

	//Create a draw shadow function here maybe. 
	void shutDownMesh();
	void Animation(bool animation);
};