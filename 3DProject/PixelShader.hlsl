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
    float time;
    int hasTexture;
    int hasNormal;
};

cbuffer constantBuffer
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
    
    return normalize(float4(normalWS, 0.0f)); //normalWS is - here cuz it's mirror otherwise? (but this fucks upp the square)
}

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;

    // sample the diffus map
    //float3 diffuseMap = diffuseTexture.Sample(testSampler, input.textureCoordinates); //Har ersatt denna med normal mappen just nu.
    
    input.normal = normalize(input.normal);
    
    output.DiffuseAlbedo = float4(1.0, 0.0, 1.0, 1.0); //Saker utan textur blir lila
    
    //output.DiffuseAlbedo = diffuseTexture.Sample(testSampler, input.textureCoordinates);
    
    if (matrixes.hasTexture) //Dessa verkar inte funka just nu, ta dem sen.
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
