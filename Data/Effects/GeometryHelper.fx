//--------------------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------------------

#define CAMERA_LINEARIZED_DEPTH	0
#define USE_PS_NORMAL 1

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float4x4 g_mWorldViewProjection	: WORLDVIEWPROJ;	// World * View * Projection matrix
float4x4 g_mWorldInvTransp		: WORLDINVTRANSPOSE;
float4 g_vLightDir				: LIGHTDIR;
float4x4 g_mView				: VIEW;
float4x4 g_mWorld				: WORLD;
float4x4 g_mViewInv				: VIEWINV;
float4 g_f4Diffuse				: DIFFUSECOLOR;

//--------------------------------------------------------------------------------------
// Vertex shader input/output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 Position	: POSITION;
	float3 Normal	: NORMAL;
	float2 UV		: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Position		: POSITION;   // vertex position
	float2 UV			: TEXCOORD0;
	float3 Light		: TEXCOORD1;
	float3 Normal		: TEXCOORD2;  // vertex normal
	float3 UV2			: TEXCOORD3;
	float  PositionZ	: TEXCOORD4;  // vertex position
};

struct VS_INPUT_COLOR
{
	float4 Position	: POSITION;
	float3 Normal	: NORMAL;
	float4 Diffuse	: COLOR0;
	float2 UV		: TEXCOORD0;
};

struct VS_OUTPUT_COLOR
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
	VS_OUTPUT vsOutput = (VS_OUTPUT)0;

	vsOutput.Position = mul(vsInput.Position, g_mWorldViewProjection);
	//vsOutput.Diffuse = vsInput.Diffuse;
	vsOutput.UV = vsInput.UV;
	vsOutput.Light = normalize(g_vLightDir);
	vsOutput.Normal = normalize(mul(vsInput.Normal, g_mWorldInvTransp));

	float4x4 matWorldView = mul(g_mWorld, g_mView);
    float4 vPositionVS = mul(vsInput.Position, matWorldView);
#if CAMERA_LINEARIZED_DEPTH
	vsOutput.Position.z *= vsOutput.Position.w;
	vsOutput.PositionZ = vPositionVS.z * vPositionVS.w;
#else // CAMERA_LINEARIZED_DEPTH
	vsOutput.PositionZ = vPositionVS.z;
#endif // CAMERA_LINEARIZED_DEPTH

	return vsOutput;    
}

VS_OUTPUT_COLOR RenderSceneVSColor(VS_INPUT_COLOR vsInput)
{
	VS_OUTPUT_COLOR vsOutput = (VS_OUTPUT_COLOR)0;

	vsOutput.Position = mul(vsInput.Position, g_mWorldViewProjection);
	vsOutput.Diffuse = vsInput.Diffuse;
	vsOutput.UV = vsInput.UV;
	vsOutput.Light = normalize(g_vLightDir);
	vsOutput.Normal = normalize(mul(vsInput.Normal, g_mWorldInvTransp));

	float4x4 matWorldView = mul(g_mWorld, g_mView);
    float4 vPositionVS = mul(vsInput.Position, matWorldView);
#if CAMERA_LINEARIZED_DEPTH
	vsOutput.Position.z *= vsOutput.Position.w;
	vsOutput.PositionZ = vPositionVS.z * vPositionVS.w;
#else // CAMERA_LINEARIZED_DEPTH
	vsOutput.PositionZ = vPositionVS.z;
#endif // CAMERA_LINEARIZED_DEPTH

	return vsOutput;    
}

//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
	float4 vColor		: COLOR0;  // Pixel color
	float4 vPosition	: COLOR1;  // Pixel position
#if USE_PS_NORMAL
	float4 vNormal		: COLOR2;  // Pixel normal
#endif // USE_PS_NORMAL
};

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT RenderScenePS(VS_OUTPUT psInput)
{ 
	PS_OUTPUT psOutput = (PS_OUTPUT)0;

	//float4 vTexColor = tex2D(DiffuseSampler0, psInput.UV);
	//psOutput.vColor = g_f4Diffuse * vTexColor;
	psOutput.vColor = g_f4Diffuse;
	//psOutput.vColor *= saturate(dot(psInput.Light, psInput.Normal));
	//psOutput.vColor.a = 1.0f;

#if USE_PS_NORMAL
	float3 vNormal = (psInput.Normal + float3(1.0f, 1.0f, 1.0f)) * 0.5f;
	psOutput.vNormal = float4(vNormal, 1.0f);
#endif // USE_PS_NORMAL

	float fDepth = psInput.PositionZ / 100000.0f;
	psOutput.vPosition = float4(fDepth, 1.0f, 1.0f, 1.0f);

	return psOutput;
}

PS_OUTPUT RenderScenePSColor(VS_OUTPUT_COLOR psInput)
{ 
	PS_OUTPUT psOutput = (PS_OUTPUT)0;

	//float4 vTexColor = tex2D(DiffuseSampler0, psInput.UV);
	//psOutput.vColor = g_f4Diffuse * vTexColor;
	psOutput.vColor = g_f4Diffuse * psInput.Diffuse;

#if USE_PS_NORMAL
	float3 vNormal = (psInput.Normal + float3(1.0f, 1.0f, 1.0f)) * 0.5f;
	psOutput.vNormal = float4(vNormal, 1.0f);
#endif // USE_PS_NORMAL

	float fDepth = psInput.PositionZ / 100000.0f;
	psOutput.vPosition = float4(fDepth, 1.0f, 1.0f, 1.0f);

	return psOutput;
}

//--------------------------------------------------------------------------------------
// Renders scene to render target
//--------------------------------------------------------------------------------------

technique RenderScene
{
    pass P0
    {
        VertexShader = compile vs_3_0 RenderSceneVS();
        PixelShader  = compile ps_3_0 RenderScenePS();
    }
}

technique RenderSceneVertexColor
{
    pass P0
    {
        VertexShader = compile vs_3_0 RenderSceneVSColor();
        PixelShader  = compile ps_3_0 RenderScenePSColor();
    }
}
