#include "stdafx.h"
#include "../Application/Application.h"
#include "../Application/Scene.h"
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
		m_vLandscapes(),
		m_f4LightDir(0.0f, 0.0f, 0.0f, 0.0f),
		m_pWaterData(NULL),
		m_pOctree(NULL),
		m_pOTFFrustum(NULL),
		m_pDisplay(NULL),
		m_uWaterDataCount(0),
		m_strName(),
		m_fWaterLevel(200.0f),
		m_uWaterLevelKey(MakeKey(string("WATERLEVEL"))),
		m_uWaterDataKey(MakeKey(string("WATERDATA"))),
		m_uFrustumModeKey(MakeKey(string("FRUSTUM"))),
		m_pUITextOverlay(NULL),
		m_uUIMainFontLabel(MakeKey(string("Normal"))),
		m_uUIRenderStage(MakeKey(string("ui"))),
		m_fDayTime(0.0f),
		m_fVerticalOffset(0.0f),
		m_oPicker()
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
		PROFILING(__FUNCTION__);
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
			m_pDisplay = Display::GetInstance();
			DebugTextOverlay::CreateInfo oDTOCInfo;
			unsigned int uTemp[2];
			m_pDisplay->GetResolution(uTemp[0], uTemp[1]);
			oDTOCInfo.m_mFontNameList[m_uUIMainFontLabel] = "Data/Fonts/arial24.fnt";
			oDTOCInfo.m_mFontMaterialList[m_uUIMainFontLabel] = "ui";
			oDTOCInfo.m_f3ScreenOffset = Vector3(-float(uTemp[0] / 2), float(uTemp[1] / 2), 0.0f);
			oDTOCInfo.m_uRenderStageKey = m_uUIRenderStage;
			oDTOCInfo.m_uMaxText = 50;
			m_pUITextOverlay = new DebugTextOverlay();
			bResult = m_pUITextOverlay->Create(boost::any(&oDTOCInfo));
		}

		if (false != bResult)
		{
			ScenePicker::CreateInfo oSPCInfo = { this };
			bResult = m_oPicker.Create(boost::any(&oSPCInfo));
		}

		if (false != bResult)
		{
			m_rApplication.GetDisplay()->AddRenderStages(m_vRenderStages);
		}


		return bResult;
	}

	void Scene::Update()
	{
		PROFILING(__FUNCTION__);

		const Key uCurrentRenderStage = m_pDisplay->GetCurrentRenderStage()->GetNameKey();
		if (m_uUIRenderStage != uCurrentRenderStage)
		{
			{
				PROFILING(__FUNCTION__" [HIERARCHY]");
				CoreObjectPtrMap::iterator iPair = m_mHierarchy.begin();
				CoreObjectPtrMap::iterator iEnd = m_mHierarchy.end();
				while (iEnd != iPair)
				{
					iPair->second->Update();
					++iPair;
				}
			}

			{
				OctreeObjectPtrVec vOctreeObjects;
				OctreeNodePtrVec vNodes;
				{
					PROFILING(__FUNCTION__" [TRAVERSE]");
					m_pOctree->Traverse(m_uFrustumModeKey, vNodes, vOctreeObjects);
					//vsoutput(__FUNCTION__" : %u objects catched from octree\n", vOctreeObjects.size());
				}

				m_oPicker.Update(vOctreeObjects);

				{
					PROFILING(__FUNCTION__" [REQUESTS]");
					OctreeObjectPtrVec::iterator iObject = vOctreeObjects.begin();
					OctreeObjectPtrVec::iterator iEnd = vOctreeObjects.end();
					while (iEnd != iObject)
					{
						DisplayObjectPtr pDisplayObject = dynamic_cast<DisplayObjectPtr>(*iObject);
						if (NULL != pDisplayObject)
						{
							const Key uRenderStageKey = pDisplayObject->GetRenderStage();
							if (uCurrentRenderStage == uRenderStageKey)
							{
								m_pDisplay->RenderRequest(uRenderStageKey, pDisplayObject);
								const CoreObjectPtrVec& rvChildren = pDisplayObject->GetChildren();
								if (false == rvChildren.empty())
								{
									CoreObjectPtrVec::const_iterator iChild = rvChildren.begin();
									CoreObjectPtrVec::const_iterator iChildEnd = rvChildren.end();
									while (iChildEnd != iChild)
									{
										DisplayObjectPtr pDisplayChild = dynamic_cast<DisplayObjectPtr>(*iChild);
										if (NULL != pDisplayChild)
										{
											const Key uChildRenderStageKey = pDisplayChild->GetRenderStage();
											if (uCurrentRenderStage == uChildRenderStageKey)
											{
												m_pDisplay->RenderRequest(uChildRenderStageKey, pDisplayChild);
											}
										}
										++iChild;
									}
								}
							}
						}
						++iObject;
					}
				}

				UpdateDebug(vOctreeObjects, vNodes);
			}
		}
		else
		{
			// project name
			{
				wstring wstrText = L"BASTION";
				Vector4 f4Color(1.0f, 1.0f, 1.0f, 1.0f);
				DrawOverlayText(0.0f, 0.0f, wstrText, f4Color);
			}
			// dev team
			{
				wstring wstrText = L"Sapporo Super Sampling";
				Vector4 f4Color(1.0f, 1.0f, 0.0f, 1.0f);
				DrawOverlayText(0.0f, -30.0f, wstrText, f4Color);
			}
			// mouse pos
			//{
			//	wchar_t wszBuffer[1024];
			//	Vector4 f4Color(0.0f, 1.0f, 1.0f, 1.0f);				
			//	int sLength = wsprintf(wszBuffer, L"%d %d", int(m_rApplication.GetMousePos().x), int(m_rApplication.GetMousePos().y));
			//	wstring wstrText(wszBuffer);
			//	DrawOverlayText(0.0f, -60.0f, wstrText, f4Color);
			//}
			m_pUITextOverlay->Update();
		}
	}

	void Scene::Release()
	{
		DisplayMaterialManagerPtr pMaterialManager = m_rApplication.GetDisplay()->GetMaterialManager();

		// dummies
		CoreObjectPtrVec::iterator iObject = m_vDummies.begin();
		CoreObjectPtrVec::iterator iEnd = m_vDummies.end();
		while (iEnd != iObject)
		{
			CoreObjectPtr pObject = *iObject;
			pObject->Release();
			delete pObject;
			++iObject;
		}
		m_vDummies.clear();

		// render stages
		m_rApplication.GetDisplay()->RemoveRenderStages(m_vRenderStages);
		while (false == m_vRenderStages.empty())
		{
			DisplayRenderStagePtr pRS = *m_vRenderStages.begin();
			pRS->Release();
			delete pRS;
			m_vRenderStages.erase(m_vRenderStages.begin());
		}

		// Picker
		m_oPicker.Release();

		// camera
		while (false == m_mCameras.empty())
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
		while (false == m_mHierarchy.empty())
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

		m_vLandscapes.clear();
	}

	void Scene::PreUpdate()
	{
	}

	void Scene::DrawOverlayText(const float _fX, const float _fY, const wstring& _wstrText, const Vector4& _f4Color)
	{
		if (m_uUIRenderStage == m_pDisplay->GetCurrentRenderStage()->GetNameKey())
		{
			const static Key uFontLabelKey = MakeKey(string("Normal"));
			m_pUITextOverlay->DrawRequest(_fX, _fY, uFontLabelKey, _wstrText, _f4Color);
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

	CoreObjectPtr Scene::GetHierarchyObject(const Key _uNameKey)
	{
		CoreObjectPtr pResult = NULL;
		CoreObjectPtrMap::iterator iObject = m_mHierarchy.find(_uNameKey);
		if (m_mHierarchy.end() != iObject)
		{
			pResult = iObject->second;
		}
		return pResult;
	}

	void Scene::ActivatePicking(const bool _bState)
	{
		m_oPicker.Activate(_bState);
	}

	void Scene::NewDummy()
	{
		DisplayObjectDummyPtr pDummy = new DisplayObjectDummy;
		bool bResult = pDummy->Create(boost::any(0));
		if (false != bResult)
		{
			DisplayObjectPtr pDummyDisplay = reinterpret_cast<DisplayObjectPtr>(m_mHierarchy[MakeKey(string("dummy"))]);
			bResult = (NULL != pDummyDisplay);
			if (false != bResult)
			{
				pDummy->SetWorldMatrix(*m_oPicker.GetWorldMatrix());
				pDummy->SetObject(pDummyDisplay);
				pDummy->SetRenderStage(pDummyDisplay->GetRenderStage());
				pDummy->SetMaterial(pDummyDisplay->GetMaterial());
				AddObject(pDummy);
				m_vDummies.push_back(pDummy);
			}

		}
		if (false == bResult)
		{
			pDummy->Release();
			delete pDummy;
		}
	}

	void Scene::AddLandscape(LandscapePtr _pLandscape)
	{
		if (m_vLandscapes.end() == find(m_vLandscapes.begin(), m_vLandscapes.end(), _pLandscape))
		{
			m_vLandscapes.push_back(_pLandscape);
		}
	}

	WaterDataPtr Scene::GetWaterData(UIntRef _uCount)
	{
		_uCount = m_uWaterDataCount;
		return m_pWaterData;
	}

	bool Scene::GetWaterLevel(const Vector3& _f3Pos, FloatRef _fLevel)
	{
		bool bResult = false;
		LandscapePtrVec::iterator iLandscape = m_vLandscapes.begin();
		LandscapePtrVec::iterator iEnd = m_vLandscapes.end();
		while (iEnd != iLandscape)
		{
			UInt uIndex;
			bResult = (*iLandscape)->GetWaterIndex(_f3Pos, uIndex);
			if (false != bResult)
			{
				_fLevel = m_pWaterData[uIndex].m_fWaterLevel;
				break;
			}
			++iLandscape;
		}
		return bResult;
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

	void Scene::AddObject(DisplayObjectPtr _pObject)
	{
		LandscapePtrVec::iterator iLandscape = m_vLandscapes.begin();
		LandscapePtrVec::iterator iEnd = m_vLandscapes.end();
		while (iEnd != iLandscape)
		{
			(*iLandscape)->UpdateObjectLocation(_pObject);
			++iLandscape;
		}
	}

	void Scene::UpdateDebug(OctreeObjectPtrVec& _rvOctreeObjects, OctreeNodePtrVec& _rvNodes)
	{
		//DisplayGeometryLineManagerPtr pLineManager = reinterpret_cast<DisplayGeometryLineManagerPtr>(m_mHierarchy[MakeKey(string("debuglines"))]);
		DisplayGeometryLineManagerPtr pLineManager = dynamic_cast<DisplayGeometryLineManagerPtr>(m_mHierarchy[MakeKey(string("debuglines"))]);
		if ((NULL != pLineManager) && (true))
		{
			PROFILING(__FUNCTION__);
			// picked object
			if (true)
			{
				DisplayObjectPtr pObject = dynamic_cast<DisplayObjectPtr>(m_oPicker.GetPickedObject());
				if (NULL != pObject)
				{
					const Vector4 f4Color(0.25f, 0.125f, 0.75f, 1.0f);
					pLineManager->NewBoundingMesh(pObject->GetBoundingMesh(), f4Color);
				}
			}
			// octree nodes
			if (false)
			{
				const Vector4 f4Color(0.25f, 0.125f, 0.75f, 1.0f);
				OctreeNodePtrVec::iterator iNode = _rvNodes.begin();
				OctreeNodePtrVec::iterator iEnd = _rvNodes.end();
				while (iEnd != iNode)
				{
					OctreeNodePtr pNode = *iNode;
					const fsVector3Vec& rvAABB = pNode->GetAABB();
					#define fsVec3ToVec3(vFSVEC3) Vector3((vFSVEC3).x(), (vFSVEC3).y(), (vFSVEC3).z())
					pLineManager->NewAABB(
						fsVec3ToVec3(rvAABB[EOctreeAABB_TOPRIGHTTFAR]),
						fsVec3ToVec3(rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR]),
						f4Color);
					#undef fsVec3ToVec3
					++iNode;
				}
			}
			// octree objects
			if (false)
			{
				const Vector4 f4Color(0.75f, 0.125f, 0.125f, 1.0f);
				OctreeObjectPtrVec::iterator iObject = _rvOctreeObjects.begin();
				OctreeObjectPtrVec::iterator iEnd = _rvOctreeObjects.end();
				while (iEnd != iObject)
				{
					OctreeObjectPtr pObject = *iObject;
					const fsVector3Vec& rvAABB = pObject->GetAABB();
					#define fsVec3ToVec3(vFSVEC3) Vector3((vFSVEC3).x(), (vFSVEC3).y(), (vFSVEC3).z())
					pLineManager->NewAABB(
						fsVec3ToVec3(rvAABB[EOctreeAABB_TOPRIGHTTFAR]),
						fsVec3ToVec3(rvAABB[EOctreeAABB_BOTTOMLEFTTNEAR]),
						f4Color);
					#undef fsVec3ToVec3
					++iObject;
				}
			}
		}
	}
}
