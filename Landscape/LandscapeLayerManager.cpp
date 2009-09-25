#include "stdafx.h"
#include "Landscape.h"
#include "../Display/Display.h"
#include "../Display/Texture.h"
#include "../Display/Effect.h"
#include "../Core/File.h"
#include "../Core/Noise.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	LandscapeLayerManagerPtr LandscapeLayerManager::s_pInstance = NULL;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	bool LandscapeLayering::Layer::Evaluate(const float& _fSlope, const float& _fHeight)
	{
		bool bResult = (m_fMinHeight <= _fHeight)
			&& (m_fMaxHeight >= _fHeight)
			&& (m_fMinSlope <= _fSlope)
			&& (m_fMaxSlope >= _fSlope);
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
		m_pNoise(NULL),
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
		DisplayTextureManagerPtr pTextureManager = LandscapeLayerManager::GetInstance()->GetDisplay().GetTextureManager();
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		Config::CreateInfo oCCInfo = { pInfo->m_strPath };
		Config oConfig;
		bool bResult = oConfig.Create(boost::any(&oCCInfo));
		int sAGS = 0;
		int sRebuildLookupTable = 1;

		if (false != bResult)
		{
			bResult = oConfig.GetValue("config.atlas_filename", m_strAtlasName)
				&& oConfig.GetValue("config.atlas_grid_size", sAGS)
				&& oConfig.GetValue("config.rebuild_lookuptable", sRebuildLookupTable);
		}

		if (false != bResult)
		{
			bResult = pTextureManager->Load(m_strAtlasName, m_strAtlasName, DisplayTexture::EType_2D);
			m_pAtlas = (false != bResult) ? pTextureManager->Get(m_strAtlasName) : NULL;
			bResult = (NULL != m_pAtlas);
		}

		if (false != bResult)
		{
			m_strSAHLUTName = pInfo->m_strPath;
			m_strSAHLUTName = m_strSAHLUTName.substr(0, m_strSAHLUTName.find_last_of('.')) + string(".bmp");
			bResult = pTextureManager->Load(m_strSAHLUTName, m_strSAHLUTName, DisplayTexture::EType_2D);
			if ((false == bResult) || (0 != sRebuildLookupTable))
			{
				if (false != bResult)
				{
					pTextureManager->Unload(m_strSAHLUTName);
					m_pSlopeAndHeightLUT = NULL;
				}
				bResult = pTextureManager->New(m_strSAHLUTName, 0x00000001 << 8, 0x00000001 << 8, D3DFMT_A8R8G8B8, false, DisplayTexture::EType_2D);
				m_pSlopeAndHeightLUT = (false != bResult) ? pTextureManager->Get(m_strSAHLUTName) : NULL;
				bResult = (NULL != m_pSlopeAndHeightLUT) && CreateSlopeAndHeightLUT(oConfig);
			}
			else
			{
				m_pSlopeAndHeightLUT = pTextureManager->Get(m_strSAHLUTName);
				bResult = (NULL != m_pSlopeAndHeightLUT);
			}
		}

		if (false != bResult)
		{
			m_strNoiseName = pInfo->m_strPath;
			m_strNoiseName = m_strNoiseName.substr(0, m_strNoiseName.find_last_of('.')) + string("Noise.bmp");
			bResult = pTextureManager->Load(m_strNoiseName, m_strNoiseName, DisplayTexture::EType_2D);
			if ((false == bResult) || (0 != sRebuildLookupTable))
			{
				if (false != bResult)
				{
					pTextureManager->Unload(m_strNoiseName);
					m_pNoise = NULL;
				}
				bResult = pTextureManager->New(m_strNoiseName, 0x00000001 << 8, 0x00000001 << 8, D3DFMT_A8R8G8B8, false, DisplayTexture::EType_2D);
				m_pNoise = (false != bResult) ? pTextureManager->Get(m_strNoiseName) : NULL;
				bResult = (NULL != m_pNoise) && CreateNoise(oConfig);
			}
			else
			{
				m_pNoise = pTextureManager->Get(m_strNoiseName);
				bResult = (NULL != m_pNoise);
			}
		}

		if (false != bResult)
		{
			m_oShaderInfo.x = 1.0f / float(sAGS);
			m_oShaderInfo.y = 1.0f / float(sAGS);
			m_oShaderInfo.z = float(m_pAtlas->GetDesc(D3DCUBEMAP_FACES(0))->Width / sAGS);
			unsigned int uPowerLevel;
			bResult = Landscape::GlobalInfo::IsPowerOf2(unsigned int(m_oShaderInfo.z), &uPowerLevel);
			if (false != bResult)
			{
				m_oShaderInfo.w = float(uPowerLevel);
			}
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
		if (NULL != m_pNoise)
		{
			pTextureManager->Unload(m_strNoiseName);
			m_pNoise = NULL;
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

	DisplayTexturePtr LandscapeLayering::GetNoise()
	{
		return m_pNoise;
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
			bResult = (NULL != pTexture) && (NULL != pDesc) && SUCCEEDED(pTexture->LockRect(0, &oLockRect, NULL, 0));
			if (false != bResult)
			{
				const float fUStep = 1.0f / float(pDesc->Width);
				const float fVStep = 1.0f / float(pDesc->Height);
				LayerVec::iterator iEnd = m_vLayers.end();
				BytePtr pData = static_cast<BytePtr>(oLockRect.pBits);
				int sRowIndex = 0;
				for (float v = 0.0f ; 1.0f > v ; v += fVStep)
				{
					UIntPtr pRow = reinterpret_cast<UIntPtr>(&pData[sRowIndex * oLockRect.Pitch]);
					for (float u = 0.0f ; 1.0f > u ; u += fUStep)
					{
						*pRow = D3DCOLOR_ARGB(255, 0, 255, 255);
						LayerVec::iterator iLayer = m_vLayers.begin();
						while (iEnd != iLayer)
						{
							LayerRef rLayer = *iLayer;
							if (false != rLayer.Evaluate(u, v))
							{
								*pRow = D3DCOLOR_ARGB(255, rLayer.m_uAtlasIndex, 0, 0);
							}
							++iLayer;
						}
						++pRow;
					}
					++sRowIndex;
				}
				bResult = SUCCEEDED(pTexture->UnlockRect(0));
				if (false != bResult)
				{
					BufferPtr pBuffer;
					bResult = SUCCEEDED(D3DXSaveTextureToFileInMemory(&pBuffer, D3DXIFF_BMP, m_pSlopeAndHeightLUT->GetBase(), NULL));
					if (false != bResult)
					{
						FilePtr pFile = FS::GetRoot()->OpenFile(m_strSAHLUTName, FS::EOpenMode_CREATEBINARY);
						if (NULL != pFile)
						{
							pFile->Write(pBuffer->GetBufferPointer(), pBuffer->GetBufferSize());
							FS::GetRoot()->CloseFile(pFile);
						}
					}
				}
			}
		}

		return bResult;
	}

	bool LandscapeLayering::CreateNoise(ConfigRef _rConfig)
	{
		NoiseGenerator oNoiseGenerator;
		bool bResult = oNoiseGenerator.Create(boost::any(0));

		if (false != bResult)
		{
			LockedRect oLockRect;
			TexturePtr pTexture = static_cast<TexturePtr>(m_pNoise->GetBase());
			SurfaceDescPtr pDesc = m_pNoise->GetDesc(D3DCUBEMAP_FACES(0));
			bResult = (NULL != pTexture) && (NULL != pDesc) && SUCCEEDED(pTexture->LockRect(0, &oLockRect, NULL, 0));
			if (false != bResult)
			{
				oNoiseGenerator.Process(int(pDesc->Width), int(pDesc->Height));
				FloatPtr pSrcData = oNoiseGenerator.GetData();
				const int sStride = oNoiseGenerator.GetStride();
				BytePtr pData = static_cast<BytePtr>(oLockRect.pBits);

				for (unsigned int j = 0 ; pDesc->Height > j ; ++j)
				{
					FloatPtr pSrcRow = pSrcData + size_t(j) * size_t(sStride);
					UIntPtr pDstRow = reinterpret_cast<UIntPtr>(&pData[j * oLockRect.Pitch]);
					for (unsigned int i = 0 ; pDesc->Width > i ; ++i)
					{
						const float fValue = ((*pSrcRow) < -1.0f) ? (*pSrcRow) + 1.0f : ((*pSrcRow) > 1.0f) ? (*pSrcRow) - 1.0f : (*pSrcRow);
						const Byte uValue = Byte((fValue + 1.0f) * 0.5f * 255.0f);
						*pDstRow = D3DCOLOR_ARGB(255, uValue, uValue, uValue);
						++pDstRow;
						++pSrcRow;
					}
				}
			}
			bResult = SUCCEEDED(pTexture->UnlockRect(0));

			if (false != bResult)
			{
				BufferPtr pBuffer;
				bResult = SUCCEEDED(D3DXSaveTextureToFileInMemory(&pBuffer, D3DXIFF_BMP, m_pNoise->GetBase(), NULL));
				if (false != bResult)
				{
					FilePtr pFile = FS::GetRoot()->OpenFile(m_strNoiseName, FS::EOpenMode_CREATEBINARY);
					if (NULL != pFile)
					{
						pFile->Write(pBuffer->GetBufferPointer(), pBuffer->GetBufferSize());
						FS::GetRoot()->CloseFile(pFile);
					}
				}
			}
		}

		oNoiseGenerator.Release();

		return bResult;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	LandscapeLayerManager::LandscapeLayerManager(DisplayRef _rDisplay)
	:	CoreObject(),
		m_mConfigs(),
		m_rDisplay(_rDisplay),
		m_pCurrentLayering(NULL)
	{
		m_uAtlasDiffuseKey = MakeKey(string("ATLASDIFFUSETEX"));
		m_uAtlasLUTKey = MakeKey(string("ATLASLUTTEX"));
		m_uAtlasDiffuseInfoKey = MakeKey(string("ATLASDIFFUSEINFO"));
		m_uNoiseKey = MakeKey(string("NOISETEX"));
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

	void LandscapeLayerManager::SetCurrentLayering(LandscapeLayeringPtr _pLayering)
	{
		if (m_pCurrentLayering != _pLayering)
		{
			m_pCurrentLayering = _pLayering;
			if (NULL != m_pCurrentLayering)
			{
				DisplayTextureManagerPtr pTextureManager = LandscapeLayerManager::GetInstance()->GetDisplay().GetTextureManager();
				pTextureManager->SetBySemantic(m_uAtlasDiffuseKey, m_pCurrentLayering->GetAtlas());
				pTextureManager->SetBySemantic(m_uAtlasLUTKey, m_pCurrentLayering->GetSlopeAndHeightLUT());
				pTextureManager->SetBySemantic(m_uNoiseKey, m_pCurrentLayering->GetNoise());
				DisplayMaterialManagerPtr pMaterialManager = LandscapeLayerManager::GetInstance()->GetDisplay().GetMaterialManager();
				pMaterialManager->SetVector4BySemantic(m_uAtlasDiffuseInfoKey, &m_pCurrentLayering->GetShaderInfo());
			}
		}
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