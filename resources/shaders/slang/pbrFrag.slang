struct FSInput
{
    [[vk::location(0)]] float2 uv;
    [[vk::location(1)]] float3 worldPos;
    [[vk::location(2)]] float3 normal;
};

struct FSOutput
{
    float4 color : SV_Target0;
};

static const float PI = 3.14159265359;
static const int MAX_POINT_LIGHTS = 10;

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

// NOTE: vk::binding(binding, set)
[[vk::binding(0, 1)]] Sampler2D albedoTexture;
[[vk::binding(1, 1)]] Sampler2D normalTexture;
[[vk::binding(2, 1)]] Sampler2D metallicRoughnessTexture;
[[vk::binding(3, 1)]] Sampler2D occlusionTexture;
[[vk::binding(4, 1)]] Sampler2D emissiveTexture;

float distributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float nDotH = max(dot(N, H), 0.0);
    float nDotH2 = nDotH * nDotH;

    float denom = (nDotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / denom;
}

float geometrySchlickGGX(float nDotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float denom = nDotV * (1.0 - k) + k;

    return nDotV / denom;
}

float geometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float nDotV = max(dot(N, V), 0.0);
    float nDotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(nDotV, roughness);
    float ggx1 = geometrySchlickGGX(nDotL, roughness);

    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

[shader("fragment")]
FSOutput main(FSInput in)
{
    float3 albedo = albedoTexture.Sample(in.uv).rgb * push.albedoFactor.rgb;
    float2 metallicRoughness = metallicRoughnessTexture.Sample(in.uv).bg;
    float metallic = metallicRoughness.x * push.metallicFactor;
    float roughness = metallicRoughness.y * push.roughnessFactor;
    float ao = occlusionTexture.Sample(in.uv).r * push.occlusionFactor;
    float3 emission = emissiveTexture.Sample(in.uv).rgb * push.emissiveFactor;

    float3 N = normalize(in.normal);
    float3 V = normalize(global.inverseMatrix[3].xyz - in.worldPos);

    float3 F0 = float3(0.04);
    F0 = lerp(F0, albedo, metallic);

    float3 Lo = float3(0.0);
    for(int i = 0; i < global.numLights; ++i)
    {
        float3 lightPos = global.pointLights[i].position.xyz;
        float3 L = normalize(lightPos);
        float3 H = normalize(V + L);

        float dist = length(lightPos - in.worldPos);
        float attenuation = 1.0 / (dist * dist);
        float3 radiance = global.pointLights[i].color.xyz * attenuation;

        float NDF = distributionGGX(N, H, roughness);
        float G = geometrySmith(N, V, L, roughness);
        float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        float3 numerator = NDF * G * F;
        float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        float3 specular = numerator / denom;

        float3 kS = F;
        float3 kD = float3(1.0) - kS;
        kD *= 1.0 - metallic;

        float nDotL = max(dot(N, L), 0.0);

        Lo += (kD * albedo / PI + specular) * radiance * nDotL;
    }

    float3 ambient = float3(0.03) * albedo * ao;
    float3 color = ambient + Lo;

    // NOTE: HDR tonemapping
    color = color / (color + float3(1.0));

    // NOTE: gamma correction
    color = pow(color, float3(1.0 / 2.2));

    return { float4(color, 1.0) };
}
