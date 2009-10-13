// Water pixel shader
// Copyright (C) Wojciech Toman 2009

float4 g_vFrustumCorners[8] : FRUSTUMCORNERS;
float4x4 matViewProj		: VIEWPROJ;
float4x4 matViewInverse		: VIEWINV;
float4x4 matView			: VIEW;
float4 cameraPos			: CAMERAPOS;
float timer					: TIME;

texture t2dheightMap		: NOISETEX;
texture t2dbackBufferMap	: RT2D00;
texture t2dpositionMap		: RT2D01;
texture t2dnormalMap		: RT2D02;
texture t2dreflectionMap	: RT2D03;
texture t2dfoamMap			: TEX2D00;

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
    AddressU = Wrap;
    AddressV = Wrap;
};

// Level at which water surface begins
float waterLevel = 100.0f;

// How fast will colours fade out. You can also think about this
// values as how clear water is. Therefore use smaller values (eg. 0.05f)
// to have crystal clear water and bigger to achieve "muddy" water.
float fadeSpeed = 0.15f;

// Normals scaling factor
float normalScale = 1.0f;

// R0 is a constant related to the index of refraction (IOR).
// It should be computed on the CPU and passed to the shader.
float R0 = 0.5f;

// Maximum waves amplitude
float maxAmplitude = 1.0f;

// Direction of the light
float3 lightDir = {0.0f, 1.0f, 0.0f};

// Colour of the sun
float3 sunColor = {1.0f, 1.0f, 1.0f};

// The smaller this value is, the more soft the transition between
// shore and water. If you want hard edges use very big value.
// Default is 1.0f.
float shoreHardness = 1.0f;

// This value modifies current fresnel term. If you want to weaken
// reflections use bigger value. If you want to empasize them use
// value smaller then 0. Default is 0.0f.
float refractionStrength = 0.0f;
//float refractionStrength = -0.3f;

// Modifies 4 sampled normals. Increase first values to have more
// smaller "waves" or last to have more bigger "waves"
float4 normalModifier = {1.0f, 2.0f, 4.0f, 8.0f};

// Strength of displacement along normal.
float displace = 1.7f;

// Describes at what depth foam starts to fade out and
// at what it is completely invisible. The fird value is at
// what height foam for waves appear (+ waterLevel).
float3 foamExistence = {0.65f, 1.35f, 0.5f};

float sunScale = 3.0f;

float4x4 matReflection =
{
	{0.5f, 0.0f, 0.0f, 0.5f},
	{0.0f, 0.5f, 0.0f, 0.5f},
	{0.0f, 0.0f, 0.0f, 0.5f},
	{0.0f, 0.0f, 0.0f, 1.0f}
};

float shininess = 0.7f;
float specular_intensity = 0.32;

// Colour of the water surface
float3 depthColour = {0.0078f, 0.5176f, 0.7f};
// Colour of the water depth
float3 bigDepthColour = {0.0039f, 0.00196f, 0.145f};
float3 extinction = {7.0f, 30.0f, 40.0f};			// Horizontal

// Water transparency along eye vector.
float visibility = 4.0f;

// Increase this value to have more smaller waves.
float2 scale = {0.005f, 0.005f};
float refractionScale = 0.005f;

// Wind force in x and z axes.
float2 wind = {-0.3f, 0.7f};


// VertexShader results
struct VertexOutput
{
	float4 position		: POSITION0;
	float2 texCoord		: TEXCOORD0;
	float3 texCoord2	: TEXCOORD1;
};

struct PS_OUTPUT
{
	float4 diffuse: COLOR0;
	float4 normal: COLOR1;
	float4 position: COLOR2;
};

// Vertex shader for rendering a full-screen quad
VertexOutput QuadVS (	in float4 in_vPositionOS				: POSITION,
						in float3 in_vTexCoordAndCornerIndex	: TEXCOORD0)
{
	VertexOutput Output = (VertexOutput)0;

	Output.position = in_vPositionOS;
	Output.texCoord = in_vTexCoordAndCornerIndex.xy;
	Output.texCoord2 = g_vFrustumCorners[(int)in_vTexCoordAndCornerIndex.z].xyz;

	return Output;
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

// Function calculating fresnel term.
// - normal - normalized normal vector
// - eyeVec - normalized eye vector
float fresnelTerm(float3 normal, float3 eyeVec)
{
		float angle = 1.0f - saturate(dot(normal, eyeVec));
		float fresnel = angle * angle;
		fresnel = fresnel * fresnel;
		fresnel = fresnel * angle;
		return saturate(fresnel * (1.0f - saturate(R0)) + R0 - refractionStrength);
}

float3 VSPositionFromDepth(float2 vTexCoord, float3 vFrustumRayWS)
{
	float fPixelDepth = tex2D(positionMap, vTexCoord).r;
	float3 PositionViewSpace = fPixelDepth * vFrustumRayWS;
	return PositionViewSpace;
}

float4 RenderScenePS(VertexOutput IN): COLOR0
{
	float3 color2 = tex2D(backBufferMap, IN.texCoord).rgb;
	float3 color = color2;
	
	float3 position = VSPositionFromDepth(IN.texCoord, IN.texCoord2);
	float level = waterLevel;
	float depth = 0.0f;

	
	// If we are underwater let's leave out complex computationsd
	if(level >= cameraPos.y)
		return float4(color2, 1.0f);
	
	if(position.y <= level + maxAmplitude)
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
			texCoord = (surfacePoint.xz + eyeVecNorm.xz * 0.1f) * scale + timer * 0.000005f * wind;
			
			float bias = tex2D(heightMap, texCoord).r;
	
			bias *= 0.1f;
			level += bias * maxAmplitude;
			t = (level - cameraPos.y) / eyeVecNorm.y;
			surfacePoint = cameraPos.xyz + eyeVecNorm * t;
		}
		
		depth = length(position - surfacePoint);
		float depth2 = surfacePoint.y - position.y;
		
		eyeVecNorm = normalize(cameraPos.xyz - surfacePoint);
		
		float normal1 = tex2D(heightMap, (texCoord + float2(-1, 0) / 256)).r;
		float normal2 = tex2D(heightMap, (texCoord + float2(1, 0) / 256)).r;
		float normal3 = tex2D(heightMap, (texCoord + float2(0, -1) / 256)).r;
		float normal4 = tex2D(heightMap, (texCoord + float2(0, 1) / 256)).r;
		
		float3 myNormal = normalize(float3((normal1 - normal2) * maxAmplitude,
										   normalScale,
										   (normal3 - normal4) * maxAmplitude));   
		
		texCoord = surfacePoint.xz * 1.6 + wind * timer * 0.00016;
		float3x3 tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		float3 normal0a = normalize(mul(2.0f * tex2D(normalMap, texCoord) - 1.0f, tangentFrame));

		texCoord = surfacePoint.xz * 0.8 + wind * timer * 0.00008;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		float3 normal1a = normalize(mul(2.0f * tex2D(normalMap, texCoord) - 1.0f, tangentFrame));
		
		texCoord = surfacePoint.xz * 0.4 + wind * timer * 0.00004;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		float3 normal2a = normalize(mul(2.0f * tex2D(normalMap, texCoord) - 1.0f, tangentFrame));
		
		texCoord = surfacePoint.xz * 0.1 + wind * timer * 0.00002;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		float3 normal3a = normalize(mul(2.0f * tex2D(normalMap, texCoord) - 1.0f, tangentFrame));
		
		float3 normal = normalize(normal0a * normalModifier.x + normal1a * normalModifier.y +
								  normal2a * normalModifier.z + normal3a * normalModifier.w);
		
		texCoord = IN.texCoord.xy;
		texCoord.x += sin(timer * 0.002f + 3.0f * abs(position.y)) * (refractionScale * min(depth2, 1.0f));
		float3 refraction = tex2D(backBufferMap, texCoord).rgb;
		//if(mul(float4(tex2D(positionMap, texCoord).xyz, 1.0f), matViewInverse).y > level)
		if(VSPositionFromDepth(texCoord, IN.texCoord2).y > level)
			refraction = color2;

		float4x4 matTextureProj = mul(matViewProj, matReflection);
				
		float3 waterPosition = surfacePoint.xyz;
		waterPosition.y -= (level - waterLevel);
		float4 texCoordProj = mul(float4(waterPosition, 1.0f), matTextureProj);
		
		float4 dPos;
		dPos.x = texCoordProj.x + displace * normal.x;
		dPos.z = texCoordProj.z + displace * normal.z;
		dPos.yw = texCoordProj.yw;
		texCoordProj = dPos;		
		
		float3 reflect = tex2Dproj(reflectionMap, texCoordProj);
		
		float fresnel = fresnelTerm(normal, eyeVecNorm);
		
		float3 depthN = depth * fadeSpeed;
		float3 waterCol = saturate(length(sunColor) / sunScale);
		refraction = lerp(lerp(refraction, depthColour * waterCol, saturate(depthN / visibility)),
						  bigDepthColour * waterCol, saturate(depth2 / extinction));

		float foam = 0.0f;		

		texCoord = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * 0.05 + timer * 0.00001f * wind + sin(timer * 0.001 + position.x) * 0.005;
		float2 texCoord2 = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * 0.05 + timer * 0.00002f * wind + sin(timer * 0.001 + position.z) * 0.005;
		
		if(depth2 < foamExistence.x)
			foam = (tex2D(foamMap, texCoord) + tex2D(foamMap, texCoord2)) * 0.5f;
		else if(depth2 < foamExistence.y)
		{
			foam = lerp((tex2D(foamMap, texCoord) + tex2D(foamMap, texCoord2)) * 0.5f, 0.0f,
						 (depth2 - foamExistence.x) / (foamExistence.y - foamExistence.x));
			
		}
		
		if(maxAmplitude - foamExistence.z > 0.0001f)
		{
			foam += (tex2D(foamMap, texCoord) + tex2D(foamMap, texCoord2)) * 0.5f * 
				saturate((level - (waterLevel + foamExistence.z)) / (maxAmplitude - foamExistence.z));
		}


		half3 specular = 0.0f;

		half3 mirrorEye = (2.0f * dot(eyeVecNorm, normal) * normal - eyeVecNorm);
		half dotSpec = saturate(dot(mirrorEye.xyz, -lightDir) * 0.5f + 0.5f);
		specular = (1.0f - fresnel) * saturate(-lightDir.y) * ((pow(dotSpec, 512.0f)) * (shininess * 1.8f + 0.2f))* sunColor;
		specular += specular * 25 * saturate(shininess - 0.05f) * sunColor;		

		color = lerp(refraction, reflect, fresnel);
		color = saturate(color + max(specular, foam * sunColor));
		
		color = lerp(refraction, color, saturate(depth * shoreHardness));
		color = float3(1.0f, 1.0f, 1.0f);
	}
	
	if(position.y > level)
		color = color2;

	return float4(color, 1.0f);
	//return float4(tex2D(positionMap, IN.texCoord).rrr, 1.0);
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------

technique RenderScene
{
    pass P0
    {          
        //VertexShader = null;
		VertexShader = compile vs_3_0 QuadVS();
        PixelShader  = compile ps_3_0 RenderScenePS();
        ZEnable = false;
    }
}
