struct FSInput
{
    [[vk::location(0)]] float3 fragColor;
};

struct FSOutput
{
    float4 color : SV_Target0;
};

[shader("pixel")]
FSOutput main(FSInput input)
{
    FSOutput output;
    output.color = float4(input.fragColor, 1.0);

    return output;
}
