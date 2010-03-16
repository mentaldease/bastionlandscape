#include "stdafx.h"
#include "../Core/FileMemory.h"

#pragma warning(push)
#pragma warning(disable : 4996)

#include <stdio.h>

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	FileMemory::FileMemory()
	:	File(),
		m_strPath(),
		m_eOpenMode(FS::EOpenMode_UNKNOWN),
		m_pFile(NULL),
		m_pBuffer(NULL),
		m_sMaxSize(0),
		m_sCurrentSize(0),
		m_sCurrentPos(0),
		m_sWriteCount(0)
	{

	}

	FileMemory::~FileMemory()
	{

	}

	bool FileMemory::Create(const boost::any& _rConfig)
	{
		FileMemory::CreateInfoPtr pInfo = boost::any_cast<FileMemory::CreateInfoPtr>(_rConfig);
		bool bResult = (NULL != pInfo);

		Release();

		if (false != bResult)
		{
			m_strPath = pInfo->m_strPath;
			m_eOpenMode = pInfo->m_eOpenMode;

			switch (m_eOpenMode)
			{
				case FS::EOpenMode_READTEXT:
				case FS::EOpenMode_READBINARY:
				{
					m_pFile = FS::GetRoot()->OpenFile(pInfo->m_strPath.c_str(), m_eOpenMode);
					bResult = (NULL != m_pFile);
					if (false != bResult)
					{
						m_sCurrentSize = m_pFile->Size();
						m_sMaxSize = s_sBlockSize * (1 + (m_sCurrentSize / s_sBlockSize));
						m_pBuffer = new Byte[m_sMaxSize];
						m_sCurrentSize = m_pFile->Read(m_pBuffer, m_sCurrentSize);
						FS::GetRoot()->CloseFile(m_pFile);
						m_pFile = NULL;
					}
					break;
				}
				case FS::EOpenMode_CREATETEXT:
				case FS::EOpenMode_CREATEBINARY:
				{
					bResult = true;
					break;
				}
			}
		}

		return bResult;
	}

	void FileMemory::Update()
	{

	}

	void FileMemory::Release()
	{
		if (NULL != m_pFile)
		{
			FS::GetRoot()->CloseFile(m_pFile);
			m_pFile = NULL;
		}

		if (NULL != m_pBuffer)
		{
			if (0 < m_sWriteCount)
			{
				const FS::EOpenMode eOpenMode = FS::EOpenMode((m_eOpenMode & ~FS::EOpenMode_READ) | FS::EOpenMode_WRITE);
				m_pFile = FS::GetRoot()->OpenFile(m_strPath, eOpenMode);
				if (NULL != m_pFile)
				{
					m_pFile->Write(m_pBuffer, m_sCurrentSize);
					FS::GetRoot()->CloseFile(m_pFile);
					m_pFile = NULL;
				}
			}

			delete[] m_pBuffer;
			m_pBuffer = NULL;
		}

		m_sMaxSize = 0;
		m_sCurrentSize = 0;
		m_sCurrentPos = 0;
		m_sWriteCount = 0;
	}

	int FileMemory::Size()
	{
		return m_sCurrentSize;
	}

	int FileMemory::Tell()
	{
		return m_sCurrentPos;
	}

	int FileMemory::Seek(const int& _sOffset, const File::ESeekMode& _eMode)
	{
		switch (_eMode)
		{
			case ESeekMode_SET:
			{
				m_sCurrentPos = _sOffset;
				break;
			}
			case ESeekMode_END:
			{
				m_sCurrentPos = m_sCurrentSize + _sOffset;
				break;
			}
			case ESeekMode_CUR:
			{
				m_sCurrentPos += _sOffset;
				break;
			}
		}
		return 0;
	}

	unsigned int FileMemory::Read(VoidPtr _pBuffer, const int& _sBytes)
	{
		if ((0 <= m_sCurrentPos) && (0 < _sBytes))
		{
			const int sDeltaToEOF = m_sCurrentSize - m_sCurrentPos;
			const int sDelta = m_sCurrentSize - (m_sCurrentPos + _sBytes);
			const int sResult = (0 <= sDelta) ? _sBytes : ((0 <= sDeltaToEOF) ? sDeltaToEOF : 0);
			memcpy_s(_pBuffer, _sBytes, m_pBuffer + m_sCurrentPos, sResult);
			m_sCurrentPos += sResult;
			return sResult;
		}
		return 0;
	}

	unsigned int FileMemory::Write(const VoidPtr _pBuffer, const int& _sBytes)
	{
		if ((0 <= m_sCurrentPos) && (0 < _sBytes))
		{
			const int sNewCurrentPos = m_sCurrentPos + _sBytes;
			const int sNewCurrentSize = (m_sCurrentSize < sNewCurrentPos) ? sNewCurrentPos : m_sCurrentSize;
			const int sNewMaxSize = (m_sMaxSize < sNewCurrentSize) ? m_sMaxSize + s_sBlockSize * (1 + (_sBytes / s_sBlockSize)) : m_sMaxSize;

			if (sNewMaxSize > m_sMaxSize)
			{
				BytePtr pNewBuffer = new Byte[sNewMaxSize];
				if (NULL != m_pBuffer)
				{
					memcpy_s(pNewBuffer, sNewMaxSize, m_pBuffer, m_sCurrentSize);
					delete[] m_pBuffer;
				}
				m_pBuffer = pNewBuffer;
				m_sMaxSize = sNewMaxSize;
			}

			memcpy_s(m_pBuffer + m_sCurrentPos, _sBytes, _pBuffer, _sBytes);
			m_sCurrentPos = sNewCurrentPos;
			m_sCurrentSize = sNewCurrentSize;
			++m_sWriteCount;

			return _sBytes;
		}
		return 0;
	}

	bool FileMemory::EndOfFile()
	{
		return (m_sCurrentPos >= m_sCurrentSize);
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	FSMemory::FSMemory()
	:	FS()
	{

	}

	FSMemory::~FSMemory()
	{

	}

	bool FSMemory::Create(const boost::any& _rConfig)
	{
		Release();
		return true;
	}

	void FSMemory::Update()
	{

	}

	void FSMemory::Release()
	{

	}

	FilePtr FSMemory::OpenFile(const string& _strPath, const EOpenMode& _eMode)
	{
		FileMemory::CreateInfo oFMCInfo = { _strPath, _eMode };
		FilePtr pResult = new FileMemory;

		if (false == pResult->Create(boost::any(&oFMCInfo)))
		{
			CloseFile(pResult);
			pResult = NULL;
		}

		return pResult;
	}

	void FSMemory::CloseFile(FilePtr _pFile)
	{
		_pFile->Release();
		delete _pFile;
	}
}

#pragma warning(pop)
