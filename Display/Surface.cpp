#include "stdafx.h"
#include "../Display/Surface.h"
#include "../Core/File.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplaySurface::DisplaySurface(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_pSurface(NULL)
	{

	}

	DisplaySurface::~DisplaySurface()
	{

	}

	bool DisplaySurface::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		FilePtr pFile = FS::GetRoot()->OpenFile(pInfo->m_strPath, FS::EOpenMode_READBINARY);
		bool bResult = (NULL != pFile);

		if (false != bResult)
		{
			int sSize = pFile->Size();
			unsigned char* pBuffer = new unsigned char[sSize];
			sSize = pFile->Read(pBuffer, sSize);

			bResult = SUCCEEDED(D3DXGetImageInfoFromFileInMemory(pBuffer, sSize, &m_oInfo));

			if (false != bResult)
			{
				bResult = SUCCEEDED(m_rDisplay.GetDevicePtr()->CreateOffscreenPlainSurface(m_oInfo.Width, m_oInfo.Height, m_oInfo.Format, D3DPOOL_DEFAULT, &m_pSurface, NULL));
			}

			if (false != bResult)
			{
				bResult = SUCCEEDED(D3DXLoadSurfaceFromFileInMemory(m_pSurface, NULL, NULL, pBuffer, sSize, NULL, D3DX_DEFAULT, 0xff000000, NULL));
			}

			delete[] pBuffer;
			FS::GetRoot()->CloseFile(pFile);
		}

		return bResult;
	}

	void DisplaySurface::Update()
	{

	}

	void DisplaySurface::Release()
	{
		if (NULL != m_pSurface)
		{
			m_pSurface->Release();
			m_pSurface = NULL;
		}
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplaySurfaceManager::DisplaySurfaceManager(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_mSurfaces()
	{

	}

	DisplaySurfaceManager::~DisplaySurfaceManager()
	{

	}

	bool DisplaySurfaceManager::Create(const boost::any& _rConfig)
	{
		bool bResult = true;
		return bResult;
	}

	void DisplaySurfaceManager::Update()
	{

	}

	void DisplaySurfaceManager::Release()
	{
		UnloadAll();
	}

	bool DisplaySurfaceManager::Load(const string& _strName, const string& _strPath)
	{
		bool bResult = (NULL == Get(_strName));

		if (false != bResult)
		{
			DisplaySurface::CreateInfo oDTCInfo = { _strName, _strPath };
			DisplaySurfacePtr pSurface = new DisplaySurface(m_rDisplay);
			bResult = pSurface->Create(boost::any(&oDTCInfo));
			if (false != bResult)
			{
				const Key uKey = MakeKey(_strName);
				m_mSurfaces[uKey] = pSurface;
			}
			else if (NULL != pSurface)
			{
				pSurface->Release();
				delete pSurface;
			}
		}

		return bResult;
	}

	void DisplaySurfaceManager::Unload(const string& _strName)
	{
		const Key uKey = MakeKey(_strName);
		DisplaySurfacePtrMap::iterator iPair = m_mSurfaces.find(uKey);
		if (m_mSurfaces.end() != iPair)
		{
			DisplaySurfacePtr pSurface = iPair->second;
			pSurface->Release();
			delete pSurface;
			m_mSurfaces.erase(iPair);
		}
	}

	DisplaySurfacePtr DisplaySurfaceManager::Get(const string& _strName)
	{
		const Key uKey = MakeKey(_strName);
		DisplaySurfacePtrMap::iterator iPair = m_mSurfaces.find(uKey);
		DisplaySurfacePtr pResult = (m_mSurfaces.end() != iPair) ? iPair->second : NULL;
		return pResult;
	}

	void DisplaySurfaceManager::UnloadAll()
	{
		DisplaySurfacePtrMap::iterator iPair = m_mSurfaces.begin();
		DisplaySurfacePtrMap::iterator iEnd = m_mSurfaces.end();

		while (iEnd != iPair)
		{
			DisplaySurfacePtr pSurface = iPair->second;
			pSurface->Release();
			delete pSurface;
			++iPair;
		}

		m_mSurfaces.clear();
	}

}
