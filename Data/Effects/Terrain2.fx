//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float4x4 g_mWorldViewProjection	: WORLDVIEWPROJ;
float4x4 g_mWorldInvTransp		: WORLDINVTRANSPOSE;
float g_fMorphFactor			: MORPHFACTOR;
float4 g_vLightDir				: LIGHTDIR;

texture DiffuseTexture : DIFFUSETEX;
sampler2D DiffuseSampler = sampler_state {
    Texture = <DiffuseTexture>;
    MinFilter = Linear;
    MipFilter = Linear;
    MagFilter = Linear;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct VS_INPUT
{
	float4 vPos		: POSITION;
	float3 vNorm	: NORMAL;
	float4 vDiffuse : COLOR0;
	float2 vUV		: TEXCOORD0;
};

struct VS_MORPHINPUT
{
	float4 vPos		: POSITION;
	float4 vDiffuse	: COLOR0;
	float4 vPos2	: POSITION1;
	float3 vNorm	: NORMAL;
	float2 vUV		: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// VertexDefault shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Position   : POSITION;   // vertex position
	float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
	float2 UV         : TEXCOORD0;
	float3 Light      : TEXCOORD1;
	float3 Normal     : TEXCOORD2;  // vertex normal
};


//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneMorphVS( VS_MORPHINPUT In )
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	float4 vMorph = In.vPos2 + (In.vPos - In.vPos2) * g_fMorphFactor;
	Output.Position = mul( vMorph, g_mWorldViewProjection );
	float4 cRed = float4(1.0f, 0.0f, 0.0f, 1.0f);
	float4 cMorph = cRed + (In.vDiffuse - cRed) * g_fMorphFactor;
	Output.Diffuse = cMorph;
	Output.Light = normalize(g_vLightDir);
	Output.Normal = normalize(mul(In.vNorm, g_mWorldInvTransp));
	Output.UV = In.vUV;

	return Output;    
}


//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS( VS_INPUT In )
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = mul(In.vPos, g_mWorldViewProjection);
	Output.Diffuse = In.vDiffuse;
	Output.Light = normalize(g_vLightDir);
	Output.Normal = normalize(mul(In.vNorm, g_mWorldInvTransp));
	Output.UV = In.vUV;

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

	Output.RGBColor = tex2D(DiffuseSampler, In.UV) * saturate(dot(In.Light, In.Normal));
	Output.RGBColor.a = 1.0f;

	return Output;
}


//--------------------------------------------------------------------------------------
// Renders scene to render target
//--------------------------------------------------------------------------------------

technique RenderSceneMorph
{
    pass P0
    {          
		Lighting = FALSE;
		FillMode = wireframe;

        VertexShader = compile vs_2_0 RenderSceneMorphVS();
        PixelShader  = compile ps_2_0 RenderScenePS();
    }
}

technique RenderScene
{
    pass P0
    {          
		//Lighting = TRUE;
		//FillMode = wireframe;
		//AlphaBlendEnable	= false;
		//AlphaTestEnable		= false;

        VertexShader = compile vs_2_0 RenderSceneVS();
        PixelShader  = compile ps_2_0 RenderScenePS();
    }
}
