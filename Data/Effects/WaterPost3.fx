// Water pixel shader
// Copyright (C) Wojciech Toman 2009

#define USE_FOAM	1

#define FOAM_MAP foamMap
#define HEIGHT_MAP normalMap
#define NORMAL_MAP normalMap
#define FOAM_INDEX 2
#define HEIGHT_INDEX 0
#define NORMAL_INDEX 0
//#define GET_NORMAL(vInput) float4((vInput).xzy * 2.0f - float3(1.0f, 1.0f, 1.0f), 1.0f)
#define GET_NORMAL(vInput) vInput
#define COMPUTE_TANGENT_FRAME compute_tangent_frame
#define GET_TIMER(fFactor) ((timer * 2000.0f) * fFactor)
#define TEX2DATLAS(index, texcoords) tex2DlodAtlas(atlasMap, index, texcoords)

// make sure that application corresponding struct has the same members declaration order
struct WaterData
{
	float	fWaterLevel;
	float	fFadeSpeed;
	float	fNormalScale;
	float	fR0;
	float	fMaxAmplitude;
	float3	vSunColor;
	float	fShoreHardness;
	float	fRefractionStrength;
	float4	vNormalModifier;
	float	fDisplace;
	float3	vFoamExistence;
	float	fSunScale;
	float	fShininess;
	float	fSpecularIntensity;
	float3	vDepthColour;
	float3	vBigDepthColour;
	float3	vExtinction;
	float	fVisibility;
	float	fScale;
	float	fRefractionScale;
	float2	vWind;
	float3	vForward;
	float4	vAtlasInfo;
};

#define WATER_COUNT 4
float g_fWaterPostCount = (float)WATER_COUNT;
float g_fWaterPostCountInv = 1.0f / (float)WATER_COUNT;
int g_uWaterIndex = 0;
WaterData g_WaterData[WATER_COUNT] : WATERDATA;

float4 g_vFrustumCorners[8] : FRUSTUMCORNERS;
float4x4 matViewProj		: VIEWPROJ;
float4x4 matViewInverse		: VIEWINV;
float4x4 matView			: VIEW;
float4 cameraPos			: CAMERAPOS;
float timer					: TIME;
float4x4 matProj			: PROJ;
float4x4 matWorldViewProj	: WORLDVIEWPROJ;
float3 g_vLightDir			: LIGHTDIR;

texture t2datlasMap			: TEX2D00;
texture t2dbackBufferMap	: RT2D00;
texture t2dpositionMap		: RT2D01;
texture t2dreflection1Map	: TEX2D01;
texture t2dreflection2Map	: TEX2D02;
texture t2dreflection3Map	: TEX2D03;
texture t2dreflection4Map	: TEX2D04;

sampler2D atlasMap = sampler_state {
    Texture = <t2datlasMap>;
    MinFilter = Linear;
    MipFilter = Linear;
    MagFilter = Linear;
    AddressU = Wrap;
    AddressV = Wrap;
};
sampler2D backBufferMap = sampler_state {
    Texture = <t2dbackBufferMap>;
    MinFilter = Linear;
    MipFilter = Linear;
    MagFilter = Linear;
    AddressU = Wrap;
    AddressV = Wrap;
};
sampler2D positionMap = sampler_state {
    Texture = <t2dpositionMap>;
    MinFilter = Linear;
    MipFilter = Linear;
    MagFilter = Linear;
    AddressU = Wrap;
    AddressV = Wrap;
};
sampler2D reflection1Map = sampler_state {
    Texture = <t2dreflection1Map>;
    MinFilter = Linear;
    MipFilter = Linear;
    MagFilter = Linear;
    AddressU = Mirror;
    AddressV = Mirror;
};
sampler2D reflection2Map = sampler_state {
    Texture = <t2dreflection2Map>;
    MinFilter = Linear;
    MipFilter = Linear;
    MagFilter = Linear;
    AddressU = Mirror;
    AddressV = Mirror;
};
sampler2D reflection3Map = sampler_state {
    Texture = <t2dreflection3Map>;
    MinFilter = Linear;
    MipFilter = Linear;
    MagFilter = Linear;
    AddressU = Mirror;
    AddressV = Mirror;
};
sampler2D reflection4Map = sampler_state {
    Texture = <t2dreflection4Map>;
    MinFilter = Linear;
    MipFilter = Linear;
    MagFilter = Linear;
    AddressU = Mirror;
    AddressV = Mirror;
};

float4x4 matReflection =
{
	// Original version (right handed it seems)
	//{0.5f, 0.0f, 0.0f, 0.5f},
	//{0.0f, 0.5f, 0.0f, 0.5f},
	//{0.0f, 0.0f, 0.0f, 0.5f},
	//{0.0f, 0.0f, 0.0f, 1.0f}
	// Modified version, suited for left handed.
	{0.5f, 0.0f, 0.0f, 0.0f},
	{0.0f, -0.5f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.5f, 0.0f},
	{0.5f, 0.5f, 0.5f, 1.0f}
};

// VertexShader results
struct VertexOutput
{
	float4 position		: POSITION0;
	float2 texCoord		: TEXCOORD0;
	float3 texCoord2	: TEXCOORD1;
	float3 texCoord3	: TEXCOORD2;
	float3 texCoord4	: TEXCOORD3;
};

struct PS_OUTPUT
{
	float4 diffuse	: COLOR0;
	float4 normal	: COLOR1;
	float4 position	: COLOR2;
};

float GetMipmapLevel(float2 vUV, float2 vTileSize)
{
	float2 dx = ddx(vUV * vTileSize.x);
    float2 dy = ddy(vUV * vTileSize.y);
    float d = max(dot(dx, dx), dot(dy, dy));
    //return max(0.5 * log2(d), 0.0);
	return log2(sqrt(d));
}

float4 tex2DAtlas(sampler2D oSampler, int sIndex, float2 vOriginalUV)
{
	float4 vResult;

	/// estimate mipmap/LOD level
	float fLod = GetMipmapLevel(vOriginalUV, float2(g_WaterData[g_uWaterIndex].vAtlasInfo.z, g_WaterData[g_uWaterIndex].vAtlasInfo.z));
	fLod = clamp(fLod, 0.0, g_WaterData[g_uWaterIndex].vAtlasInfo.w - 2.0); // "- 2.0" removes 1x1 and 2x2 LODs

	/// get width/height of the whole pack texture for the current lod level
	float fSize = pow(2.0, g_WaterData[g_uWaterIndex].vAtlasInfo.w - fLod);
	float fSizex = fSize / g_WaterData[g_uWaterIndex].vAtlasInfo.x; // width in pixels
	float fSizey = fSize / g_WaterData[g_uWaterIndex].vAtlasInfo.y; // height in pixels

	/// perform tiling
	float2 vUV = frac(vOriginalUV);
	int nbTiles = int(1.0 / g_WaterData[g_uWaterIndex].vAtlasInfo.x);
	float2 vTile = float2(sIndex % nbTiles, sIndex / nbTiles);

	/// tweak pixels for correct bilinear filtering, and add offset for the wanted tile
	vUV.x = vUV.x * ((fSizex * g_WaterData[g_uWaterIndex].vAtlasInfo.x - 1.0) / fSizex) + 0.5 / fSizex + g_WaterData[g_uWaterIndex].vAtlasInfo.x * vTile.x;
	vUV.y = vUV.y * ((fSizey * g_WaterData[g_uWaterIndex].vAtlasInfo.y - 1.0) / fSizey) + 0.5 / fSizey + g_WaterData[g_uWaterIndex].vAtlasInfo.y * vTile.y;

    vResult = tex2D(oSampler, vUV);

	return vResult;
}


float4 tex2DlodAtlas(sampler2D oSampler, int sIndex, float2 vOriginalUV)
{
	float4 vResult;

	/// estimate mipmap/LOD level
	float fLod = GetMipmapLevel(vOriginalUV, float2(g_WaterData[g_uWaterIndex].vAtlasInfo.z, g_WaterData[g_uWaterIndex].vAtlasInfo.z));
	fLod = clamp(fLod, 0.0, g_WaterData[g_uWaterIndex].vAtlasInfo.w - 2.0); // "- 2.0" removes 1x1 and 2x2 LODs

	/// get width/height of the whole pack texture for the current lod level
	float fSize = pow(2.0, g_WaterData[g_uWaterIndex].vAtlasInfo.w - fLod);
	float fSizex = fSize / g_WaterData[g_uWaterIndex].vAtlasInfo.x; // width in pixels
	float fSizey = fSize / g_WaterData[g_uWaterIndex].vAtlasInfo.y; // height in pixels

	/// perform tiling
	float2 vUV = frac(vOriginalUV);
	int nbTiles = int(1.0 / g_WaterData[g_uWaterIndex].vAtlasInfo.x);
	float2 vTile = float2(sIndex % nbTiles, sIndex / nbTiles);

	/// tweak pixels for correct bilinear filtering, and add offset for the wanted tile
	vUV.x = vUV.x * ((fSizex * g_WaterData[g_uWaterIndex].vAtlasInfo.x - 1.0) / fSizex) + 0.5 / fSizex + g_WaterData[g_uWaterIndex].vAtlasInfo.x * vTile.x;
	vUV.y = vUV.y * ((fSizey * g_WaterData[g_uWaterIndex].vAtlasInfo.y - 1.0) / fSizey) + 0.5 / fSizey + g_WaterData[g_uWaterIndex].vAtlasInfo.y * vTile.y;

    vResult = tex2Dlod(oSampler, float4(vUV, 0.0, fLod));

	return vResult;
}

float3 VSPositionFromDepth(float2 vTexCoord, float3 vFrustumFarPointWS, float3 vFrustumNearPointWS)
{
	float fPixelDepth = tex2D(positionMap, vTexCoord).r;
	float3 vPosition = vFrustumNearPointWS + fPixelDepth * (vFrustumFarPointWS - vFrustumNearPointWS);
	return vPosition;
}


float3x3 compute_tangent_frame(float3 N, float3 P, float2 UV)
{
	float3 dp1 = ddx(P);
	float3 dp2 = ddy(P);
	float2 duv1 = ddx(UV);
	float2 duv2 = ddy(UV);
	
	float3x3 M = float3x3(dp1, dp2, cross(dp1, dp2));
	float2x3 inverseM = float2x3( cross( M[1], M[2] ), cross( M[2], M[0] ) );
	float3 T = mul(float2(duv1.x, duv2.x), inverseM);
	float3 B = mul(float2(duv1.y, duv2.y), inverseM);
	
	return float3x3(normalize(T), normalize(B), N);
}

float3x3 invert_3x3(float3x3 mat)
{
	float det = determinant(mat);
	float3x3 T = transpose(mat);
	return float3x3(cross(T[1], T[2]),
					cross(T[2], T[0]),
					cross(T[0], T[1])) / det;
}

float3x3 compute_tangent_frame2(float3 N, float3 P, float2 UV)
{
	float3 dp1 = ddx(P);
	float3 dp2 = ddy(P);
	float2 duv1 = ddx(UV);
	float2 duv2 = ddy(UV);
	
	float3x3 M = float3x3(dp1, dp2, cross(dp1, dp2));
	float3x3 inverseM = invert_3x3(M);
	float3 T = mul(inverseM, float3(duv1.x, duv2.x, 0.0f));
	float3 B = mul(inverseM, float3(duv1.y, duv2.y, 0.0f));
	
	float maxLength = max(length(T), length(B));
	return float3x3(T / maxLength, B / maxLength, N);
}


// Function calculating fresnel term.
// - normal - normalized normal vector
// - eyeVec - normalized eye vector
float fresnelTerm(float3 normal, float3 eyeVec)
{
		float angle = 1.0f - saturate(dot(normal, eyeVec));
		float fresnel = angle * angle;
		fresnel = fresnel * fresnel;
		fresnel = fresnel * angle;
		return saturate(fresnel * (1.0f - saturate(g_WaterData[g_uWaterIndex].fR0)) + g_WaterData[g_uWaterIndex].fR0 - g_WaterData[g_uWaterIndex].fRefractionStrength);
}

float4 RenderScenePS(VertexOutput IN): COLOR0
{
	float4 base_color = tex2D(backBufferMap, IN.texCoord);
	float3 color2 = base_color.rgb;
	float3 color = color2;

	g_uWaterIndex = (int)(base_color.a / g_fWaterPostCountInv);

	float3 position = VSPositionFromDepth(IN.texCoord, IN.texCoord3, IN.texCoord4);
	float level = g_WaterData[g_uWaterIndex].fWaterLevel;
	float depth = 0.0f;

	float3 lightDir = -g_vLightDir;

	// If we are underwater let's leave out complex computationsd
	if(level >= cameraPos.y)
		return float4(color2, 1.0f);
	
	if(position.y <= level + g_WaterData[g_uWaterIndex].fMaxAmplitude)
	{
		float3 eyeVec = position - cameraPos.xyz;
		float diff = level - position.y;
		float cameraDepth = cameraPos.y - position.y;
		
		// Find intersection with water surface
		float3 eyeVecNorm = normalize(eyeVec);
		float t = (level - cameraPos.y) / eyeVecNorm.y;
		float3 surfacePoint = cameraPos.xyz + eyeVecNorm * t;
		
		eyeVecNorm = normalize(eyeVecNorm);
		
		float2 texCoord;
		for(int i = 0; i < 10; ++i)
		{
			texCoord = (surfacePoint.xz + eyeVecNorm.xz * 0.1f) * g_WaterData[g_uWaterIndex].fScale + GET_TIMER(0.000005f) * g_WaterData[g_uWaterIndex].vWind;
			
			float bias = TEX2DATLAS(HEIGHT_INDEX, texCoord).r;
	
			bias *= 0.1f;
			level += bias * g_WaterData[g_uWaterIndex].fMaxAmplitude;
			t = (level - cameraPos.y) / eyeVecNorm.y;
			surfacePoint = cameraPos.xyz + eyeVecNorm * t;
		}
		
		depth = length(position - surfacePoint);
		float depth2 = abs(surfacePoint.y - position.y);
		
		eyeVecNorm = normalize(cameraPos.xyz - surfacePoint);
		
		float normal1 = TEX2DATLAS(HEIGHT_INDEX, (texCoord + float2(-1, 0) / 256)).r;
		float normal2 = TEX2DATLAS(HEIGHT_INDEX, (texCoord + float2(1, 0) / 256)).r;
		float normal3 = TEX2DATLAS(HEIGHT_INDEX, (texCoord + float2(0, -1) / 256)).r;
		float normal4 = TEX2DATLAS(HEIGHT_INDEX, (texCoord + float2(0, 1) / 256)).r;
		
		float3 myNormal = normalize(float3((normal1 - normal2) * g_WaterData[g_uWaterIndex].fMaxAmplitude,
										   g_WaterData[g_uWaterIndex].fNormalScale,
										   (normal3 - normal4) * g_WaterData[g_uWaterIndex].fMaxAmplitude));   
		
		texCoord = surfacePoint.xz * 1.6 + g_WaterData[g_uWaterIndex].vWind * GET_TIMER(0.00016);
		float3x3 tangentFrame = COMPUTE_TANGENT_FRAME(myNormal, eyeVecNorm, texCoord);
		float3 normal0a = normalize(mul(2.0f * GET_NORMAL(TEX2DATLAS(NORMAL_INDEX, texCoord)) - 1.0f, tangentFrame));

		texCoord = surfacePoint.xz * 0.8 + g_WaterData[g_uWaterIndex].vWind * GET_TIMER(0.00008);
		tangentFrame = COMPUTE_TANGENT_FRAME(myNormal, eyeVecNorm, texCoord);
		float3 normal1a = normalize(mul(2.0f * GET_NORMAL(TEX2DATLAS(NORMAL_INDEX, texCoord)) - 1.0f, tangentFrame));
		
		texCoord = surfacePoint.xz * 0.4 + g_WaterData[g_uWaterIndex].vWind * GET_TIMER(0.00004);
		tangentFrame = COMPUTE_TANGENT_FRAME(myNormal, eyeVecNorm, texCoord);
		float3 normal2a = normalize(mul(2.0f * GET_NORMAL(TEX2DATLAS(NORMAL_INDEX, texCoord)) - 1.0f, tangentFrame));
		
		texCoord = surfacePoint.xz * 0.1 + g_WaterData[g_uWaterIndex].vWind * GET_TIMER(0.00002);
		tangentFrame = COMPUTE_TANGENT_FRAME(myNormal, eyeVecNorm, texCoord);
		float3 normal3a = normalize(mul(2.0f * GET_NORMAL(TEX2DATLAS(NORMAL_INDEX, texCoord)) - 1.0f, tangentFrame));
		
		float3 normal = normalize(normal0a * g_WaterData[g_uWaterIndex].vNormalModifier.x + normal1a * g_WaterData[g_uWaterIndex].vNormalModifier.y +
								  normal2a * g_WaterData[g_uWaterIndex].vNormalModifier.z + normal3a * g_WaterData[g_uWaterIndex].vNormalModifier.w);
		
		texCoord = IN.texCoord.xy;
		texCoord.x += sin(GET_TIMER(0.002f) + 3.0f * abs(position.y)) * (g_WaterData[g_uWaterIndex].fRefractionScale * min(depth2, 1.0f));
		float3 refraction = tex2D(backBufferMap, texCoord).rgb;
		if (mul(float4(VSPositionFromDepth(texCoord, IN.texCoord3, IN.texCoord4).xyz, 1.0f), matViewInverse).y > level)
			refraction = color2;

		float4x4 matTextureProj = mul(matViewProj, matReflection);
				
		float3 waterPosition = surfacePoint.xyz;
		waterPosition.y -= (level - g_WaterData[g_uWaterIndex].fWaterLevel);
		float4 texCoordProj = mul(float4(waterPosition, 1.0f), matTextureProj);
		
		float4 dPos;
		dPos.x = texCoordProj.x + g_WaterData[g_uWaterIndex].fDisplace * normal.x;
		dPos.z = texCoordProj.z + g_WaterData[g_uWaterIndex].fDisplace * normal.z;
		dPos.yw = texCoordProj.yw;
		texCoordProj = dPos;
		float3 reflect;
		if (0 == g_uWaterIndex)
		{
			reflect = tex2Dproj(reflection1Map, texCoordProj);
		}
		else if (1 == g_uWaterIndex)
		{
			reflect = tex2Dproj(reflection2Map, texCoordProj);
		}
		else if (2 == g_uWaterIndex)
		{
			reflect = tex2Dproj(reflection3Map, texCoordProj);
		}
		else if (3 == g_uWaterIndex)
		{
			reflect = tex2Dproj(reflection4Map, texCoordProj);
		}

		float fresnel = fresnelTerm(normal, eyeVecNorm);
		
		float3 depthN = depth * g_WaterData[g_uWaterIndex].fFadeSpeed;
		float3 waterCol = saturate(length(g_WaterData[g_uWaterIndex].vSunColor) / g_WaterData[g_uWaterIndex].fSunScale);
		refraction = lerp(lerp(refraction, g_WaterData[g_uWaterIndex].vDepthColour * waterCol, saturate(depthN / g_WaterData[g_uWaterIndex].fVisibility)),
						  g_WaterData[g_uWaterIndex].vBigDepthColour * waterCol, saturate(depth2 / g_WaterData[g_uWaterIndex].vExtinction));

		float foam = 0.0f;		
#if USE_FOAM
		texCoord = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * 0.05 + GET_TIMER(0.00001f) * g_WaterData[g_uWaterIndex].vWind + sin(GET_TIMER(0.001) + position.x) * 0.005;
		float2 texCoord2 = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * 0.05 + GET_TIMER(0.00002f) * g_WaterData[g_uWaterIndex].vWind + sin(GET_TIMER(0.001) + position.z) * 0.005;

		// coastal foam - ground part
		if(depth2 < g_WaterData[g_uWaterIndex].vFoamExistence.x)
			foam = (TEX2DATLAS(FOAM_INDEX, texCoord) + TEX2DATLAS(FOAM_INDEX, texCoord2)) * 0.5f;
		// coastal foam - water part
		else if(depth2 < g_WaterData[g_uWaterIndex].vFoamExistence.y)
		{
			foam = lerp((TEX2DATLAS(FOAM_INDEX, texCoord) + TEX2DATLAS(FOAM_INDEX, texCoord2)) * 0.5f, 0.0f,
						 (depth2 - g_WaterData[g_uWaterIndex].vFoamExistence.x) / (g_WaterData[g_uWaterIndex].vFoamExistence.y - g_WaterData[g_uWaterIndex].vFoamExistence.x));
		}

		// wave foam
		////if(g_WaterData[g_uWaterIndex].fMaxAmplitude - g_WaterData[g_uWaterIndex].vFoamExistence.z > 0.0001f)
		//if((level - g_WaterData[g_uWaterIndex].fWaterLevel) > g_WaterData[g_uWaterIndex].vFoamExistence.z)
		//{
		//	foam += (TEX2DATLAS(FOAM_INDEX, texCoord) + TEX2DATLAS(FOAM_INDEX, texCoord2)) * 0.5f * 
		//		saturate((level - (g_WaterData[g_uWaterIndex].fWaterLevel + g_WaterData[g_uWaterIndex].vFoamExistence.z)) / (g_WaterData[g_uWaterIndex].fMaxAmplitude - g_WaterData[g_uWaterIndex].vFoamExistence.z));
		//}
#endif // USE_FOAM

		// sun reflection
		float3 H = normalize(lightDir + eyeVecNorm);
		float e = g_WaterData[g_uWaterIndex].fShininess * 64;
		float kD = saturate(dot(normal, lightDir)); 
		float kS = kD * g_WaterData[g_uWaterIndex].fSpecularIntensity * pow( saturate( dot( normal, H ) ), e ) * sqrt( ( e + 1 ) / 2 );
		refraction *= dot(normal, lightDir);
		refraction += kS;

		half3 specular = 0.0f;
		half3 mirrorEye = (2.0f * dot(eyeVecNorm, normal) * normal - eyeVecNorm);
		half dotSpec = saturate(dot(mirrorEye.xyz, -lightDir) * 0.5f + 0.5f);
		specular = (1.0f - fresnel) * saturate(-lightDir.y) * ((pow(dotSpec, 512.0f)) * (g_WaterData[g_uWaterIndex].fShininess * 1.8f + 0.2f));//* g_WaterData[g_uWaterIndex].vSunColor;
		specular += specular * 25 * saturate(g_WaterData[g_uWaterIndex].fShininess - 0.05f);// * g_WaterData[g_uWaterIndex].vSunColor;

		color = lerp(refraction, reflect, fresnel);
		color = saturate(color + max(specular, foam * g_WaterData[g_uWaterIndex].vSunColor));
		color = lerp(refraction, color, saturate(depth * g_WaterData[g_uWaterIndex].fShoreHardness));
	}
	
	if(position.y > level)
		color = color2;

	return float4(color, 1.0f);
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
