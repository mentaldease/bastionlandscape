#include "stdafx.h"
#include "Landscape.h"
#include "../Display/Display.h"
#include "../Display/Texture.h"
#include "../Display/Effect.h"
#include "../Display/Surface.h"
#include "../Core/File.h"
#include "../Core/Noise.h"
#include "../Core/Scripting.h"

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
		m_strSAHLUTName(),
		m_strNoiseName()
	{

	}

	LandscapeLayering::~LandscapeLayering()
	{

	}

	bool LandscapeLayering::Create(const boost::any& _rConfig)
	{
		CreateInfoPtr pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		string strFileExt;
		FS::GetFileExt(pInfo->m_strPath, strFileExt);

		bool bResult = false;

		if (string("lua") != strFileExt)
		{
			bResult = CreateFromLibConfig(*pInfo);
		}
		else
		{
			bResult = CreateFromLuaConfig(*pInfo);
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

	bool LandscapeLayering::CreateFromLibConfig(CreateInfoRef _rInfo)
	{
		DisplayTextureManagerPtr pTextureManager = LandscapeLayerManager::GetInstance()->GetDisplay().GetTextureManager();
		Config::CreateInfo oCCInfo = { _rInfo.m_strPath };
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
			bResult = CreateAtlas(oConfig);
		}

		if (false != bResult)
		{
			m_strSAHLUTName = _rInfo.m_strPath;
			m_strSAHLUTName = m_strSAHLUTName.substr(0, m_strSAHLUTName.find_last_of('.')) + string(".bmp");
			bResult = pTextureManager->Load(m_strSAHLUTName, m_strSAHLUTName, DisplayTexture::EType_2D);
			if ((false == bResult) || (0 != sRebuildLookupTable))
			{
				if (false != bResult)
				{
					pTextureManager->Unload(m_strSAHLUTName);
					m_pSlopeAndHeightLUT = NULL;
				}
				bResult = pTextureManager->New(m_strSAHLUTName, 0x00000001 << 8, 0x00000001 << 8, D3DFMT_A8R8G8B8, false, DisplayTexture::EType_2D, DisplayTexture::EUsage_DEFAULT);
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
			m_strNoiseName = _rInfo.m_strPath;
			m_strNoiseName = m_strNoiseName.substr(0, m_strNoiseName.find_last_of('.')) + string("Noise.bmp");
			bResult = pTextureManager->Load(m_strNoiseName, m_strNoiseName, DisplayTexture::EType_2D);
			if ((false == bResult) || (0 != sRebuildLookupTable))
			{
				if (false != bResult)
				{
					pTextureManager->Unload(m_strNoiseName);
					m_pNoise = NULL;
				}
				bResult = pTextureManager->New(m_strNoiseName, 0x00000001 << 8, 0x00000001 << 8, D3DFMT_A8R8G8B8, false, DisplayTexture::EType_2D, DisplayTexture::EUsage_DEFAULT);
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
			bResult = Display::IsPowerOf2(unsigned int(m_oShaderInfo.z), &uPowerLevel);
			if (false != bResult)
			{
				m_oShaderInfo.w = float(uPowerLevel);
			}
		}

		return bResult;
	}

	bool LandscapeLayering::CreateAtlas(ConfigRef _rConfig)
	{
		DisplayTextureManagerPtr pTextureManager = LandscapeLayerManager::GetInstance()->GetDisplay().GetTextureManager();
		DisplaySurfaceManagerPtr pSurfaceManager = m_rLandscapeLayerManager.GetDisplay().GetSurfaceManager();
		bool bResult = pSurfaceManager->Load(m_strAtlasName, m_strAtlasName);
		DisplaySurfacePtr pSurface = (false != bResult) ? pSurfaceManager->Get(m_strAtlasName) : NULL;

		if (false != bResult)
		{
			ImageInfoRef rSurfaceInfo = pSurface->GetInfo();
			bResult = pTextureManager->New(m_strAtlasName, rSurfaceInfo.Width, rSurfaceInfo.Height, D3DFMT_A8R8G8B8, true, DisplayTexture::EType_2D, DisplayTexture::EUsage_DEFAULT);
			m_pAtlas = (false != bResult) ? pTextureManager->Get(m_strAtlasName) : NULL;
		}
		if (false != bResult)
		{
			unsigned int uWidthPow2Level;
			unsigned int uHeightPow2Level;
			ImageInfoRef rSurfaceInfo = pSurface->GetInfo();
			bResult = Display::IsPowerOf2(rSurfaceInfo.Width, &uWidthPow2Level)
				&& Display::IsPowerOf2(rSurfaceInfo.Height, &uHeightPow2Level)
				&& pSurface->Lock(true);

			if (false != bResult)
			{
				const unsigned int uMaxLOD = (uWidthPow2Level <= uHeightPow2Level) ? uWidthPow2Level : uHeightPow2Level;
				TexturePtr pTexture = static_cast<TexturePtr>(m_pAtlas->GetBase());
				LockedRect oLockRect;
				UIntPtr pPixel;

				for (unsigned int k = 0 ; uMaxLOD > k ; ++k)
				{
					bResult = SUCCEEDED(pTexture->LockRect(k, &oLockRect, NULL, 0));
					if (false != bResult)
					{
						BytePtr pData = static_cast<BytePtr>(oLockRect.pBits);
						const unsigned int uIncrement = 0x00000001 << k;
						for (unsigned int j = 0, uRow = 0 ; rSurfaceInfo.Height > j ; j += uIncrement, ++uRow)
						{
							UIntPtr pRow = reinterpret_cast<UIntPtr>(&pData[uRow * oLockRect.Pitch]);
							for (unsigned int i = 0 ; rSurfaceInfo.Width > i ; i += uIncrement)
							{
								pPixel = static_cast<UIntPtr>(pSurface->GetDataXY(i , j));
								if (NULL == pPixel)
								{
									bResult = false;
									break;
								}
								switch (rSurfaceInfo.Format)
								{
									case D3DFMT_A8R8G8B8:
									{
										*pRow = *pPixel;
										break;
									}
									case D3DFMT_X8R8G8B8:
									{
										//*pRow = D3DCOLOR_ARGB(255, pPixel[0], pPixel[1], pPixel[2]);
										*pRow = *pPixel | 0xff000000;
										break;
									}
								}
								++pRow;
							}
							if (false == bResult)
							{
								break;
							}
						}
						bResult = SUCCEEDED(pTexture->UnlockRect(k)) && (false != bResult);
					}
					if (false == bResult)
					{
						break;
					}
				}
				pSurface->Unlock();
			}
		}

		if (false != bResult)
		{
			BufferPtr pBuffer;
			bResult = SUCCEEDED(D3DXSaveTextureToFileInMemory(&pBuffer, D3DXIFF_DDS, m_pAtlas->GetBase(), NULL));
			if (false != bResult)
			{
				const string strDDSName = m_strAtlasName.substr(0, m_strAtlasName.find_last_of('.')) + string(".dds");
				FilePtr pFile = FS::GetRoot()->OpenFile(strDDSName, FS::EOpenMode_CREATEBINARY);
				if (NULL != pFile)
				{
					pFile->Write(pBuffer->GetBufferPointer(), pBuffer->GetBufferSize());
					FS::GetRoot()->CloseFile(pFile);
				}
				pBuffer->Release();
			}
		}

		if (NULL != pSurface)
		{
			pSurfaceManager->Unload(m_strAtlasName);
		}

		return bResult;
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
						pBuffer->Release();
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
					pBuffer->Release();
				}
			}
		}

		oNoiseGenerator.Release();

		return bResult;
	}

	bool LandscapeLayering::CreateFromLuaConfig(CreateInfoRef _rInfo)
	{
		bool bResult = Scripting::Lua::Loadfile(_rInfo.m_strPath);

		if (false != bResult)
		{
			DisplayTextureManagerPtr pTextureManager = LandscapeLayerManager::GetInstance()->GetDisplay().GetTextureManager();
			string strNameSpace;
			FS::GetFileNameWithoutExt(_rInfo.m_strPath, strNameSpace);
			LuaObject oNameSpace = Scripting::Lua::GetStateInstance()->GetGlobal(strNameSpace.c_str());
			bResult = (false == oNameSpace.IsNil());

			if (false != bResult)
			{
				int sAGS = oNameSpace["atlas_grid_size"].GetInteger();
				bool bRebuildLookupTable = oNameSpace["rebuild_lookuptable"].GetBoolean();
				m_strAtlasName = oNameSpace["atlas_filename"].GetString();

				if (false != bResult)
				{
					bResult = CreateAtlas(oNameSpace);
				}

				if (false != bResult)
				{
					m_strSAHLUTName = _rInfo.m_strPath;
					m_strSAHLUTName = m_strSAHLUTName.substr(0, m_strSAHLUTName.find_last_of('.')) + string(".bmp");
					bResult = pTextureManager->Load(m_strSAHLUTName, m_strSAHLUTName, DisplayTexture::EType_2D);
					if ((false == bResult) || (false != bRebuildLookupTable))
					{
						if (false != bResult)
						{
							pTextureManager->Unload(m_strSAHLUTName);
							m_pSlopeAndHeightLUT = NULL;
						}
						bResult = pTextureManager->New(m_strSAHLUTName, 0x00000001 << 8, 0x00000001 << 8, D3DFMT_A8R8G8B8, false, DisplayTexture::EType_2D, DisplayTexture::EUsage_DEFAULT);
						m_pSlopeAndHeightLUT = (false != bResult) ? pTextureManager->Get(m_strSAHLUTName) : NULL;
						bResult = (NULL != m_pSlopeAndHeightLUT) && CreateSlopeAndHeightLUT(oNameSpace);
					}
					else
					{
						m_pSlopeAndHeightLUT = pTextureManager->Get(m_strSAHLUTName);
						bResult = (NULL != m_pSlopeAndHeightLUT);
					}
				}

				if (false != bResult)
				{
					m_strNoiseName = _rInfo.m_strPath;
					m_strNoiseName = m_strNoiseName.substr(0, m_strNoiseName.find_last_of('.')) + string("Noise.bmp");
					bResult = pTextureManager->Load(m_strNoiseName, m_strNoiseName, DisplayTexture::EType_2D);
					if ((false == bResult) || (false != bRebuildLookupTable))
					{
						if (false != bResult)
						{
							pTextureManager->Unload(m_strNoiseName);
							m_pNoise = NULL;
						}
						bResult = pTextureManager->New(m_strNoiseName, 0x00000001 << 8, 0x00000001 << 8, D3DFMT_A8R8G8B8, false, DisplayTexture::EType_2D, DisplayTexture::EUsage_DEFAULT);
						m_pNoise = (false != bResult) ? pTextureManager->Get(m_strNoiseName) : NULL;
						bResult = (NULL != m_pNoise) && CreateNoise(oNameSpace);
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
					bResult = Display::IsPowerOf2(unsigned int(m_oShaderInfo.z), &uPowerLevel);
					if (false != bResult)
					{
						m_oShaderInfo.w = float(uPowerLevel);
					}
				}
			}
		}

		return bResult;
	}

	bool LandscapeLayering::CreateAtlas(LuaObjectRef _rLuaObject)
	{
		DisplayTextureManagerPtr pTextureManager = LandscapeLayerManager::GetInstance()->GetDisplay().GetTextureManager();
		DisplaySurfaceManagerPtr pSurfaceManager = m_rLandscapeLayerManager.GetDisplay().GetSurfaceManager();
		bool bResult = pSurfaceManager->Load(m_strAtlasName, m_strAtlasName);
		DisplaySurfacePtr pSurface = (false != bResult) ? pSurfaceManager->Get(m_strAtlasName) : NULL;

		if (false != bResult)
		{
			ImageInfoRef rSurfaceInfo = pSurface->GetInfo();
			bResult = pTextureManager->New(m_strAtlasName, rSurfaceInfo.Width, rSurfaceInfo.Height, D3DFMT_A8R8G8B8, true, DisplayTexture::EType_2D, DisplayTexture::EUsage_DEFAULT);
			m_pAtlas = (false != bResult) ? pTextureManager->Get(m_strAtlasName) : NULL;
		}
		if (false != bResult)
		{
			unsigned int uWidthPow2Level;
			unsigned int uHeightPow2Level;
			ImageInfoRef rSurfaceInfo = pSurface->GetInfo();
			bResult = Display::IsPowerOf2(rSurfaceInfo.Width, &uWidthPow2Level)
				&& Display::IsPowerOf2(rSurfaceInfo.Height, &uHeightPow2Level)
				&& pSurface->Lock(true);

			if (false != bResult)
			{
				const unsigned int uMaxLOD = (uWidthPow2Level <= uHeightPow2Level) ? uWidthPow2Level : uHeightPow2Level;
				TexturePtr pTexture = static_cast<TexturePtr>(m_pAtlas->GetBase());
				LockedRect oLockRect;
				UIntPtr pPixel;

				for (unsigned int k = 0 ; uMaxLOD > k ; ++k)
				{
					bResult = SUCCEEDED(pTexture->LockRect(k, &oLockRect, NULL, 0));
					if (false != bResult)
					{
						BytePtr pData = static_cast<BytePtr>(oLockRect.pBits);
						const unsigned int uIncrement = 0x00000001 << k;
						for (unsigned int j = 0, uRow = 0 ; rSurfaceInfo.Height > j ; j += uIncrement, ++uRow)
						{
							UIntPtr pRow = reinterpret_cast<UIntPtr>(&pData[uRow * oLockRect.Pitch]);
							for (unsigned int i = 0 ; rSurfaceInfo.Width > i ; i += uIncrement)
							{
								pPixel = static_cast<UIntPtr>(pSurface->GetDataXY(i , j));
								if (NULL == pPixel)
								{
									bResult = false;
									break;
								}
								switch (rSurfaceInfo.Format)
								{
								case D3DFMT_A8R8G8B8:
									{
										*pRow = *pPixel;
										break;
									}
								case D3DFMT_X8R8G8B8:
									{
										//*pRow = D3DCOLOR_ARGB(255, pPixel[0], pPixel[1], pPixel[2]);
										*pRow = *pPixel | 0xff000000;
										break;
									}
								}
								++pRow;
							}
							if (false == bResult)
							{
								break;
							}
						}
						bResult = SUCCEEDED(pTexture->UnlockRect(k)) && (false != bResult);
					}
					if (false == bResult)
					{
						break;
					}
				}
				pSurface->Unlock();
			}
		}

		if (false != bResult)
		{
			BufferPtr pBuffer;
			bResult = SUCCEEDED(D3DXSaveTextureToFileInMemory(&pBuffer, D3DXIFF_DDS, m_pAtlas->GetBase(), NULL));
			if (false != bResult)
			{
				const string strDDSName = m_strAtlasName.substr(0, m_strAtlasName.find_last_of('.')) + string(".dds");
				FilePtr pFile = FS::GetRoot()->OpenFile(strDDSName, FS::EOpenMode_CREATEBINARY);
				if (NULL != pFile)
				{
					pFile->Write(pBuffer->GetBufferPointer(), pBuffer->GetBufferSize());
					FS::GetRoot()->CloseFile(pFile);
				}
				pBuffer->Release();
			}
		}

		if (NULL != pSurface)
		{
			pSurfaceManager->Unload(m_strAtlasName);
		}

		return bResult;
	}

	bool LandscapeLayering::CreateSlopeAndHeightLUT(LuaObjectRef _rLuaObject)
	{
		LuaObject oLayers = _rLuaObject["layers"];
		int sCount = 0;
		bool bResult = (false == oLayers.IsNil()) && (sCount = oLayers.GetCount());

		if (false != bResult)
		{
			const char* pszAtlasIndex = "atlas_index";
			const char* pszMinHeight = "min_height";
			const char* pszMaxHeight = "max_height";
			const char* pszMinSlope = "min_slope";
			const char* pszMaxSlope = "max_slope";
			for (int i = 0 ; sCount > i ; ++i)
			{
				LuaObject oLuaLayer = oLayers[i + 1];
				Layer oLayer;
				m_vLayers.push_back(oLayer);
				LayerRef rLayer = m_vLayers.back();
				rLayer.m_uAtlasIndex = oLuaLayer[pszAtlasIndex].GetInteger();
				rLayer.m_fMinHeight = oLuaLayer[pszMinHeight].GetFloat();
				rLayer.m_fMaxHeight = oLuaLayer[pszMaxHeight].GetFloat();
				rLayer.m_fMinSlope = oLuaLayer[pszMinSlope].GetFloat();
				rLayer.m_fMaxSlope = oLuaLayer[pszMaxSlope].GetFloat();
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
						pBuffer->Release();
					}
				}
			}
		}

		return bResult;
	}

	bool LandscapeLayering::CreateNoise(LuaObjectRef _rLuaObject)
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
					pBuffer->Release();
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
			m_mConfigs.erase(iPair);
			pConfig->Release();
			delete pConfig;
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