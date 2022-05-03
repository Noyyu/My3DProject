Texture2D diffuseTexture : register(t0);
Texture2D normalTexture  : register(t1);
SamplerState testSampler : register(s0);

//Infomration about pixelShader https://www.3dgep.com/texturing-lighting-directx-11/#Pixel_Shader
//detta är nog nu min g-buffer
struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float2 textureCoordinates : TEXCOORD;
    float4 normal : NORMAL;
    float4 worldPosition : POSITION;
    float3 tangentWS : TANGENTWS;
    float3 biTangentWS : BITANGENTWS;
};

struct PixelShaderOutput
{
    float4 Normal : SV_Target0;
    float4 Position : SV_Target1;
    float4 DiffuseAlbedo : SV_Target2;
};

struct Matrixes
{
    float4x4 WorldViewProjection;
    float4x4 World;
    float4x4 WorldInverseTransposeMatrix; //This might have to be a row_major matrix;
    float time;
    int hasTexture;
    int hasNormal;
    int animated;
};

cbuffer constantBuffer : register(b0)
{
    Matrixes matrixes;
};


//Function for normalmaping
float4 DoNormalMapping(PixelShaderInput input)
{
    // Normalize the TBN after interpolation.
    const float3x3 TBN = float3x3
    (normalize(input.tangentWS.xyz),
     normalize(input.biTangentWS.xyz),
     normalize(input.normal.xyz));
    
    // Sample the tangent-space normal map in range [0,1] and decompress. 
    // Tangent and binormal (UV) are used as direction vectors.
    float3 normalTS = normalTexture.Sample(testSampler, input.textureCoordinates.xy).rgb;
    
    // Change normal map range from [0, 1] to [-1, 1].
    normalTS = normalize(normalTS * 2.0f - 1.0f);
    
    // Transform normal from local tangent space to world space.
    float3 normalWS = mul(normalTS, TBN);
    
    return normalize(float4(normalWS, 0.0f));
}

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;

    // sample the diffus map
    
    input.normal = normalize(input.normal);
    
    output.DiffuseAlbedo = float4(1.0, 0.0, 1.0, 1.0); //Saker utan textur blir lila
    
    if (matrixes.hasTexture)
    {
        output.DiffuseAlbedo = diffuseTexture.Sample(testSampler, input.textureCoordinates);
    }
    
    float4 normalWorldSpace = normalize(input.normal);
    output.Normal = normalWorldSpace;
    
    if (matrixes.hasNormal == 1)
    {
        output.Normal = DoNormalMapping(input);
    }
    
    output.Position = input.worldPosition;
        return output;
}
