// basic blur post effect shader

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

float4 RenderScenePS( float2 Tex : TEXCOORD0 ) : COLOR0
{
#if 0
	float4 vBaseColor = tex2D(ColorSampler, Tex);
	float4 vLeftColor = tex2D(ColorSampler, float2(Tex.x - 0.001, Tex.y));
	float4 vRightColor = tex2D(ColorSampler, float2(Tex.x + 0.001, Tex.y));
	float4 vTopColor = tex2D(ColorSampler, float2(Tex.x, Tex.y - 0.001));
	float4 vBottomColor = tex2D(ColorSampler, float2(Tex.x, Tex.y + 0.001));
	return (vBaseColor * 0.5) + ((vLeftColor + vRightColor + vTopColor + vBottomColor) * 0.25 * 0.5);
#else
	return (tex2D(ColorSampler, Tex)
		+ tex2D(ColorSampler, Tex + 0.001)
		+ tex2D(ColorSampler, Tex + 0.002)
		+ tex2D(ColorSampler, Tex + 0.003)
		+ tex2D(ColorSampler, Tex + 0.004)
		+ tex2D(ColorSampler, Tex + 0.005)
		+ tex2D(ColorSampler, Tex + 0.006)
		+ tex2D(ColorSampler, Tex + 0.008)
		) / 8.0;
#endif
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
