#include "stdafx.h"
#include "../Application/Application.h"
#include "../Application/Scene.h"
#include "../Application/Sky.h"
#include "../Application/DebugTextOverlay.h"
#include "../Core/Scripting.h"

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
		m_mRenderStages(),
		m_vRenderStages(),
		m_f4LightDir(0.0f, 0.0f, 0.0f, 0.0f),
		m_pWaterData(NULL),
		m_pOctree(NULL),
		m_pOTFFrustum(NULL),
		m_uWaterDataCount(0),
		m_strName(),
		m_fWaterLevel(200.0f),
		m_uWaterLevelKey(MakeKey(string("WATERLEVEL"))),
		m_uWaterDataKey(MakeKey(string("WATERDATA"))),
		m_uFrustumModeKey(MakeKey(string("FRUSTUM"))),
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
			oDTOCInfo.m_uRenderStageKey = m_uUIRenderPass;
			oDTOCInfo.m_uMaxText = 50;
			m_pUITextOverlay = new DebugTextOverlay();
			bResult = m_pUITextOverlay->Create(boost::any(&oDTOCInfo));
		}

		if (false != bResult)
		{
			m_rApplication.GetDisplay()->AddRenderStages(m_vRenderStages);
		}


		return bResult;
	}

	void Scene::Update()
	{
		DisplayPtr pDisplay = Display::GetInstance();
		const Key uCurrentRenderStage = pDisplay->GetCurrentRenderStage()->GetNameKey();
		if (m_uUIRenderPass != uCurrentRenderStage)
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
				OctreeObjectPtrVec vObjects;
				OctreeNodePtrVec vNodes;
				m_pOctree->Traverse(m_uFrustumModeKey, vNodes, vObjects);
				//vsoutput(__FUNCTION__" : %u objects catched from octree\n", vObjects.size());

				OctreeObjectPtrVec::iterator iObject = vObjects.begin();
				OctreeObjectPtrVec::iterator iEnd = vObjects.end();
				while (iEnd != iObject)
				{
					DisplayObjectPtr pDisplayObject = dynamic_cast<DisplayObjectPtr>(*iObject);
					const Key uRenderStageName = pDisplayObject->GetRenderStage();
					if (uCurrentRenderStage == uRenderStageName)
					{
						pDisplay->RenderRequest(uRenderStageName, pDisplayObject);
					}
					++iObject;
				}
			}
		}
		else if (NULL != m_pUITextOverlay)
		{
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
			m_pUITextOverlay->Update();
		}
	}

	void Scene::Release()
	{
		DisplayMaterialManagerPtr pMaterialManager = m_rApplication.GetDisplay()->GetMaterialManager();

		m_rApplication.GetDisplay()->RemoveRenderStages(m_vRenderStages);

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

		// octree
		if (NULL != m_pOTFFrustum)
		{
			m_pOTFFrustum->Release();
			delete m_pOTFFrustum;
			m_pOTFFrustum = NULL;
		}
		if (NULL != m_pOctree)
		{
			m_pOctree->RemoveTraverseMode(m_uFrustumModeKey);
			m_pOctree->Release();
			delete m_pOctree;
			m_pOctree = NULL;
		}

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
		if (m_uUIRenderPass == pDisplay->GetCurrentRenderStage()->GetNameKey())
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

	OctreePtr Scene::GetOctree()
	{
		return m_pOctree;
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
				&& CreateLoadOctree(oRoot)
				&& CreateLoadHierarchy(oRoot)
				&& CreateLoadWaterDataList(oRoot)
				&& CreateLoadCameras(oRoot)
				&& CreateLoadRenderStages(oRoot);
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
		string strRenderStage;

		oLOInfo.m_strName = _rTable["name"].GetString();
		oLOInfo.m_uGridSize = _rTable["grid_size"].GetInteger();
		oLOInfo.m_uQuadSize = _rTable["grid_chunk_size"].GetInteger();
		oLOInfo.m_fPixelErrorMax = _rTable["pixel_error_max"].GetFloat();
		oLOInfo.m_fFloorScale = _rTable["floor_scale"].GetFloat();
		oLOInfo.m_fHeightScale = _rTable["height_scale"].GetFloat();
		Scripting::Lua::Get(_rTable, "target_stage", strRenderStage, strRenderStage);
		oLOInfo.m_uRenderStageKey = MakeKey(strRenderStage);

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
			oLOInfo.m_pOctree = _pScene->GetOctree();
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
			Vector3 oPos(0.0f, 0.0f, 0.0f);
			Scripting::Lua::Get(_rTable, "position", oPos, oPos);
			Matrix m4World;
			D3DXMatrixTranslation(&m4World, oPos.x, oPos.y, oPos.z);
			pLandscape->SetWorldMatrix(m4World);
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
		Scripting::Lua::Get(_rTable, "radius", Vector3(fSize, fSize, fSize), oGSCInfo.m_oRadius);
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

			string strRenderStage;
			bResult = Scripting::Lua::Get(_rTable, "target_stage", strRenderStage, strRenderStage);
			if (false == bResult)
			{
				break;
			}

			pSphere->SetMaterial(pMaterial);
			pSphere->SetRenderStage(MakeKey(strRenderStage));
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
