#ifndef __WATER_H__
#define __WATER_H__

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	typedef Vector2 float2;
	typedef Vector3 float3;
	typedef Vector4 float4;

	struct WaterData
	{
		float	m_fWaterLevel;
		float	m_fFadeSpeed;
		float	m_fNormalScale;
		float	m_fR0;
		float	m_fMaxAmplitude;
		float3	m_vSunColor;
		float	m_fShoreHardness;
		float	m_fRefractionStrength;
		float4	m_vNormalModifier;
		float	m_fDisplace;
		float3	m_vFoamExistence;
		float	m_fSunScale;
		float	m_fShininess;
		float	m_fSpecularIntensity;
		float3	m_vDepthColour;
		float3	m_vBigDepthColour;
		float3	m_vExtinction;
		float	m_fVisibility;
		float	m_fScale;
		float	m_fRefractionScale;
		float2	m_vWind;
		float3	m_vForward;
	};
}

#endif // __WATER_H__
