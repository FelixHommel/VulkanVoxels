struct VSInput
{
    [[vk::location(0)]] float3 position;
    [[vk::location(1)]] float3 color;
    [[vk::location(2)]] float3 normal;
    [[vk::location(3)]] float3 uv;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 color;
};

struct PushData
{
    float4x4 transform;
    float4x4 normalMatrix;
};

[[push_constant]]
PushData push;

static const float3 DIRECTION_TO_LIGHT = normalize(float3(1.0, -3.0, -1.0));
static const float AMBIENT_LIGHT_STRENGTH = 0.02;

[shader("vertex")]
VSOutput main(VSInput input)
{
    float3 normalWorldSpace = normalize(mul(float3x3(push.normalMatrix), input.normal));
    float lightIntensity = AMBIENT_LIGHT_STRENGTH + max(dot(normalWorldSpace, DIRECTION_TO_LIGHT), 0);

    VSOutput output;
    output.position = mul(push.transform, float4(input.position, 1.0));
    output.color = lightIntensity * input.color;

    return output;
}
