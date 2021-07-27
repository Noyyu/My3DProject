cbuffer DepthMatrixBuffer : register(b0)
{
    float4x4 LightViewProjectionMatrix;
    
}

cbuffer matrixes : register(b1)
{
    float4x4 WorldViewProjection;
    float4x4 World;
}

float4 main(float4 position : POSITION) : SV_POSITION
{
    position.w = 1.0f;
    position = mul(position, World);
    return mul(position, LightViewProjectionMatrix);
}