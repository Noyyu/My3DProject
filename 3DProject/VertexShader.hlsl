//hlsl stands for : high level shader language
//float 4 are for the homo... coordinates

// Registers
// t – for shader resourceviews (SRV)
// s – for samplers
// u – for unordered accessviews (UAV)
// b – for constant bufferviews (CBV)

struct Matrixes
{
    float4x4 WorldViewProjection;
    float4x4 World;
    float time;
    int hasTexture;
    int hasNormal;
    int animated;
};

cbuffer constantBuffer
{
    Matrixes matrixes;
};

struct VertexShaderInput
{
    float3 position : POSITION;
    float3 color : COLOR;
    float2 textureCoordinates : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};
struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float2 textureCoordinates : TEXCOORD;
    float4 normal : NORMAL;
    float4 worldPosition : POSITION; //Denna heter inte _SV efter som att
    float3 tangentWS : TANGENTWS;
    float3 biTangentWS : BITANGENTWS;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    float speed = 0.0005;
    
    float4 tangentWS = normalize(mul(input.tangent, matrixes.World));
    float4 normalWS = mul(input.normal, matrixes.World);
    
    output.position = mul(float4(input.position, 1.0f), matrixes.WorldViewProjection); //converts position to projection space
    
    if (matrixes.animated)
    {
        output.textureCoordinates = float2(input.textureCoordinates.x, input.textureCoordinates.y +(matrixes.time * speed)); //Moving stuff. 

    }
    else
    {
        output.textureCoordinates = float2(input.textureCoordinates.x, input.textureCoordinates.y); //Moving stuff. 
    }

    output.color = input.color;
    output.normal = mul(float4(input.normal, 1), matrixes.World); //Converts normal to world space
    
    //New output
    output.worldPosition = mul(input.position, matrixes.World); //Konverts worldposition to world space
    output.tangentWS = normalize(tangentWS - dot(tangentWS, normalWS) * normalWS);
    output.biTangentWS = float4(normalize(cross(normalWS.xyz, tangentWS.xyz)), 0.0f);
    
    return output;
}