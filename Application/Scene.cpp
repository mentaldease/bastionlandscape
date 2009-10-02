#include "stdafx.h"
#include "../Application/Application.h"
#include "../Application/Scene.h"

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
		m_strName()
	{

	}

	Scene::~Scene()
	{

	}

	bool Scene::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		Config::CreateInfo oCCInfo = { pInfo->m_strPath };
		Config oConfig;
		bool bResult = oConfig.Create(boost::any(&oCCInfo));

		if (false != bResult)
		{
			bResult = oConfig.GetValue(string("scene.name"), m_strName)
				&& CreateLoadMaterials(oConfig)
				&& CreateLoadLandscapes(oConfig)
				&& CreateLoadPostProcesses(oConfig);
		}

		oConfig.Release();

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
		if (false == m_vPostProcesses.empty())
		{
			m_rApplication.GetDisplay()->AddPostProcessesList(&m_vPostProcesses);
		}
	}

	void Scene::Release()
	{
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
		DisplayMaterialManagerPtr pMaterialManager = m_rApplication.GetDisplay()->GetMaterialManager();
		while (m_mMaterials.end() != m_mMaterials.begin())
		{
			pMaterialManager->UnloadMaterial(m_mMaterials.begin()->first);
			m_mMaterials.erase(m_mMaterials.begin());
		}
		//m_mMaterials.clear();

		// render post process
		while (m_mPostProcesses.end() != m_mPostProcesses.begin())
		{
			DisplayPostProcessPtr pPostProcess = m_mPostProcesses.begin()->second;
			pPostProcess->Release();
			delete pPostProcess;
			m_mPostProcesses.erase(m_mPostProcesses.begin());
		}
		m_vPostProcesses.clear();
	}

	bool Scene::CreateLoadMaterials(Config& _rConfig)
	{
		const string strName("name");
		const string strPath("path");
		const unsigned int uBufferSize = 1024;
		char szBuffer[uBufferSize];
		string strMaterialPath;
		string strMaterialName;
		bool bResult = true;

		DisplayMaterialManagerPtr pMaterialManager = m_rApplication.GetDisplay()->GetMaterialManager();
		const int uCount = _rConfig.GetCount("scene.materials");
		for (int i = 0 ; uCount > i ; ++i)
		{
			_snprintf(szBuffer, uBufferSize, "scene.materials.[%u]", i);
			ConfigShortcutPtr pShortcut = _rConfig.GetShortcut(string(szBuffer));
			if (NULL == pShortcut)
			{
				bResult = false;
				break;
			}
			bResult = _rConfig.GetValue(pShortcut, strPath, strMaterialPath);
			if (false == bResult)
			{
				break;
			}
			bResult = _rConfig.GetValue(pShortcut, strName, strMaterialName);
			if (false == bResult)
			{
				break;
			}
			bResult = pMaterialManager->LoadMaterial(strMaterialName, strMaterialPath);
			if (false == bResult)
			{
				break;
			}
			m_mMaterials[MakeKey(strMaterialName)] = pMaterialManager->GetMaterial(strMaterialName);
		}

		return bResult;
	}

	bool Scene::CreateLoadLandscapes(Config& _rConfig)
	{
		const unsigned int uBufferSize = 1024;
		char szBuffer[uBufferSize];
		const int uCount = _rConfig.GetCount("scene.landscapes");
		bool bResult = true;

		for (int i = 0 ; uCount > i ; ++i)
		{
			_snprintf(szBuffer, uBufferSize, "scene.landscapes.[%u]", i);
			ConfigShortcutPtr pShortcut = _rConfig.GetShortcut(string(szBuffer));
			if (NULL == pShortcut)
			{
				bResult = false;
				break;
			}
			bResult = CreateLoadLandscape(_rConfig, pShortcut);
			if (false == bResult)
			{
				break;
			}
		}

		return bResult;
	}

	bool Scene::CreateLoadLandscape(Config& _rConfig, ConfigShortcutPtr pShortcut)
	{
		LandscapePtr pLandscape = new Landscape(*m_rApplication.GetDisplay());
		Landscape::OpenInfo oLOInfo;
		bool bResult = (false != pLandscape->Create(boost::any(0)))
			&& _rConfig.GetValue(pShortcut, "name", oLOInfo.m_strName)
			&& (m_mLandscapes.end() == m_mLandscapes.find(MakeKey(oLOInfo.m_strName))) // <== check that there is NOT another landscape with the same name
			&& _rConfig.GetValue(pShortcut, "grid_size", oLOInfo.m_uGridSize)
			&& _rConfig.GetValue(pShortcut, "grid_chunk_size", oLOInfo.m_uQuadSize)
			&& _rConfig.GetValue(pShortcut, "pixel_error_max", oLOInfo.m_fPixelErrorMax)
			&& _rConfig.GetValue(pShortcut, "floor_scale", oLOInfo.m_fFloorScale)
			&& _rConfig.GetValue(pShortcut, "height_scale", oLOInfo.m_fHeightScale);

		if (false != bResult)
		{
			string strFormat = "";
			_rConfig.GetValue(pShortcut, "vertex_format", strFormat);
			oLOInfo.m_eFormat = Landscape::StringToVertexFormat(strFormat);
			bResult = (ELandscapeVertexFormat_UNKNOWN != oLOInfo.m_eFormat);
		}
		if (false != bResult)
		{
			oLOInfo.m_strHeightmap.clear();
			_rConfig.GetValue(pShortcut, "heightmap", oLOInfo.m_strHeightmap);
			oLOInfo.m_strLayersConfig.clear();
			_rConfig.GetValue(pShortcut, "layers_config", oLOInfo.m_strLayersConfig);
			bResult = pLandscape->Open(oLOInfo);
		}
		if (false != bResult)
		{
			string strMaterialName;
			_rConfig.GetValue(pShortcut, "material", strMaterialName);
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
			bResult = _rConfig.GetValue(pShortcut, "position_x", oPos.x)
				&& _rConfig.GetValue(pShortcut, "position_y", oPos.y)
				&& _rConfig.GetValue(pShortcut, "position_z", oPos.z);
			if (false != bResult)
			{
				D3DXMatrixTranslation(pLandscape->GetWorldMatrix(), oPos.x, oPos.y, oPos.z);
			}
		}

		if (false != bResult)
		{
			m_mLandscapes[MakeKey(oLOInfo.m_strName)] = pLandscape;
		}
		else
		{
			pLandscape->Release();
			delete pLandscape;
		}

		return bResult;
	}

	bool Scene::CreateLoadPostProcesses(Config& _rConfig)
	{
		const unsigned int uBufferSize = 1024;
		char szBuffer[uBufferSize];
		const int uCount = _rConfig.GetCount("scene.postprocesses");
		bool bResult = true;

		for (int i = 0 ; uCount > i ; ++i)
		{
			_snprintf(szBuffer, uBufferSize, "scene.postprocesses.[%u]", i);
			ConfigShortcutPtr pShortcut = _rConfig.GetShortcut(string(szBuffer));
			if (NULL == pShortcut)
			{
				bResult = false;
				break;
			}
			bResult = CreateLoadPostProcess(_rConfig, pShortcut);
			if (false == bResult)
			{
				break;
			}
		}

		return bResult;
	}

	bool Scene::CreateLoadPostProcess(Config& _rConfig, ConfigShortcutPtr pShortcut)
	{
		DisplayPostProcessPtr pPostProcess = new DisplayPostProcess(*m_rApplication.GetDisplay());
		DisplayPostProcess::CreateInfo oPPCInfo;
		string strMaterialName;
		bool bImmediateWrite = false;
		_rConfig.GetValue(pShortcut, "immediate_write", bImmediateWrite);
		oPPCInfo.m_bImmediateWrite = bImmediateWrite;
		bool bResult = _rConfig.GetValue(pShortcut, "name", oPPCInfo.m_strName)
			&& (m_mPostProcesses.end() == m_mPostProcesses.find(MakeKey(oPPCInfo.m_strName))) // <== check that there is NOT another post process with the same name
			&& _rConfig.GetValue(pShortcut, "material", strMaterialName)
			&& (oPPCInfo.m_uMaterialNameKey = MakeKey(strMaterialName))
			&& pPostProcess->Create(boost::any(&oPPCInfo));


		if (false != bResult)
		{
			m_mPostProcesses[MakeKey(oPPCInfo.m_strName)] = pPostProcess;
			m_vPostProcesses.push_back(pPostProcess);
		}
		else
		{
			pPostProcess->Release();
			delete pPostProcess;
		}

		return bResult;
	}
}
