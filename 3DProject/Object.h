#include "pch.h"
#include "ConstantBuffer.h"
#include "Structs.h"

class Object
{
public:
	Object(ID3D11Device* pDevice);

	void setModel(Model* pModel);

	//Transforms
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);

	const sm::Vector3& GetPosition() const;
	const sm::Vector3& GetRotation() const;
	const sm::Vector3& GetScale() const;


	// Update and return the WorldMatrix.
	const DirectX::XMFLOAT4X4& GetMatrix();

	// Call this once every frame to render the object.
	void Draw(ID3D11DeviceContext* pDeviceContext);

private:

	//Model* model;

	DirectX::XMFLOAT4X4 worldMatrix;
	sm::Vector3 position;
	sm::Vector3 rotation;
	sm::Vector3 scale;

	std::unique_ptr<ConstantBuffer> perObjectConstantBuffer;
	std::unique_ptr<ConstantBuffer> materialConstantBuffer;

	void updateConstantBuffers(ID3D11DeviceContext* pDeviceContext);

};