// simpliest post process shader (ever ??!!)

//--------------------------------------------------------------------------------------
// Data
//--------------------------------------------------------------------------------------

texture g_ColorTex : RT2D00;

sampler2D ColorSampler = sampler_state
{
    Texture = <g_ColorTex>;
    AddressU = Wrap;
    AddressV = Wrap;
    MinFilter = Point;
    MagFilter = Linear;
    MipFilter = Linear;
};

//--------------------------------------------------------------------------------------
// Pixel shaders
//--------------------------------------------------------------------------------------

float4 RenderScenePS( float2 Tex : TEXCOORD0 ) : COLOR
{
    return tex2D(ColorSampler, Tex);
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------

technique RenderScene
{
    pass P0
    {          
        VertexShader = null;
        PixelShader  = compile ps_3_0 RenderScenePS();
        ZEnable = false;
    }
}
