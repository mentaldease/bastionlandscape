#include "stdafx.h"
#include "../Display/Texture.h"
#include "../Core/File.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayTexture::DisplayTexture(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_eType(),
		m_pTexture(NULL),
		m_pCubeTexture(NULL)
	{

	}

	DisplayTexture::~DisplayTexture()
	{

	}

	bool DisplayTexture::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = (NULL != pInfo);
		if (false != bResult)
		{
			Release();
			bResult = (false != pInfo->m_bLoadMode) ? Load(*pInfo) : New(*pInfo);
		}
		return bResult;
	}

	void DisplayTexture::Update()
	{

	}

	void DisplayTexture::Release()
	{
		if (NULL != m_pTexture)
		{
			m_pTexture->Release();
			m_pTexture = NULL;
		}
		if (NULL != m_pCubeTexture)
		{
			m_pCubeTexture->Release();
			m_pCubeTexture = NULL;
		}
	}

	BaseTexturePtr DisplayTexture::GetBase()
	{
		if (NULL != m_pTexture)
		{
			return m_pTexture;
		}
		if (NULL != m_pCubeTexture)
		{
			return m_pCubeTexture;
		}
		return NULL;
	}

	SurfaceDescPtr DisplayTexture::GetDesc(const D3DCUBEMAP_FACES& _eFace)
	{
		if (NULL != m_pTexture)
		{
			return &m_aSurfaceDescs[0];
		}
		if (NULL != m_pCubeTexture)
		{
			return &m_aSurfaceDescs[_eFace];
		}
		return NULL;
	}

	bool DisplayTexture::Load(CreateInfoRef _rInfo)
	{
		FilePtr pFile = FS::GetRoot()->OpenFile(_rInfo.m_strPath, FS::EOpenMode_READBINARY);
		bool bResult = (NULL != pFile);

		if (false != bResult)
		{
			int sSize = pFile->Size();
			unsigned char* pBuffer = new unsigned char[sSize];
			sSize = pFile->Read(pBuffer, sSize);

			switch (_rInfo.m_eType)
			{
				case EType_2D:
				{
					bResult = SUCCEEDED(D3DXCreateTextureFromFileInMemory(m_rDisplay.GetDevicePtr(), pBuffer, sSize, &m_pTexture));
					if (false != bResult)
					{
						bResult = SUCCEEDED(m_pTexture->GetLevelDesc(0, &m_aSurfaceDescs[0]));
						if (false == bResult)
						{
							break;
						}
					}
					break;
				}
				case EType_CUBE:
				{
					bResult = SUCCEEDED(D3DXCreateCubeTextureFromFileInMemory(m_rDisplay.GetDevicePtr(), pBuffer, sSize, &m_pCubeTexture));
					if (false != bResult)
					{
						SurfacePtr pSurface;
						for (int i = 0 ; 6 > i ; ++i)
						{
							bResult = SUCCEEDED(m_pCubeTexture->GetCubeMapSurface(D3DCUBEMAP_FACES(i), 0, &pSurface)
								&& pSurface->GetDesc(&m_aSurfaceDescs[i]));
							if (false == bResult)
							{
								break;
							}
							pSurface->Release();
						}
					}
					break;
				}
			}

			delete[] pBuffer;
			FS::GetRoot()->CloseFile(pFile);
		}

		return bResult;
	}

	bool DisplayTexture::New(CreateInfoRef _rInfo)
	{
		unsigned int uWidthPow2Level = 0;
		unsigned int uHeightPow2Level = 0;
		bool bResult = (0 != _rInfo.m_uWidth) && (0 != _rInfo.m_uHeight)
			&& ((EUsage_RENDERTARGET == _rInfo.m_eUsage) || Display::IsPowerOf2(_rInfo.m_uWidth, &uWidthPow2Level))
			&& ((EUsage_RENDERTARGET == _rInfo.m_eUsage) || Display::IsPowerOf2(_rInfo.m_uHeight, &uHeightPow2Level));

		if (false != bResult)
		{
			const unsigned int uMaxLOD = (uWidthPow2Level <= uHeightPow2Level) ? uWidthPow2Level : uHeightPow2Level;
			switch (_rInfo.m_eType)
			{
				case EType_2D:
				{
					bResult = SUCCEEDED(D3DXCreateTexture(m_rDisplay.GetDevicePtr(),
						_rInfo.m_uWidth,
						_rInfo.m_uHeight,
						(EUsage_RENDERTARGET != _rInfo.m_eUsage) && (false != _rInfo.m_bMipmap) ? uMaxLOD : 1,
						//(false != _rInfo.m_bMipmap) ? D3DUSAGE_AUTOGENMIPMAP : 0, // don't use D3DUSAGE_AUTOGENMIPMAP if you want to access each mipmap level
						(EUsage_RENDERTARGET == _rInfo.m_eUsage) ? D3DUSAGE_RENDERTARGET : 0,
						_rInfo.m_eFormat,
						(EUsage_RENDERTARGET == _rInfo.m_eUsage) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
						&m_pTexture));
					if (false != bResult)
					{
						bResult = SUCCEEDED(m_pTexture->GetLevelDesc(0, &m_aSurfaceDescs[0]));
					}
					break;
				}
				case EType_CUBE:
				{
					bResult = (_rInfo.m_uWidth == _rInfo.m_uHeight) && SUCCEEDED(D3DXCreateCubeTexture(m_rDisplay.GetDevicePtr(),
						_rInfo.m_uWidth,
						//(false != _rInfo.m_bMipmap) ? D3DX_DEFAULT : 1,
						(EUsage_RENDERTARGET != _rInfo.m_eUsage) && (false != _rInfo.m_bMipmap) ? uMaxLOD : 1,
						//(false != _rInfo.m_bMipmap) ? D3DUSAGE_AUTOGENMIPMAP : 0, // don't use D3DUSAGE_AUTOGENMIPMAP if you want to access each mipmap level
						(EUsage_RENDERTARGET == _rInfo.m_eUsage) ? D3DUSAGE_RENDERTARGET : 0,
						_rInfo.m_eFormat,
						(EUsage_RENDERTARGET == _rInfo.m_eUsage) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
						&m_pCubeTexture));
					if (false != bResult)
					{
						SurfacePtr pSurface;
						for (int i = 0 ; 6 > i ; ++i)
						{
							bResult = SUCCEEDED(m_pCubeTexture->GetCubeMapSurface(D3DCUBEMAP_FACES(i), 0, &pSurface)
								&& pSurface->GetDesc(&m_aSurfaceDescs[i]));
							if (false == bResult)
							{
								break;
							}
							pSurface->Release();
						}
					}
					break;
				}
			}
		}

		return bResult;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayTextureManager::DisplayTextureManager(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_mTextures()
	{

	}

	DisplayTextureManager::~DisplayTextureManager()
	{

	}

	bool DisplayTextureManager::Create(const boost::any& _rConfig)
	{
		Release();
		return true;
	}

	void DisplayTextureManager::Update()
	{

	}

	void DisplayTextureManager::Release()
	{
		UnloadAll();
	}

	bool DisplayTextureManager::Load(const string& _strName, const string& _strPath, const DisplayTexture::EType& _eType)
	{
		bool bResult = (NULL == Get(_strName));

		if (false != bResult)
		{
			DisplayTexture::CreateInfo oDTCInfo = { true, _strName, _strPath, _eType, 0, 0, D3DFMT_UNKNOWN, DisplayTexture::EUsage_DEFAULT, false };
			DisplayTexturePtr pTexture = new DisplayTexture(m_rDisplay);
			bResult = pTexture->Create(boost::any(&oDTCInfo));
			if (false != bResult)
			{
				const Key uKey = MakeKey(_strName);
				m_mTextures[uKey] = pTexture;
			}
			else if (NULL != pTexture)
			{
				pTexture->Release();
				delete pTexture;
			}
		}

		return bResult;
	}

	bool DisplayTextureManager::New(const string& _strName, const unsigned int& _uWidth, const unsigned int& _uHeight, const D3DFORMAT& _eFormat, const bool& _bMipmap, const DisplayTexture::EType& _eType, const DisplayTexture::EUsage& _eUsage)
	{
		bool bResult = (NULL == Get(_strName));

		if (false != bResult)
		{
			DisplayTexture::CreateInfo oDTCInfo = { false, _strName, _strName, _eType, _uWidth, _uHeight, _eFormat, _eUsage, _bMipmap };
			DisplayTexturePtr pTexture = new DisplayTexture(m_rDisplay);
			bResult = pTexture->Create(boost::any(&oDTCInfo));
			if (false != bResult)
			{
				const Key uKey = MakeKey(_strName);
				m_mTextures[uKey] = pTexture;
			}
			else if (NULL != pTexture)
			{
				pTexture->Release();
				delete pTexture;
			}
		}

		return bResult;
	}

	void DisplayTextureManager::Unload(const string& _strName)
	{
		const Key uKey = MakeKey(_strName);
		Unload(uKey);
	}

	void DisplayTextureManager::Unload(const Key& _strNameKey)
	{
		DisplayTexturePtrMap::iterator iPair = m_mTextures.find(_strNameKey);
		if (m_mTextures.end() != iPair)
		{
			DisplayTexturePtr pTexture = iPair->second;
			pTexture->Release();
			delete pTexture;
			m_mTextures.erase(iPair);
		}
	}

	DisplayTexturePtr DisplayTextureManager::Get(const string& _strName)
	{
		const Key uKey = MakeKey(_strName);
		return Get(uKey);
	}

	DisplayTexturePtr DisplayTextureManager::Get(const Key& _strNameKey)
	{
		DisplayTexturePtrMap::iterator iPair = m_mTextures.find(_strNameKey);
		DisplayTexturePtr pResult = (m_mTextures.end() != iPair) ? iPair->second : NULL;
		return pResult;
	}

	void DisplayTextureManager::UnloadAll()
	{
		DisplayTexturePtrMap::iterator iPair = m_mTextures.begin();
		DisplayTexturePtrMap::iterator iEnd = m_mTextures.end();

		while (iEnd != iPair)
		{
			DisplayTexturePtr pTexture = iPair->second;
			pTexture->Release();
			delete pTexture;
			++iPair;
		}

		m_mTextures.clear();
	}

	DisplayTexturePtr DisplayTextureManager::GetBySemantic(const Key& _uSemanticKey)
	{
		DisplayTexturePtrMap::iterator iPair = m_mSemanticTextures.find(_uSemanticKey);
		DisplayTexturePtr pResult = (m_mSemanticTextures.end() != iPair) ? iPair->second : NULL;
		return pResult;
	}

	void DisplayTextureManager::SetBySemantic(const Key& _uSemanticKey, DisplayTexturePtr _pTexture)
	{
		m_mSemanticTextures[_uSemanticKey] = _pTexture;
	}
}
