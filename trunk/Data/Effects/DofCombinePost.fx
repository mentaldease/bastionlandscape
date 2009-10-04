//-----------------------------------------------------------------------------
// File: PP_DofCombine.fx
//
// Desc: Effect file for image post-processing sample.  This effect contains
//       a single technique with a pixel shader that outputs a value between
//       the original pixel and blurred pixel based on how far the pixel is
//       from a focal plane.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------




float4 FocalPlane = float4( 0.0f, 0.0f, 0.2f, -0.6f );

texture g_txSrcColor : RT2D00;
texture g_txSrcNormal;
texture g_txSrcPosition : RT2D01;
texture g_txSrcVelocity;

texture g_txSceneColor : ORT2D00;
texture g_txSceneNormal;
texture g_txScenePosition;
texture g_txSceneVelocity;

sampler2D g_samSrcColor =
sampler_state
{
    Texture = <g_txSrcColor>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Point;
    MagFilter = Linear;
    MipFilter = Linear;
};
sampler2D g_samSrcNormal =
sampler_state
{
    Texture = <g_txSrcNormal>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Point;
    MagFilter = Linear;
    MipFilter = Linear;
};
sampler2D g_samSrcPosition =
sampler_state
{
    Texture = <g_txSrcPosition>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Point;
    MagFilter = Linear;
    MipFilter = Linear;
};
sampler2D g_samSrcVelocity =
sampler_state
{
    Texture = <g_txSrcVelocity>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Point;
    MagFilter = Linear;
    MipFilter = Linear;
};

sampler2D g_samSceneColor = sampler_state
{
    Texture = <g_txSceneColor>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Point;
    MagFilter = Linear;
    MipFilter = Linear;
};
sampler2D g_samSceneNormal = sampler_state
{
    Texture = <g_txSceneNormal>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Point;
    MagFilter = Linear;
    MipFilter = Linear;
};
sampler2D g_samScenePosition = sampler_state
{
    Texture = <g_txScenePosition>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Point;
    MagFilter = Linear;
    MipFilter = Linear;
};
sampler2D g_samSceneVelocity = sampler_state
{
    Texture = <g_txSceneVelocity>;
    AddressU = Clamp;
    AddressV = Clamp;
    MinFilter = Point;
    MagFilter = Linear;
    MipFilter = Linear;
};




float2 PixelCoordsDownFilter[16] =
{
    { 1.5,  -1.5 },
    { 1.5,  -0.5 },
    { 1.5,   0.5 },
    { 1.5,   1.5 },

    { 0.5,  -1.5 },
    { 0.5,  -0.5 },
    { 0.5,   0.5 },
    { 0.5,   1.5 },

    {-0.5,  -1.5 },
    {-0.5,  -0.5 },
    {-0.5,   0.5 },
    {-0.5,   1.5 },

    {-1.5,  -1.5 },
    {-1.5,  -0.5 },
    {-1.5,   0.5 },
    {-1.5,   1.5 },
};

float2 TexelCoordsDownFilter[16]
<
    string ConvertPixelsToTexels = "PixelCoordsDownFilter";
>;




//-----------------------------------------------------------------------------
// Pixel Shader: DofCombine
// Desc: Combine the source image with the original image based on pixel
//       depths.
//-----------------------------------------------------------------------------
void DofCombine( float2 Tex : TEXCOORD0,
                 float2 Tex2 : TEXCOORD1,
                 out float4 oCol : COLOR0 )
{
    float3 ColorOrig = tex2D( g_samSceneColor, Tex2 );
    float3 ColorBlur = tex2D( g_samSrcColor, Tex );

#if 0
	float Blur = dot( tex2D( g_samSrcPosition, Tex ), FocalPlane );
    oCol = float4( lerp( ColorOrig, ColorBlur, saturate(abs(Blur)) ), 1.0f );
#else
	float Distance = 100.0f;
	float Range = 100.0f;
	float Near = 1.0f;
	float Far = 10000.0f;
	Far = Far / (Far - Near);
	// Get the depth texel
	float  fDepth = tex2D(g_samSrcPosition, Tex).z;
	// Invert the depth texel so the background is white and the nearest objects are black
	fDepth = 1 - fDepth;
	// Calculate the distance from the selected distance and range on our DoF effect, set from the application
	float fSceneZ = (-Near * Far) / (fDepth - Far);
	float Blur = saturate(abs(fSceneZ - Distance) / Range);
	// Based on how far the texel is from "distance" in Distance, stored in blurFactor, mix the scene
	//return lerp(NormalScene,BlurScene,blurFactor);
	oCol = float4(lerp(ColorOrig, ColorBlur, Blur), 1.0f);
#endif
}




//-----------------------------------------------------------------------------
// Technique: RenderScene
// Desc: Performs post-processing effect that down-filters.
//-----------------------------------------------------------------------------
technique RenderScene
<
    string Parameter0 = "FocalPlane";
    float4 Parameter0Def = float4( 0.0f, 0.0f, 0.2f, -0.6f );
    int Parameter0Size = 4;
    string Parameter0Desc = " (vector of 4 floats)";
>
{
    pass p0
    {
        VertexShader = null;
        PixelShader = compile ps_3_0 DofCombine();
        ZEnable = false;
    }
}
