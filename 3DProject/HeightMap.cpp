#include "HeightMap.h"

HeightMap::HeightMap(const char* TextureName)
	:textureHeight(0), textureWidth(0), texturename(TextureName)
{
	readImageData();
}

HeightMap::~HeightMap()
{
}

void HeightMap::readImageData()
{
	int channels;
	unsigned char* textureData = stbi_load(this->texturename, &textureWidth, &textureHeight, &channels, STBI_grey);

	if (textureHeight < 1)
	{
		std::cout << "textureHeight is smaller then 1" << std::endl;
	}

	std::vector<float> heightValue;
	for (int i = 0; i < textureHeight * textureWidth; i++)
	{
		heightValue.emplace_back(textureData[i]);
	}

	float x = 0;
	float y = 0;
	float z = 0;


	//Create locations for every pixel in the texture map in the 3D space. 
	for (int j = 0; j < textureHeight; j++)
	{
		for (int i = 0; i < textureWidth; i++)
		{
			x = float(static_cast<float>(i) / 10.0f); //Devide by 10 to not make it gigantic. 
			y = (heightValue[i + (j * textureHeight)] / 25.50f ) + 2; //Height goes up to 10 units in 3D Space
			z = float(static_cast<float>(j) / 10.0f);

			DirectX::XMFLOAT3 tempVertexPosition = DirectX::XMFLOAT3(x,y,z);

			this->vertexPosition.emplace_back(tempVertexPosition);
		}
	}

	stbi_image_free(textureData);
}

float HeightMap::getHegihtOnPosition(float x, float z)
{
	if (x > 0.0f && z > 0.0f)
	{
		float height = this->vertexPosition.at(static_cast<int>(x) + static_cast<int>(z) * textureHeight).y;
		return height;
		
	}
	else
	{
		return 0.0f;
	}
}


