struct FSInput {};

struct FSOutput
{
    float4 color : SV_Target0;
};

struct PushData
{
    float2x2 transform;
    float2 offset;
    float3 color;
};

[[push_constant]]
PushData push;

[shader("pixel")]
FSOutput main(FSInput input)
{
    FSOutput output;
    output.color = float4(push.color, 1.0);

    return output;
}
