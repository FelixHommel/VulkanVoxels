struct VSInput
{
    [[vk::location(0)]] float3 position;
    [[vk::location(1)]] float3 color;
    [[vk::location(2)]] float3 normal;
    [[vk::location(3)]] float2 uv;
};

struct VSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float3 color;
    [[vk::location(1)]] float3 fragPosWorld;
    [[vk::location(2)]] float3 fragNormalWorld;
};

struct PushData
{
    float4x4 modelMatrix;
    float4x4 normalMatrix;
};

[[push_constant]]
PushData push;

struct PointLight
{
    float4 position;
    float4 color;
};

struct GlobalUniformBuffer
{
    float4x4 projection;
    float4x4 view;
    float4x4 inverseView;
    float4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
};

// NOTE: vk::binding(binding, set): https://docs.shader-slang.org/en/latest/coming-from-glsl.html#option-2-glsl-style-layout-syntax
[[vk::binding(0, 0)]]
ConstantBuffer<GlobalUniformBuffer> ubo;

[shader("vertex")]
VSOutput main(VSInput input)
{
    const float4 worldPos = mul(push.modelMatrix, float4(input.position, 1.0));

    VSOutput output;
    output.position = mul(mul(ubo.projection, ubo.view), worldPos);
    output.color = input.color;
    output.fragPosWorld = worldPos.xyz;
    output.fragNormalWorld = normalize(mul(float3x3(push.normalMatrix), input.normal));

    return output;
}
