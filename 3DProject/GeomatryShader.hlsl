struct GSinput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float2 textureCoordinates : TEXCOORD;
    float4 normal : NORMAL;
    float4 worldPosition : POSITION; //Denna heter inte _SV efter som att
    float3 tangentWS : TANGENTWS;
    float3 biTangentWS : BITANGENTWS;
};

struct GSOutput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float2 textureCoordinates : TEXCOORD;
    float4 normal : NORMAL;
    float4 worldPosition : POSITION;
    float3 tangentWS : TANGENTWS;
    float3 biTangentWS : BITANGENTWS;
};

[maxvertexcount(3)]
void main(
	triangle float4 position[3]           : SV_POSITION,
	triangle float3 color[3]              : COLOR,
	triangle float2 textureCoordinates[3] : TEXCOORD,
	triangle float4 normal[3]             : NORMAL,
    triangle float4 worldPosition[3]      : POSITION,
	triangle float3 tangentWS[3]          : TANGENTWS,
    triangle float3 biTangentWS[3]        : BITANGENTWS,
	inout TriangleStream< GSOutput > output
)
{
    float3x3 vertices =
    {
        float3(position[0].xyz),
		float3(position[1].xyz), // Replace with 2 for counter-clockwise
		float3(position[2].xyz)  // Replace with 1 for counter-clockwise
    };
	
    float d = determinant(vertices);
	
	if (d <= 0.001f)
    {
        for (uint i = 0; i < 3; i++)
        {
            GSOutput element;
            element.position            = position[i];
            element.color               = color[i];
            element.textureCoordinates  = textureCoordinates[i];
            element.normal            = normal[i];
            element.worldPosition       = worldPosition[i];
            element.tangentWS           = tangentWS[i];
            element.biTangentWS         = biTangentWS[i];
            output.Append(element);
        }
    }
}