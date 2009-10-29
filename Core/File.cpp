#include "stdafx.h"

#include "../Core/File.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	FSPtr FS::s_pRootFS = NULL;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	File::File()
	:	CoreObject()
	{

	}

	File::~File()
	{

	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	FS::FS()
	:	CoreObject(),
		m_vSubFS(),
		m_mSubFS(),
		m_mFiles()
	{
	}

	FS::~FS()
	{

	}

	FSPtr FS::GetRoot()
	{
		return s_pRootFS;
	}

	void FS::SetRoot(FSPtr _pFS)
	{
		s_pRootFS = _pFS;
	}

	bool FS::Create(const boost::any& _rConfig)
	{
		bool bResult = true;
		return bResult;
	}

	void FS::Update()
	{

	}

	void FS::Release()
	{

	}

	FilePtr FS::OpenFile(const string& _strPath, const EOpenMode& _eMode)
	{
		FSPtr pFS = GetFSFromPath(_strPath);
		FilePtr pFile = NULL;

		if (NULL != pFS)
		{
			string strPathWOFS;
			GetPathWithoutFS(_strPath, strPathWOFS);
			pFile = pFS->OpenFile(strPathWOFS, _eMode);
		}
		else
		{
			FSPtrVec::iterator iFS = m_vSubFS.begin();
			FSPtrVec::iterator iEnd = m_vSubFS.end();
			while (iEnd != iFS)
			{
				pFS = *iFS;
				pFile = pFS->OpenFile(_strPath, _eMode);
				if (NULL != pFile)
				{
					break;
				}
				++iFS;
			}
		}

		if (NULL != pFile)
		{
			m_mFiles[pFile] = pFS;
		}

		return pFile;
	}

	void FS::CloseFile(FilePtr _pFile)
	{
		OpenFileMap::iterator iPair = m_mFiles.find(_pFile);
		if (m_mFiles.end() != iPair)
		{
			iPair->second->CloseFile(_pFile);
			m_mFiles.erase(iPair);
		}
	}

	bool FS::AddFS(const string& _strFSName, FSPtr _pFS)
	{
		Key uKey = MakeKey(_strFSName);
		bool bResult = ((NULL == m_mSubFS[uKey]) && (m_vSubFS.end() == find(m_vSubFS.begin(), m_vSubFS.end(), _pFS)));
		if (false != bResult)
		{
			m_mSubFS[uKey] = _pFS;
			m_vSubFS.push_back(_pFS);
		}
		return bResult;
	}

	void FS::RemoveFS(const string& _strFSName, FSPtr _pFS)
	{
		Key uKey = MakeKey(_strFSName);
		FSPtrVec::iterator iFS = find(m_vSubFS.begin(), m_vSubFS.end(), _pFS);
		FSPtrMap::iterator iPair = m_mSubFS.find(uKey);
		bool bResult = ((m_mSubFS.end() != iPair) && (m_vSubFS.end() != iFS));
		if (false != bResult)
		{
			m_mSubFS.erase(iPair);
			m_vSubFS.erase(iFS);
		}
	}

	FSPtr FS::GetFS(const string& _strFSName)
	{
		FSPtr pFS = NULL;
		Key uKey = MakeKey(_strFSName);
		FSPtrMap::iterator iPair = m_mSubFS.find(uKey);

		if (m_mSubFS.end() != iPair)
		{
			pFS = iPair->second;
		}

		return pFS;
	}

	FSPtr FS::GetFSFromPath(const string& _strPath)
	{
		const string::size_type uPos = _strPath.find_first_of(s_FSMarkerInPath);
		FSPtr pFS = NULL;

		if (string::npos != uPos)
		{
			const string strFSName = _strPath.substr(0, uPos);
			pFS = GetFS(strFSName);
		}

		return pFS;
	}

	void FS::GetPathWithoutFS(const string& _strSrcPath, string& _strDstPath)
	{
		const string::size_type uPos = _strSrcPath.find_first_of(s_FSMarkerInPath);
		_strDstPath = _strSrcPath.substr((string::npos != uPos) ? uPos + 1 : 0);
	}

	void FS::GetFileNameWithoutExt(const string& _strSrcPath, string& _strDstPath)
	{
		const string::size_type uWPos = _strSrcPath.find_last_of(s_WDirSeparator);
		const string::size_type uUPos = _strSrcPath.find_last_of(s_UDirSeparator);
		string::size_type uPos = (string::npos == uWPos) ? uUPos : ((string::npos == uUPos) ? uWPos : ((uUPos > uWPos) ? uUPos : uWPos));
		_strDstPath = (string::npos != uPos) ? _strSrcPath.substr(uPos + 1) : _strSrcPath;
		uPos = _strDstPath.find_first_of(s_ExtSeparator);
		_strDstPath = (string::npos != uPos) ? _strDstPath.substr(0, uPos) : _strDstPath;
	}

	void FS::GetFileExt(const string& _strSrcPath, string& _strDstExt)
	{
		const string::size_type uWPos = _strSrcPath.find_last_of(s_WDirSeparator);
		const string::size_type uUPos = _strSrcPath.find_last_of(s_UDirSeparator);
		string::size_type uPos = (string::npos == uWPos) ? uUPos : ((string::npos == uUPos) ? uWPos : ((uUPos > uWPos) ? uUPos : uWPos));
		_strDstExt = (string::npos != uPos) ? _strSrcPath.substr(uPos + 1) : _strSrcPath;
		uPos = _strDstExt.find_first_of(s_ExtSeparator);
		_strDstExt = (string::npos != uPos) ? _strDstExt.substr(uPos + 1) : _strDstExt;
	}
}
