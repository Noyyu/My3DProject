// The output UAV used by the CS, in this case the backbuffer. 
// Float4 is specified in swap chain description.
RWTexture2D<unorm float4> backBuffer : register(u0);
Texture2D frameBufferObject : register(t0);

// Group size may not exceed 1024.
#define SIZE_X 8
#define SIZE_Y 8
#define KERNAL_SIZE 7 // The kernel size = blur strenght. 


//--------------------------------------------------------------------------------------
// The weights are calculated by numerical integration of the continuous 
// gaussian distribution over each discrete kernel tap, and will be used directly
// in a single pass blur algorithm: n^2 samples per pixel.
// Sigma: 2.4   Kernal Size: 7x7
// Source: http://dev.theomader.com/gaussian-kernel-calculator/
//--------------------------------------------------------------------------------------
static const float kernal_coefficients[KERNAL_SIZE][KERNAL_SIZE] =
{
    0.007982, 0.012243, 0.015826, 0.017239, 0.015826, 0.012243, 0.007982,
    0.012243, 0.01878, 0.024275, 0.026443, 0.024275, 0.01878, 0.012243,
    0.015826, 0.024275, 0.031379, 0.034181, 0.031379, 0.024275, 0.015826,
    0.017239, 0.026443, 0.034181, 0.037234, 0.034181, 0.026443, 0.017239,
    0.015826, 0.024275, 0.031379, 0.034181, 0.031379, 0.024275, 0.015826,
    0.012243, 0.01878, 0.024275, 0.026443, 0.024275, 0.01878, 0.012243,
    0.007982, 0.012243, 0.015826, 0.017239, 0.015826, 0.012243, 0.007982
};

//Aggressive way of implementing gaussian filter.
[numthreads(SIZE_X, SIZE_Y, 1)]
void main(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    // Offset the texture location to the first sample location.
    uint3 textureCoords = DispatchThreadID - int3(3, 3, 0);

    float4 outputColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    for (int x = 0; x < KERNAL_SIZE; x++)
        for (int y = 0; y < KERNAL_SIZE; y++)
            outputColor += frameBufferObject.Load(textureCoords + int3(x, y, 0)) * kernal_coefficients[x][y];
      
    // Output to chosen UAV.
    backBuffer[DispatchThreadID.xy] = outputColor;
}