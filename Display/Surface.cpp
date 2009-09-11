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
		m_oInfo(),
		m_oLockedRect(),
		m_rDisplay(_rDisplay),
		m_pSurface(NULL)
	{
		memset(&m_oLockedRect, 0, sizeof(LockedRect));
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
				#pragma message(__FUNCTION__" : for better support some image formats must be converted into supported surface format. For example luminance image should be translated into paletted surface.")
				bResult = SUCCEEDED(m_rDisplay.GetDevicePtr()->CreateOffscreenPlainSurface(m_oInfo.Width, m_oInfo.Height, m_oInfo.Format, D3DPOOL_DEFAULT, &m_pSurface, NULL));
			}

			if (false != bResult)
			{
				bResult = SUCCEEDED(D3DXLoadSurfaceFromFileInMemory(m_pSurface, NULL, NULL, pBuffer, sSize, NULL, D3DX_FILTER_NONE, 0xff000000, NULL));
			}

			if (false != bResult)
			{
				m_uBPP = m_rDisplay.GetFormatBitsPerPixel(m_oInfo.Format);
				bResult = (0 != m_uBPP);
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

	ImageInfoRef DisplaySurface::GetInfo()
	{
		return m_oInfo;
	}

	VoidPtr DisplaySurface::Lock(const bool& _bReadOnly)
	{
		VoidPtr pResult = NULL;

		if ((NULL != m_pSurface) && (SUCCEEDED(m_pSurface->LockRect(&m_oLockedRect, NULL, _bReadOnly ? D3DLOCK_READONLY : 0))))
		{
			pResult = m_oLockedRect.pBits;
		}

		return pResult;
	}

	VoidPtr DisplaySurface::GetDataXY(const unsigned int& _uX, const unsigned int& _uY)
	{
		VoidPtr pResult = NULL;

		if ((NULL != m_pSurface) && (NULL != m_oLockedRect.pBits))
		{
			const unsigned int uBPPByte = m_uBPP / 8;
			const unsigned int uOffset = _uX * uBPPByte + _uY * m_oLockedRect.Pitch;
			pResult = &((BytePtr)m_oLockedRect.pBits)[uOffset];
		}

		return pResult;
	}

	VoidPtr DisplaySurface::GetDataUV(const float& _fU, const float& _fV)
	{
		VoidPtr pResult = NULL;

		if ((NULL != m_pSurface) && (NULL != m_oLockedRect.pBits)
			&& (0.0f <= _fU) && (1.0f >= _fU)
			&& (0.0f <= _fV) && (1.0f >= _fV))
		{
			const unsigned int uBPPByte = m_uBPP / 8;
			const float fX = (m_oInfo.Width - 1) * _fU;
			const float fY = (m_oInfo.Height - 1) * _fV;
			const unsigned int uMinX = static_cast<unsigned int>(fX);
			const unsigned int uMinY = static_cast<unsigned int>(fY);
			const unsigned int uOffset = uMinX * uBPPByte + uMinY * m_oLockedRect.Pitch;
			pResult = &((BytePtr)m_oLockedRect.pBits)[uOffset];
		}

		return pResult;
	}

	bool DisplaySurface::GetDataUV(const float& _fU, const float& _fV, UVInfo& _rInfo)
	{
		bool bResult = false;

		if ((NULL != m_pSurface) && (NULL != m_oLockedRect.pBits)
			&& (0.0f <= _fU) && (1.0f >= _fU)
			&& (0.0f <= _fV) && (1.0f >= _fV))
		{
			const unsigned int uBPPByte = m_uBPP / 8;
			const float fX = (m_oInfo.Width - 1) * _fU;
			const float fY = (m_oInfo.Height - 1) * _fV;
			const unsigned int uMinX = static_cast<unsigned int>(fX);
			const unsigned int uMinY = static_cast<unsigned int>(fY);
			const unsigned int uOffset = uMinX * uBPPByte + uMinY * m_oLockedRect.Pitch;
			_rInfo.m_aData[EUVInfoData_TOPLEFT] = &((BytePtr)m_oLockedRect.pBits)[uOffset];
			_rInfo.m_aData[EUVInfoData_TOPRIGHT] = (1.0f != _fU) ? &((BytePtr)m_oLockedRect.pBits)[uOffset + uBPPByte] : _rInfo.m_aData[EUVInfoData_TOPLEFT];
			_rInfo.m_aData[EUVInfoData_BOTTOMLEFT] = (1.0f != _fV) ? &((BytePtr)m_oLockedRect.pBits)[uOffset + m_oLockedRect.Pitch] : _rInfo.m_aData[EUVInfoData_TOPLEFT];
			_rInfo.m_aData[EUVInfoData_BOTTOMRIGHT] = ((1.0f != _fV) && (1.0f != _fV)) ? &((BytePtr)m_oLockedRect.pBits)[uOffset + uBPPByte + m_oLockedRect.Pitch] : _rInfo.m_aData[EUVInfoData_BOTTOMLEFT];
			_rInfo.m_fLocalU = fX - uMinX;
			_rInfo.m_fLocalV = fY - uMinY;
			bResult = true;
		}

		return bResult;
	}

	void DisplaySurface::Unlock()
	{
		if ((NULL != m_pSurface) && (NULL != m_oLockedRect.pBits))
		{
			m_pSurface->UnlockRect();
			memset(&m_oLockedRect, 0, sizeof(LockedRect));
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
