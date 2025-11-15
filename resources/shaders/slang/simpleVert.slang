struct VSInput
{
    [[vk::location(0)]] float2 position;
    [[vk::location(1)]] float3 color;
};

struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float3 fragColor;
};

[shader("vertex")]
VSOutput main(VSInput input)
{
    VSOutput output;
    output.position = float4(input.position, 0.0, 1.0);
    output.fragColor = input.color;

    return output;
}
