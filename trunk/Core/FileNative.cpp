#include "stdafx.h"
#include "../Core/FileNative.h"

#pragma warning(push)
#pragma warning(disable : 4996)

#include <stdio.h>

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	FileNative::FileNative()
	:	File(),
		m_pFile(NULL)
	{

	}

	FileNative::~FileNative()
	{

	}

	bool FileNative::Create(const boost::any& _rConfig)
	{
		FileNative::CreateInfoPtr pInfo = boost::any_cast<FileNative::CreateInfoPtr>(_rConfig);
		bool bResult = (NULL != pInfo);

		Release();

		if (false != bResult)
		{
			switch (pInfo->m_eOpenMode)
			{
				case FS::EOpenMode_READTEXT:
				{
					m_pFile = fopen(pInfo->m_strPath.c_str(), "rt");
					break;
				}
				case FS::EOpenMode_READBINARY:
				{
					m_pFile = fopen(pInfo->m_strPath.c_str(), "rb");
					break;
				}
				case FS::EOpenMode_CREATETEXT:
				{
					m_pFile = fopen(pInfo->m_strPath.c_str(), "wt");
					break;
				}
				case FS::EOpenMode_CREATEBINARY:
				{
					m_pFile = fopen(pInfo->m_strPath.c_str(), "wb");
					break;
				}
			}
			bResult = (NULL != m_pFile);
		}

		return bResult;
	}

	void FileNative::Update()
	{

	}

	void FileNative::Release()
	{
		if (NULL != m_pFile)
		{
			fclose(m_pFile);
			m_pFile = NULL;
		}
	}

	int FileNative::Size()
	{
		const int sCursor = ftell(m_pFile);
		fseek(m_pFile, 0, SEEK_END);
		int sResult = ftell(m_pFile);
		fseek(m_pFile, 0, SEEK_SET);
		sResult -= ftell(m_pFile);
		fseek(m_pFile, sCursor, SEEK_SET);
		return sResult;
	}

	int FileNative::Tell()
	{
		int sResult = ftell(m_pFile);
		return sResult;
	}

	int FileNative::Seek(const int& _sOffset, const File::ESeekMode& _eMode)
	{
		const int sSeek = (ESeekMode_SET == _eMode) ? SEEK_SET : (ESeekMode_END == _eMode) ? SEEK_END : SEEK_CUR;
		const int sResult = fseek(m_pFile, _sOffset, SEEK_SET);
		return sResult;
	}

	unsigned int FileNative::Read(VoidPtr _pBuffer, const int& _sBytes)
	{
		const unsigned int sResult = (unsigned int)fread(_pBuffer, 1, _sBytes, m_pFile);
		return sResult;
	}

	unsigned int FileNative::Write(const VoidPtr _pBuffer, const int& _sBytes)
	{
		const unsigned int sResult = (unsigned int)fwrite(_pBuffer, 1, _sBytes, m_pFile);
		return sResult;
	}

	bool FileNative::EndOfFile()
	{
		return (0 != feof(m_pFile));
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	FSNative::FSNative()
	:	FS()
	{

	}

	FSNative::~FSNative()
	{

	}

	bool FSNative::Create(const boost::any& _rConfig)
	{
		Release();
		return true;
	}

	void FSNative::Update()
	{

	}

	void FSNative::Release()
	{

	}

	FilePtr FSNative::OpenFile(const string& _strPath, const EOpenMode& _eMode)
	{
		FileNative::CreateInfo oFNCInfo = { _strPath, _eMode };
		FilePtr pResult = new FileNative;

		if (false == pResult->Create(boost::any(&oFNCInfo)))
		{
			CloseFile(pResult);
			pResult = NULL;
		}

		return pResult;
	}

	void FSNative::CloseFile(FilePtr _pFile)
	{
		CoreObject::ReleaseDeleteReset(_pFile);
	}
}

#pragma warning(pop)
