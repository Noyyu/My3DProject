// Textures
Texture2D normalTexture : register(t0);
Texture2D positionTexture : register(t1);
Texture2D diffuseTexture : register(t2);
Texture2D depthTexture : register(t3); //For shadow mapping //Denna funkar inte just nu. 
SamplerState samplerThing : register(s0);

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
    float4x4 lightViewProjectionMatrix;
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
    
    
    
    //// Shadow calculations
    
    float4 positionL = mul(float4(position, 1.0f), lightViewProjectionMatrix);
    positionL.xy /= positionL.w; //ndc?
    float2 smTex = float2(0.5f * positionL.x + 0.5f, -0.5f * positionL.y + 0.5f);
    float depth = positionL.z / positionL.w;
    float bias = 0.001f;
    
    float dx = 1.0f / 640; // this must be the same as the texture size!!
    float dy = 1.0f / 640; //ditto
    
    // To prevent pixelation
    float s0 = (depthTexture.Sample(samplerThing, smTex + float2(0.0f, 0.0f)).r + bias < depth) ? 0.0f : 1.0f;
    float s1 = (depthTexture.Sample(samplerThing, smTex + float2(dx, 0.0f)).r + bias < depth) ? 0.0f : 1.0f;
    float s2 = (depthTexture.Sample(samplerThing, smTex + float2(0.0f, dy)).r + bias < depth) ? 0.0f : 1.0f;
    float s3 = (depthTexture.Sample(samplerThing, smTex + float2(dx, dy)).r + bias < depth) ? 0.0f : 1.0f;
    
    float2 texelPos = float2(smTex.x * dx, smTex.y * dy);
    
    float2 lerps = frac(texelPos);
    
    float shadowCoeff = lerp(lerp(s0, s1, lerps.x), lerp(s2, s3, lerps.x), lerps.y);
    
    //finalColor = ((diffuse + specular) * albedo) * (shadowCoeff + 0.3);
    
    //--------------------
    
    finalColor = (diffuse + specular) * albedo;
    if (light.range < distance) //Denna funkar inte just nu av någon anledning. 
    {
        return (albedo * ambient);
    }
    
    return float4(finalColor);

}
