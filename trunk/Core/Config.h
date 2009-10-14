#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "../Core/Core.h"

#pragma warning(push)
#pragma warning(disable : 4290)
#include <libconfig.h++>
#pragma warning(pop)

typedef libconfig::Config LibConfig;
typedef LibConfig* LibConfigPtr;
typedef LibConfig& LibConfigRef;
typedef libconfig::Setting ConfigShortcut;
typedef ConfigShortcut* ConfigShortcutPtr;
typedef ConfigShortcut& ConfigShortcutRef;

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Config : public CoreObject
	{
	public:
		struct CreateInfo
		{
			string	m_strPath;
		};

	public:
		Config();
		virtual ~Config();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		template<typename T>
		bool GetValue(const string& _strPath, T& _rValue)
		{
			return (NULL != m_pConfig) ? m_pConfig->lookupValue(_strPath, _rValue) : NULL;
		}

		int GetCount(const string& _strPath);
		ConfigShortcutPtr GetShortcut(const string& _strPath);

		template<typename T>
		bool GetValue(ConfigShortcutPtr _pShorcut, const string& _strPath, T& _rValue)
		{
			return (NULL != _pShorcut) ? _pShorcut->lookupValue(_strPath, _rValue) : false;
		}

	protected:
		LibConfigPtr	m_pConfig;

	private:
	};
}

#endif // __CONFIG_H__
