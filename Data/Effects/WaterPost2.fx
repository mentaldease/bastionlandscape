// Water pixel shader
// Copyright (C) Wojciech Toman 2009

#define USE_FOAM	1

#define FOAM_MAP foamMap
#define HEIGHT_MAP normalMap
#define NORMAL_MAP normalMap
//#define GET_NORMAL(vInput) float4((vInput).xzy * 2.0f - float3(1.0f, 1.0f, 1.0f), 1.0f)
#define GET_NORMAL(vInput) vInput
#define COMPUTE_TANGENT_FRAME compute_tangent_frame
#define GET_TIMER(fFactor) ((timer * 2000.0f) * fFactor)

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
};

#ifndef WATER_COUNT
#define WATER_COUNT 4
#endif // WATER_COUNT
float g_fWaterCount = (float)WATER_COUNT;
float g_fWaterCountInv = 1.0f / (float)WATER_COUNT;
uint g_uWaterIndex = 0;
WaterData g_WaterData[WATER_COUNT] : WATERDATA;

float4 g_vFrustumCorners[8] : FRUSTUMCORNERS;
float4x4 matViewProj		: VIEWPROJ;
float4x4 matViewInverse		: VIEWINV;
float4x4 matView			: VIEW;
float4 cameraPos			: CAMERAPOS;
float timer					: TIME;
float4x4 matProj			: PROJ;
float4x4 matWorldViewProj	: WORLDVIEWPROJ;
float3 g_vLightDir			: LIGHTDIR; //= {0.0f, 1.0f, 0.0f};

texture t2dheightMap		: NOISETEX;
texture t2dnormalMap		: TEX2D01;
texture t2dfoamMap			: TEX2D00;
texture t2dbackBufferMap	: RT2D00;
texture t2dpositionMap		: RT2D01;
texture t2dreflectionMap	: RT2D03;

sampler2D heightMap = sampler_state {
    Texture = <t2dheightMap>;
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
sampler2D normalMap = sampler_state {
    Texture = <t2dnormalMap>;
    MinFilter = Linear;
    MipFilter = Linear;
    MagFilter = Linear;
    AddressU = Wrap;
    AddressV = Wrap;
};
sampler2D foamMap = sampler_state {
    Texture = <t2dfoamMap>;
    MinFilter = Linear;
    MipFilter = Linear;
    MagFilter = Linear;
    AddressU = Wrap;
    AddressV = Wrap;
};
sampler2D reflectionMap = sampler_state {
    Texture = <t2dreflectionMap>;
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
	g_uWaterIndex = 0;

	float3 color2 = tex2D(backBufferMap, IN.texCoord).rgb;
	float3 color = color2;
	
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
			
			float bias = tex2D(HEIGHT_MAP, texCoord).r;
	
			bias *= 0.1f;
			level += bias * g_WaterData[g_uWaterIndex].fMaxAmplitude;
			t = (level - cameraPos.y) / eyeVecNorm.y;
			surfacePoint = cameraPos.xyz + eyeVecNorm * t;
		}
		
		depth = length(position - surfacePoint);
		float depth2 = abs(surfacePoint.y - position.y);
		
		eyeVecNorm = normalize(cameraPos.xyz - surfacePoint);
		
		float normal1 = tex2D(HEIGHT_MAP, (texCoord + float2(-1, 0) / 256)).r;
		float normal2 = tex2D(HEIGHT_MAP, (texCoord + float2(1, 0) / 256)).r;
		float normal3 = tex2D(HEIGHT_MAP, (texCoord + float2(0, -1) / 256)).r;
		float normal4 = tex2D(HEIGHT_MAP, (texCoord + float2(0, 1) / 256)).r;
		
		float3 myNormal = normalize(float3((normal1 - normal2) * g_WaterData[g_uWaterIndex].fMaxAmplitude,
										   g_WaterData[g_uWaterIndex].fNormalScale,
										   (normal3 - normal4) * g_WaterData[g_uWaterIndex].fMaxAmplitude));   
		
		texCoord = surfacePoint.xz * 1.6 + g_WaterData[g_uWaterIndex].vWind * GET_TIMER(0.00016);
		float3x3 tangentFrame = COMPUTE_TANGENT_FRAME(myNormal, eyeVecNorm, texCoord);
		float3 normal0a = normalize(mul(2.0f * GET_NORMAL(tex2D(NORMAL_MAP, texCoord)) - 1.0f, tangentFrame));

		texCoord = surfacePoint.xz * 0.8 + g_WaterData[g_uWaterIndex].vWind * GET_TIMER(0.00008);
		tangentFrame = COMPUTE_TANGENT_FRAME(myNormal, eyeVecNorm, texCoord);
		float3 normal1a = normalize(mul(2.0f * GET_NORMAL(tex2D(NORMAL_MAP, texCoord)) - 1.0f, tangentFrame));
		
		texCoord = surfacePoint.xz * 0.4 + g_WaterData[g_uWaterIndex].vWind * GET_TIMER(0.00004);
		tangentFrame = COMPUTE_TANGENT_FRAME(myNormal, eyeVecNorm, texCoord);
		float3 normal2a = normalize(mul(2.0f * GET_NORMAL(tex2D(NORMAL_MAP, texCoord)) - 1.0f, tangentFrame));
		
		texCoord = surfacePoint.xz * 0.1 + g_WaterData[g_uWaterIndex].vWind * GET_TIMER(0.00002);
		tangentFrame = COMPUTE_TANGENT_FRAME(myNormal, eyeVecNorm, texCoord);
		float3 normal3a = normalize(mul(2.0f * GET_NORMAL(tex2D(NORMAL_MAP, texCoord)) - 1.0f, tangentFrame));
		
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
		float3 reflect = tex2Dproj(reflectionMap, texCoordProj);
		
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
			foam = (tex2D(FOAM_MAP, texCoord) + tex2D(FOAM_MAP, texCoord2)) * 0.5f;
		// coastal foam - water part
		else if(depth2 < g_WaterData[g_uWaterIndex].vFoamExistence.y)
		{
			foam = lerp((tex2D(FOAM_MAP, texCoord) + tex2D(FOAM_MAP, texCoord2)) * 0.5f, 0.0f,
						 (depth2 - g_WaterData[g_uWaterIndex].vFoamExistence.x) / (g_WaterData[g_uWaterIndex].vFoamExistence.y - g_WaterData[g_uWaterIndex].vFoamExistence.x));
		}

		// wave foam
		////if(g_WaterData[g_uWaterIndex].fMaxAmplitude - g_WaterData[g_uWaterIndex].vFoamExistence.z > 0.0001f)
		//if((level - g_WaterData[g_uWaterIndex].fWaterLevel) > g_WaterData[g_uWaterIndex].vFoamExistence.z)
		//{
		//	foam += (tex2D(FOAM_MAP, texCoord) + tex2D(FOAM_MAP, texCoord2)) * 0.5f * 
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
