#include "stdafx.h"
#include "Landscape.h"
#include "../Display/Display.h"
#include "../Display/Texture.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	LandscapeLayerManagerPtr LandscapeLayerManager::s_pInstance = NULL;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	bool LandscapeLayering::Layer::Evaluate(const float& _fHeight, const float& _fSlope)
	{
		bool bResult = false;
		return bResult;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	LandscapeLayering::LandscapeLayering(LandscapeLayerManagerRef _rLandscapeLayerManager)
	:	CoreObject(),
		m_vLayers(),
		m_rLandscapeLayerManager(_rLandscapeLayerManager),
		m_pAtlas(NULL),
		m_pSlopeAndHeightLUT(NULL),
		m_oShaderInfo(0.0f, 0.0f, 0.0f, 0.0f),
		m_strAtlasName(),
		m_strSAHLUTName()
	{

	}

	LandscapeLayering::~LandscapeLayering()
	{

	}

	bool LandscapeLayering::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		Config::CreateInfo oCCInfo = { pInfo->m_strPath };
		Config oConfig;
		bool bResult = oConfig.Create(boost::any(&oCCInfo));
		int sAGS = 0;

		if (false != bResult)
		{
			bResult = oConfig.GetValue("config.atlas_filename", m_strAtlasName)
				&& oConfig.GetValue("config.atlas_grid_size", sAGS);
		}

		if (false != bResult)
		{
			m_oShaderInfo.w = float(sAGS);
			DisplayTextureManagerPtr pTextureManager = LandscapeLayerManager::GetInstance()->GetDisplay().GetTextureManager();
			bResult = pTextureManager->Load(m_strAtlasName, m_strAtlasName, DisplayTexture::EType_2D);
			m_pAtlas = (false != bResult) ? pTextureManager->Get(m_strAtlasName) : NULL;
			bResult = (NULL != m_pAtlas);
		}

		if (false != bResult)
		{
			DisplayTextureManagerPtr pTextureManager = LandscapeLayerManager::GetInstance()->GetDisplay().GetTextureManager();
			m_strSAHLUTName = pInfo->m_strPath;
			bResult = pTextureManager->New(m_strSAHLUTName, 0x00000001 << 8, 0x00000001 << 8, D3DFMT_A8R8G8B8, false, DisplayTexture::EType_2D);
			m_pSlopeAndHeightLUT = (false != bResult) ? pTextureManager->Get(m_strSAHLUTName) : NULL;
			bResult = (NULL != m_pSlopeAndHeightLUT) && CreateSlopeAndHeightLUT(oConfig);
		}

		return bResult;
	}

	void LandscapeLayering::Update()
	{

	}

	void LandscapeLayering::Release()
	{
		DisplayTextureManagerPtr pTextureManager = LandscapeLayerManager::GetInstance()->GetDisplay().GetTextureManager();
		if (NULL != m_pSlopeAndHeightLUT)
		{
			pTextureManager->Unload(m_strSAHLUTName);
			m_pSlopeAndHeightLUT = NULL;
		}
		if (NULL != m_pAtlas)
		{
			pTextureManager->Unload(m_strAtlasName);
			m_pAtlas = NULL;
		}
		m_vLayers.clear();
	}

	DisplayTexturePtr LandscapeLayering::GetAtlas()
	{
		return m_pAtlas;
	}

	DisplayTexturePtr LandscapeLayering::GetSlopeAndHeightLUT()
	{
		return m_pSlopeAndHeightLUT;
	}

	Vector4& LandscapeLayering::GetShaderInfo()
	{
		return m_oShaderInfo;
	}

	bool LandscapeLayering::CreateSlopeAndHeightLUT(ConfigRef _rConfig)
	{
		const unsigned int uBufferSize = 1024;
		char szBuffer[uBufferSize];
		const int sCount = _rConfig.GetCount("config.layers");
		bool bResult = (0 < sCount);

		if (false != bResult)
		{
			const string strAtlasIndex = "atlas_index";
			const string strMinHeight = "min_height";
			const string strMaxHeight = "max_height";
			const string strMinSlope = "min_slope";
			const string strMaxSlope = "max_slope";
			for (int i = 0 ; sCount > i ; ++i)
			{
				_snprintf(szBuffer, uBufferSize, "config.layers.[%d]", i);
				ConfigShortcutPtr pShortcut = _rConfig.GetShortcut(string(szBuffer));
				if (NULL == pShortcut)
				{
					bResult = false;
					break;
				}
				Layer oLayer;
				m_vLayers.push_back(oLayer);
				LayerRef rLayer = m_vLayers.back();
				bResult = _rConfig.GetValue(pShortcut, strAtlasIndex, rLayer.m_uAtlasIndex)
					&& _rConfig.GetValue(pShortcut, strMinHeight, rLayer.m_fMinHeight)
					&& _rConfig.GetValue(pShortcut, strMaxHeight, rLayer.m_fMaxHeight)
					&& _rConfig.GetValue(pShortcut, strMinSlope, rLayer.m_fMinSlope)
					&& _rConfig.GetValue(pShortcut, strMaxSlope, rLayer.m_fMaxSlope);
				if (false == bResult)
				{
					m_vLayers.pop_back();
					break;
				}
			}
		}

		if (false != bResult)
		{
			LockedRect oLockRect;
			TexturePtr pTexture = static_cast<TexturePtr>(m_pSlopeAndHeightLUT->GetBase());
			SurfaceDescPtr pDesc = m_pSlopeAndHeightLUT->GetDesc(D3DCUBEMAP_FACES(0));
			bResult = (NULL != pTexture) && (NULL != pDesc) && SUCCEEDED(pTexture->LockRect(0, &oLockRect, NULL, D3DLOCK_DISCARD));
			if (false != bResult)
			{
				const float fUStep = 1.0f / float(pDesc->Width);
				const float fVStep = 1.0f / float(pDesc->Height);
				LayerVec::iterator iEnd = m_vLayers.end();
				BytePtr pData = static_cast<BytePtr>(oLockRect.pBits);
				int sRawIndex = 0;
				for (float v = 0.0f ; 1.0f > v ; v += fVStep)
				{
					UIntPtr pRaw = reinterpret_cast<UIntPtr>(&pData[sRawIndex * oLockRect.Pitch]);
					for (float u = 0.0f ; 1.0f > u ; v += fUStep)
					{
						LayerVec::iterator iLayer = m_vLayers.begin();
						while (iEnd != iLayer)
						{
							if (false != (*iLayer).Evaluate(v, u))
							{
								*pRaw = D3DCOLOR_ARGB(255, (*iLayer).m_uAtlasIndex, 0, 0);
							}
							++iLayer;
						}
						++pRaw;
					}
					++sRawIndex;
				}
				bResult = SUCCEEDED(pTexture->UnlockRect(0));
			}
		}

		return bResult;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	LandscapeLayerManager::LandscapeLayerManager(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_mConfigs()
	{

	}

	LandscapeLayerManager::~LandscapeLayerManager()
	{

	}

	void LandscapeLayerManager::SetInstance(LandscapeLayerManagerPtr _pInstance)
	{
		s_pInstance = _pInstance;
	}

	LandscapeLayerManagerPtr LandscapeLayerManager::GetInstance()
	{
		return s_pInstance;
	}

	bool LandscapeLayerManager::Create(const boost::any& _rConfig)
	{
		bool bResult = true;
		return bResult;
	}

	void LandscapeLayerManager::Update()
	{

	}

	void LandscapeLayerManager::Release()
	{
		UnloadAll();
	}

	LandscapeLayeringPtr LandscapeLayerManager::Get(const string& _strFileName)
	{
		LandscapeLayeringPtr pResult = NULL;
		const Key uKey = MakeKey(_strFileName);
		LandscapeLayeringPtrMap::iterator iPair = m_mConfigs.find(uKey);

		if (m_mConfigs.end() == iPair)
		{
			pResult = Load(_strFileName);
			if (NULL != pResult)
			{
				m_mConfigs[uKey] = pResult;
			}
		}
		else
		{
			pResult = iPair->second;
		}

		return pResult;
	}

	void LandscapeLayerManager::UnloadAll()
	{
		while (m_mConfigs.end() != m_mConfigs.begin())
		{
			LandscapeLayeringPtrMap::iterator iPair = m_mConfigs.begin();
			LandscapeLayeringPtr pConfig = iPair->second;
			pConfig->Release();
			delete pConfig;
			m_mConfigs.erase(m_mConfigs.begin());
		}
	}

	DisplayRef LandscapeLayerManager::GetDisplay()
	{
		return m_rDisplay;
	}

	LandscapeLayeringPtr LandscapeLayerManager::Load(const string& _strFileName)
	{
		LandscapeLayeringPtr pResult = new LandscapeLayering(*this);
		LandscapeLayering::CreateInfo oLLCInfo = { _strFileName };
		bool bResult = pResult->Create(boost::any(&oLLCInfo));

		if (false == bResult)
		{
			pResult->Release();
			delete pResult;
			pResult = NULL;
		}

		return pResult;
	}
}