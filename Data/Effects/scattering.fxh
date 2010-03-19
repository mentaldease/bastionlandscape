float3 betaRayleigh			: SKY_BETARAYLEIGH;			// Rayleigh scattering total factor
float3 betaMie				: SKY_BETAMIE;				// Mie scattering total factor               
float4 sunColorIntensity	: SKY_SUNCOLORINTENSITY;	// Sun color * intensity
float3 oneOverRayleighMie	: SKY_ONEOVERRAYLEIGHMIE;	// 1 / (betaRayleigh + betaMie)
float3 betaDashRayleigh		: SKY_BETADASHRAYLEIGH;		// 3/16PI * betaRayleigh
float3 betaDashMie			: SKY_BETADASHMIE;			// 1/4PI * betaMie
float3 hgData				: SKY_HGDATA;				// optimisation for Henyey-Greenstein phase function (Mie scattering)
//float3 skyDistance			: SKY_DISTANCE;				// TODO : sky distance (for terrain blending with sky)
float3 eyePos				: CAMERAPOS;				// camera position (world coordinates)
float4x4 matWorld			: WORLD;					// World matrix
float4x4 matWorldView		: WORLDVIEW;				// World * View matrix



////////////////////////////////////////////////////////
/// GLOBAL SCATTERING VERTEX SHADER
////////////////////////////////////////////////////////

struct SCATTERING_OUTPUT 
{
    float4 Position         : POSITION;   
    float3 InScattering     : TEXCOORD0;
};


struct SCATTERING_OUTPUT2
{
    float4 Position         : POSITION;   
    float3 ViewDir          : TEXCOORD0;
    float3 InScattering     : TEXCOORD1;
};

SCATTERING_OUTPUT ScatteringVS( float3 vPos : POSITION, float3 Norm : NORMAL, float2 Tex : TEXCOORD) 
{
    SCATTERING_OUTPUT Output;
    
    Output.Position = mul(float4(vPos, 1.0f), matWorldViewProj); 

    float s = mul(vPos, matWorldView).z / 100.0f;
    
    
    // computing in scattering
    float3 worldPos = mul(float4(vPos, 1.0f), matWorld);
    float3 viewDir = worldPos - eyePos;
    //float s = length(viewDir);
    viewDir = normalize(viewDir);
    
    float cosTheta = dot(viewDir, sunPosition);   


    // computing extinction = absorption + out scattering
    float3 extinction = exp(-(betaRayleigh + betaMie) * s);  // * log2 e ???


    float3 betaReyleighTheta = betaDashRayleigh * (1.0f + cosTheta * cosTheta); // this is correct but not nice
    //float3 betaReyleighTheta = betaDashRayleigh * (2.0 + 0.5 * cosTheta * cosTheta); // adjusted to avoid dark band effect
    // here we use + to correct sign between hgData.y and hgData.z
    float3 betaMieTheta = betaDashMie * (hgData.x / pow( (hgData.y + hgData.z * cosTheta), 1.5f));
    
    float3 inScattering = (betaReyleighTheta + betaMieTheta);
    inScattering *= (1.0f - extinction);
    inScattering *= oneOverRayleighMie;
    
    
    inScattering *= 0.3f;                   // in scattering multiplier
    inScattering *= sunColorIntensity.xyz;  // color
    inScattering *= sunColorIntensity.w;    // intensity
    
    
    Output.InScattering = inScattering;
    
    return Output;
}



SCATTERING_OUTPUT2 ScatteringVS2( float3 vPos : POSITION, float3 Norm : NORMAL, float2 Tex : TEXCOORD) 
{
    SCATTERING_OUTPUT2 Output;

    Output.Position = mul(float4(vPos, 1.0f), matWorldViewProj); 
	Output.Position.z = Output.Position.w - 0.5f;

	// TODO : faire en sorte que s se scale automatiquement par rapport à la taille de la sphere
    float s = mul(float4(vPos, 1.0f), matWorldView).z / 50.0f;
    
    // computing extinction = absorption + out scattering
    float3 extinction = exp(-(betaRayleigh+betaMie)* s);  // * log2 e ???
    
    // computing in scattering
    float3 worldPos = mul(float4(vPos, 1.0f), matWorld);
    float3 viewDir = eyePos - worldPos;
    
    
    float3 inScattering = (1.0f - extinction);
    inScattering *= oneOverRayleighMie;

    inScattering *= 0.3f;                   // in scattering multiplier
    inScattering *= sunColorIntensity.xyz;  // color
    inScattering *= sunColorIntensity.w ;   // intensity
    
    
    Output.InScattering = inScattering;
    Output.ViewDir = viewDir;
   
    
    return Output;
}

