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
		const static UInt c_uMaxGBuffers = 32;
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
		UInt			m_aDXGBufferFormat[c_uMaxGBuffers];
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct CoreCommand
	{
		CoreCommand();

		bool AddArg(VoidPtr _pArg);

		static const UInt s_uMaxArgs = 13;
		VoidPtr			m_pArgs[s_uMaxArgs];
		CoreObjectPtr	m_pObject;
		UInt			m_uID;
		UInt			m_uCount;
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

		virtual bool UpdateRecord();
		virtual bool RecordCommand(CoreCommandPtr _pCommand);
		virtual bool ReplayCommand(CoreCommandRef _pCommand);

		inline void IncRef() { ++m_uRefCount; };
		inline void DecRef() { --m_uRefCount; };
		inline UInt GetRef() { return m_uRefCount; };

		static CoreObjectPtrCounterMap s_mObjects;

	protected:
		UInt	m_uRefCount;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	template <typename T>
	class WeakSingleton
	{
	public:
		WeakSingleton() {};
		virtual ~WeakSingleton() {};

		inline static void SetInstance(T* _pInstance) { s_pInstance = _pInstance; }
		inline static T* GetInstance() { return s_pInstance; }

	protected:
		static T* s_pInstance;
	};

	template <typename T>
	T* WeakSingleton<T>::s_pInstance = NULL;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	#define DECLARE_WEAKSINGLETON(TypeName) \
	protected: \
		static TypeName* s_pInstance; \
	public: \
		static void SetInstance(TypeName* _pInstance); \
		static TypeName* GetInstance();


	#define DEFINE_WEAKSINGLETON(TypeName) \
		TypeName* TypeName::s_pInstance = NULL; \
		void TypeName::SetInstance(TypeName* _pInstance) \
		{ \
			s_pInstance = _pInstance; \
		} \
		TypeName* TypeName::GetInstance() \
		{ \
			return s_pInstance; \
		}
}

#endif // __CORE_H__
