// Textures
Texture2D normalTexture : register(t0);
Texture2D positionTexture : register(t1);
Texture2D diffuseTexture : register(t2);
Texture2D depthTexture : register(t3);
//Both are point samplers
SamplerState samplerThing : register(s0);
SamplerState shadowSampler : register(s1);

struct PSInput
{
    float4 position : SV_Position;
    float3 color : COLOR;
    float2 textureCoordinates : TEXCOORD;
    float3 normal : NORMAL; 
    float4 worldPosition : POSITION;
};

struct Light
{
    float4 position;
    float4 attenuation;
    float4 ambient;
    float4 diffuse;
    float4 cameraPosition;
    float4 direction;
    float range;
};

cbuffer pixelConstantBuffer :register(b0)
{
    Light light;
};

cbuffer DepthMatrixBuffer : register(b1) //Shadow constant buffer
{
    row_major matrix lightViewProjectionMatrix;
}

struct myMatrixes
{
    row_major matrix ProjectionMatrix;
    row_major matrix ViewMatrix;
    float4 CameraPosition;

};
cbuffer matrixes : register(b2) //PerframeMatrixes
{
    myMatrixes matrixes;
};

//Pixel shaderfor final pass of light prepass deferredrendering
float4 main(in PSInput input) : sv_Target ////Skriver SV_OutputControlPointID till backbuffern
{
    const float lightIntensity = 0.5;
    const float lightShininess = 128;
    const float lightSpecularPower = 10;
    
    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 lightColor = float4(1.0f, 1.0f, 0.90f, 1.0f);
    
    float4 albedo = diffuseTexture.Sample(samplerThing, input.textureCoordinates);
    float3 normals = normalTexture.Sample(samplerThing, input.textureCoordinates);
    float3 position = positionTexture.Sample(samplerThing, input.textureCoordinates);
    float depthMap = depthTexture.Sample(samplerThing, input.textureCoordinates);
    
    normals = normalize(normals);
    
    //Ambient factor
    const float4 ambient = float4(light.ambient.xyz * light.ambient.w, 1.0f);
    finalColor = ambient * albedo;

    //Vectors
    const float3 pointToLight = -normalize(light.position.xyz - position);
    const float3 pointToCamera = -normalize(light.cameraPosition.xyz - position.xyz);
    
    
    //POINT LIGHT THINGS
    float4 diffuse =  float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 specular = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float intensity = 1;
    
    //Distance to light source
    const float distance = length(pointToLight);
    float3 normPointToLight = pointToLight / distance;
    
    //Get attenuation
    const float attenuation = 1.0f - smoothstep(light.range * 0.75f, light.range, distance); // Smoothstep() returns 0 when the distance to the light is less than 3/4.
    
    //Compute diffuse
    const float diffuseFactor = saturate(dot(normals, normPointToLight));
    diffuse = (lightColor * diffuseFactor) * attenuation * intensity;
    
    //Compute specular
    [flatten]
    if (length(diffuse) > 0.0f)
    {
        const float3 reflection = reflect(-normPointToLight, normals);
        float specularFactor = saturate(dot(pointToCamera, reflection));
        
        if (lightShininess > 1.0f)
        {
            specularFactor = pow(specularFactor, lightShininess);
        }
        
        specular = (lightColor * specularFactor * lightSpecularPower) * attenuation * lightIntensity;
    }
    
    // ---------------------
    // Shadow calculations
    // ---------------------
    
    float4 lightViewPosition = mul(float4(position, 1.0f), lightViewProjectionMatrix);
    //lightViewPosition.xy /= lightViewPosition.w; //ndc (normalized device coordinate)(This isn't needed for ortographic projection cuz the coordinates are already in NDC space) (leaving it it means deviding by 1)
    float2 shadowMapTexel = float2(0.5f * lightViewPosition.x + 0.5f, -0.5f * lightViewPosition.y + 0.5f);  //(Transform from NDC space to texture space by doing: u = 0.5x + 0.5  v = -0.5y + 0.5)
    float depth = lightViewPosition.z;; // / lightViewPosition.w; (This isn't needed for ortographic projection)
    float bias = 0.03f; //Simple bias for simple scene without slopes. 
    
    float dx = 1.0f / 640; // size of shadow map
    float dy = 1.0f / 640; 
    
    //Sample shadow map to get nearest depth to light. 
    //generates either 0 or 1. 
    float s0 = (depthTexture.Sample(shadowSampler, shadowMapTexel + float2(0.0f, 0.0f)).r + bias < depth) ? 0.0f : 1.0f;
    float s1 = (depthTexture.Sample(shadowSampler, shadowMapTexel + float2(dx, 0.0f)).r + bias < depth) ? 0.0f : 1.0f;
    float s2 = (depthTexture.Sample(shadowSampler, shadowMapTexel + float2(0.0f, dy)).r + bias < depth) ? 0.0f : 1.0f;
    float s3 = (depthTexture.Sample(shadowSampler, shadowMapTexel + float2(dx, dy)).r + bias < depth) ? 0.0f : 1.0f;
    
    //Transform to texel space. 
    float2 texelPos = float2(shadowMapTexel.x * dx, shadowMapTexel.y * dy);
    
    //Determine the interpolation amounts. 
    float2 lerps = frac(texelPos);
    //The HLSL frac function returns the fractional part of a floating-point number (i.e., the mantissa). For example, if
    //SMAP_SIZE = 1024 and projTex.xy = (0.23, 0.68), then texelPos = (235.52, 696.32) and frac(texelPos) = (0.52, 0.32).
    
    //intepolate shadow map valuses
    // https://en.wikipedia.org/wiki/Bilinear_interpolation
    float shadowCoeff = lerp(lerp(s0, s1, lerps.x), lerp(s2, s3, lerps.x), lerps.y);
    
    finalColor += ((diffuse + specular) * albedo) * (shadowCoeff);

    //--------------------
    
    if (light.range < distance)
    {
        return (albedo * ambient);
    }
    
    return float4(finalColor);
}
