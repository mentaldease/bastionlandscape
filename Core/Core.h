#ifndef __CORE_H__
#define __CORE_H__

#include <Windows.h>
#include <boost/any.hpp>

#include "../Core/CoreTypes.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct WindowData
	{
		WindowData();

		HWND			(*m_pCreateWindow)(WindowData& _rConfig);
		HINSTANCE		m_hInstance;
		HINSTANCE		m_hPrevInstance;
		LPTSTR			m_lpCmdLine;
		int				m_nCmdShow;
		HACCEL			m_hAccelTable;
		HWND			m_hWnd;
		RECT			m_oClientRect;
		RECT			m_oWindowRect;
		int				m_sColorMode;
		bool			m_bFullScreen;
		unsigned int	m_uDXColorFormat;
		unsigned int	m_uDXDepthFormat;
		unsigned int	m_uDXGBufferFormat;
		unsigned int	m_uDXGBufferCount;
		float			m_fZNear;
		float			m_fZFar;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class CoreObject
	{
	public:
		CoreObject();
		virtual ~CoreObject();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		static CoreObjectPtrCounterMap s_mObjects;

	protected:
	private:
	};
}

#endif // __CORE_H__
