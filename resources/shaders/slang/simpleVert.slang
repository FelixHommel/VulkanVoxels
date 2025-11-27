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
    float3 fragPosWorld;
    float3 fragNormalWorld;
};

struct PushData
{
    float4x4 modelMatrix;
    float4x4 normalMatrix;
};

[[push_constant]]
PushData push;

struct GlobalUniformBuffer
{
    float4x4 projection;
    float4x4 view;
    float4 ambientLightColor;
    float3 lightPosition;
    float4 lightColor;
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
