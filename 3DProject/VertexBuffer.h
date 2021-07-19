#pragma once
#include "Structs.h"

class VertexBuffer
{
public:
	VertexBuffer();
	VertexBuffer(const VertexBuffer& other) = default;
	VertexBuffer(VertexBuffer&& other) = delete;
	VertexBuffer& operator=(const VertexBuffer& other) = default;
	VertexBuffer& operator=(VertexBuffer&& other) = delete;
	virtual ~VertexBuffer() = default;

	void setDevice(ID3D11Device * device);
	bool createVertexBuffer(const Vertex * vertexData, size_t size);

	ID3D11Buffer* const* GetAddressOf() const;
	ID3D11Buffer* Get() const;
private:
	ID3D11Device* pDevice;
	ComPtr<ID3D11Buffer> vertexBuffer;
};