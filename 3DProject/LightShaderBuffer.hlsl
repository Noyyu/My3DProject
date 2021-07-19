//// Textures
//Texture2D normalTexture : register(t0);
//Texture2D positionTexture : register(t1);
////Texture2D diffuseTexture : register(t2);

//// Constants
//cbuffer LightParams
//{
//    float3 LightPos;
//    float3 LightColor;
//    float4 LightRange;
//}; 

//cbuffer CameraParams
//{
//    float3 CameraPos;
//}

//// Vertex shader for lighting pass of light prepass deferred rendering
//float4 main(in float4 Position : POSITION) : SV_Position
//{
//// Just pass along the position of the full-screen quad
//    return Position;
//}

//// Helper function for extracting G-Buffer attributes
//void GetGBufferAttributes(in float2 screenPos, out float3 normal, out float3 position, out float3 diffuse)  //THIS ONE NEEDS THE TEXTURES
//{
//// Determine our indices for sampling the texture based on the current
//// screen position
//    int3 samplelndices = int3(screenPos.xy, 0);
//    float4 normalTex = normalTexture.Load(samplelndices);
//    normal = normalTex.xyz;
//    position = positionTexture.Load(samplelndices).xyz;
//    diffuse = diffuseTexture.Load(samplelndices).xyz;
//}

//// Calculates the lighting term for a single G-Buffer texel
//float4 CalcLighting(in float3 normal, in float3 position, in float3 diffuse)
//{

//}

//// Pixel shader for lighting pass of light prepass deferred rendering
//float4 PSMain(in float4 screenPos : SV_Position) : SV_Target0
//{
//    float3 normal;
//    float3 position;
//    float3 diffuse;
//// Get the G-Buffer values
//    GetGBufferAttributes(screenPos.xy, normal, position, diffuse);
//    return CalcLighting(normal, position, diffuse);
//}

