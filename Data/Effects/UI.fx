//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float4x4 g_mWorldViewProjection	: WORLDVIEWPROJ;	// World * View * Projection matrix
texture g_DiffuseTexture0 		: BITMAPFONTTEX0;
float4 g_DiffuseColor			: BITMAPFONTDIFFUSE;

sampler2D DiffuseSampler0 = sampler_state
{
    Texture = <g_DiffuseTexture0>;
    MinFilter = Linear;
    MipFilter = Linear;
    MagFilter = Linear;
    AddressU = Wrap;
    AddressV = Wrap;
};

//--------------------------------------------------------------------------------------
// VertexDefault shader input/output structure
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 vPos 	: POSITION;
	float3 vUV 		: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Position	: POSITION;
	float3 UV		: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS(VS_INPUT vsInput)
{
	VS_OUTPUT vsOutput;

	vsOutput.Position = mul(vsInput.vPos, g_mWorldViewProjection );
	vsOutput.UV = vsInput.vUV;

	return vsOutput;    
}


//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
	float4 vColor		: COLOR0;  // Pixel color
};


//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT RenderScenePS( VS_OUTPUT psInput ) 
{ 
	PS_OUTPUT psOutput;

	float4 vTexColor = tex2D(DiffuseSampler0, psInput.UV);
	psOutput.vColor = g_DiffuseColor * vTexColor;

	return psOutput;
}


//--------------------------------------------------------------------------------------
// Renders scene to render target
//--------------------------------------------------------------------------------------

technique RenderScene
{
    pass P0
    {          
		Lighting = FALSE;
		CullMode = NONE;
		ZEnable = FALSE;
		AlphaBlendEnable = TRUE;
		DestBlend = INVSRCALPHA;
		SrcBlend = SRCALPHA;

        VertexShader = compile vs_2_0 RenderSceneVS();
        PixelShader  = compile ps_2_0 RenderScenePS();
    }
}
