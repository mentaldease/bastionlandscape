#include "stdafx.h"
#include "../Application/Application.h"
#include "../Application/Scene.h"
#include "../Application/Sky.h"
#include "../Application/DebugTextOverlay.h"
#include "../Core/Scripting.h"
#include "../Core/Util.h"

namespace BastionGame
{
	bool Scene::CreateLoadRenderTargets(LuaObjectRef _rLuaObject)
	{
		PROFILING(__FUNCTION__);
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
					vsoutput(__FUNCTION__" : %s texture already exists\n", strName.c_str());
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
					vsoutput(__FUNCTION__" : could not create %s texture\n", strName.c_str());
					break;
				}

				pTexture = pTextureManager->Get(uNameKey);
				bResult = (NULL != pTexture);
				if (false == bResult)
				{
					vsoutput(__FUNCTION__" : could not access %s texture\n", strName.c_str());
					break;
				}
				m_mAdditionalRTs[uNameKey] = pTexture;
			}
		}

		return bResult;
	}

	bool Scene::CreateLoadMaterials(LuaObjectRef _rLuaObject)
	{
		PROFILING(__FUNCTION__);
		string strMaterialName;
		bool bResult = true;

		DisplayMaterialManagerPtr pMaterialManager = m_rApplication.GetDisplay()->GetMaterialManager();
		LuaObject oMaterialLibs = _rLuaObject["materials"];
		const int uCount = oMaterialLibs.GetCount();
		for (int i = 0 ; uCount > i ; ++i)
		{
			PROFILING(__FUNCTION__" [MATERIALLIB]");
			string strFileName = oMaterialLibs[i + 1].GetString();
			bool bResult = Scripting::Lua::Loadfile(strFileName);
			if (false == bResult)
			{
				vsoutput(__FUNCTION__" : could not load %s\n", strFileName.c_str());
				break;
			}
			string strMaterialLibrary;
			FS::GetFileNameWithoutExt(strFileName, strMaterialLibrary);
			strMaterialLibrary = strtolower(strMaterialLibrary);
			LuaObject oMaterialLibrary = _rLuaObject.GetState()->GetGlobal(strMaterialLibrary.c_str());
			bResult = (false == oMaterialLibrary.IsNil());
			if (false == bResult)
			{
				vsoutput(__FUNCTION__" : could not access %s lua table\n", strMaterialLibrary.c_str());
				bResult = false;
				break;
			}
			const int uMaterialCount = oMaterialLibrary.GetCount();
			for (int j = 0 ; uMaterialCount > j ; ++j)
			{
				PROFILING(__FUNCTION__" [MATERIAL]");
				LuaObject oMaterial = oMaterialLibrary[j + 1];
				strMaterialName = oMaterial["name"].GetString();
				Key uMaterialNameKey = MakeKey(strMaterialName);
				bResult = (NULL == pMaterialManager->GetMaterial(uMaterialNameKey))
					&& (false != pMaterialManager->CreateMaterial(uMaterialNameKey, oMaterial));
				if (false == bResult)
				{
					vsoutput(__FUNCTION__" : could not create %s material\n", strMaterialName.c_str());
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

	bool Scene::CreateLoadOctree(LuaObjectRef _rLuaObject)
	{
		PROFILING(__FUNCTION__);
		LuaObject oOctree = _rLuaObject["octree"];
		bool bResult = (false == oOctree.IsNil());

		if (false != bResult)
		{
			Octree::CreateInfo oOCInfo;
			Scripting::Lua::Get(oOctree, "leaf_size", 0.0f, oOCInfo.m_fLeafSize);
			Scripting::Lua::Get(oOctree, "depth", 1u, oOCInfo.m_uDepth);
			Scripting::Lua::Get(oOctree, "position", fsVector3(0.0f, 0.0f, 0.0f), oOCInfo.m_fs3Center);

			m_pOctree = new Octree();
			bResult = m_pOctree->Create(boost::any(&oOCInfo));
		}

		if (false != bResult)
		{
			m_pOTFFrustum = new OctreeTraverseFuncFrustum;
			bResult = m_pOTFFrustum->Create(boost::any(0));
			if (false != bResult)
			{
				bResult = m_pOctree->AddTraverseMode(m_uFrustumModeKey, boost::bind(&OctreeTraverseFuncFrustum::Do, m_pOTFFrustum, _1));
			}
		}

		return bResult;
	}

	bool Scene::CreateLoadWaterDataList(LuaObjectRef _rLuaObject)
	{
		PROFILING(__FUNCTION__);
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
						vsoutput(__FUNCTION__" : failed to create water data #%d\n", i);
						break;
					}
				}
			}
		}

		return bResult;
	}

	bool Scene::CreateLoadWaterData(LuaObjectRef _rLuaObject, WaterDataRef _rWaterData)
	{
		PROFILING(__FUNCTION__);
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
		PROFILING(__FUNCTION__);
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
				vsoutput(__FUNCTION__" : could not load %s\n", strFileName.c_str());
				break;
			}
			string strCameraLibrary;
			FS::GetFileNameWithoutExt(strFileName, strCameraLibrary);
			strCameraLibrary = strtolower(strCameraLibrary);
			LuaObject oCameraLibrary = _rLuaObject.GetState()->GetGlobal(strCameraLibrary.c_str());
			bResult = (false == oCameraLibrary.IsNil());
			if (false == bResult)
			{
				vsoutput(__FUNCTION__" : failed to access %s lua table\n", strCameraLibrary.c_str());
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
					vsoutput(__FUNCTION__" : failed to create %s camera\n", strCameraName.c_str());
					break;
				}
				m_mCameras[uCameraNameKey] = pDisplay->GetCamera(uCameraNameKey);
			}
		}

		return bResult;
	}

	bool Scene::CreateLoadRenderStages(LuaObjectRef _rLuaObject)
	{
		PROFILING(__FUNCTION__);
		LuaObject oRenderStages = _rLuaObject["render_stages"];
		bool bResult = true;

		if (false == oRenderStages.IsNil())
		{
			const int sCount = oRenderStages.GetCount();
			for (int i = 0 ; sCount > i ; ++i)
			{
				bResult = CreateLoadRenderStage(oRenderStages[i + 1]);
				if (false == bResult)
				{
					break;
				}
			}
		}

		return bResult;
	}

	bool Scene::CreateLoadRenderStage(LuaObjectRef _rLuaObject)
	{
		PROFILING(__FUNCTION__);
		DisplayRenderStagePtr pRenderStage = new DisplayRenderStage(*m_rApplication.GetDisplay());
		string strName;
		Scripting::Lua::Get(_rLuaObject, "name", string(""), strName);
		const Key uNameKey = MakeKey(strName);
		bool bResult = (false == strName.empty())
			&& (m_mRenderStages.end() == m_mRenderStages.find(uNameKey)) // <== check that there is NOT another render stage with the same name
			&& pRenderStage->Create(boost::any(&_rLuaObject));

		if (false != bResult)
		{
			m_mRenderStages[uNameKey] = pRenderStage;
			m_vRenderStages.push_back(pRenderStage);
		}
		else
		{
			vsoutput(__FUNCTION__" : failed to create %s render stage\n", strName.c_str());
			pRenderStage->Release();
			delete pRenderStage;
		}

		return bResult;
	}

	bool Scene::CreateLoadHierarchy(LuaObjectRef _rLuaObject)
	{
		PROFILING(__FUNCTION__);
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
					vsoutput(__FUNCTION__" : missing class for object #%d\n", i);
					break;
				}

				const Key uClassKey = MakeKey(strClass);
				CreateClassFuncMap::iterator iPair = s_mClasses.find(uClassKey);
				bResult = (s_mClasses.end() != iPair);
				if (false == bResult)
				{
					vsoutput(__FUNCTION__" : undeclared %s class for object #%d\n", strClass.c_str(), i);
					break;
				}

				string strName;
				bResult = Scripting::Lua::Get(oObject, "name", strName, strName);
				if (false == bResult)
				{
					vsoutput(__FUNCTION__" : missing name for object #%d\n", i);
					break;
				}

				const Key uObjectNameKey = MakeKey(strName);
				bResult = (m_mHierarchy.end() == m_mHierarchy.find(uObjectNameKey));
				if (false == bResult)
				{
					vsoutput(__FUNCTION__" : %s already exists for object #%d\n", strName.c_str(), i);
					break;
				}

				CreateClassFunc& pCreateClass = iPair->second;
				CoreObjectPtr pObject = pCreateClass(oObject, this);
				bResult = (NULL != pObject);
				if (false == bResult)
				{
					vsoutput(__FUNCTION__" : failed to create %s object #%d\n", strName.c_str(), i);
					break;
				}

				m_mHierarchy[uObjectNameKey] = pObject;

				// hack....
				Scripting::Lua::Get(oObject, "daytime", m_fDayTime, m_fDayTime);
			}
		}

		return bResult;
	}
}