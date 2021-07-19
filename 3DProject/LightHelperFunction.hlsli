//Light structure

struct Light
{
    float4 position;
    float4 color;
    float specularPower;
    float shininess;
    float intensity;
    float range;
    
};

float CalculateAttenuation(float distance, float Range)
{
    return 1.0f - smoothstep(Range * 0.75f, Range, distance); // Smoothstep() returns 0 when the distance to the light is less than 3/4.
}
