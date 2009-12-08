//--------------------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------------------

#define TERRAIN2_USE_NOISE		1
#define CAMERA_LINEARIZED_DEPTH	0

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

float4x4 g_mWorldViewProjection	: WORLDVIEWPROJ;
float4x4 g_mWorldInvTransp		: WORLDINVTRANSPOSE;
float g_fMorphFactor			: MORPHFACTOR;
float4 g_vLightDir				: LIGHTDIR;
float4 g_vAtlasInfo				: ATLASDIFFUSEINFO;
float4x4 g_mView				: VIEW;
float4x4 g_mWorld				: WORLD;
float4x4 g_mViewInv				: VIEWINV;

texture AtlasDiffuseTexture : ATLASDIFFUSETEX;
sampler2D AtlasDiffuseSampler = sampler_state {
    Texture = <AtlasDiffuseTexture>;
    MinFilter = Linear;
    MipFilter = Linear;
    MagFilter = Linear;
    AddressU = Wrap;
    AddressV = Wrap;
};

texture AtlasLUTTexture : ATLASLUTTEX;
sampler2D AtlasLUTSampler = sampler_state {
    Texture = <AtlasLUTTexture>;
    //MinFilter = None;
    //MipFilter = None;
    //MagFilter = None;
    AddressU = Wrap;
    AddressV = Wrap;
};

texture NoiseTexture : NOISETEX;
sampler2D NoiseSampler = sampler_state {
    Texture = <NoiseTexture>;
    //MinFilter = None;
    //MipFilter = None;
    //MagFilter = None;
    AddressU = Wrap;
    AddressV = Wrap;
};

#ifndef WATER_COUNT
#define WATER_COUNT 4
#endif // WATER_COUNT
float g_fWaterCount = (float)WATER_COUNT;
float g_fWaterCountInv = 1.0f / (float)WATER_COUNT;

//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

struct VS_INPUT
{
	float4 vPos		: POSITION;
	float3 vNorm	: NORMAL;
	float4 vDiffuse : COLOR0;
	float2 vUV		: TEXCOORD0;
	float3 vUV2		: TEXCOORD1;
};

struct VS_MORPHINPUT
{
	float4 vPos		: POSITION;
	float4 vDiffuse	: COLOR0;
	float4 vPos2	: POSITION1;
	float3 vNorm	: NORMAL;
	float2 vUV		: TEXCOORD0;
	float3 vUV2		: TEXCOORD1;
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

struct PS_OUTPUT
{
	float4 vColor		: COLOR0;  // Pixel color
	float4 vPosition	: COLOR1;  // Pixel position
	float4 vNormal		: COLOR2;  // Pixel normal
};

//--------------------------------------------------------------------------------------
// Vertex functions
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
	Output.UV2 = In.vUV2;

	float4x4 matWorldView = mul(g_mWorld, g_mView);
    float4 vPositionVS = mul(In.vPos, matWorldView);
#if CAMERA_LINEARIZED_DEPTH
	Output.Position.z *= Output.Position.w;
	Output.PositionZ = vPositionVS.z * vPositionVS.w;
#else // CAMERA_LINEARIZED_DEPTH
	Output.PositionZ = vPositionVS.z;
#endif // CAMERA_LINEARIZED_DEPTH

	return Output;    
}

VS_OUTPUT RenderSceneVS( VS_INPUT In )
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.UV2.z = In.vUV2.z * g_fWaterCountInv; // water level index
	Output.Position = mul(In.vPos, g_mWorldViewProjection);
	Output.Diffuse = In.vDiffuse;
	Output.Light = normalize(g_vLightDir);
	Output.Normal = normalize(mul(In.vNorm, g_mWorldInvTransp));
	Output.UV = In.vUV * 10.0f;
	Output.UV2.xy = In.vUV2.xy;

	float4x4 matWorldView = mul(g_mWorld, g_mView);
    float4 vPositionVS = mul(In.vPos, matWorldView);
#if CAMERA_LINEARIZED_DEPTH
	Output.Position.z *= Output.Position.w;
	Output.PositionZ = vPositionVS.z * vPositionVS.w;
#else // CAMERA_LINEARIZED_DEPTH
	Output.PositionZ = vPositionVS.z;
#endif // CAMERA_LINEARIZED_DEPTH

	return Output;    
}

//--------------------------------------------------------------------------------------
// Pixel functions
//--------------------------------------------------------------------------------------

float GetMipmapLevel(float2 vUV, float2 vTileSize)
{
	float2 dx = ddx(vUV * vTileSize.x);
    float2 dy = ddy(vUV * vTileSize.y);
    float d = max(dot(dx, dx), dot(dy, dy));
    //return max(0.5 * log2(d), 0.0);
	return log2(sqrt(d));
}

// This pixel function is based on Ysaneya's work
// http://www.gamedev.net/community/forums/mod/journal/journal.asp?jn=263350&cmonth=4&cyear=2008 (see Thursday, April 10, 2008)
// http://www.infinity-universe.com/Infinity/
PS_OUTPUT RenderScenePS( VS_OUTPUT In ) 
{ 
	PS_OUTPUT Output = (PS_OUTPUT)0;

	/// estimate mipmap/LOD level
	float fLod = GetMipmapLevel(In.UV, float2(g_vAtlasInfo.z, g_vAtlasInfo.z));
	fLod = clamp(fLod, 0.0, g_vAtlasInfo.w - 2.0); // "- 2.0" removes 1x1 and 2x2 LODs

	/// get width/height of the whole pack texture for the current lod level
	float fSize = pow(2.0, g_vAtlasInfo.w - fLod);
	float fSizex = fSize / g_vAtlasInfo.x; // width in pixels
	float fSizey = fSize / g_vAtlasInfo.y; // height in pixels

	/// perform tiling
	float2 vUV = frac(In.UV);

#if TERRAIN2_USE_NOISE
	float2 vNoise = float2(1.0, 1.0);
	float fNoiseFactor = 1.0;
	vNoise += tex2D(NoiseSampler, In.UV * 16.0).xx * fNoiseFactor;
	vNoise += tex2D(NoiseSampler, In.UV * 8.0).xx * fNoiseFactor;
	vNoise += tex2D(NoiseSampler, In.UV * 4.0).xx * fNoiseFactor;
	vNoise += tex2D(NoiseSampler, In.UV * 2.0).xx * fNoiseFactor;
	vNoise += tex2D(NoiseSampler, In.UV * 1.0).xx * fNoiseFactor;
	vNoise /= 5.0;
	float2 vLUT = clamp(In.UV2 * 0.99 + vNoise * 0.01, float2(0.0, 0.0), float2(1.0, 1.0));
	float4 vTexID = tex2D(AtlasLUTSampler, vLUT);
#else // TERRAIN2_USE_NOISE
	float4 vTexID = tex2D(AtlasLUTSampler, In.UV2);
#endif // TERRAIN2_USE_NOISE
	int nbTiles = int(1.0 / g_vAtlasInfo.x);
	int id0 = int(vTexID.x * 255.0);
	float2 vTile = float2(id0 % nbTiles, id0 / nbTiles);
	/// tweak pixels for correct bilinear filtering, and add offset for the wanted tile
	vUV.x = vUV.x * ((fSizex * g_vAtlasInfo.x - 1.0) / fSizex) + 0.5 / fSizex + g_vAtlasInfo.x * vTile.x;
	vUV.y = vUV.y * ((fSizey * g_vAtlasInfo.y - 1.0) / fSizey) + 0.5 / fSizey + g_vAtlasInfo.y * vTile.y;

    Output.vColor = tex2Dlod(AtlasDiffuseSampler, float4(vUV, 0.0, fLod));
	//Output.vColor = float4(vTexID.x * 16.0, 0.0, 0.0, 1.0); // color by slope intensity
	Output.vColor *= saturate(dot(In.Light, In.Normal));
	Output.vColor.a = In.UV2.z;  // store water index in color buffer fourth component.

	float3 vNormal = (In.Normal + float3(1.0f, 1.0f, 1.0f)) * 0.5f;
	Output.vNormal = float4(vNormal, 1.0f);
	float fDepth = In.PositionZ / 10000.0f;
	Output.vPosition = float4(fDepth, 1.0f, 1.0f, 1.0f);

	return Output;
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------

technique RenderSceneMorph
{
    pass P0
    {          
		Lighting = FALSE;
		FillMode = wireframe;

        VertexShader = compile vs_3_0 RenderSceneMorphVS();
        PixelShader  = compile ps_3_0 RenderScenePS();
    }
}

technique RenderScene
{
    pass P0
    {          
		//FillMode = wireframe;

        VertexShader = compile vs_3_0 RenderSceneVS();
        PixelShader  = compile ps_3_0 RenderScenePS();
    }
}
