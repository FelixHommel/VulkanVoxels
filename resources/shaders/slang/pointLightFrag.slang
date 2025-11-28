struct FSInput {
    float2 fragOffset;
};

struct FSOutput
{
    float4 color : SV_Target0;
};

struct PushData
{
    float4 position;
    float4 color;
    float radius;
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
    const float distance = sqrt(dot(input.fragOffset, input.fragOffset));

    if(distance >= 1.f)
        discard;

    FSOutput output;
    output.color = float4(push.color.xyz, 1.f);

    return output;
}
