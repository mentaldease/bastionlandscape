#include "stdafx.h"
#include "../Application/Application.h"
#include "../Application/Sky.h"
#include "../Application/Scene.h"
#include "../Core/Scripting.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Sky::Sky(SceneRef _rScene)
	:	CoreObject(),
		m_rScene(_rScene),
		m_pSphere(NULL),
		m_f4SkySunColorIntensity(0.0f, 0.0f, 0.0f, 0.0f),
		m_f3SkyBetaRayleigh(0.0f, 0.0f, 0.0f),
		m_f3BetaDashRayleigh(0.0f, 0.0f, 0.0f),
		m_f3SkyBetaDashRayleigh(0.0f, 0.0f, 0.0f),
		m_f3SkyBetaMie(0.0f, 0.0f, 0.0f),
		m_f3SkyBetaDashMie(0.0f, 0.0f, 0.0f),
		m_f3SkyOneOverRayleighMie(0.0f, 0.0f, 0.0f),
		m_f3SkyHgData(0.0f, 0.0f, 0.0f),
		m_f3HazeColor(0.0f, 0.0f, 0.0f),
		m_f3SunPosition(0.0f, 0.0f, 0.0f),
		m_fHazeHeight(0.0f),
		m_fHazeIntensity(0.0f),
		m_fDayTime(12.0f * 60.0f),
		m_fVerticalOffset(0.0f),
		m_fIntensity(100.0f),
		m_bAlwaysVisible(false),
		m_bInOctree(true)
	{

	}
	
	Sky::~Sky()
	{

	}

	bool Sky::Create(const boost::any& _rConfig)
	{
		LuaObject rTable = boost::any_cast<LuaObject>(_rConfig);
		m_pSphere = new DisplayGeometrySphere();
		const float fSize = 100.f;
		DisplayGeometrySphere::CreateInfo oGSCInfo;
		Scripting::Lua::Get(rTable, "bottom_hemisphere", true, oGSCInfo.m_bBottomHemisphere);
		Scripting::Lua::Get(rTable, "top_hemisphere", true, oGSCInfo.m_bTopHemisphere);
		Scripting::Lua::Get(rTable, "view_from_inside", false, oGSCInfo.m_bViewFromInside);
		Scripting::Lua::Get(rTable, "position", Vector3(0.0f, 0.0f, 0.0f), oGSCInfo.m_oPos);
		Scripting::Lua::Get(rTable, "rotation", Vector3(0.0f, 0.0f, 0.0f), oGSCInfo.m_oRot);
		Scripting::Lua::Get(rTable, "radius", Vector3(fSize, fSize, fSize), oGSCInfo.m_oRadius);
		Scripting::Lua::Get(rTable, "horiz_slices", UInt(10), oGSCInfo.m_uHorizSlices);
		Scripting::Lua::Get(rTable, "vert_slices", UInt(10), oGSCInfo.m_uVertSlices);
		Scripting::Lua::Get(rTable, "color", Vector4(26.0f / 255.0f, 103.0f / 255.0f, 149.0f / 255.0f, 1.0f), oGSCInfo.m_f4Color);

		bool bResult = m_pSphere->Create(boost::any(&oGSCInfo));
		while (false != bResult)
		{
			DisplayPtr pDisplay = Display::GetInstance();
			string strMaterialName;
			Scripting::Lua::Get(rTable, "material", strMaterialName, strMaterialName);
			const Key uMaterialKey = MakeKey(strMaterialName);
			DisplayMaterialPtr pMaterial = pDisplay->GetMaterialManager()->GetMaterial(uMaterialKey);
			bResult = (NULL != pMaterial);
			if (false == bResult)
			{
				break;
			}

			string strRenderStage;
			bResult = Scripting::Lua::Get(rTable, "target_stage", strRenderStage, strRenderStage);
			if (false == bResult)
			{
				break;
			}

			Scripting::Lua::Get(rTable, "always_visible", m_bAlwaysVisible, m_bAlwaysVisible);
			Scripting::Lua::Get(rTable, "in_octree", m_bInOctree, m_bInOctree);

			m_pSphere->SetMaterial(pMaterial);
			m_pSphere->SetRenderStage(MakeKey(strRenderStage));
			bResult = InitSkyParameters();

			break;
		}

		return bResult;
	}

	void Sky::Update()
	{
		if (NULL != m_pSphere)
		{
			UpdateSkyParameters();
			m_pSphere->Update();
			if (false != m_bAlwaysVisible)
			{
				Display::GetInstance()->RenderRequest(m_pSphere->GetRenderStage(), m_pSphere);
			}
		}
	}

	void Sky::Release()
	{
		if (NULL != m_pSphere)
		{
			m_pSphere->Release();
			delete m_pSphere;
			m_pSphere = NULL;
		}
	}

	bool Sky::InitSkyParameters()
	{
		DisplayPtr pDisplay = Display::GetInstance();
		DisplayMaterialManagerPtr pMaterialManager = pDisplay->GetMaterialManager();
		bool bResult = true;

		// wavelenghts
		double lambdaRed = 650e-9;
		double lambdaGreen = 570e-9;
		double lambdaBlue = 475e-9;


		// multipliers
		float rayleighMultiplier = 0.4f;
		float mieMultiplier = 0.00009999f;


		//// Rayleigh total factor ////

		// refractive index of air
		double n = 1.003;
		// molecular density of air
		double N = 2.545e25;
		// depolarization factor
		double pn = 0.035;



		Vector3 betaRayleigh;
		betaRayleigh.x = (float)(1.0 / (3.0 * N * lambdaRed * lambdaRed * lambdaRed * lambdaRed));
		betaRayleigh.y = (float)(1.0 / (3.0 * N * lambdaGreen * lambdaGreen * lambdaGreen * lambdaGreen));
		betaRayleigh.z = (float)(1.0 / (3.0 * N * lambdaBlue * lambdaBlue * lambdaBlue * lambdaBlue));

		betaRayleigh *= (float)(8.0 * D3DX_PI * D3DX_PI * D3DX_PI * (n * n - 1.0) * (n * n - 1.0));
		betaRayleigh *= (float)((6 + 3 * pn) / (6 - 7 * pn));
		// test
		//betaRayleigh = new Vector3(6.95f * 10e-6f, 1.18f * 10e-5f, 2.44f * 10e-5f);

		m_f3SkyBetaRayleigh = betaRayleigh * rayleighMultiplier;
		pMaterialManager->SetVector3BySemantic(MakeKey(string("SKY_BETARAYLEIGH")), &m_f3SkyBetaRayleigh);

		m_f3BetaDashRayleigh = (3.0f * betaRayleigh / (float)(16.0 * D3DX_PI));
		m_f3SkyBetaDashRayleigh = m_f3BetaDashRayleigh * rayleighMultiplier;
		pMaterialManager->SetVector3BySemantic(MakeKey(string("SKY_BETADASHRAYLEIGH")), &m_f3SkyBetaDashRayleigh);


		//// Mie total factor ////

		// turbidity of air
		double T = 2.0;
		// concentration factor (dependent on tubidity)
		double c = (6.544 * T - 6.51) * 1e-17;
		//double c = (0.6544 * T - 0.6510) * 10e-16;
		// K factor (varies with lambda)
		double kRed = 0.685;
		double kGreen = 0.679;
		double kBlue = 0.67;

		Vector3 betaMie;
		betaMie.x = (float)((1.0 / (lambdaRed * lambdaRed)) * kRed);
		betaMie.y = (float)((1.0 / (lambdaGreen * lambdaGreen)) * kGreen);
		betaMie.z = (float)((1.0 / (lambdaBlue * lambdaBlue)) * kBlue);

		betaMie *= (float)(0.434 * c * D3DX_PI * ((4 * D3DX_PI * D3DX_PI)));

		// test
		//betaMie = new Vector3(8f * 10e-5f, 10e-4f, 1.2f * 10e-4f);

		m_f3SkyBetaMie = betaMie * mieMultiplier;
		pMaterialManager->SetVector3BySemantic(MakeKey(string("SKY_BETAMIE")), &m_f3SkyBetaMie);

		//Vector3 betaDashMie = ((float)(1.0 / (4.0 * D3DX_PI)) * betaMie);
		Vector3 betaDashMie = ((float)(1.0 / (4.0 * D3DX_PI)) * betaMie);
		betaDashMie.x = (float)(1.0 / (lambdaRed * lambdaRed));
		betaDashMie.y = (float)(1.0 / (lambdaGreen * lambdaGreen));
		betaDashMie.z = (float)(1.0 / (lambdaBlue * lambdaBlue));
		betaDashMie *= (float)(0.434 * c * (4 * D3DX_PI * D3DX_PI) * 0.5f);

		m_f3SkyBetaDashMie = betaDashMie * mieMultiplier;
		pMaterialManager->SetVector3BySemantic(MakeKey(string("SKY_BETADASHMIE")), &m_f3SkyBetaDashMie);


		Vector3 oneOverRayleighMie;
		oneOverRayleighMie.x = (1.0f / (betaRayleigh.x * rayleighMultiplier + betaMie.x * mieMultiplier));
		oneOverRayleighMie.y = (1.0f / (betaRayleigh.y * rayleighMultiplier + betaMie.y * mieMultiplier));
		oneOverRayleighMie.z = (1.0f / (betaRayleigh.z * rayleighMultiplier + betaMie.z * mieMultiplier));

		m_f3SkyOneOverRayleighMie = oneOverRayleighMie;
		pMaterialManager->SetVector3BySemantic(MakeKey(string("SKY_ONEOVERRAYLEIGHMIE")), &m_f3SkyOneOverRayleighMie);


		// optimisation for Henyey-Greenstein phase function (Mie scattering)
		float g = 0.95f;
		Vector3 hgData = Vector3((1.0f - g * g), (1.0f + g * g), (2.0f * g));

		m_f3SkyHgData = hgData;
		pMaterialManager->SetVector3BySemantic(MakeKey(string("SKY_HGDATA")), &m_f3SkyHgData);

		// Sun color * intensity
		m_f4SkySunColorIntensity = Vector4(1.0f ,1.0f, 1.0f, 1.0f);
		pMaterialManager->SetVector4BySemantic(MakeKey(string("SKY_SUNCOLORINTENSITY")), &m_f4SkySunColorIntensity);

		return bResult;
	}

	void Sky::UpdateSkyParameters()
	{
		DisplayPtr pDisplay = Display::GetInstance();
		DisplayMaterialManagerPtr pMaterialManager = pDisplay->GetMaterialManager();
		DisplayCameraPtr pCamera = pDisplay->GetCurrentCamera();

		m_fDayTime += m_rScene.GetApplication().GetDeltaTime();
		const float fMaxDayTime = 24.0f * 60.0f;
		while (m_fDayTime > fMaxDayTime)
		{
			m_fDayTime -= fMaxDayTime;
		}

		//static Vector3 f3Pos;
		//static float fPosY;
		//f3Pos = pCamera->GetPosition();
		//fPosY += m_fVerticalOffset;

		// sun position (starts at 0 h)
		const Vector4 f4LightDir = m_rScene.GetLightDir();
		m_f3SunPosition = Vector3(0.0f, -1.0f, 0.0f);
		m_f3SunPosition = Vector3(f4LightDir.x, f4LightDir.y, f4LightDir.z);

		float sunAngle = (float)(2 * D3DX_PI / 1440.0f * m_fDayTime);

		Matrix m3Temp;
		Vector4 f4Temp;
		D3DXVec3Transform(&f4Temp, &m_f3SunPosition, D3DXMatrixRotationZ(&m3Temp, sunAngle));
		m_f3SunPosition = Vector3(f4Temp.x, f4Temp.y, f4Temp.z);

		//shader.SetVariable("dayTime", m_fDayTime / 1440.0f); // optim
		pMaterialManager->SetVector3BySemantic(MakeKey(string("SKY_SUNPOSITION")), &m_f3SunPosition);

		// when sun visible we scale to fit between PI/2 and 3PI/4 (1.57 and 2.35)
		// in order to obtain a nice sunset/sunrise
		float bestAngleEast = 2.0f;
		float bestAngleWest = 1.7f;

		float zenithAngle = (float)D3DX_PI - sunAngle;
		if (zenithAngle > 0)
		{
			if ((float)fabs(zenithAngle) > bestAngleEast)
			{
				// sun hidden -> avoid white color (PI/2 is black)
				zenithAngle = (float)D3DX_PI / 2.0f;
			}
			else
			{
				// sun visible : scale to fit
				zenithAngle = zenithAngle * ((float)D3DX_PI / 2.0f) / bestAngleEast;
			}
		}
		else
		{
			if ((float)fabs(zenithAngle) > bestAngleWest)
			{
				// sun hidden -> avoid white color (PI/2 is black)
				zenithAngle = (float)D3DX_PI / 2.0f;
			}
			else
			{
				// sun visible : scale to fit
				zenithAngle = zenithAngle * ((float)D3DX_PI / 2.0f) / bestAngleWest;
			}
		}
		m_f4SkySunColorIntensity = GetSunColorWithIntensity(zenithAngle);
		pMaterialManager->SetVector4BySemantic(MakeKey(string("SKY_SUNCOLORINTENSITY")), &m_f4SkySunColorIntensity);

		// haze color
		m_fHazeHeight = 1.0f;
		m_fHazeIntensity = 1.0f;

		Vector4 f4HazeColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

		if (m_fDayTime >= 17.0f * 60.0f && m_fDayTime <= 19.0f * 60.0f)
		{
			//Vector4 SunSetColor = Vector4(0.96f, 0.27f, 0, 1);
			Vector4 SunSetColor = Vector4(0.99f, 0.619f, 0.176f, 1);

			if (m_fDayTime <= 18.0f * 60.0f)
			{
				float factor = m_fDayTime * 0.016666666f - 17.0f;
				f4HazeColor = Interpolate(f4HazeColor, SunSetColor, factor);
				m_fHazeHeight = 1.0f - factor*0.9f;
			}
			else
			{

				float factor = m_fDayTime * 0.016666666f - 18.0f;
				f4HazeColor = Interpolate(SunSetColor, f4HazeColor, factor);
				m_fHazeHeight = 0.1f + factor * 0.9f;
			}

		}
		m_f3HazeColor = Vector3(f4HazeColor.x, f4HazeColor.y, f4HazeColor.z);

		pMaterialManager->SetFloatBySemantic(MakeKey(string("SKY_HAZEHEIGHT")), &m_fHazeHeight);
		pMaterialManager->SetFloatBySemantic(MakeKey(string("SKY_HAZEINTENSITY")), &m_fHazeIntensity);
		pMaterialManager->SetVector3BySemantic(MakeKey(string("SKY_HAZECOLOR")), &m_f3HazeColor);

		// 		if (OverrideBrumeLightning)
		// 		{
		// 			if (brume.DirectionalLight == null)
		// 				brume.DirectionalLight = new BrumeDirectionalLight("SkyDomeLight", BrumeVector.XAXIS, BrumeVector.YAXIS, Color.White);
		// 
		// 			brume.DirectionalLight.Dir = sunPosition;
		// 
		// 			int ambientIntensity = 0;
		// 			if (m_fDayTime >= 17.0f * 60.0f && m_fDayTime <= 19.0f * 60.0f)
		// 			{
		// 				float factor = (m_fDayTime * 0.016666666f - 17.0f)*0.5f;
		// 				ambientIntensity = (int)(128 * (1.0f - factor));
		// 			}
		// 			else if (m_fDayTime >= 6.0f * 60.0f && m_fDayTime <= 8.0f * 60.0f)
		// 			{
		// 				float factor = (m_fDayTime * 0.016666666f - 6.0f) * 0.5f;
		// 				ambientIntensity = (int)(128 * factor);
		// 			}
		// 			else if (m_fDayTime > 8.0f * 60.0f && m_fDayTime < 17.0f * 60.0f)
		// 			{
		// 				// day
		// 				ambientIntensity = 128;
		// 			}
		// 			else
		// 			{
		// 				// night
		// 				ambientIntensity = 0;
		// 			}
		// 
		// 			brume.GlobalAmbientLight = Color.FromArgb(255, ambientIntensity, ambientIntensity, ambientIntensity);
		// 
		// 			brume.DirectionalLight.Diffuse = Color.FromArgb((int)(f4HazeColor.W * (127.0f + ambientIntensity)), (int)(f4HazeColor.X * (127.0f + ambientIntensity)), (int)(f4HazeColor.Y * (127.0f + ambientIntensity)), (int)(f4HazeColor.Z * (127.0f + ambientIntensity)));
		// 
		// 		}
	}

	Vector4 Sky::Interpolate(Vector4 src, Vector4 dst, float factor)
	{
		Vector4 res;
		res.x = src.x * (1.0f - factor) + dst.x * factor;
		res.y = src.y * (1.0f - factor) + dst.y * factor;
		res.z = src.z * (1.0f - factor) + dst.z * factor;
		res.w = src.w * (1.0f - factor) + dst.w * factor;
		return res;
	}

	Vector3 Sky::Interpolate(Vector3 src, Vector3 dst, float factor)
	{
		Vector3 res;
		res.x = src.x * (1.0f - factor) + dst.x * factor;
		res.y = src.y * (1.0f - factor) + dst.y * factor;
		res.z = src.z * (1.0f - factor) + dst.z * factor;
		return res;
	}

	Vector4 Sky::GetSunColorWithIntensity(float zenithAngle)
	{
		Vector4 color = GetSunColor(zenithAngle);
		color.w = GetSunIntensity(); 
		return color;
	}

	Vector4 Sky::GetSunColor(float zenithAngle)
	{
		// Note: Sun color changes with the sun position.
		return ComputeSunAttenuation(zenithAngle, 2);
	}

	Vector4 Sky::ComputeSunAttenuation(float fTheta, int nTurbidity/* = 2*/)
	// fTheta is in radians.
	// nTurbidity >= 2
	{
		float fBeta = 0.04608365822050f * nTurbidity - 0.04586025928522f;
		float fTauR, fTauA;
		float fTau[3];
		float m = 1.0f / (float)(cos(fTheta) + 0.15f * pow(93.885f - fTheta / D3DX_PI * 180.0f, -1.253f));  // Relative Optical Mass

		int i;
		float fLambda[3];
		fLambda[0] = 0.65f;	// red (in um.)
		fLambda[1] = 0.57f;	// green (in um.)
		fLambda[2] = 0.475f;	// blue (in um.)

		for (i = 0; i < 3; i++)
		{
			// Rayleigh Scattering
			// Results agree with the graph (pg 115, MI) */
			// lambda in um.
			fTauR = (float)exp(-m * 0.008735f * pow(fLambda[i], -4.08f));

			// Aerosal (water + dust) attenuation
			// beta - amount of aerosols present 
			// alpha - ratio of small to large particle sizes. (0:4,usually 1.3)
			// Results agree with the graph (pg 121, MI) 
			const float fAlpha = 1.3f;
			fTauA = (float)exp(-m * fBeta * pow(fLambda[i], -fAlpha));  // lambda should be in um


			fTau[i] = fTauR * fTauA;

		}

		Vector4 vAttenuation = Vector4(fTau[0], fTau[1], fTau[2], 1);
		return vAttenuation;
	}

	float Sky::GetSunIntensity()
	{
		return m_fIntensity;
	}
}
