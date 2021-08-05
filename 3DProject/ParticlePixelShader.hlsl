struct GeoOut
{
    float4 position : SV_POSITION;
};
struct PixelShaderOutput
{
    float4 position : SV_Target0;
};

float4 main(GeoOut input) : SV_TARGET
{
    float4 color = (0, 0, 0, 0.5);
    return color;
}