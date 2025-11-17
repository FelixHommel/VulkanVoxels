struct VSInput
{
    [[vk::location(0)]] float3 position;
    [[vk::location(1)]] float3 color;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 color;
};

struct PushData
{
    float4x4 transform;
    float3 color;
};

[[push_constant]]
PushData push;

[shader("vertex")]
VSOutput main(VSInput input)
{
    VSOutput output;
    output.position = mul(push.transform, float4(input.position, 1.0));
    output.color = input.color;

    return output;
}
