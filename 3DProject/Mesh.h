#include "Structs.h"
#include "Camera.h"

class Mesh
{
private:

	std::wstring filePath;
	float time;

	std::vector<DWORD> indices; // DWORD https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-dtyp/262627d8-3418-4627-9218-4ffe110850b2
	int totalVertices;
	int meshSubsets;
	bool hasNormalMap;
	int animation;
	int followMe;
	int move;

	constantBufferMatrixes objMats;
	MatrixFunctions matrixFunction;

	std::vector<int> meshSubsetIndexStart;
	std::vector<int> meshSubsetTexture;
	std::vector<int> meshSubsetMaterialArray;

	ComPtr<ID3D11Texture2D> mtlTexture;
	ComPtr<ID3D11Texture2D> mtlNormalTexture;
	ComPtr<ID3D11ShaderResourceView> mtlShaderResourceView;
	ComPtr<ID3D11ShaderResourceView> mtlNormalShaderResourceView;
	ComPtr<ID3D11RenderTargetView> mtlRenderTargetView;

	ID3D11Buffer* meshVertexBuffer; //Needs to be released
	ID3D11Buffer* meshIndexBuffer;  //Needs to be released

	std::vector<SurfaceMaterial> material;
	std::vector<ID3D11ShaderResourceView*> meshShaderResourceView; //Needs to be released
	std::vector<std::wstring> textureNameArray; //https://www.cplusplus.com/reference/string/wstring/

public:

	Mesh();
	virtual ~Mesh() = default;

	void SetFilePath(std::wstring filePath);
	bool LoadObjModel(ID3D11Device*& device, std::wstring fileName, bool computeNormals);

	void DrawObjModel(ID3D11DeviceContext*& immediateContext, ID3D11Buffer*& pConstantBuffer, Deferred &deferred, ID3D11VertexShader*& vertexShader, ID3D11PixelShader*& pixelShader, ID3D11Buffer*& pPixelConstantBuffer, Camera* camera);
	void DrawShadow(ID3D11DeviceContext*& immediateContext, Camera* camera, ID3D11Buffer*& pConstantBuffer);

	//Create a draw shadow function here maybe. 
	void Animation(bool animation);
	void FollowMe(bool follow);

};