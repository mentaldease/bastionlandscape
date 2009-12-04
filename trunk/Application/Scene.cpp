#include "stdafx.h"
#include "../Application/Application.h"
#include "../Application/Scene.h"
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
		m_mPostProcesses(),
		m_vPostProcesses(),
		m_mNormalProcesses(),
		m_vNormalProcesses(),
		m_oLightDir(0.0f, 0.0f, 0.0f, 0.0f),
		m_pWaterData(NULL),
		m_uWaterDataCount(0),
		m_strName(),
		m_fWaterLevel(200.0f),
		m_uWaterLevelKey(MakeKey(string("WATERLEVEL"))),
		m_uWaterDataKey(MakeKey(string("WATERDATA")))
	{

	}

	Scene::~Scene()
	{

	}

	bool Scene::Create(const boost::any& _rConfig)
	{
		DisplayMaterialManagerPtr pMaterialManager = m_rApplication.GetDisplay()->GetMaterialManager();
		pMaterialManager->RegisterParamCreator(m_uWaterLevelKey, boost::bind(&DisplayEffectParamFLOAT::CreateParam, _1));
		pMaterialManager->RegisterParamCreator(m_uWaterDataKey, boost::bind(&DisplayEffectParamSTRUCT::CreateParam, _1));

		CreateInfoPtr pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
		bool bResult = CreateFromLuaConfig(pInfo);

		if (false != bResult)
		{
			pMaterialManager->SetFloatBySemantic(m_uWaterLevelKey, &m_fWaterLevel);
			pMaterialManager->SetStructBySemantic(m_uWaterDataKey, m_pWaterData, WATER_COUNT * sizeof(WaterData));
			m_oLightDir = Vector4(0.0f, -1.0f, 1.0f, 0.0f);
			D3DXVec4Normalize(&m_oLightDir, &m_oLightDir);
			pMaterialManager->SetVector4BySemantic(MakeKey(string("LIGHTDIR")), &m_oLightDir);
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
	}

	void Scene::Release()
	{
		DisplayMaterialManagerPtr pMaterialManager = m_rApplication.GetDisplay()->GetMaterialManager();

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

		// render post processes
		while (m_mPostProcesses.end() != m_mPostProcesses.begin())
		{
			DisplayPostProcessPtr pPostProcess = m_mPostProcesses.begin()->second;
			pPostProcess->Release();
			delete pPostProcess;
			m_mPostProcesses.erase(m_mPostProcesses.begin());
		}
		m_vPostProcesses.clear();

		// render normal processes
		while (m_mNormalProcesses.end() != m_mNormalProcesses.begin())
		{
			DisplayNormalProcessPtr pNormalProcess = m_mNormalProcesses.begin()->second;
			pNormalProcess->Release();
			delete pNormalProcess;
			m_mNormalProcesses.erase(m_mNormalProcesses.begin());
		}
		m_vNormalProcesses.clear();

		// specific param creators
		pMaterialManager->UnregisterParamCreator(m_uWaterDataKey);
		pMaterialManager->UnregisterParamCreator(m_uWaterLevelKey);
	}

	void Scene::PreUpdate()
	{
		if (false == m_vPostProcesses.empty())
		{
			m_rApplication.GetDisplay()->SetPostProcessesList(&m_vPostProcesses);
		}
		if (false == m_vNormalProcesses.empty())
		{
			m_rApplication.GetDisplay()->SetNormalProcessesList(&m_vNormalProcesses);
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
			bResult = CreateLoadMaterials(oRoot)
				&& CreateLoadLandscapes(oRoot)
				&& CreateLoadPostProcesses(oRoot)
				&& CreateLoadNormalProcesses(oRoot)
				&& CreateLoadWaterDataList(oRoot);
		}
		return bResult;
	}

	bool Scene::CreateLoadMaterials(LuaObjectRef _rLuaObject)
	{
		string strMaterialName;
		bool bResult = true;

		DisplayMaterialManagerPtr pMaterialManager = m_rApplication.GetDisplay()->GetMaterialManager();
		LuaObject oMaterialLibs = _rLuaObject["materiallibs"];
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

	bool Scene::CreateLoadPostProcesses(LuaObjectRef _rLuaObject)
	{
		LuaObject oPostProcesses = _rLuaObject["postprocesses"];
		bool bResult = true;

		if (false == oPostProcesses.IsNil())
		{
			const int sCount = oPostProcesses.GetCount();
			for (int i = 0 ; sCount > i ; ++i)
			{
				bResult = CreateLoadPostProcess(oPostProcesses[i + 1]);
				if (false == bResult)
				{
					break;
				}
			}
		}

		return bResult;
	}

	bool Scene::CreateLoadPostProcess(LuaObjectRef _rLuaObject)
	{
		DisplayPostProcessPtr pPostProcess = new DisplayPostProcess(*m_rApplication.GetDisplay());
		DisplayPostProcess::CreateInfo oPPCInfo;
		oPPCInfo.m_bImmediateWrite = _rLuaObject["immediate_write"].GetBoolean();
		oPPCInfo.m_strName = _rLuaObject["name"].GetString();
		const string strMaterialName = _rLuaObject["material"].GetString();
		const Key uMaterialNameKey = MakeKey(strMaterialName);
		const Key uNameKey = MakeKey(oPPCInfo.m_strName);
		bool bResult = (m_mPostProcesses.end() == m_mPostProcesses.find(uNameKey)) // <== check that there is NOT another post process with the same name
			&& (oPPCInfo.m_uMaterialNameKey = uMaterialNameKey)
			&& pPostProcess->Create(boost::any(&oPPCInfo));

		if (false != bResult)
		{
			m_mPostProcesses[uNameKey] = pPostProcess;
			m_vPostProcesses.push_back(pPostProcess);
		}
		else
		{
			pPostProcess->Release();
			delete pPostProcess;
		}

		return bResult;
	}

	bool Scene::CreateLoadNormalProcesses(LuaObjectRef _rLuaObject)
	{
		LuaObject oNormalProcesses = _rLuaObject["normalprocesses"];
		bool bResult = true;

		if (false == oNormalProcesses.IsNil())
		{
			const int sCount = oNormalProcesses.GetCount();
			for (int i = 0 ; sCount > i ; ++i)
			{
				bResult = CreateLoadNormalProcess(oNormalProcesses[i + 1]);
				if (false == bResult)
				{
					break;
				}
			}
		}

		return bResult;
	}

	bool Scene::CreateLoadNormalProcess(LuaObjectRef _rLuaObject)
	{
		DisplayNormalProcessPtr pNormalProcess = new DisplayNormalProcess(*m_rApplication.GetDisplay());
		DisplayNormalProcess::CreateInfo oNPCInfo = { &_rLuaObject };
		bool bResult = pNormalProcess->Create(boost::any(&oNPCInfo));


		if (false != bResult)
		{
			m_mNormalProcesses[pNormalProcess->GetNameKey()] = pNormalProcess;
			m_vNormalProcesses.push_back(pNormalProcess);
		}
		else
		{
			pNormalProcess->Release();
			delete pNormalProcess;
		}

		return bResult;
	}

	bool Scene::CreateLoadWaterDataList(LuaObjectRef _rLuaObject)
	{
		LuaObject oWater = _rLuaObject["water"];
		bool bResult = true;

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
}
