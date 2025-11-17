struct FSInput {
    float3 color;
};

struct FSOutput
{
    float4 color : SV_Target0;
};

struct PushData
{
    float4x4 transform;
    float3 color;
};

[[push_constant]]
PushData push;

[shader("pixel")]
FSOutput main(FSInput input)
{
    FSOutput output;
    output.color = float4(input.color, 1.0);

    return output;
}
