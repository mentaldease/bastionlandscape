//--------------------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------------------

#define CAMERA_LINEARIZED_DEPTH	0

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float4x4 g_mWorldViewProjection	: WORLDVIEWPROJ;	// World * View * Projection matrix
float4x4 g_mWorldInvTransp		: WORLDINVTRANSPOSE;
float4 g_vLightDir				: LIGHTDIR;
float4x4 g_mView				: VIEW;
float4x4 g_mWorld				: WORLD;
float4x4 g_mViewInv				: VIEWINV;

//--------------------------------------------------------------------------------------
// Vertex shader input/output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 Position	: POSITION;
	float3 Normal	: NORMAL;
	float4 Diffuse	: COLOR0;
	float2 UV		: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Position		: POSITION;   // vertex position
	float4 Diffuse		: COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
	float2 UV			: TEXCOORD0;
	float3 Light		: TEXCOORD1;
	float3 Normal		: TEXCOORD2;  // vertex normal
	float3 UV2			: TEXCOORD3;
	float  PositionZ	: TEXCOORD4;  // vertex position
};


//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS(VS_INPUT vsInput)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = mul(vsInput.Position, g_mWorldViewProjection);
	Output.Diffuse = vsInput.Diffuse;
	Output.UV = vsInput.UV;
	Output.Light = normalize(g_vLightDir);
	Output.Normal = normalize(mul(vsInput.Normal, g_mWorldInvTransp));

	float4x4 matWorldView = mul(g_mWorld, g_mView);
    float4 vPositionVS = mul(vsInput.Position, matWorldView);
#if CAMERA_LINEARIZED_DEPTH
	Output.Position.z *= Output.Position.w;
	Output.PositionZ = vPositionVS.z * vPositionVS.w;
#else // CAMERA_LINEARIZED_DEPTH
	Output.PositionZ = vPositionVS.z;
#endif // CAMERA_LINEARIZED_DEPTH

	return Output;    
}


//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
	float4 vColor		: COLOR0;  // Pixel color
	float4 vPosition	: COLOR1;  // Pixel position
	float4 vNormal		: COLOR2;  // Pixel normal
};


//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT RenderScenePS( VS_OUTPUT psInput ) 
{ 
	PS_OUTPUT Output = (PS_OUTPUT)0;

	//float4 vTexColor = tex2D(DiffuseSampler0, psInput.UV);
	//Output.vColor = psInput.Diffuse * vTexColor;
	Output.vColor = psInput.Diffuse;

	float3 vNormal = (psInput.Normal + float3(1.0f, 1.0f, 1.0f)) * 0.5f;
	Output.vNormal = float4(vNormal, 1.0f);

	float fDepth = psInput.PositionZ / 10000.0f;
	Output.vPosition = float4(fDepth, 1.0f, 1.0f, 1.0f);

	return Output;
}


//--------------------------------------------------------------------------------------
// Renders scene to render target
//--------------------------------------------------------------------------------------

technique RenderScene
{
    pass P0
    {
		// PointSize = 5.0;
		// Lighting = FALSE;
		// CullMode = NONE;
		// AlphaBlendEnable = TRUE;
		// DestBlend = INVSRCALPHA;
		// SrcBlend = SRCALPHA;

        VertexShader = compile vs_3_0 RenderSceneVS();
        PixelShader  = compile ps_3_0 RenderScenePS();
    }
}
