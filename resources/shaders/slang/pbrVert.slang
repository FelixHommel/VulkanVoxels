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
    [[vk::location(0)]] float2 uv;
    [[vk::location(1)]] float3 worldPos;
    [[vk::location(2)]] float3 normal;
};

struct PushData
{
    float4x4 modelMatrix;
    float4x4 normalMatrix;
    float4 albedoFactor;
    float normalFactor;
    float metallicFactor;
    float roughnessFactor;
    float occlusionFactor;
    float3 emissiveFactor;
};

[[push_constant]]
PushData push;

struct PointLight
{
    float4 position;
    float4 color;
};

static const int MAX_POINT_LIGHTS = 10;

struct GlobalUniformBuffer
{
    float4x4 projection;
    float4x4 view;
    float4x4 inverseMatrix;
    float4 ambientLightColor;
    PointLight pointLights[MAX_POINT_LIGHTS];
    int numLights;
};

[[vk::binding(0, 0)]]
ConstantBuffer<GlobalUniformBuffer> global;

[shader("vertex")]
VSOutput main(VSInput in)
{
    VSOutput out;
    out.uv = in.uv;
    out.worldPos = mul(push.modelMatrix, float4(in.position, 1.0)).xyz;
    out.normal = mul(float3x3(push.normalMatrix), in.normal);
    out.position = mul(mul(global.projection, global.view), float4(out.worldPos, 1.0));

    return out;
}
