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
    float4x4 modelMatrix;
    float4x4 normalMatrix;
};

[[push_constant]]
PushData push;

struct GlobalUniformBuffer
{
    float4x4 projectionViewMatrix;
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
    float4 worldPos = mul(push.modelMatrix, float4(input.position, 1.0));

    float3 normalWorldSpace = normalize(mul(float3x3(push.normalMatrix), input.normal));

    float3 directionToLight = ubo.lightPosition - worldPos.xyz;
    float attenuation = 1.0 / dot(directionToLight, directionToLight); // dot(x, x) == distance^2
    
    float3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
    float3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    float3 diffuseLight = lightColor * max(dot(normalWorldSpace, normalize(directionToLight)), 0);

    VSOutput output;
    output.position = mul(ubo.projectionViewMatrix, worldPos);
    output.color = (diffuseLight + ambientLight) * input.color;

    return output;
}
