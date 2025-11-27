struct FSInput {
    float2 fragOffset;
};

struct FSOutput
{
    float4 color : SV_Target0;
};

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
    const float distance = sqrt(dot(input.fragOffset, input.fragOffset));

    if(distance >= 1.f)
        discard;

    FSOutput output;
    output.color = float4(ubo.lightColor.xyz, 1.f);

    return output;
}
