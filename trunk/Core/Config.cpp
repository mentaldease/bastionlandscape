#include "stdafx.h"
#include "../Core/Config.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Config::Config()
	:	CoreObject(),
		m_pConfig(NULL)
	{

	}

	Config::~Config()
	{

	}

	bool Config::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = false;

		m_pConfig = new LibConfig;
		try
		{
			m_pConfig->readFile(pInfo->m_strPath.c_str());
			bResult = true;
		}
		catch (libconfig::ParseException* pException)
		{
			string strError = pException->getError();
			bResult = false;
			delete m_pConfig;
			m_pConfig = NULL;
		}

		return bResult;
	}

	void Config::Update()
	{

	}

	void Config::Release()
	{
		if (NULL != m_pConfig)
		{
			delete m_pConfig;
			m_pConfig = NULL;
		}
	}

	int Config::GetCount(const string& _strPath)
	{
		int sResult = 0;
		if (false != m_pConfig->exists(_strPath))
		{
			sResult = m_pConfig->lookup(_strPath).getLength();
		}
		return sResult;
	}

	ConfigShortcutPtr Config::GetShortcut(const string& _strPath)
	{
		ConfigShortcutPtr pResult = NULL;
		if (false != m_pConfig->exists(_strPath))
		{
			pResult = &(m_pConfig->lookup(_strPath));
		}
		return pResult;
	}
}
