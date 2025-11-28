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

struct PointLight
{
    float4 position;
    float4 color;
};

struct GlobalUniformBuffer
{
    float4x4 projection;
    float4x4 view;
    float4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
};

// NOTE: vk::binding(binding, set): https://docs.shader-slang.org/en/latest/coming-from-glsl.html#option-2-glsl-style-layout-syntax
[[vk::binding(0, 0)]]
ConstantBuffer<GlobalUniformBuffer> ubo;

[shader("pixel")]
FSOutput main(FSInput input)
{
    float3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    float3 surfaceNormal = normalize(input.fragNormalWorld);

    for(int i = 0; i < ubo.numLights; ++i)
    {
        PointLight light = ubo.pointLights[i];
        const float3 directionToLight = light.position.xyz - input.fragPosWorld;
        const float attenuation = 1.0 / dot(directionToLight, directionToLight); // dot(x, x) == distance^2
        const float cosAngIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0);
        const float3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosAngIncidence;
    }

    FSOutput output;
    output.color = float4(diffuseLight * input.color, 1.0);

    return output;
}
