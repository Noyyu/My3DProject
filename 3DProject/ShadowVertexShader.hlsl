cbuffer DepthMatrixBuffer : register(b0)
{
    row_major matrix LightViewProjectionMatrix;
    
}

cbuffer matrixes : register(b1)
{
    row_major matrix WorldViewProjection;
    row_major matrix World;
}

float4 main(float4 position : POSITION) : SV_POSITION
{
    position.w = 1.0f;
    position = mul(position, World);
    return mul(position, LightViewProjectionMatrix);
}