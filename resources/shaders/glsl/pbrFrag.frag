#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 worldPos;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec4 outColor;

struct PointLight
{
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform UniformBufferGlobal
{
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} global;

layout(set = 1, binding = 0) uniform sampler2D albedoTexture;
layout(set = 1, binding = 1) uniform sampler2D normalTexture;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessTexture;
layout(set = 1, binding = 3) uniform sampler2D occlusionTexture;
layout(set = 1, binding = 4) uniform sampler2D emissiveTexture;

layout(push_constant) uniform Push
{
    mat4 modelMatrix;
    mat4 normalMatrix;
    vec4 albedoFactor;
    float normalFactor;
    float metallicFactor;
    float roughnessFactor;
    float occlusionFactor;
    vec3 emissiveFactor;
} push;

const float PI = 3.14159265359;

float distributionGGX(vec3 N, vec3 H, float roughness)
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

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float nDotV = max(dot(N, V), 0.0);
    float nDotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(nDotV, roughness);
    float ggx1 = geometrySchlickGGX(nDotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // NOTE: sample textures
    vec3 albedo = texture(albedoTexture, uv).rgb * push.albedoFactor.rgb;
    vec2 metallicRoughness = texture(metallicRoughnessTexture, uv).bg; // NOTE: b = metallic, g = roughness
    float metallic = metallicRoughness.x * push.metallicFactor;
    float roughness = metallicRoughness.y * push.roughnessFactor;
    float ao = texture(occlusionTexture, uv).r * push.occlusionFactor;
    vec3 emissive = texture(emissiveTexture, uv).rgb * push.emissiveFactor;

    vec3 N = normalize(normal);
    vec3 V = normalize(global.inverseView[3].xyz - worldPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < global.numLights; ++i)
    {
        vec3 lightPos = global.pointLights[i].position.xyz;
        vec3 L = normalize(lightPos - worldPos);
        vec3 H = normalize(V + L);

        float distance = length(lightPos - worldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = global.pointLights[i].color.rgb * global.pointLights[i].color.w * attenuation;

        float NDF = distributionGGX(N, H, roughness);
        float G = geometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float nDotL = max(dot(N, L), 0.0);

        Lo += (kD * albedo / PI + specular) * radiance * nDotL;
    }

    vec3 ambient = global.ambientLightColor.rgb * global.ambientLightColor.w * albedo * ao;
    vec3 color = ambient + Lo;

    // NOTE: HDR tonemapping
    color = color / (color + vec3(1.0));
    // NOTE: gamma correction
    color = pow(color, vec3(1.0/2.2));

    outColor = vec4(color, 1.0);
}
