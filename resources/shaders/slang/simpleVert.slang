static const float2 positions[3] = {
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
};

struct VSOutput
{
    float4 position : SV_Position;
};

VSOutput main(uint vertexID : SV_VertexID)
{
    VSOutput output;
    output.position = float4(positions[vertexID], 0.0, 1.0);

    return output;
}
