struct GeoInputType
{
    float4 position : SV_POSITION;
};

RWStructuredBuffer<GeoInputType> particles : register(u0);

#define particle particles[particleID.x]

[numthreads(768, 1, 1)]
void main(uint3 particleID : SV_DispatchThreadID)
{
    
    particle.position.y -= 0.5;
    
    if (particle.position.y < -5)
    {
        particle.position.y = 50;
    }
    
}