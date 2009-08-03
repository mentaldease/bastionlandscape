#ifndef __CORE_H__
#define __CORE_H__

#include <Windows.h>
#include <boost/any.hpp>

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	typedef std::size_t Key;

	template<typename T>
	inline Key MakeKey(const T& _rData)
	{
		boost::hash<T> T_hash;
		return T_hash(_rData);
	}

	template<typename T>
	inline Key MakeKey(const T* _pData)
	{
		boost::hash<T> T_hash;
		return T_hash(*_pData);
	}

	class CoreObject;
	typedef CoreObject* CoreObjectPtr;
	typedef CoreObject& CoreObjectRef;
	typedef vector<CoreObjectPtr> CoreObjectPtrVec;
	typedef map<CoreObjectPtr, int> CoreObjectPtrMap;

	typedef void*			VoidPtr;
	typedef unsigned int*	UIntPtr;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct WindowData
	{
		WindowData();

		HWND		(*m_pCreateWindow)(WindowData& _rConfig);
		HINSTANCE	m_hInstance;
		HINSTANCE	m_hPrevInstance;
		LPTSTR		m_lpCmdLine;
		int			m_nCmdShow;
		HACCEL		m_hAccelTable;
		HWND		m_hWnd;
		RECT		m_oClientRect;
		RECT		m_oWindowRect;
		int			m_sColorMode;
		bool		m_bFullScreen;
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

		static CoreObjectPtrMap s_mObjects;

	protected:
	private:
	};
}

#endif // __CORE_H__
