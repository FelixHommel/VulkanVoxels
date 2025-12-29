#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUv;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 worldPos;
layout(location = 2) out vec3 normal;

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
layout(set = 1, binding = 1) uniform sampler2D normaTexturel;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessTexture;
layout(set = 1, binding = 3) uniform sampler2D occlusionTexture;
layout(set = 1, binding = 4) uniform sampler2D emissiveTexture;

layout(push_constant) uniform Push
{
    // offset(0)
    mat4 modelMatrix;
    mat4 normalMatrix;

    // offset(SimplePushData)
    vec4 albedo;
    float normal;
    float metallic;
    float roughness;
    float occlusion;
    vec3 emissive;
} push;

void main()
{
    uv = inUv;
    worldPos = vec3(push.modelMatrix * vec4(inPosition, 1.0));
    normal = mat3(push.normalMatrix) * inNormal;

    gl_Position = global.projection * global.view * vec4(worldPos, 1.0);
}
