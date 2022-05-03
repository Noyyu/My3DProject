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

	//Places all the pixels in the fector as float values. (height values)
	std::vector<float> heightValue;
	for (int i = 0; i < textureHeight * textureWidth; i++)
	{
		heightValue.emplace_back(textureData[i]); //Emplace back does not make unnecessary copies. 
	}

	float x = 0;
	float y = 0;
	float z = 0;


	//Create locations for every pixel in the texture map in the 3D space. 
	for (int j = 0; j < textureHeight; j++)
	{
		for (int i = 0; i < textureWidth; i++)
		{
			//(I had problems geting c++ to devide things the right way, thats why i use static_cast<float>)
			x = float(static_cast<float>(i) / 10.0f); //Devide by 10 to not make it gigantic.(I made the plane 10 x 10 in Maya, the plane would have t be 100 x 100 to be the right size without the /10) 
			y = (heightValue[i + (j * textureHeight)] / 25.50f ) + 2; //Height goes up to 10 units in 3D Space (+ 2 for player height) (/ 25.5 cuz thats how high i made the height map in Maya )
			z = float(static_cast<float>(j) / 10.0f);

			DirectX::XMFLOAT3 tempVertexPosition = DirectX::XMFLOAT3(x,y,z);

			//now the pixel has a positon in te world. 
			this->vertexPosition.emplace_back(tempVertexPosition);
		}
	}

	stbi_image_free(textureData);
}

float HeightMap::getHegihtOnPosition(float x, float z) //Used to "teleport" the player to the right height.
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


