struct FSInput {
    float3 color;
    float3 fragPosWorld;
    float3 fragNormalWorld;
};

struct FSOutput
{
    float4 color : SV_Target0;
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

[shader("pixel")]
FSOutput main(FSInput input)
{
    const float3 directionToLight = ubo.lightPosition - input.fragPosWorld;
    const float attenuation = 1.0 / dot(directionToLight, directionToLight); // dot(x, x) == distance^2
    
    const float3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
    const float3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    const float3 diffuseLight = lightColor * max(dot(normalize(input.fragNormalWorld), normalize(directionToLight)), 0);

    FSOutput output;
    output.color = float4((diffuseLight + ambientLight) * input.color, 1.0);

    return output;
}
