
struct VertexInputType
{
    float4 position : POSITION;
};
struct GeoInputType
{
    float4 position : SV_POSITION;
};

StructuredBuffer<VertexInputType> input : register(t0);

GeoInputType main(uint id : SV_InstanceID)
{
    GeoInputType output;
    
    output.position = float4(input[id].position.xyz, 1.0f);
    
    return output;
}