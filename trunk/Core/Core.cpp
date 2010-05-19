#include "stdafx.h"
#include "Core.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	CoreObjectPtrCounterMap CoreObject::s_mObjects;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	GraphicConfigData::GraphicConfigData()
	:	m_pCreateWindow(NULL),
		m_hInstance(NULL),
		m_hPrevInstance(NULL),
		m_lpCmdLine(NULL),
		m_nCmdShow(0),
		m_hAccelTable(NULL),
		m_hWnd(NULL),
		m_oClientRect(),
		m_sColorMode(0),
		m_bFullScreen(false),
		m_uDXColorFormat(0),
		m_uDXDepthFormat(0),
		m_uDXGBufferFormat(0),
		m_uDXGBufferCount(0),
		m_fZNear(1.0f),
		m_fZFar(100.0f),
		m_sDXGufferDepthIndex(-1)
	{

	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	CoreObject::CoreObject()
	:	m_uRefCount(0)
	{
		//s_mObjects[this] = 1;
	}

	CoreObject::~CoreObject()
	{
		//CoreObjectPtrCounterMap::iterator iPair = s_mObjects.find(this);
		//if (s_mObjects.end() != iPair)
		//{
		//	s_mObjects.erase(iPair);
		//}
	}

	bool CoreObject::Create(const boost::any& _rConfig)
	{
		Release();
		return true;
	}

	void CoreObject::Update()
	{

	}

	void CoreObject::Release()
	{
		m_vChildren.clear();
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Entity::Entity()
	:	CoreObject(),
		m_mComponents()
	{

	}

	Entity::~Entity()
	{

	}

	void Entity::Release()
	{
		m_mComponents.clear();
	}

	void Entity::AddComponent(ComponentPtr _pComponent)
	{
		const Key uSignature = _pComponent->GetSignature();
		if (m_mComponents.end() == m_mComponents.find(uSignature))
		{
			m_mComponents[uSignature] = _pComponent;
		}
	}

	ComponentPtr Entity::GetComponent(const Key _uSignature)
	{
		ComponentPtrMap::iterator iPair = m_mComponents.find(_uSignature);
		return ((m_mComponents.end() != iPair) ? iPair->second : NULL);
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Component::Component(EntityRef _rParent)
	:	CoreObject(),
		m_rParent(_rParent),
		m_uSignature(0)
	{

	}

	Component::~Component()
	{

	}

	EntityRef Component::GetParent()
	{
		return m_rParent;
	}

	Key Component::GetSignature()
	{
		return m_uSignature;
	}

	ComponentPtr Component::GetParentComponent(const Key _uSignature)
	{
		return m_rParent.GetComponent(_uSignature);
	}
}
