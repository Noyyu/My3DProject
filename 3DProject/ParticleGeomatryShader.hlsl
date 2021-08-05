cbuffer particlecbPerObj : register(b0)
{
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
    float4 cameraPosition;
};

struct GeoInputType
{
    float4 position : SV_POSITION;
};

struct GeoOut
{
    float4 position : SV_POSITION;
};

[maxvertexcount(6)]
void main(point GeoInputType input[1], inout TriangleStream<GeoOut> triStream)
{
    
    int size = 1;
    
    //Billboarding
    float3 upVector = float3(0.0f, 1.0f, 0.0f);
    float3 camToVert = input[0].position.xyz - cameraPosition.xyz;
    float3 rightVector = -normalize(cross(camToVert, upVector));
    
    if (length(camToVert) > 30)
    {
        return;
    }
    
    //Compute trianglestrip vertices into worldspace
    float halfWidth = 0.01f * size;
    float halfHeight = 0.1f * size;
    
    float4 right = float4(halfWidth * rightVector, 0.0f);
    float4 up = float4(halfHeight * upVector, 0.0f);
    
    float4 v[6];
    v[0] = input[0].position + right - up;
    v[1] = input[0].position - right - up;
    v[2] = input[0].position - right + up;
    v[3] = input[0].position + right + up;
    v[4] = v[0];
    v[5] = v[2];
    
    GeoOut gOutput;
	[unroll]
    for (int i = 0; i < 6; i++)
    {
        gOutput.position = mul(v[i], viewMatrix);
        gOutput.position = mul(gOutput.position, projectionMatrix);
        triStream.Append(gOutput);
    }
}