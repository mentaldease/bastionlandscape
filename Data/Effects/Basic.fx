//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float4x4 g_mWorldViewProjection : WORLDVIEWPROJ;	// World * View * Projection matrix
float g_fMorphFactor : MORPHFACTOR;

//--------------------------------------------------------------------------------------
// LandscapeVertexDefault shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Position   : POSITION;   // vertex position 
	float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
};


//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS( float4 vPos : POSITION, float4 vDiffuse : COLOR0, float4 vPos2 : POSITION1 )
{
	VS_OUTPUT Output;

	float4 vMorph = vPos2 + (vPos - vPos2) * g_fMorphFactor;
	Output.Position = mul( vMorph, g_mWorldViewProjection );
	float4 cRed = float4(1.0f, 0.0f, 0.0f, 1.0f);
	float4 cMorph = cRed + (vDiffuse - cRed) * g_fMorphFactor;
	Output.Diffuse = cMorph;
	//Output.Position = mul( vPos, g_mWorldViewProjection );
	//Output.Diffuse = vDiffuse;

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

	Output.RGBColor = In.Diffuse;
	//Output.RGBColor = float4(1.0, 1.0, 1.0, 1.0);

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
