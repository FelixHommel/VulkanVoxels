static const float2 OFFSETS[6] = {
    float2(-1.0, -1.0),
    float2(-1.0, 1.0),
    float2(1.0, -1.0),
    float2(1.0, -1.0),
    float2(-1.0, 1.0),
    float2(1.0, 1.0)
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 fragOffset;
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

[shader("vertex")]
VSOutput main(uint vertexID : SV_VertexID)
{
    VSOutput output;
    output.fragOffset = OFFSETS[vertexID];

    const float3 cameraRightWorld = float3(ubo.view[0][0], ubo.view[0][1], ubo.view[0][2]);
    const float3 cameraUpWorld = float3(ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]);

    const float3 positionWorld = push.position.xyz +
        (push.radius * output.fragOffset.x * cameraRightWorld) +
        (push.radius * output.fragOffset.y * cameraUpWorld);

    output.position = mul(mul(ubo.projection, ubo.view), float4(positionWorld, 1.0));

    return output;
}
