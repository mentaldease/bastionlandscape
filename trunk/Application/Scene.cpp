#include "stdafx.h"
#include "../Application/Application.h"
#include "../Application/Scene.h"
#include "../Application/DebugTextOverlay.h"
#include "../Core/Scripting.h"
#include "../Core/Util.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Scene::Scene(ApplicationRef _rApplication)
	:	CoreObject(),
		m_rApplication(_rApplication),
		m_mAllObjects(),
		m_mLandscapes(),
		m_mMaterials(),
		m_mAdditionalRTs(),
		m_mCameras(),
		m_mRenderPasses(),
		m_vRenderPasses(),
		m_oLightDir(0.0f, 0.0f, 0.0f, 0.0f),
		m_pWaterData(NULL),
		m_uWaterDataCount(0),
		m_strName(),
		m_fWaterLevel(200.0f),
		m_uWaterLevelKey(MakeKey(string("WATERLEVEL"))),
		m_uWaterDataKey(MakeKey(string("WATERDATA"))),
		m_pUITextOverlay(NULL),
		m_uUIMainFontLabel(0),
		m_uUIRenderPass(0),
		m_pSphere(NULL)
	{

	}

	Scene::~Scene()
	{

	}

	bool Scene::Create(const boost::any& _rConfig)
	{
		DisplayMaterialManagerPtr pMaterialManager = m_rApplication.GetDisplay()->GetMaterialManager();
		CreateInfoPtr pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
		bool bResult = (NULL != pInfo);

		if (false != bResult)
		{
			Release();
			pMaterialManager->RegisterParamCreator(m_uWaterLevelKey, boost::bind(&DisplayEffectParamFLOAT::CreateParam, _1));
			pMaterialManager->RegisterParamCreator(m_uWaterDataKey, boost::bind(&DisplayEffectParamSTRUCT::CreateParam, _1));
			bResult = CreateFromLuaConfig(pInfo);
		}

		if (false != bResult)
		{
			pMaterialManager->SetFloatBySemantic(m_uWaterLevelKey, &m_fWaterLevel);
			pMaterialManager->SetStructBySemantic(m_uWaterDataKey, m_pWaterData, WATER_COUNT * sizeof(WaterData));
			m_oLightDir = Vector4(0.0f, -1.0f, 1.0f, 0.0f);
			D3DXVec4Normalize(&m_oLightDir, &m_oLightDir);
			pMaterialManager->SetVector4BySemantic(MakeKey(string("LIGHTDIR")), &m_oLightDir);
		}

		if (false != bResult)
		{
			m_uUIMainFontLabel = MakeKey(string("Normal"));
			m_uUIRenderPass = MakeKey(string("ui"));

			DisplayPtr pDisplay = Display::GetInstance();
			DebugTextOverlay::CreateInfo oDTOCInfo;
			unsigned int uTemp[2];
			pDisplay->GetResolution(uTemp[0], uTemp[1]);
			oDTOCInfo.m_mFontNameList[m_uUIMainFontLabel] = "Data/Fonts/arial24.fnt";
			oDTOCInfo.m_mFontMaterialList[m_uUIMainFontLabel] = "ui";
			oDTOCInfo.m_f3ScreenOffset = Vector3(-float(uTemp[0] / 2), float(uTemp[1] / 2), 0.0f);
			oDTOCInfo.m_uRenderPassKey = m_uUIRenderPass;
			oDTOCInfo.m_uMaxText = 50;
			m_pUITextOverlay = new DebugTextOverlay();
			bResult = m_pUITextOverlay->Create(boost::any(&oDTOCInfo));
		}

		if (false != bResult)
		{
			m_rApplication.GetDisplay()->AddRenderPasses(m_vRenderPasses);
		}

		if (false != bResult)
		{
			m_pSphere = new DisplayGeometrySphere();
			DisplayGeometrySphere::CreateInfo oGSCInfo;
			oGSCInfo.m_bBottomHemisphere = true;
			oGSCInfo.m_bTopHemisphere = true;
			oGSCInfo.m_bViewFromInside = false;
			oGSCInfo.m_oPos = Vector3(0.0f, 0.0f, 0.0f);
			oGSCInfo.m_oRadius = Vector3(100.0f, 100.0f, 100.0f);
			oGSCInfo.m_oRot = Vector3(0.0f, 0.0f, 0.0f);
			oGSCInfo.m_uHorizSlices = 10;
			oGSCInfo.m_uVertSlices = 10;
			bResult = m_pSphere->Create(boost::any(&oGSCInfo));
			if (false != bResult)
			{
				DisplayPtr pDisplay = Display::GetInstance();
				const Key uNameKey = MakeKey(string("geomhelper"));
				DisplayMaterialPtr pMaterial = pDisplay->GetMaterialManager()->GetMaterial(uNameKey);
				if (NULL != pMaterial)
				{
					m_pSphere->SetMaterial(pMaterial);
				}
				else
				{
					m_pSphere->Release();
					delete m_pSphere;
					m_pSphere = NULL;
				}
			}
		}

		return bResult;
	}

	void Scene::Update()
	{
		LandscapePtrMap::iterator iPair = m_mLandscapes.begin();
		LandscapePtrMap::iterator iEnd = m_mLandscapes.end();
		while (iEnd != iPair)
		{
			iPair->second->Update();
			++iPair;
		}

		{
			const static Key uPassNameKey = MakeKey(string("scene"));
			m_rApplication.GetDisplay()->RenderRequest(uPassNameKey, m_pSphere);
		}

		{
			wstring wstrText = L"BASTION";
			Vector4 f4Color(1.0f, 1.0f, 1.0f, 1.0f);
			DrawOverlayText(0.0f, 0.0f, wstrText, f4Color);
		}
		{
			wstring wstrText = L"33S";
			Vector4 f4Color(1.0f, 1.0f, 0.0f, 1.0f);
			DrawOverlayText(0.0f, -30.0f, wstrText, f4Color);
		}

		if (NULL != m_pUITextOverlay)
		{
			m_pUITextOverlay->Update();
		}
	}

	void Scene::Release()
	{
		DisplayMaterialManagerPtr pMaterialManager = m_rApplication.GetDisplay()->GetMaterialManager();

		m_rApplication.GetDisplay()->RemoveRenderPasses(m_vRenderPasses);

		// sphere
		if (NULL != m_pSphere)
		{
			m_pSphere->Release();
			delete m_pSphere;
			m_pSphere = NULL;
		}

		// camera
		while (m_mCameras.end() != m_mCameras.begin())
		{
			m_rApplication.GetDisplay()->ReleaseCamera(m_mCameras.begin()->first);
			m_mCameras.erase(m_mCameras.begin());
		}

		// ui
		if (NULL != m_pUITextOverlay)
		{
			m_pUITextOverlay->Release();
			delete m_pUITextOverlay;
			m_pUITextOverlay = NULL;
		}

		// water rendering configuration data
		if (NULL != m_pWaterData)
		{
			delete[] m_pWaterData;
			m_pWaterData = NULL;
			m_uWaterDataCount = 0;
		}

		// landscapes
		while (m_mLandscapes.end() != m_mLandscapes.begin())
		{
			LandscapePtr pLandscape = m_mLandscapes.begin()->second;
			pLandscape->Release();
			delete pLandscape;
			m_mLandscapes.erase(m_mLandscapes.begin());
		}

		// landscape layer system
		LandscapeLayerManager::GetInstance()->UnloadAll();

		// materials
		while (m_mMaterials.end() != m_mMaterials.begin())
		{
			pMaterialManager->UnloadMaterial(m_mMaterials.begin()->first);
			m_mMaterials.erase(m_mMaterials.begin());
		}

		// specific param creators
		pMaterialManager->UnregisterParamCreator(m_uWaterDataKey);
		pMaterialManager->UnregisterParamCreator(m_uWaterLevelKey);

		// additional render targets
		DisplayTextureManagerPtr pTextureManager = m_rApplication.GetDisplay()->GetTextureManager();
		while (false == m_mAdditionalRTs.empty())
		{
			DisplayTexturePtrMap::iterator iPair = m_mAdditionalRTs.begin();
			pTextureManager->Unload(iPair->first);
			m_mAdditionalRTs.erase(iPair);
		}
	}

	void Scene::PreUpdate()
	{
	}

	void Scene::DrawOverlayText(const float _fX, const float _fY, const wstring& _wstrText, const Vector4& _f4Color)
	{
		DisplayPtr pDisplay = Display::GetInstance();
		if (m_uUIRenderPass == pDisplay->GetCurrentRenderPass()->GetNameKey())
		{
			if (NULL != m_pUITextOverlay)
			{
				const static Key uFontLabelKey = MakeKey(string("Normal"));
				m_pUITextOverlay->DrawRequest(_fX, _fY, uFontLabelKey, _wstrText, _f4Color);
			}
		}
	}

	bool Scene::CreateFromLuaConfig(Scene::CreateInfoPtr _pInfo)
	{
		bool bResult = Scripting::Lua::Loadfile(_pInfo->m_strPath);

		if (false != bResult)
		{
			string strRootName;
			FS::GetFileNameWithoutExt(_pInfo->m_strPath, strRootName);
			strRootName = strtolower(strRootName);
			LuaStatePtr pLuaState = Scripting::Lua::GetStateInstance();
			LuaObject oGlobals = pLuaState->GetGlobals();
			LuaObject oRoot = oGlobals[strRootName.c_str()];

			m_strName = oRoot["name"].GetString();
			bResult = CreateLoadRenderTargets(oRoot)
				&& CreateLoadMaterials(oRoot)
				&& CreateLoadLandscapes(oRoot)
				&& CreateLoadWaterDataList(oRoot)
				&& CreateLoadCameras(oRoot)
				&& CreateLoadRenderPasses(oRoot);
		}
		return bResult;
	}

	bool Scene::CreateLoadRenderTargets(LuaObjectRef _rLuaObject)
	{
		bool bResult = true;
		LuaObject oRenderTargets = _rLuaObject["render_targets"];

		LuaObject oAdditionals = oRenderTargets["additionals"];
		if (false == oAdditionals.IsNil())
		{
			const WindowData& rWindowData = m_rApplication.GetWindowData();
			DisplayTextureManagerPtr pTextureManager = m_rApplication.GetDisplay()->GetTextureManager();
			const int uCount = oAdditionals.GetCount();
			for (int i = 0 ; uCount > i ; ++i)
			{
				LuaObject oAdditional = oAdditionals[i + 1];

				const string strName = oAdditional["name"].GetString();
				const Key uNameKey = MakeKey(strName);
				DisplayTexturePtr pTexture = pTextureManager->Get(uNameKey);
				bResult = (NULL == pTexture);
				if (false == bResult)
				{
					break;
				}

				const string strFormat = oAdditional["format"].GetString();
				D3DFORMAT uFormat = Display::StringToDisplayFormat(strFormat, D3DFORMAT(rWindowData.m_uDXColorFormat));
				bResult = pTextureManager->New(strName,
					rWindowData.m_oClientRect.right,
					rWindowData.m_oClientRect.bottom,
					uFormat,
					false,
					DisplayTexture::EType_2D,
					DisplayTexture::EUsage_RENDERTARGET);
				if (false == bResult)
				{
					break;
				}

				pTexture = pTextureManager->Get(uNameKey);
				bResult = (NULL != pTexture);
				if (false == bResult)
				{
					break;
				}
				m_mAdditionalRTs[uNameKey] = pTexture;
			}
		}

		return bResult;
	}

	bool Scene::CreateLoadMaterials(LuaObjectRef _rLuaObject)
	{
		string strMaterialName;
		bool bResult = true;

		DisplayMaterialManagerPtr pMaterialManager = m_rApplication.GetDisplay()->GetMaterialManager();
		LuaObject oMaterialLibs = _rLuaObject["materials"];
		const int uCount = oMaterialLibs.GetCount();
		for (int i = 0 ; uCount > i ; ++i)
		{
			string strFileName = oMaterialLibs[i + 1].GetString();
			bool bResult = Scripting::Lua::Loadfile(strFileName);
			if (false == bResult)
			{
				break;
			}
			string strMaterialLibrary;
			FS::GetFileNameWithoutExt(strFileName, strMaterialLibrary);
			strMaterialLibrary = strtolower(strMaterialLibrary);
			LuaObject oMaterialLibrary = _rLuaObject.GetState()->GetGlobal(strMaterialLibrary.c_str());
			if (false != oMaterialLibrary.IsNil())
			{
				bResult = false;
				break;
			}
			const int uMaterialCount = oMaterialLibrary.GetCount();
			for (int j = 0 ; uMaterialCount > j ; ++j)
			{
				LuaObject oMaterial = oMaterialLibrary[j + 1];
				strMaterialName = oMaterial["name"].GetString();
				Key uMaterialNameKey = MakeKey(strMaterialName);
				bResult = (NULL == pMaterialManager->GetMaterial(uMaterialNameKey))
					&& (false != pMaterialManager->CreateMaterial(uMaterialNameKey, oMaterial));
				if (false == bResult)
				{
					break;
				}
				m_mMaterials[uMaterialNameKey] = pMaterialManager->GetMaterial(strMaterialName);
			}
			if (false == bResult)
			{
				break;
			}
		}

		return bResult;
	}

	bool Scene::CreateLoadLandscapes(LuaObjectRef _rLuaObject)
	{
		LuaObject oLandscapes = _rLuaObject["landscapes"];
		bool bResult = true;

		if (false == oLandscapes.IsNil())
		{
			const int sCount = oLandscapes.GetCount();
			for (int i = 0 ; sCount > i ; ++i)
			{
				bResult = CreateLoadLandscape(oLandscapes[i + 1]);
				if (false == bResult)
				{
					break;
				}
			}
		}

		return bResult;
	}

	bool Scene::CreateLoadLandscape(LuaObjectRef _rLuaObject)
	{
		LandscapePtr pLandscape = new Landscape(*m_rApplication.GetDisplay());
		Landscape::OpenInfo oLOInfo;

		oLOInfo.m_strName = _rLuaObject["name"].GetString();
		oLOInfo.m_uGridSize = _rLuaObject["grid_size"].GetInteger();
		oLOInfo.m_uQuadSize = _rLuaObject["grid_chunk_size"].GetInteger();
		oLOInfo.m_fPixelErrorMax = _rLuaObject["pixel_error_max"].GetFloat();
		oLOInfo.m_fFloorScale = _rLuaObject["floor_scale"].GetFloat();
		oLOInfo.m_fHeightScale = _rLuaObject["height_scale"].GetFloat();

		const Key uNameKey = MakeKey(oLOInfo.m_strName);

		bool bResult = (false != pLandscape->Create(boost::any(0)))
			&& (m_mLandscapes.end() == m_mLandscapes.find(uNameKey)); // <== check that there is NOT another landscape with the same name

		if (false != bResult)
		{
			string strFormat = _rLuaObject["vertex_format"].GetString();
			oLOInfo.m_eFormat = Landscape::StringToVertexFormat(strFormat);
			bResult = (ELandscapeVertexFormat_UNKNOWN != oLOInfo.m_eFormat);
		}
		if (false != bResult)
		{
			oLOInfo.m_strHeightmap.clear();
			oLOInfo.m_strHeightmap = _rLuaObject["heightmap"].GetString();
			oLOInfo.m_strLayersConfig.clear();
			oLOInfo.m_strLayersConfig = _rLuaObject["layers_config"].GetString();
			bResult = pLandscape->Open(oLOInfo);
		}
		if (false != bResult)
		{
			string strMaterialName = _rLuaObject["material"].GetString();
			Key uKey = MakeKey(strMaterialName);
			DisplayMaterialPtr pMaterial = m_mMaterials[uKey];
			if (NULL == pMaterial)
			{
				pMaterial = m_rApplication.GetDisplay()->GetMaterialManager()->GetMaterial(strMaterialName);
			}
			bResult = (NULL != pMaterial);
			pLandscape->SetMaterial(pMaterial);
		}

		if (false != bResult)
		{
			Vector3 oPos;
			oPos.x = _rLuaObject["position"][1].GetFloat();
			oPos.y = _rLuaObject["position"][2].GetFloat();
			oPos.z = _rLuaObject["position"][3].GetFloat();
			D3DXMatrixTranslation(pLandscape->GetWorldMatrix(), oPos.x, oPos.y, oPos.z);
			m_mLandscapes[uNameKey] = pLandscape;
		}
		else
		{
			pLandscape->Release();
			delete pLandscape;
		}

		return bResult;
	}

	bool Scene::CreateLoadWaterDataList(LuaObjectRef _rLuaObject)
	{
		string strWaterConfig;
		Scripting::Lua::Get(_rLuaObject, "water_config", string(""), strWaterConfig);
		bool bResult = (false == strWaterConfig.empty()) && (false != Scripting::Lua::Loadfile(strWaterConfig));

		if (false != bResult)
		{
			string strGlobalName;
			FS::GetFileNameWithoutExt(strWaterConfig, strGlobalName);
			strGlobalName = strtolower(strGlobalName);
			LuaObject oGlobals = Scripting::Lua::GetStateInstance()->GetGlobals();
			LuaObject oWater = oGlobals[strGlobalName.c_str()];

			if ((false == oWater.IsNil()) && (m_uWaterDataCount = oWater.GetCount()))
			{
				m_uWaterDataCount = (WATER_COUNT < m_uWaterDataCount) ? WATER_COUNT : m_uWaterDataCount;
				m_pWaterData = new WaterData[m_uWaterDataCount];
				for (UInt i = 0 ; m_uWaterDataCount > i ; ++i)
				{
					bResult = CreateLoadWaterData(oWater[i + 1], m_pWaterData[i]);
					if (false == bResult)
					{
						break;
					}
				}
			}
		}

		return bResult;
	}

	bool Scene::CreateLoadWaterData(LuaObjectRef _rLuaObject, WaterDataRef _rWaterData)
	{
		bool bResult = true;

		memset(&_rWaterData, 0, sizeof(WaterData));
		Scripting::Lua::Get(_rLuaObject, "WaterLevel", 0.0f, _rWaterData.m_fWaterLevel);
		Scripting::Lua::Get(_rLuaObject, "FadeSpeed", 0.15f, _rWaterData.m_fFadeSpeed);
		Scripting::Lua::Get(_rLuaObject, "NormalScale", 1.0f, _rWaterData.m_fNormalScale);
		Scripting::Lua::Get(_rLuaObject, "R0", 0.5f, _rWaterData.m_fR0);
		Scripting::Lua::Get(_rLuaObject, "MaxAmplitude", 1.0f, _rWaterData.m_fMaxAmplitude);
		Scripting::Lua::Get(_rLuaObject, "SunColor", Vector3(1.0f, 1.0f, 1.0f), _rWaterData.m_vSunColor);
		Scripting::Lua::Get(_rLuaObject, "ShoreHardness", 1.0f, _rWaterData.m_fShoreHardness);
		Scripting::Lua::Get(_rLuaObject, "RefractionStrength", 0.0f, _rWaterData.m_fRefractionStrength);
		Scripting::Lua::Get(_rLuaObject, "NormalModifier", Vector4(1.0f, 2.0f, 4.0f, 8.0f), _rWaterData.m_vNormalModifier);
		Scripting::Lua::Get(_rLuaObject, "Displace", 1.7f, _rWaterData.m_fDisplace);
		Scripting::Lua::Get(_rLuaObject, "FoamExistence", Vector3(0.65f, 1.35f, 0.5f), _rWaterData.m_vFoamExistence);
		Scripting::Lua::Get(_rLuaObject, "SunScale", 3.0f, _rWaterData.m_fSunScale);
		Scripting::Lua::Get(_rLuaObject, "Shininess", 0.7f, _rWaterData.m_fShininess);
		Scripting::Lua::Get(_rLuaObject, "SpecularIntensity", 0.32f, _rWaterData.m_fSpecularIntensity);
		Scripting::Lua::Get(_rLuaObject, "DepthColour", Vector3(0.0078f, 0.5176f, 0.7f), _rWaterData.m_vDepthColour);
		Scripting::Lua::Get(_rLuaObject, "BigDepthColour", Vector3(0.0039f, 0.00196f, 0.145f), _rWaterData.m_vBigDepthColour);
		Scripting::Lua::Get(_rLuaObject, "Extinction", Vector3(7.0f, 30.0f, 40.0f), _rWaterData.m_vExtinction);
		Scripting::Lua::Get(_rLuaObject, "Visibility", 4.0f, _rWaterData.m_fVisibility);
		Scripting::Lua::Get(_rLuaObject, "Scale", 0.005f, _rWaterData.m_fScale);
		Scripting::Lua::Get(_rLuaObject, "RefractionScale", 0.005f, _rWaterData.m_fRefractionScale);
		Scripting::Lua::Get(_rLuaObject, "Wind", Vector2(-0.3f, 0.7f), _rWaterData.m_vWind);
		Scripting::Lua::Get(_rLuaObject, "Forward", Vector3(0.0f, 0.0f, 0.0f), _rWaterData.m_vForward);
		Scripting::Lua::Get(_rLuaObject, "AtlasInfo", Vector4(0.0f, 0.0f, 0.0f, 0.0f), _rWaterData.m_vAtlasInfo);

		return bResult;
	}

	bool Scene::CreateLoadCameras(LuaObjectRef _rLuaObject)
	{
		DisplayPtr pDisplay = m_rApplication.GetDisplay();
		string strCameraName;
		bool bResult = true;

		LuaObject oCameraLibs = _rLuaObject["cameras"];
		const int uCount = oCameraLibs.GetCount();
		for (int i = 0 ; uCount > i ; ++i)
		{
			string strFileName = oCameraLibs[i + 1].GetString();
			bool bResult = Scripting::Lua::Loadfile(strFileName);
			if (false == bResult)
			{
				break;
			}
			string strCameraLibrary;
			FS::GetFileNameWithoutExt(strFileName, strCameraLibrary);
			strCameraLibrary = strtolower(strCameraLibrary);
			LuaObject oCameraLibrary = _rLuaObject.GetState()->GetGlobal(strCameraLibrary.c_str());
			if (false != oCameraLibrary.IsNil())
			{
				bResult = false;
				break;
			}
			const int uCameraCount = oCameraLibrary.GetCount();
			for (int j = 0 ; uCameraCount > j ; ++j)
			{
				LuaObject oCamera = oCameraLibrary[j + 1];
				strCameraName = oCamera["name"].GetString();
				Key uCameraNameKey = MakeKey(strCameraName);
				bResult = (NULL == pDisplay->GetCamera(uCameraNameKey))
					&& (false != pDisplay->CreateCamera(uCameraNameKey, oCamera));
				if (false == bResult)
				{
					break;
				}
				m_mCameras[uCameraNameKey] = pDisplay->GetCamera(uCameraNameKey);
			}
		}

		return bResult;
	}

	bool Scene::CreateLoadRenderPasses(LuaObjectRef _rLuaObject)
	{
		LuaObject oRenderPasses = _rLuaObject["renderpasses"];
		bool bResult = true;

		if (false == oRenderPasses.IsNil())
		{
			const int sCount = oRenderPasses.GetCount();
			for (int i = 0 ; sCount > i ; ++i)
			{
				bResult = CreateLoadRenderPass(oRenderPasses[i + 1]);
				if (false == bResult)
				{
					break;
				}
			}
		}

		return bResult;
	}

	bool Scene::CreateLoadRenderPass(LuaObjectRef _rLuaObject)
	{
		DisplayRenderPassPtr pRenderPass = new DisplayRenderPass(*m_rApplication.GetDisplay());
		string strName;
		Scripting::Lua::Get(_rLuaObject, "name", string(""), strName);
		const Key uNameKey = MakeKey(strName);
		bool bResult = (false == strName.empty())
			&& (m_mRenderPasses.end() == m_mRenderPasses.find(uNameKey)) // <== check that there is NOT another render pass with the same name
			&& pRenderPass->Create(boost::any(&_rLuaObject));

		if (false != bResult)
		{
			m_mRenderPasses[uNameKey] = pRenderPass;
			m_vRenderPasses.push_back(pRenderPass);
		}
		else
		{
			pRenderPass->Release();
			delete pRenderPass;
		}

		return bResult;
	}
}
