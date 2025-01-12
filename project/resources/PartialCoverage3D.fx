
// Worldview
float4x4 gWorldViewProj : WorldViewProjection;
// World
float4x4 gWorldMatrix : World;
float3 gCameraPosition = float3(0, 0, 0);

Texture2D gDiffuseMap : DiffuseMap;

//Sampling
SamplerState gSampleState : register(s0);

// RasterizerState
RasterizerState gRasterizerState
{
    CullMode = none;
    FrontCounterClockwise = false;
};

// Blending
 //https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_render_target_blend_desc
BlendState gBlendState
{
    BlendEnable[0] = true;
    SrcBlend = src_alpha;
    DestBlend = inv_src_alpha;
    BlendOp = add;
    SrcBlendAlpha = zero;
    DestBlendAlpha = zero;
    BlendOpAlpha = add;
    RenderTargetWriteMask[0] = 0x0F;
};

//Depth
DepthStencilState gDepthStencilState
{   
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less;
    StencilEnable = false;
    
    //Others are reduntdant because StencilEnable is false
    StencilReadMask = 0x0F;
    StencilWriteMask = 0x0F;

    FrontFaceStencilFunc = always;
    BackFaceStencilFunc = always;
    
    FrontFaceStencilDepthFail = keep;
    BackFaceStencilDepthFail = keep;

    FrontFaceStencilPass = keep;
    BackFaceStencilPass = keep;

    FrontFaceStencilFail = keep;
    BackFaceStencilFail = keep;
};

struct VS_INPUT
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
    
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : WORLD;
    float2 TexCoord : TEXCOORD;
    
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

// Vertrex Shader
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
    output.TexCoord = input.TexCoord;
    return output;
}

// Pixel Shader 
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    return gDiffuseMap.Sample(gSampleState, input.TexCoord);
}

// Technique 
technique11 DefaultTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}