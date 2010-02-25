float4x4 matWorldViewProj : WORLDVIEWPROJ;    // World * View * Projection matrix
float dayTime;
float3 sunPosition;
float HazeIntensity = 1.0f;
float HazeHeight = 1.0f;
float3 HazeColor = float3(0.96f, 0.27f, 0.0f);

#include "scattering.fxh"


texture SkyGradientTexture;
sampler SkyGradientSampler = sampler_state
{
  Texture = <SkyGradientTexture>;
#ifndef D3D10
    MipFilter = Linear;
    MinFilter = Linear;
    MagFilter = Linear;
#else 
	Filter = MIN_MAG_MIP_LINEAR;
#endif         
  AddressU = Mirror;
  AddressV = Clamp;
};
           
texture StarsTexture;
sampler StarsSampler = sampler_state
{
  Texture = <StarsTexture>;
#ifndef D3D10
    MipFilter = Linear;
    MinFilter = Linear;
    MagFilter = Linear;
#else
	Filter = MIN_MAG_MIP_LINEAR;
#endif
};   


struct VS_OUTPUT 
{
    float4 Position 		: POSITION;   
    float2 TexCoordStars 	: TEXCOORD0;
    float2 TexCoordGradient	: TEXCOORD1;
    float3 Normal 			: TEXCOORD2;   
    float3 WorldPosition 	: TEXCOORD3;   
};


VS_OUTPUT SkyDomeGradientVS(float3 Pos : POSITION, float3 Norm : NORMAL, float2 Tex : TEXCOORD)
{
    VS_OUTPUT Output;
    
    Output.Position = mul(float4(Pos, 1.0), matWorldViewProj);
    Output.Normal = Norm;
    Output.WorldPosition = Pos;

    Output.TexCoordStars = Tex;
    // update U coordinate (24 h)
    Output.TexCoordGradient.x = dayTime; 

    // update V coordinate (sun position)
    Output.TexCoordGradient.y = 1.0-Pos.y;

    return Output;    
}



VS_OUTPUT SkyDomeMovingGradientVS(float3 Pos : POSITION, float3 Norm : NORMAL, float2 Tex : TEXCOORD)
{
    VS_OUTPUT Output;
    
    Output.Position = mul(float4(Pos, 1.0), matWorldViewProj);
    Output.Normal = Norm;
    Output.WorldPosition = Pos;

    Output.TexCoordStars = Tex;
    // update U coordinate (24 h)
    Output.TexCoordGradient.x = dayTime; 

    // update V coordinate (sun position)
    Output.TexCoordGradient.y = (1.0-dot(Pos, sunPosition))/2;

    return Output;    
}

    
    

float4 SkyDomePS(VS_OUTPUT inPs) : COLOR
{ 
    
    // fetch gradient
    float4 gradientColor = tex2D(SkyGradientSampler, inPs.TexCoordGradient);
    
    // fetch stars
    float4 starsColor = tex2D(StarsSampler, inPs.TexCoordStars);
    
    // blending
    float4 finalColor = gradientColor*gradientColor.w+starsColor*(1-gradientColor.w);
    finalColor.w = 1.0f; // setting alpha to 1 as textures are already blended
    return finalColor;
   
}


float4 SkyDomeScatteringPS(SCATTERING_OUTPUT psIn) : COLOR
{
    // athmosphere below the ozone layer is not attenuating light
    // so we will use only in scattering for sky color (this may 
    // not be sufficient for flight simulators)  
    return float4(psIn.InScattering,1); 
}




float4 SkyDomeScatteringPS2(SCATTERING_OUTPUT2 psIn) : COLOR
{
    // athmosphere below the ozone layer is not attenuating light
    // so we will use only in scattering for sky color (this may 
    // not be sufficient for flight simulators)  
    
    float cosTheta = dot(normalize(psIn.ViewDir), sunPosition);   
    //float3 betaReyleighTheta = betaDashRayleigh * (2.0 + 0.5 * cosTheta * cosTheta); // adjusted to avoid dark band effect
    float3 betaReyleighTheta = betaDashRayleigh * (1.0 + cosTheta * cosTheta); 
    float3 betaMieTheta = betaDashMie * (hgData.x / pow( (hgData.y + hgData.z * cosTheta), 1.5));
    float3 finalInScattering = (betaReyleighTheta + betaMieTheta) * psIn.InScattering;
    
    
    // gradient on horizon
    
	float hazeDot = dot(float3(0,1,0), normalize(psIn.ViewDir-float3(0,1000,0)));
	float hazeFactor = abs(hazeDot*HazeHeight);
	float3 hazeFinalColor = HazeColor * (exp(-hazeFactor*10));	
	
    
    return float4(saturate(finalInScattering+hazeFinalColor*HazeIntensity), 1.0);
}
    
    

         

////////////////////////////////////////////////////////
/// TECHNIQUES
////////////////////////////////////////////////////////

#ifdef D3D10



RasterizerState CullBack
{
	CullMode = FRONT;
};

technique10 SkyDomeGradient
{
    pass P0
    {       
        SetVertexShader( CompileShader( vs_4_0, SkyDomeGradientVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, SkyDomePS() ) );
    }
}

technique10 SkyDomeMovingGradient
{
    pass P0
    {       
        SetVertexShader( CompileShader( vs_4_0, SkyDomeMovingGradientVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, SkyDomePS() ) );
    }
}



technique10 SkyDomeScattering
{
    pass P0
    {       
        SetVertexShader( CompileShader( vs_4_0, ScatteringVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, SkyDomeScatteringPS() ) );
    }
}

technique10 SkyDomeScatteringPerPixel
{
    pass P0
    {       
        SetVertexShader( CompileShader( vs_4_0, ScatteringVS2() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, SkyDomeScatteringPS2() ) );
    
        SetRasterizerState(CullBack);
    }
}

#else

technique SkyDomeGradient
{
    pass P0
    {          
        VertexShader = compile vs_2_0 SkyDomeGradientVS();
        PixelShader  = compile ps_2_0 SkyDomePS();
    }

}

technique SkyDomeMovingGradient
{
    pass P0
    {          
        VertexShader = compile vs_2_0 SkyDomeMovingGradientVS();
        PixelShader  = compile ps_2_0 SkyDomePS();
    }

}


technique SkyDomeScattering
{
    pass P0
    {          
        VertexShader = compile vs_2_0 ScatteringVS();
        PixelShader  = compile ps_2_0 SkyDomeScatteringPS();
    }

}


technique SkyDomeScatteringPerPixel
{
    pass P0
    {          
        VertexShader = compile vs_3_0 ScatteringVS2();
        PixelShader  = compile ps_3_0 SkyDomeScatteringPS2();
		
		
    }

}
#endif
