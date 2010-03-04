#ifndef __SKY_H__
#define __SKY_H__

#include "../Application/ApplicationIncludes.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Sky : public CoreObject
	{
	public:
		Sky(SceneRef _rScene);
		virtual ~Sky();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

	protected:
		bool InitSkyParameters();
		void UpdateSkyParameters();

		Vector4 Interpolate(Vector4 src, Vector4 dst, float factor);
		Vector3 Interpolate(Vector3 src, Vector3 dst, float factor);
		Vector4 GetSunColorWithIntensity(float zenithAngle);
		Vector4 GetSunColor(float zenithAngle);
		Vector4 ComputeSunAttenuation(float fTheta, int nTurbidity/* = 2*/);
		float GetSunIntensity();

	protected:
		SceneRef					m_rScene;
		DisplayGeometrySpherePtr	m_pSphere;
		Vector4						m_f4SkySunColorIntensity;
		Vector3						m_f3SkyBetaRayleigh;
		Vector3						m_f3BetaDashRayleigh;
		Vector3						m_f3SkyBetaDashRayleigh;
		Vector3						m_f3SkyBetaMie;
		Vector3						m_f3SkyBetaDashMie;
		Vector3						m_f3SkyOneOverRayleighMie;
		Vector3						m_f3SkyHgData;
		Vector3						m_f3HazeColor;
		Vector3						m_f3SunPosition;
		float						m_fHazeHeight;
		float						m_fHazeIntensity;
		float						m_fDayTime;
		float						m_fVerticalOffset;
		float						m_fIntensity;
		bool						m_bAlwaysVisible;
		bool						m_bInOctree;
	};
}

#endif // __SKY_H__
