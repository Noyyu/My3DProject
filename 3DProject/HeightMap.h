#pragma once
#include "Structs.h"

class HeightMap
{
public:
	HeightMap(const char* TextureName);

	void  readImageData();
	float getHegihtOnPosition(float x, float z);

private:

	int textureHeight;
	int textureWidth;
	const char* texturename;

	std::vector <DirectX::XMFLOAT3> vertexPosition;
};