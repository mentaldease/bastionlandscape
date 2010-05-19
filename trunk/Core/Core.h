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

	struct GraphicConfigData;
	typedef GraphicConfigData* GraphicConfigDataPtr;
	typedef GraphicConfigData& GraphicConfigDataRef;

	struct GraphicConfigData
	{
		const static UInt c_uMaxGBuffers = 32;
		GraphicConfigData();

		HWND			(*m_pCreateWindow)(GraphicConfigDataRef _rConfig);
		HINSTANCE		m_hInstance;
		HINSTANCE		m_hPrevInstance;
		LPTSTR			m_lpCmdLine;
		int				m_nCmdShow;
		HACCEL			m_hAccelTable;
		HWND			m_hWnd;
		RECT			m_oClientRect;
		RECT			m_oGraphicConfigRect;
		int				m_sColorMode;
		bool			m_bFullScreen;
		unsigned int	m_uDXColorFormat;
		unsigned int	m_uDXDepthFormat;
		unsigned int	m_uDXGBufferFormat;
		unsigned int	m_uDXGBufferCount;
		float			m_fZNear;
		float			m_fZFar;
		UInt			m_aDXGBufferFormat[c_uMaxGBuffers];
		int				m_sDXGufferDepthIndex;
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

		inline void IncRef() { ++m_uRefCount; };
		inline void DecRef() { --m_uRefCount; };
		inline UInt GetRef() { return m_uRefCount; };

		inline void AddChild(CoreObjectPtr _pChild)
		{
			assert(NULL != _pChild);
			CoreObjectPtrVec::iterator iEnd = m_vChildren.end();
			CoreObjectPtrVec::iterator iChild = find(m_vChildren.begin(), iEnd, _pChild);
			if (iEnd == iChild)
			{
				m_vChildren.push_back(_pChild);
			}
		}

		inline void RemoveChild(CoreObjectPtr _pChild)
		{
			assert(NULL != _pChild);
			CoreObjectPtrVec::iterator iEnd = m_vChildren.end();
			CoreObjectPtrVec::iterator iChild = find(m_vChildren.begin(), iEnd, _pChild);
			if (iEnd != iChild)
			{
				m_vChildren.erase(iChild);
			}
		}

		inline bool IsChild(CoreObjectPtr _pObject)
		{
			CoreObjectPtrVec::iterator iEnd = m_vChildren.end();
			return (iEnd != find(m_vChildren.begin(), iEnd, _pObject));
		}

		inline const CoreObjectPtrVec& GetChildren() { return m_vChildren; }

		static CoreObjectPtrCounterMap s_mObjects;

	protected:
		CoreObjectPtrVec	m_vChildren;
		UInt				m_uRefCount;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Entity : public CoreObject
	{
	public:
		Entity();
		virtual ~Entity();

		virtual void Release();

		virtual void AddComponent(ComponentPtr _pComponent);
		virtual ComponentPtr GetComponent(const Key _uSignature);

	protected:
		ComponentPtrMap	m_mComponents;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Component : public CoreObject
	{
	public:
		Component(EntityRef _rParent);
		virtual ~Component();

		EntityRef GetParent();
		Key GetSignature();
		virtual ComponentPtr GetParentComponent(const Key _uSignature);

	protected:
		EntityRef	m_rParent;
		Key			m_uSignature;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	template<typename T>
	class ComponentFactory : public CoreObject
	{
	public:
		ComponentFactory();
		virtual ~ComponentFactory();

		virtual void Release();

		ComponentPtr New(EntityRef _rParent, const boost::any& _rConfig);
		void Delete(ComponentPtr _pComponent);

	protected:
		vector<ComponentPtr>	m_vComponents;
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

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	template<typename T>
	ComponentFactory<T>::ComponentFactory()
	:	CoreObject(),
		m_vComponents()
	{

	}

	template<typename T>
	ComponentFactory<T>::~ComponentFactory()
	{

	}

	template<typename T>
	void ComponentFactory<T>::Release()
	{
		vector<ComponentPtr>::iterator iComponent = m_vComponents.begin();
		vector<ComponentPtr>::iterator iEnd = m_vComponents.end();
		while (iEnd != iComponent)
		{
			ComponentPtr pComponent = *iComponent;
			pComponent->Release();
			delete pComponent;
			++iComponent;
		}
		m_vComponents.clear();
	}

	template<typename T>
	ComponentPtr ComponentFactory<T>::New(EntityRef _rParent, const boost::any& _rConfig)
	{
		ComponentPtr pComponent = new T(_rParent);
		if (false == pComponent->Create(_rConfig))
		{
			pComponent->Release();
			delete pComponent;
			pComponent = NULL;
		}
		else
		{
			m_vComponents.push_back(pComponent);
		}
		return pComponent;
	}

	template<typename T>
	void ComponentFactory<T>::Delete(ComponentPtr _pComponent)
	{
		vector<ComponentPtr>::iterator iEnd = m_vComponents.end();
		vector<ComponentPtr>::iterator iComponent = find(m_vComponents.begin(), iEnd, _pComponent);
		if (iEnd != iComponent)
		{
			_pComponent->Release();
			delete _pComponent;
			m_vComponents.erase(iComponent);
		}
	}
}

#endif // __CORE_H__
