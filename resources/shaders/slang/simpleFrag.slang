struct FSInput {
    [[vk::location(0)]] float3 color;
    [[vk::location(1)]] float3 fragPosWorld;
    [[vk::location(2)]] float3 fragNormalWorld;
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
    float4x4 inverseView;
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
    float3 specularLight = float3(0.f);

    const float3 surfaceNormal = normalize(input.fragNormalWorld);
    const float3 cameraPosWorld = ubo.inverseView[3].xyz;
    const float3 viewDirection = normalize(cameraPosWorld - input.fragPosWorld);

    for(int i = 0; i < ubo.numLights; ++i)
    {
        PointLight light = ubo.pointLights[i];
        float3 directionToLight = light.position.xyz - input.fragPosWorld;
        const float attenuation = 1.0 / dot(directionToLight, directionToLight); // dot(x, x) == distance^2

        directionToLight = normalize(directionToLight);
        const float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
        const float3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosAngIncidence;

        const float3 halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0.f, 1.f);
        blinnTerm = pow(blinnTerm, 512.f); // higher power = sharper highlight

        specularLight += intensity * blinnTerm;
    }

    FSOutput output;
    output.color = float4(diffuseLight * input.color + specularLight * input.color, 1.f);

    return output;
}
