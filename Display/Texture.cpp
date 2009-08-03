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
		FilePtr pFile = FS::GetRoot()->OpenFile(pInfo->m_strPath, FS::EOpenMode_READBINARY);
		bool bResult = (NULL != pFile);

		if (false != bResult)
		{
			int sSize = pFile->Size();
			unsigned char* pBuffer = new unsigned char[sSize];
			sSize = pFile->Read(pBuffer, sSize);

			switch (pInfo->m_eType)
			{
				case EType_2D:
				{
					bResult = SUCCEEDED(D3DXCreateTextureFromFileInMemory(m_rDisplay.GetDevicePtr(), pBuffer, sSize, &m_pTexture));
					break;
				}
				case EType_CUBE:
				{
					bResult = SUCCEEDED(D3DXCreateCubeTextureFromFileInMemory(m_rDisplay.GetDevicePtr(), pBuffer, sSize, &m_pCubeTexture));
					break;
				}
			}

			delete[] pBuffer;
			FS::GetRoot()->CloseFile(pFile);
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
		bool bResult = true;
		return bResult;
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
			DisplayTexture::CreateInfo oDTCInfo = { _strName, _strPath, _eType };
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
		DisplayTexturePtrMap::iterator iPair = m_mTextures.find(uKey);
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
		DisplayTexturePtrMap::iterator iPair = m_mTextures.find(uKey);
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

}
