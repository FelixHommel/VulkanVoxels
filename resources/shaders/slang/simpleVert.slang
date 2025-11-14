struct VSOutput
{
    float4 position : SV_Position;
};

struct VSinput
{
    [[vk::location(0)]] float2 position;
};

[shader("vertex")]
VSOutput main(VSinput input)
{
    VSOutput output;
    output.position = float4(input.position, 0.0, 1.0);

    return output;
}
