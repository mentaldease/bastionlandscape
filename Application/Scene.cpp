#include "stdafx.h"
#include "../Application/Application.h"
#include "../Application/Scene.h"
#include "../Application/Sky.h"
#include "../Application/DebugTextOverlay.h"
#include "../Core/Scripting.h"
#include "../Core/Util.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	CreateClassFuncMap Scene::s_mClasses;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Scene::Scene(ApplicationRef _rApplication)
	:	CoreObject(),
		m_rApplication(_rApplication),
		m_mHierarchy(),
		m_mMaterials(),
		m_mAdditionalRTs(),
		m_mCameras(),
		m_mRenderPasses(),
		m_vRenderPasses(),
		m_f4LightDir(0.0f, 0.0f, 0.0f, 0.0f),
		m_pWaterData(NULL),
		m_uWaterDataCount(0),
		m_strName(),
		m_fWaterLevel(200.0f),
		m_uWaterLevelKey(MakeKey(string("WATERLEVEL"))),
		m_uWaterDataKey(MakeKey(string("WATERDATA"))),
		m_pUITextOverlay(NULL),
		m_uUIMainFontLabel(0),
		m_uUIRenderPass(0),
		m_fDayTime(0.0f),
		m_fVerticalOffset(0.0f)
	{

	}

	Scene::~Scene()
	{

	}

	bool Scene::RegisterClass(const Key& _uClassNameKey, CreateClassFunc _Func)
	{
		CreateClassFuncMap::iterator iPair = s_mClasses.find(_uClassNameKey);
		bool bResult (s_mClasses.end() == iPair);

		if (false != bResult)
		{
			s_mClasses[_uClassNameKey] = _Func;
		}

		return bResult;
	}

	bool Scene::UnregisterClass(const Key& _uClassNameKey)
	{
		CreateClassFuncMap::iterator iPair = s_mClasses.find(_uClassNameKey);
		bool bResult (s_mClasses.end() != iPair);

		if (false != bResult)
		{
			s_mClasses.erase(iPair);
		}

		return bResult;
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
			m_f4LightDir = Vector4(0.0f, -1.0f, 0.0f, 0.0f);
			D3DXVec4Normalize(&m_f4LightDir, &m_f4LightDir);
			pMaterialManager->SetVector4BySemantic(MakeKey(string("LIGHTDIR")), &m_f4LightDir);
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


		return bResult;
	}

	void Scene::Update()
	{
		{
			CoreObjectPtrMap::iterator iPair = m_mHierarchy.begin();
			CoreObjectPtrMap::iterator iEnd = m_mHierarchy.end();
			while (iEnd != iPair)
			{
				iPair->second->Update();
				++iPair;
			}
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
		// hierarchy
		while (m_mHierarchy.end() != m_mHierarchy.begin())
		{
			CoreObjectPtr pObject = m_mHierarchy.begin()->second;
			pObject->Release();
			delete pObject;
			m_mHierarchy.erase(m_mHierarchy.begin());
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

	ApplicationRef Scene::GetApplication()
	{
		return m_rApplication;
	}

	Vector4 Scene::GetLightDir()
	{
		return m_f4LightDir;
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
				&& CreateLoadHierarchy(oRoot)
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
			bResult = (false == oMaterialLibrary.IsNil());
			if (false == bResult)
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
			bResult = (false == oCameraLibrary.IsNil());
			if (false == bResult)
			{
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

	bool Scene::CreateLoadHierarchy(LuaObjectRef _rLuaObject)
	{
		LuaObject oHierarchy = _rLuaObject["hierarchy"];
		bool bResult = true;

		if (false == oHierarchy.IsNil())
		{
			const int sCount = oHierarchy.GetCount();
			for (int i = 0 ; sCount > i ; ++i)
			{
				LuaObject oObject = oHierarchy[i + 1];
				string strClass;
				bResult = Scripting::Lua::Get(oObject, "class", strClass, strClass);
				if (false == bResult)
				{
					break;
				}

				const Key uClassKey = MakeKey(strClass);
				CreateClassFuncMap::iterator iPair = s_mClasses.find(uClassKey);
				bResult = (s_mClasses.end() != iPair);
				if (false == bResult)
				{
					break;
				}

				string strName;
				bResult = Scripting::Lua::Get(oObject, "name", strName, strName);
				if (false == bResult)
				{
					break;
				}

				const Key uObjectNameKey = MakeKey(strName);
				bResult = (m_mHierarchy.end() == m_mHierarchy.find(uObjectNameKey));
				if (false == bResult)
				{
					break;
				}

				CreateClassFunc& pCreateClass = iPair->second;
				CoreObjectPtr pObject = pCreateClass(oObject, this);
				bResult = (NULL != pObject);
				if (false == bResult)
				{
					break;
				}

				m_mHierarchy[uObjectNameKey] = pObject;

				// hack....
				Scripting::Lua::Get(oObject, "daytime", m_fDayTime, m_fDayTime);
			}
		}

		return bResult;
	}

	bool Scene::RegisterClasses()
	{
		bool bResult = true;

		#define CHECK_RETURN(ReturnValue, Expression) if (false != ReturnValue) { ReturnValue = Expression; }
		CHECK_RETURN(bResult, RegisterClass(MakeKey(string("landscape")), boost::bind(&Scene::CreateClassLandscape, _1, _2)));
		CHECK_RETURN(bResult, RegisterClass(MakeKey(string("sphere")), boost::bind(&Scene::CreateClassShpere, _1, _2)));
		CHECK_RETURN(bResult, RegisterClass(MakeKey(string("sky")), boost::bind(&Scene::CreateClassSky, _1, _2)));
		#undef CHECK_RETURN

		return bResult;
	}

	void Scene::UnregisterClasses()
	{
		s_mClasses.clear();
	}

	CoreObjectPtr Scene::CreateClassLandscape(LuaObjectRef _rTable, ScenePtr _pScene)
	{
		DisplayPtr pDisplay = Display::GetInstance();
		LandscapePtr pLandscape = new Landscape(*pDisplay);
		Landscape::OpenInfo oLOInfo;
		string strTargetPass;

		oLOInfo.m_strName = _rTable["name"].GetString();
		oLOInfo.m_uGridSize = _rTable["grid_size"].GetInteger();
		oLOInfo.m_uQuadSize = _rTable["grid_chunk_size"].GetInteger();
		oLOInfo.m_fPixelErrorMax = _rTable["pixel_error_max"].GetFloat();
		oLOInfo.m_fFloorScale = _rTable["floor_scale"].GetFloat();
		oLOInfo.m_fHeightScale = _rTable["height_scale"].GetFloat();
		Scripting::Lua::Get(_rTable, "target_pass", strTargetPass, strTargetPass);
		oLOInfo.m_uRenderPassKey = MakeKey(strTargetPass);

		bool bResult = (false != pLandscape->Create(boost::any(0)));

		if (false != bResult)
		{
			string strFormat = _rTable["vertex_format"].GetString();
			oLOInfo.m_eFormat = Landscape::StringToVertexFormat(strFormat);
			bResult = (ELandscapeVertexFormat_UNKNOWN != oLOInfo.m_eFormat);
		}
		if (false != bResult)
		{
			oLOInfo.m_strHeightmap.clear();
			oLOInfo.m_strHeightmap = _rTable["heightmap"].GetString();
			oLOInfo.m_strLayersConfig.clear();
			oLOInfo.m_strLayersConfig = _rTable["layers_config"].GetString();
			bResult = pLandscape->Open(oLOInfo);
		}
		if (false != bResult)
		{
			string strMaterialName = _rTable["material"].GetString();
			Key uKey = MakeKey(strMaterialName);
			DisplayMaterialPtr pMaterial = pDisplay->GetMaterialManager()->GetMaterial(uKey);
			bResult = (NULL != pMaterial);
			pLandscape->SetMaterial(pMaterial);
		}
		if (false != bResult)
		{
			Vector3 oPos;
			oPos.x = _rTable["position"][1].GetFloat();
			oPos.y = _rTable["position"][2].GetFloat();
			oPos.z = _rTable["position"][3].GetFloat();
			D3DXMatrixTranslation(pLandscape->GetWorldMatrix(), oPos.x, oPos.y, oPos.z);
		}
		if (false == bResult)
		{
			pLandscape->Release();
			delete pLandscape;
			pLandscape = NULL;
		}

		return pLandscape;
	}

	CoreObjectPtr Scene::CreateClassShpere(LuaObjectRef _rTable, ScenePtr _pScene)
	{
		DisplayGeometrySpherePtr pSphere = new DisplayGeometrySphere();
		const float fSize = 100.f;
		DisplayGeometrySphere::CreateInfo oGSCInfo;
		Scripting::Lua::Get(_rTable, "bottom_hemisphere", true, oGSCInfo.m_bBottomHemisphere);
		Scripting::Lua::Get(_rTable, "top_hemisphere", true, oGSCInfo.m_bTopHemisphere);
		Scripting::Lua::Get(_rTable, "view_from_inside", false, oGSCInfo.m_bViewFromInside);
		Scripting::Lua::Get(_rTable, "position", Vector3(0.0f, 0.0f, 0.0f), oGSCInfo.m_oPos);
		Scripting::Lua::Get(_rTable, "rotation", Vector3(0.0f, 0.0f, 0.0f), oGSCInfo.m_oRot);
		Scripting::Lua::Get(_rTable, "size", Vector3(fSize, fSize, fSize), oGSCInfo.m_oRadius);
		Scripting::Lua::Get(_rTable, "horiz_slices", UInt(10), oGSCInfo.m_uHorizSlices);
		Scripting::Lua::Get(_rTable, "vert_slices", UInt(10), oGSCInfo.m_uVertSlices);
		Scripting::Lua::Get(_rTable, "color", Vector4(26.0f / 255.0f, 103.0f / 255.0f, 149.0f / 255.0f, 1.0f), oGSCInfo.m_f4Color);

		bool bResult = pSphere->Create(boost::any(&oGSCInfo));
		while (false != bResult)
		{
			DisplayPtr pDisplay = Display::GetInstance();
			string strMaterialName;
			Scripting::Lua::Get(_rTable, "material", strMaterialName, strMaterialName);
			const Key uMaterialKey = MakeKey(strMaterialName);
			DisplayMaterialPtr pMaterial = pDisplay->GetMaterialManager()->GetMaterial(uMaterialKey);
			bResult = (NULL != pMaterial);
			if (false == bResult)
			{
				break;
			}

			string strRenderPass;
			bResult = Scripting::Lua::Get(_rTable, "target_pass", strRenderPass, strRenderPass);
			if (false == bResult)
			{
				break;
			}

			pSphere->SetMaterial(pMaterial);
			pSphere->SetRenderPass(MakeKey(strRenderPass));
			break;
		}

		if (false == bResult)
		{
			pSphere->Release();
			delete pSphere;
			pSphere = NULL;
		}

		return pSphere;
	}

	CoreObjectPtr Scene::CreateClassSky(LuaObjectRef _rTable, ScenePtr _pScene)
	{
		SkyPtr pResult = new Sky(*_pScene);
		bool bResult = pResult->Create(boost::any(_rTable));

		if (false == bResult)
		{
			pResult->Release();
			delete pResult;
			pResult = NULL;
		}

		return pResult;
	}
}
