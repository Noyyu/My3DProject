#include "Mesh.h"
#include<iostream>
#include<string>



Mesh::Mesh(ID3D11Device* pDevice)
    :model(nullptr), perObjectConstantBuffer(std::make_unique<ConstantBuffer>(pDevice, sizeof(constantBufferMatrixes)))
{
}

std::vector<DirectX::XMFLOAT3> Mesh::getModelVertecies()
{
    return this->vertexPosition;
}

void Mesh::setFilePath(std::wstring filePath)
{
    this->filePath = filePath;
}

bool Mesh::loadObjModel(ID3D11Device* device, std::wstring fileName, bool isRightHandCoordSystem, bool computeNormals)
{
    using namespace DirectX; //Needed for vector operations
    HRESULT hr = {};

    //open file
    std::wifstream fileIn(fileName.c_str()); // wifstream http://www.cplusplus.com/reference/fstream/wifstream/ c_str http://www.cplusplus.com/reference/string/string/c_str/
    std::wstring meshMaterialLib = {};

    //array that stores mesh information
    std::vector<DWORD> indices = {}; // DWORD https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-dtyp/262627d8-3418-4627-9218-4ffe110850b2
    std::vector<DirectX::XMFLOAT3> vertexPosition = {};
    std::vector<DirectX::XMFLOAT3> vertexNormal = {};
    std::vector<DirectX::XMFLOAT2> vertexTextureCoordinates = {};
    std::vector<std::wstring> meshMaterials = {};

    //Vertex definition indices
    std::vector<int> vertexPositionIndex = {};
    std::vector<int> vertexNormalIndex = {};
    std::vector<int> vertexTextureCoordinateIndex = {};

    bool hasTextureCoordinates = false;
    bool hasNormals = false;

    //temp vairables to store things
    std::wstring meshMaterialsTemp = {};
    int vertexPositionIndexTemp = 0;
    int vertexNormalIndexTemp = 0;
    int vertexTextureCoordinatesIndexTemp = 0;

    wchar_t checkChar = {}; //stores one char from file at a time.  wchar_t https://docs.microsoft.com/en-us/cpp/cpp/char-wchar-t-char16-t-char32-t?view=msvc-160
    std::wstring face = {}; //holds the string containing the face vertices.
    int vertexIndex = 0; //Used to keep track of vertex index when for example knowing what vertex a new subset start or when a new face start. 
    int triangleCount = 0; // total triangles
    totalVertices = 0; 
    int meshTriangles = 0;


    ///----- Opening the OBJ file -----///
    //checks if the file can be opened. If i add full screen mode some time, make sure we are out if it when this happenes
    if (fileIn)
    {
        std::cout << "OBJ file is open" << std::endl;
    }
    else
    {
        std::cout << "OBJ file could not be opened" << std::endl;
        //oh no

        return false;
    }

    //Time to read the file
    //This while loop will go untill the end of the file. Reads char by char. 

    while (fileIn)
    {
        //1: checks if a line is a comment with #, in that case, just get() untill we reach a \n. 
        //2: checks if its a vertex position by looking for a "v", if it was, store the valuse. 
        //3: checks if this was a right handed coordinate system, if it was (which it probably will be), we inverse the z value (* -1.0f) and then push back the valuse in the vertexPosition vector.
        //4: do the same thing with the textrue coordinates. intvert the v if its a right handed coordinate system. 
        //5: check for normals, same as looking for vertex position, tho after pusing the normal into the vertexNormal vector, it is needed to set the hasNormals to true.
        //6: checking for "g's". whenever a g is found the subset count is increased and a new meshSubsetIndexStart is pushed back. 
        //7: face information
        //8: check for material "mtllib". I will only support one per object >:c Store the mtllib name in the meshMaterialLib string. 
        //9: get group material "usemtl" and push it back into the meshMAterials vector. 

        checkChar = fileIn.get(); //gets the next char

        switch (checkChar)
        {
        ///----- Comment ----- ///

        case '#': // 1 check if comment
            checkChar = fileIn.get();
            while (checkChar != '\n')
            {
                checkChar = fileIn.get();
            }
            break;


        ///----- Vertex "settings" -----///
        //-------------------------------//
        
        case 'v': // 2: Vertex position
            checkChar = fileIn.get();
            if (checkChar == ' ') // checks if there's a space after the v to see if its a vertex
            {
                float vx = {}, vy = {}, vz = {};
                fileIn >> vx >> vy >> vz; //Stores the vertices in the float variables

                //std::cout << vx << vy << vz << std::endl;

                //3: Checks if the model uses a right hand coordinatiopn system, if so we invert the z value. 
                if (isRightHandCoordSystem == true) 
                {
                    //vertexPosition.push_back(DirectX::XMFLOAT3(vx, vy, vz * -1.0f));
                    vertexPosition.push_back(DirectX::XMFLOAT3(vx, vy, vz));
                }
                else
                {
                    vertexPosition.push_back(DirectX::XMFLOAT3(vx, vy, vz));
                }
            }
            ///----- Vertex texture coordinate -----///

            if (checkChar == 't') //4: checks if theres a "t" after the "v" to see if its a vertex texture coodinate (vtc)
            {
                float vtcu = {}, vtcv = {};
                fileIn >> vtcu >> vtcv; //Stores the texture coordinates (2d) in the float variables.

                if (isRightHandCoordSystem) // 3 checks if its right handed again, if so, remove 1 from the v coordinate. 
                {
                    //vertexTextureCoordinates.push_back(DirectX::XMFLOAT2(vtcu, vtcv));
                    vertexTextureCoordinates.push_back(DirectX::XMFLOAT2(vtcu, 1.0f - vtcv));
                }
                else
                {
                    vertexTextureCoordinates.push_back(DirectX::XMFLOAT2(vtcu, 1.0f - vtcv));
                }

                hasTextureCoordinates = true;
            }

            ///----- Vertex Normals -----///
            if (checkChar == 'n') // 5, chekcs if theres a "n" after the "v" to see if this is vertex normals (vn)
            {
                float vnx = {}, vny = {}, vnz = {};
                fileIn >> vnx >> vny >> vnz; //stores the vertex normals in the float variables. 
                
                if (isRightHandCoordSystem) // inverts z if this is a right hand coordinate system
                {
                    //vertexNormal.push_back(DirectX::XMFLOAT3(vnx, vny, vnz * -1.0f));
                    vertexNormal.push_back(DirectX::XMFLOAT3(vnx, vny, vnz));
                }
                else
                {
                    vertexNormal.push_back(DirectX::XMFLOAT3(vnx , vny, vnz));
                }

                hasNormals == true;
            }
            break;

        ///----- Groups ----- ///
        //---------------------//
        case 'g': // 6, g, group, is a set of faces that uses the same attributes, like makterials. Groups are also called "subsets"
            checkChar = fileIn.get();
            if (checkChar == ' ')
            {
                meshSubsetIndexStart.push_back(vertexIndex);
                meshSubsets++;
            }
            break;


        ///----- Face attributes -----///
        //-----------------------------//

        case 'f': // 7, get faces
            checkChar = fileIn.get();
            //Get the whole line untill the theres a space. 
            if (checkChar == ' ') //Checks for spaces to see when the next vertex starts. 
            {
                face = L""; //L means "wchar_t" 
                std::wstring vertexDefinition = {}; //hold one vertex definition at a time
                triangleCount = 0;

                checkChar = fileIn.get();
                while (checkChar != '\n') //As long as the line has not ended
                {
                    face += checkChar;//add a char to the face wchar_t

                    checkChar = fileIn.get();
                    if (checkChar == ' ') //if theres a space...
                    {
                        triangleCount++; //theres a triangle and we add it to the triangle count
                    }
                }
                //At the end of this ^ we should have a char filled with the definition of one face.

                //makes sure the last char of the face line was not a space, as a space would add another triangle, and that would make a non existent triangle. 
                if (face[face.length() - 1] == ' ')
                {
                    triangleCount--;
                }

                triangleCount -= 1; //Ever vertex in the face AFTER the first two are new faces

                std::wstringstream stringStream(face); //Stores the face data (vertex position, vertex texture coordinate , vertex normal) into the stringStream

                if (face.length() > 0)
                {
                    int firstVertexIndex = {}, lastVertexIndex = {};//Holds the first and last vertex indexes. these are used for triangulating the thing later. 

                    for (int i = 0; i < 3; ++i) //this loop will exit when the first three vertices in the face have been turned into a tringale and stored in the index list. 
                    {
                        stringStream >> vertexDefinition; //Send the face data to the vertexDefinition

                        std::wstring vertexPart = {};
                        int whichPart = 0; // checks if its vertex pos, vertex tc or vertex normal, this one gets added on after every itteration. 

                        for (int j = 0; j < vertexDefinition.length(); ++j) //goes through the face data string
                        {
                            if (vertexDefinition[j] != '/') //If there is no divider "/", add a char to vertexPart
                            {
                                vertexPart += vertexDefinition[j];
                            }

                            if (vertexDefinition[j] == '/' || j == vertexDefinition.length()-1) //if the current char is a devider, or the last character in the string the description for the face can be set to the temps
                            {   
                                ///----- writes down the face description values -----///


                                std::wistringstream wstringToInt(vertexPart); //Convert this to a int. wistringstream http://www.cplusplus.com/reference/sstream/wistringstream/



                                ///---------- Vertex Position ----------///
                                
                                if (whichPart == 0) //if we're at vertex position
                                {
                                    wstringToInt >> vertexPositionIndexTemp; //This one becomes 1,2,3
                                    vertexPositionIndexTemp -= 1; //subtract one since c++ arrays start with 0, and obj start with 1

                                    //check to see if the vertex position was only thing specified
                                    if (j == vertexDefinition.length()-1)
                                    {
                                        vertexNormalIndexTemp = 0;
                                        vertexTextureCoordinatesIndexTemp = 0;
                                    }
                                }

                                ///---------- Vertex Texture Coordinate ----------///

                                else if (whichPart == 1) //if we're at vertex tecture coordinates pos
                                {
                                    if (vertexPart != L"")
                                    {
                                        wstringToInt >> vertexTextureCoordinatesIndexTemp;
                                        vertexTextureCoordinatesIndexTemp -= 1; //subtract one since c++ arrays start with 0, and obj start with 1
                                    }
                                    else //if there is no texture coordinates, make it default, or 0. 
                                    {
                                        vertexTextureCoordinatesIndexTemp = 0;
                                    }
                                    //if the current char is the second to last in the string, then there must be no normal, so set to default or 0. 
                                    if (j == vertexDefinition.length() - 1)
                                    {
                                        vertexNormalIndexTemp = 0;
                                    }
                                }

                                ///---------- Vertex Normal ----------///

                                else if (whichPart == 2) //if we're at the vertex normal position
                                {
                                    std::wistringstream wstringToInt(vertexPart);

                                    wstringToInt >> vertexNormalIndexTemp;
                                    vertexNormalIndexTemp -= 1; //you know why we do this. 
                                }

                                vertexPart = L""; //Resets it and makes it ready for the next itteration. 
                                whichPart++; //moves to the next vertex part. 
                            }
                        }

                        //here maybe?
                        //check to make sure there is at least one subset (As we know a subset is a group with the same attributes. there need to be at least one.)
                        if (meshSubsets == 0)
                        {
                            meshSubsetIndexStart.push_back(vertexIndex); //start index for this subset
                            meshSubsets++;
                        }

                        //Avoid dublicate verts (For index buffer stuff)
                        bool vertAlreadyExists = false;

                        //If this vertex doen't already exist, put it in the array. 
                        if (vertAlreadyExists == false)
                        {
                            vertexPositionIndex.push_back(vertexPositionIndexTemp);
                            vertexTextureCoordinateIndex.push_back(vertexTextureCoordinatesIndexTemp);
                            vertexNormalIndex.push_back(vertexNormalIndexTemp);
                            totalVertices++;
                            indices.push_back(totalVertices - 1); //Set index for this vertex. 
                        }

                        //If this is the very first vertex in the face, i need to make sure the rest of the tirangles use use this vertex
                        if (i == 0)
                        {
                            firstVertexIndex = indices[vertexIndex]; //The first vertex index of this -face-. 
                        }
                        //if this was the last vertex in the first triangle, i'll make sure the next trianglr uses this one (eg. tri1(1,2,3) tri2(1,3,4) tri3(1,4,5))
                        if (i == 2)
                        {
                            lastVertexIndex = indices[vertexIndex]; //The last vertex index of the -triangle-.
                        }
                        vertexIndex++;
                    }

                    meshTriangles++; //Omg this is my triangle :DDD

                    //If the model were to not be triangulated already, do it here. But i dont want to just keep fucking teack of your meshes. 
                    //Do it anyway.

                    //If there ar more then 3 verts in a face, thats not a triangle. Lets convert it to a triangle. 

                    for (int l = 0; l < triangleCount-1; l++)
                    {
                        indices.push_back(firstVertexIndex);
                        vertexIndex++;

                        indices.push_back(lastVertexIndex);
                        vertexIndex++;

                        stringStream >> vertexDefinition;

                        std::wstring vertPart;
                        int whichPart = 0;

                        for (int j = 0; j < vertexDefinition.length(); j++)
                        {
                            if (vertexDefinition[j] != '/')
                            {
                                vertPart += vertexDefinition[j];
                            }
                            if (vertexDefinition[j] == '/' || j == vertexDefinition.length() -1)
                            {
                                std::wstringstream wstringToInt(vertPart);

                                if (whichPart == 0)
                                {
                                    wstringToInt >> vertexPositionIndexTemp;
                                    vertexPositionIndexTemp -= 1;

                                    //Check to see if the vert pos was the only thing specified
                                    if (j == vertexDefinition.length()-1)
                                    {
                                        vertexTextureCoordinatesIndexTemp = 0;
                                        vertexNormalIndexTemp = 0;
                                    }
                                }
                                else if (whichPart == 1)
                                {
                                    if (vertPart != L"")
                                    {
                                        wstringToInt >> vertexTextureCoordinatesIndexTemp;
                                        vertexTextureCoordinatesIndexTemp -= 1;
                                    }
                                    else
                                    {
                                        vertexTextureCoordinatesIndexTemp = 0;
                                    }
                                    if (j == vertexDefinition.length() -1)
                                    {
                                        vertexNormalIndexTemp = 0;
                                    }
                                }
                                else if (whichPart == 2)
                                {
                                    std::wistringstream wstringToInt(vertPart);
                                    wstringToInt >> vertexNormalIndexTemp;
                                    vertexNormalIndexTemp -= 1;
                                }

                                vertPart = L"";
                                whichPart++;
                            }
                        }
                        //Check for double verts
                        bool vertexAlreadyExists = false;
                        if (totalVertices >= 3)
                        {
                            for (int iCheck = 0; iCheck < totalVertices; iCheck++)
                            {
                                if (vertexPositionIndexTemp == vertexPositionIndex[iCheck] && !vertexAlreadyExists)
                                {
                                    if (vertexTextureCoordinatesIndexTemp == vertexTextureCoordinateIndex[iCheck])
                                    {
                                        indices.push_back(iCheck);
                                        vertexAlreadyExists = true;

                                    }
                                }
                            }
                        }

                        if (vertexAlreadyExists == false)
                        {
                            vertexPositionIndex.push_back(vertexPositionIndexTemp);
                            vertexTextureCoordinateIndex.push_back(vertexTextureCoordinatesIndexTemp);
                            vertexNormalIndex.push_back(vertexNormalIndexTemp);
                            totalVertices++;                             //New vertex created, add to total verts
                            indices.push_back(totalVertices - 1);        //Set index for this vertex
                        }

                        //Set the second vertex for the next triangle to yhe last vertex i got.
                        lastVertexIndex = indices[vertexIndex];
                        meshTriangles++;
                        vertexIndex++;
                    }
                }
            }
            //I could add triangulation here but ima skip that for now
            break;

        case 'm': // 8, mtllib

            checkChar = fileIn.get();
            if (checkChar == 't')
            {
                checkChar = fileIn.get();
                if (checkChar == 'l')
                {
                    checkChar = fileIn.get();
                    if (checkChar == 'l')
                    {
                        checkChar = fileIn.get();
                        if (checkChar == 'i')
                        {
                            checkChar = fileIn.get();
                            if (checkChar == 'b')
                            {
                                checkChar = fileIn.get();
                                if (checkChar == ' ')
                                {
                                    fileIn >> meshMaterialLib;
                                    //What a nice little pyramid c:
                                }
                            }
                        }
                    }
                }
            }
            break;


        case 'u':// 9, usemtl
            checkChar = fileIn.get();
            if (checkChar == 's')
            {
                checkChar = fileIn.get();
                if (checkChar == 'e')
                {
                    checkChar = fileIn.get();
                    if (checkChar == 'm')
                    {
                        checkChar = fileIn.get();
                        if (checkChar == 't')
                        {
                            checkChar = fileIn.get();
                            if (checkChar == 'l')
                            {
                                checkChar = fileIn.get();
                                if (checkChar == ' ')
                                {
                                    meshMaterialsTemp = L"";    //Make sure this is cleared

                                    fileIn >> meshMaterialsTemp; //Get next type (string)

                                    meshMaterials.push_back(meshMaterialsTemp);
                                    //antoher one
                                }
                            }
                        }
                    }
                }
            }
            break;
        default:
            break;
        }
    }

    //1: push back the final meshSubsetIndexStart
    //2: make sure the subset actually contains vertices
    //3: Make sure we have default texture coordinates and normals.
    //4: close the obj file
    //5: create some stuff to prepare for opening the mtl file
    //6: open the mtl file

    //1: 
    meshSubsetIndexStart.push_back(vertexIndex);

    //2: Sometimes theres a random group at the beginning of a obj file that's empty. Remove that shit. 
    if (meshSubsetIndexStart[1] == 0)
    {
        meshSubsetIndexStart.erase(meshSubsetIndexStart.begin() + 1);
        meshSubsets--;
    }

    //3: check for defaults
    if (!hasNormals)
    {
        vertexNormal.push_back(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
    }
    if (!hasTextureCoordinates)
    {
        vertexTextureCoordinates.push_back(DirectX::XMFLOAT2(0.0f, 0.0f));
    }

    //4: close obj file
    fileIn.close();

    std::wstring path = this->filePath;

    //5: make stuff
    fileIn.open(path + meshMaterialLib.c_str());
    std::wstring lastStringRead = {};
    int materialCount = material.size();

    //kdset - If the diffuse color was not set, use ambient color. If the diffuse WAS set, dont set diffuse color or ambient
    bool kdset = false;

    //6: open mtl file
    if (fileIn)
    {
        std::cout << "MTL opened" << std::endl;
    }
    else
    {
        std::cout << "MTL Could not be opened" << std::endl;
    }
    
    /// MTL FILE
    //1: make a while loop that lasts untill the end of the file.
    //2: get character with the get() command
    //3: make a switch case thing again
    //3.1: What is needed for this asignment is: Diffues (kd), specular (ks) ambient (Ka) and texture
    //3.2: Read kd (diffuse)
    //3.3: read ks (specular)
    //3.4: read ka (ambient)
    //3.5: read texture: map_kd

    //1:
    while (fileIn)
    {
        //2: 
        checkChar = fileIn.get();

        switch (checkChar)
        {
        //Ignores comments
        case '#':
            checkChar = fileIn.get();
            while (checkChar != '\n')
            {
                checkChar = fileIn.get();
            }
            break;

        //Now geting the k's
        case 'K':
            checkChar = fileIn.get();

            if (checkChar == 'd') // if this is a Kd line (diffuse)
            {
                checkChar = fileIn.get(); //Remove space

                fileIn >> material[materialCount - 1].color.x;
                fileIn >> material[materialCount - 1].color.y;
                fileIn >> material[materialCount - 1].color.z;
            }

            if (checkChar == 'a') //Checks if this line is a Ka (ambient)
            {
                checkChar = fileIn.get(); //remove space

                fileIn >> material[materialCount - 1].ambient.x;
                fileIn >> material[materialCount - 1].ambient.y;
                fileIn >> material[materialCount - 1].ambient.z;
            }

            if (checkChar == 's') //checks if this line has specular
            {
                checkChar = fileIn.get(); //remove space

                fileIn >> material[materialCount - 1].specular.x;
                fileIn >> material[materialCount - 1].specular.y;
                fileIn >> material[materialCount - 1].specular.z;
            }

            break;


            //-----------------------------------------------------------------//
            // Load Textures / Maps
            //-----------------------------------------------------------------//
        case 'm':
        
            checkChar = fileIn.get();
            if (checkChar == 'a')
            {
                checkChar = fileIn.get();
                if (checkChar == 'p')
                {
                    checkChar = fileIn.get();
                    if (checkChar == '_')
                    {
                        //-----------------------------------------------------------------//
                        // Diffuse map
                        //-----------------------------------------------------------------//
                        checkChar = fileIn.get(); 
                        if (checkChar == 'K')
                        {
                            checkChar = fileIn.get();
                            if (checkChar == 'd')
                            {
                                std::wstring fileNamePath = {};

                                fileIn.get(); //remove space between map_Kd and the file

                                //Get the file path, read the path name char by char since pathnames can sometimes contain spaces.
                                //read untill a file extension is found. 

                                bool textureFilePathEnd = false;
                                while (!textureFilePathEnd)
                                {
                                    checkChar = fileIn.get();

                                    fileNamePath += checkChar;

                                    if (checkChar == '.') //file extention (like .jpg)
                                    {
                                        for (int i = 0; i < 3; ++i) //Will only spoort file extentions with 3 letter (which is fine >:c)
                                        {
                                            fileNamePath += fileIn.get();
                                            textureFilePathEnd = true;
                                        }
                                    }
                                }

                                fileNamePath = this->filePath + fileNamePath;

                                //check if this texture is already loaded
                                bool alreadyLoaded = false;

                                for (int i = 0; i < textureNameArray.size(); ++i)
                                {
                                    if (fileNamePath == textureNameArray[i])
                                    {
                                        alreadyLoaded = true;
                                        material[materialCount - 1].textureArrayIndex = i;
                                        material[materialCount - 1].hasTexture = true;
                                    }
                                }

                                //if the texture is not already loaded, load it now
                                if (!alreadyLoaded)
                                {
                                    ID3D11ShaderResourceView* tempMeshShaderResourceView;

                                    //Use the stb lib to load the picture data and then make a D3D11Texture2D and its views (shaderResourceView, shaderTargetView)
                                    //hur fan då
                                    //I'll have to like.. convert wstring to cont char, but wstring is unicode and char is ASCII.
                                    int textureWidth = 0; 
                                    int textureHeight = 0; 
                                    int channels = 0; 
                                        
                                    _bstr_t imageFilePath(fileNamePath.c_str());
                                    unsigned char* image = stbi_load(imageFilePath, &textureWidth, &textureHeight,&channels,STBI_rgb_alpha);

                                    D3D11_TEXTURE2D_DESC desc = {};
                                    desc.Width = textureWidth;
                                    desc.Height = textureHeight;
                                    desc.MipLevels = 1; // Use 1 for a multisampled texture
                                    desc.ArraySize = 1; //One buffer
                                    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                                    desc.SampleDesc.Count = 1; // The number of multisamples per pixel.
                                    desc.SampleDesc.Quality = 0;
                                    desc.Usage = D3D11_USAGE_IMMUTABLE;//can only be read by the GPU and can not the written by the GPU. 
                                    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // Bind a buffer or texture to a shader stage
                                    desc.CPUAccessFlags = 0;
                                    desc.MiscFlags = 0;

                                    //Information about D3D11_SUBRESOURCE_DATA https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_subresource_data
                                    D3D11_SUBRESOURCE_DATA data = {};
                                    data.pSysMem = image;
                                    data.SysMemPitch = textureWidth * 4;
                                    data.SysMemSlicePitch = 0;

                                    hr = device->CreateTexture2D(&desc, &data, &mtlTexture);

                                    if ((FAILED(hr)))
                                    {
                                        std::cout << "Could not find Texture for MTL file" << std::endl;
                                    };

                                    stbi_image_free(image);

                                    hr = device->CreateShaderResourceView(mtlTexture, nullptr, &mtlShaderResourceView);
                                    if (FAILED(hr)) { return false; };


                                    if (SUCCEEDED(hr))
                                    {
                                        textureNameArray.push_back(fileNamePath.c_str());
                                        material[materialCount - 1].textureArrayIndex = meshShaderResourceView.size();
                                        meshShaderResourceView.push_back(mtlShaderResourceView);
                                        material[materialCount - 1].hasTexture = true;
                                    }
                                    if (FAILED(hr)) { return false; };

                                }
                            }
                        }
                    }
                }
            }

            break;

        //-----------------------------------------------------------------//
        // Normal map
        //-----------------------------------------------------------------//
        case 'b': //bump

            checkChar = fileIn.get();
            if (checkChar == 'u')
            {
                checkChar = fileIn.get();
                if (checkChar == 'm')
                {
                    checkChar = fileIn.get();
                    if (checkChar == 'p')
                    {
                        checkChar = fileIn.get(); //space
                        checkChar = fileIn.get(); //space
                        checkChar = fileIn.get(); //-
                        checkChar = fileIn.get(); //b
                        checkChar = fileIn.get(); //m
                        checkChar = fileIn.get(); //space
                        checkChar = fileIn.get(); //1
                        checkChar = fileIn.get(); //space

                        std::wstring fileNamePath = {};

                        //Get the file path, read the path name char by char since pathnames can sometimes contain spaces.
                        //read untill a file extension is found. 

                        bool textureFilePathEnd = false;
                        while (!textureFilePathEnd)
                        {
                            checkChar = fileIn.get();

                            fileNamePath += checkChar;

                            if (checkChar == '.') //file extention (like .jpg)
                            {
                                for (int i = 0; i < 3; ++i) //Will only spoort file extentions with 3 letter (which is fine >:c)
                                {
                                    fileNamePath += fileIn.get();
                                    textureFilePathEnd = true;
                                }
                            }
                        }

                        fileNamePath = this->filePath + fileNamePath;


                        //check if this texture is already loaded
                        bool alreadyLoaded = false;

                        for (int i = 0; i < textureNameArray.size(); ++i)
                        {
                            if (fileNamePath == textureNameArray[i])
                            {
                                alreadyLoaded = true;
                                material[materialCount - 1].textureArrayIndex = i;
                                material[materialCount - 1].hasTexture = true;
                            }
                        }

                        //if the texture is not already loaded, load it now
                        if (!alreadyLoaded)
                        {
                            ID3D11ShaderResourceView* tempMeshShaderResourceView; //Hittar inte vart denna används just nu.

                            //Use the stb lib to load the picture data and then make a D3D11Texture2D and its views (shaderResourceView, shaderTargetView)
                            //hur fan då
                            //I'll have to like.. convert wstring to cont char, but wstring is unicode and char is ASCII.
                            int textureWidth = 0;
                            int textureHeight = 0;
                            int channels = 0;

                            _bstr_t imageFilePath(fileNamePath.c_str());
                            unsigned char* image = stbi_load(imageFilePath, &textureWidth, &textureHeight, &channels, STBI_rgb_alpha);

                            D3D11_TEXTURE2D_DESC desc = {};
                            desc.Width = textureWidth;
                            desc.Height = textureHeight;
                            desc.MipLevels = 1; // Use 1 for a multisampled texture
                            desc.ArraySize = 1; //One buffer
                            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                            desc.SampleDesc.Count = 1; // The number of multisamples per pixel.
                            desc.SampleDesc.Quality = 0;
                            desc.Usage = D3D11_USAGE_IMMUTABLE;//can only be read by the GPU and can not the written by the GPU. 
                            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // Bind a buffer or texture to a shader stage
                            desc.CPUAccessFlags = 0;
                            desc.MiscFlags = 0;

                            //Information about D3D11_SUBRESOURCE_DATA https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_subresource_data
                            D3D11_SUBRESOURCE_DATA data = {};
                            data.pSysMem = image;
                            data.SysMemPitch = textureWidth * 4;
                            data.SysMemSlicePitch = 0;

                            hr = device->CreateTexture2D(&desc, &data, &mtlNormalTexture);

                            if ((FAILED(hr)))
                            {
                                std::cout << "Could not find Normal Texture for MTL file" << std::endl;
                                this->hasNormalMap = false;
                            };

                            stbi_image_free(image);

                            hr = device->CreateShaderResourceView(mtlNormalTexture, nullptr, &mtlNormalShaderResourceView);
                            if (FAILED(hr))
                            { 
                                this->hasNormalMap = true;
                                return false;
                            };


                            if (SUCCEEDED(hr)) //TODO Idono if this works. 
                            {
                                textureNameArray.push_back(fileNamePath.c_str());
                                material[materialCount - 1].textureArrayIndex = meshShaderResourceView.size();
                                meshShaderResourceView.push_back(mtlNormalShaderResourceView);
                                material[materialCount - 1].hasNormalMap = true;
                                this->hasNormalMap = true;
                            }
                            if (FAILED(hr)) 
                            {
                                this->hasNormalMap = true;
                                return false;
                            };
                        }
                    }
                }
            }
            break;


        case 'n': //newmtl - Declares new material

            checkChar = fileIn.get();
            if (checkChar == 'e')
            {
                checkChar = fileIn.get();
                if (checkChar == 'w')
                {
                    checkChar = fileIn.get();
                    if (checkChar == 'm')
                    {
                        checkChar = fileIn.get();
                        if (checkChar == 't')
                        {
                            checkChar = fileIn.get();
                            if (checkChar == 'l')
                            {
                                checkChar = fileIn.get();
                                if (checkChar == ' ')
                                {
                                    //New material, set it's default
                                    SurfaceMaterial tempmaterial;
                                    material.push_back(tempmaterial);
                                    fileIn >> material[materialCount].materialName;
                                    material[materialCount].hasTexture = false;
                                    material[materialCount].textureArrayIndex = 0;
                                    material[materialCount].hasNormalMap = false;
                                    material[materialCount].normalMapArrayIndex = 0;
                                    materialCount++;
                                    kdset = false;

                                }
                            }
                        }
                    }
                }
            }
            break;

        default:
            break;
        }
    }

    //Now that we have loaded all our materials, we need to basically bind each subset to it's material. We do this by looping through each subset, 
    //and comparing its material name stored in meshMaterials with each of the material names stored in matName of the material vector. 
    //When we come across the right material in the material vector, we push back subsetMaterialArray to the index element of the material vector. 
    //Then we set hasMat to true to exit the loop. If all the materials in the material vector have been checked, and the subsets material was not there, 
    //we will give it a default material, which will be the first material in the material vector.

    //Set the subsets material to the index value of the material in the material array. 
    for (int i = 0; i < meshSubsets; ++i)
    {
        bool hasMaterial = false;
        for (int j = 0; j < material.size(); j++)
        {
            if (meshMaterials[i] == material[j].materialName)
            {
                meshSubsetMaterialArray.push_back(j);
                hasMaterial = true;
            }
        }
        if (hasMaterial == false)
        {
            meshSubsetMaterialArray.push_back(0); // use the first material in the array
        }
    }



    //-----------Create vertices-----------///
   //-------------------------------------///

    std::vector<Vertex> vertices;
    Vertex tempVertex = {};
    //MeshData meshData;
    //Add all the vertex attributes into a Mesh Data Struct for tangent calculations. 
    for (int j = 0; j < totalVertices; ++j)
    {
        tempVertex.pos = vertexPosition[vertexPositionIndex[j]];
        
        tempVertex.normal = vertexNormal[vertexNormalIndex[j]];
        tempVertex.textureCoordenates = vertexTextureCoordinates[vertexTextureCoordinateIndex[j]];

        vertices.push_back(tempVertex);

        vertexPosition.emplace_back(vertexPosition[vertexPositionIndex[j]]); //For the hight map later. 
    }

    vertexPosition.shrink_to_fit();


    //----------- Compute normals -----------///
    //--------------------------------------///
    //If computeNormals was set to true then we will create our own
    //normals, if it was set to false we will use the obj files normals
    if (computeNormals)
    {
        std::vector<DirectX::XMFLOAT3> tempNormal;

        //normalized and unnormalized normals
        DirectX::XMFLOAT3 unnormalized = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

        //tangent stuff
        std::vector<DirectX::XMFLOAT3> tempTangent;
        DirectX::XMFLOAT3 tangent = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        float tcU1, tcV1, tcU2, tcV2;

        //Used to get vectors (sides) from the position of the verts
        float vecX, vecY, vecZ;

        //Two edges of our triangle
        DirectX::XMVECTOR edge1 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        DirectX::XMVECTOR edge2 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

        //Compute face normals
        //And Tangents
        for (int i = 0; i < meshTriangles; ++i)
        {
            //Get the vector describing one edge of our triangle (edge 0,2)
            vecX = vertices[indices[(i * 3)]].pos.x - vertices[indices[(i * 3) + 2]].pos.x; //indices används inte just nu så därför funkar det inte. 
            vecY = vertices[indices[(i * 3)]].pos.y - vertices[indices[(i * 3) + 2]].pos.y;
            vecZ = vertices[indices[(i * 3)]].pos.z - vertices[indices[(i * 3) + 2]].pos.z;
            edge1 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f);    //Create our first edge

            //Get the vector describing another edge of our triangle (edge 2,1)
            vecX = vertices[indices[(i * 3) + 2]].pos.x - vertices[indices[(i * 3) + 1]].pos.x;
            vecY = vertices[indices[(i * 3) + 2]].pos.y - vertices[indices[(i * 3) + 1]].pos.y;
            vecZ = vertices[indices[(i * 3) + 2]].pos.z - vertices[indices[(i * 3) + 1]].pos.z;
            edge2 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f);    //Create our second edge

            //Cross multiply the two edge vectors to get the un-normalized face normal
            XMStoreFloat3(&unnormalized, DirectX::XMVector3Cross(edge1, edge2));

            tempNormal.push_back(unnormalized);

            //Find first texture coordinate edge 2d vector
            tcU1 = vertices[indices[(i * 3)]].textureCoordenates.x - vertices[indices[(i * 3) + 2]].textureCoordenates.x;
            tcV1 = vertices[indices[(i * 3)]].textureCoordenates.y - vertices[indices[(i * 3) + 2]].textureCoordenates.y;

            //Find second texture coordinate edge 2d vector
            tcU2 = vertices[indices[(i * 3) + 2]].textureCoordenates.x - vertices[indices[(i * 3) + 1]].textureCoordenates.x;
            tcV2 = vertices[indices[(i * 3) + 2]].textureCoordenates.y - vertices[indices[(i * 3) + 1]].textureCoordenates.y;

            //Find tangent using both tex coord edges and position edges
            tangent.x = (tcV1 * DirectX::XMVectorGetX(edge1) - tcV2 * DirectX::XMVectorGetX(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
            tangent.y = (tcV1 * DirectX::XMVectorGetY(edge1) - tcV2 * DirectX::XMVectorGetY(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
            tangent.z = (tcV1 * DirectX::XMVectorGetZ(edge1) - tcV2 * DirectX::XMVectorGetZ(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));

            tempTangent.push_back(tangent);
        }

        //Compute vertex normals (normal Averaging)
        DirectX::XMVECTOR normalSum = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        DirectX::XMVECTOR tangentSum = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        int facesUsing = 0;
        float tX, tY, tZ;    //temp axis variables

        //Go through each vertex
        for (int i = 0; i < totalVertices; ++i)
        {
            //Check which triangles use this vertex
            for (int j = 0; j < meshTriangles; ++j)
            {
                if (indices[j * 3] == i ||
                    indices[(j * 3) + 1] == i ||
                    indices[(j * 3) + 2] == i)
                {
                    tX = DirectX::XMVectorGetX(normalSum) + tempNormal[j].x;
                    tY = DirectX::XMVectorGetY(normalSum) + tempNormal[j].y;
                    tZ = DirectX::XMVectorGetZ(normalSum) + tempNormal[j].z;

                    normalSum = DirectX::XMVectorSet(tX, tY, tZ, 0.0f);    //If a face is using the vertex, add the unormalized face normal to the normalSum
       
                    //We can reuse tX, tY, tZ to sum up tangents
                    tX = DirectX::XMVectorGetX(tangentSum) + tempTangent[j].x;
                    tY = DirectX::XMVectorGetY(tangentSum) + tempTangent[j].y;
                    tZ = DirectX::XMVectorGetZ(tangentSum) + tempTangent[j].z;

                    tangentSum = DirectX::XMVectorSet(tX, tY, tZ, 0.0f); //sum up face tangents using this vertex

                    facesUsing++;
                }
            }

            //Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
            if (isRightHandCoordSystem)
            {
                normalSum = normalSum / facesUsing; 
                tangentSum = tangentSum / facesUsing;
            }
            else
            {
                normalSum = -normalSum / facesUsing; //invert cuz of coordinet system stuff
                tangentSum = -tangentSum / facesUsing;
            }

            //Normalize the normalSum vector and tangent
            normalSum = DirectX::XMVector3Normalize(normalSum);
            tangentSum = DirectX::XMVector3Normalize(tangentSum);

            //Store the normal and tangent in our current vertex
            vertices[i].normal.x = DirectX::XMVectorGetX(normalSum);
            vertices[i].normal.y = DirectX::XMVectorGetY(normalSum);
            vertices[i].normal.z = DirectX::XMVectorGetZ(normalSum);

            vertices[i].tangents.x = DirectX::XMVectorGetX(tangentSum);
            vertices[i].tangents.y = DirectX::XMVectorGetY(tangentSum);
            vertices[i].tangents.z = DirectX::XMVectorGetZ(tangentSum);

            //Clear normalSum, tangentSum and facesUsing for next vertex
            normalSum = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
            tangentSum = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
            facesUsing = 0;

        }
    }


    //-----------Create Index Buffer-----------///
    //------------------------------------------///

    D3D11_BUFFER_DESC indexBufferDesc = {};
    ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(DWORD) * meshTriangles * 3;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iinitData;

    iinitData.pSysMem = &indices[0];
    hr = device->CreateBuffer(&indexBufferDesc, &iinitData, &meshIndexBuffer);
    if (FAILED(hr))
    {
        std::cout << "Couldn't create Index Buffer" << std::endl;
        return false;
    }


    //-----------Create Vertex Buffer-----------///
    //------------------------------------------///

    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * totalVertices;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexBufferData;
    ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
    vertexBufferData.pSysMem = &vertices[0];
    hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &meshVertexBuffer);
    if (FAILED(hr))
    { 
        std::cout << "Couldn't create Vertex Buffer" << std::endl;
        return false; 
    };

    return true;
}


bool Mesh::drawObjModel(ID3D11DeviceContext* immediateContext, ID3D11Buffer*& pConstantBuffer, Deferred deferred,
    ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader, ID3D11SamplerState* sampler, ID3D11Buffer*& pPixelConstantBuffer, Camera* camera)
{
    for (int i = 0; i < meshSubsets; i++)
    {
        static UINT stride = sizeof(Vertex);
        static UINT offset = 0;

        //Set buffers
        immediateContext->IASetIndexBuffer(meshIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        immediateContext->IASetVertexBuffers(0, 1, &meshVertexBuffer, &stride, &offset);

        DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(1,1,1);
        DirectX::XMMATRIX translate = DirectX::XMMatrixTranslation(0,0,0);
        DirectX::XMMATRIX world = scale * translate;

        this->objMats.setWorld(world);
        this->objMats.setWVP(world * camera->getCameraView() * camera->getCameraProjection());

        this->objMats.setHasTexture(material[i].hasTexture);
        this->objMats.setHasNormal(material[i].hasNormalMap);

        immediateContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
        
        immediateContext->PSSetConstantBuffers(1, 1, &pConstantBuffer);

        //Skickar vidare textureen till den första slotten om den finns. 
        if (material[i].hasTexture == true)
        {
            this->objMats.setHasTexture(true);
            immediateContext->PSSetShaderResources(0, 1, &meshShaderResourceView[0]);
        }
        
        //Skicka in nomral map på den andra slotten om den finns. 
        if (material[i].hasNormalMap == true)
        {
            this->objMats.setHasNormal(true);
            immediateContext->PSSetShaderResources(1, 1, &meshShaderResourceView[1]);
        }

        this->time += 1;
        this->objMats.setTime(time);

        immediateContext->UpdateSubresource(pConstantBuffer, 0, NULL, &objMats, 0, 0);

        immediateContext->VSSetShader(vertexShader, nullptr, 0);
        immediateContext->PSSetShader(pixelShader, nullptr, 0);

        
        immediateContext->PSSetSamplers(0, 1, &sampler);
        immediateContext->PSSetConstantBuffers(0u, 1, &pPixelConstantBuffer);
        
        //deferred.setRenderTargets(immediateContext);
        immediateContext->Draw(totalVertices, 0);
    }
    return false;
}

//bool Mesh::drawOBJModelV2(ID3D11DeviceContext* immediateContext)
//{
//    if (this->model != nullptr)
//    {
//        ID3D11ShaderResourceView* SRV = nullptr;
//
//        immediateContext->PSSetShaderResources(0, 1, &SRV);
//        immediateContext->PSSetShaderResources(1, 1, &SRV);
//
//        static UINT stride = sizeof(Vertex);
//        static UINT offset = 0;
//
//        immediateContext->IASetVertexBuffers(0, 1, this->model->mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
//        immediateContext->IASetIndexBuffer(this->model->mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
//
//        DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(1, 1, 1);
//        DirectX::XMMATRIX translate = DirectX::XMMatrixTranslation(0, 0, 0);
//        DirectX::XMMATRIX world = scale * translate;
//
//        this->objMats.setWorld(world);
//        this->objMats.setWVP(world * camera->getCameraView() * camera->getCameraProjection());
//        
//       
//        //----------
//
//        immediateContext->VSSetConstantBuffers(0, 1, this->perObjectConstantBuffer->GetAddressOf());
//        immediateContext->DrawIndexed(this->model->mesh->indexBuffer.getIndexCount(), 0, 0);
//    }
//    return false;
//}

void Mesh::shutDownMesh()
{
    meshVertexBuffer->Release();
    meshIndexBuffer->Release();
}
