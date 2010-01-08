//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float4x4 g_mWorldViewProjection	: WORLDVIEWPROJ;	// World * View * Projection matrix
texture DiffuseTexture0 		: BITMAPFONTTEX0;

sampler2D DiffuseSampler0 = sampler_state
{
    Texture = <DiffuseTexture0>;
    MinFilter = Linear;
    MipFilter = Linear;
    MagFilter = Linear;
    AddressU = Wrap;
    AddressV = Wrap;
};

//--------------------------------------------------------------------------------------
// VertexDefault shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Position	: POSITION;   // vertex position
	float4 Diffuse	: COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
	float3 UV		: TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS(float4 vPos : POSITION, float4 vDiffuse : COLOR0, float3 vUV : TEXCOORD0)
{
	VS_OUTPUT Output;

	Output.Position = vPos;//mul( vPos, g_mWorldViewProjection );
	Output.Diffuse = vDiffuse;
	Output.UV = vUV;

	return Output;    
}


//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
	float4 RGBColor : COLOR0;  // Pixel color    
};


//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT RenderScenePS( VS_OUTPUT In ) 
{ 
	PS_OUTPUT Output;

	Output.RGBColor = In.Diffuse * tex2D(DiffuseSampler0, In.UV);

	return Output;
}


//--------------------------------------------------------------------------------------
// Renders scene to render target
//--------------------------------------------------------------------------------------

technique RenderScene
{
    pass P0
    {          
		Lighting = FALSE;
		FillMode = wireframe;

        VertexShader = compile vs_2_0 RenderSceneVS();
        PixelShader  = compile ps_2_0 RenderScenePS();
    }
}
