#include "Structs.h"
#include "Camera.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"



class Mesh
{
private:

	std::wstring filePath;
	float time = 0;

	std::vector<DWORD> indices; // DWORD https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-dtyp/262627d8-3418-4627-9218-4ffe110850b2
	int totalVertices = 0;
	int meshSubsets = 0;
	bool hasNormalMap = false;
	int animation = 0;

	constantBufferMatrixes objMats;
	MatrixFunctions matrixFunction;

	std::vector<int> meshSubsetIndexStart = {};
	std::vector<int> meshSubsetTexture = {};
	std::vector<int> meshSubsetMaterialArray = {};

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

public:

	Mesh(ID3D11Device* pDevice);
	virtual ~Mesh() = default;
	void setFilePath(std::wstring filePath);
	bool loadObjModel(ID3D11Device* device, std::wstring fileName, bool isRightHandCoordSystem, bool computeNormals);

	void drawObjModel(ID3D11DeviceContext* immediateContext, ID3D11Buffer*& pConstantBuffer, Deferred deferred,
		ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader, ID3D11SamplerState* sampler, ID3D11Buffer*& pPixelConstantBuffer, Camera* camera);

	void DrawShadow(ID3D11DeviceContext* immediateContext, Camera* camera, ID3D11Buffer*& pConstantBuffer);

	void createObjectConstantBuffer(ID3D11Device* device);

	//Create a draw shadow function here maybe. 
	void shutDownMesh();
	void Animation(bool animation);

};