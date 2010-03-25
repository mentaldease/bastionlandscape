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

	WindowData::WindowData()
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
		m_fZFar(100.0f)
	{

	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	CoreCommand::CoreCommand()
	:	m_pObject(NULL),
		m_uID(0),
		m_uCount(0)
	{

	}

	bool CoreCommand::AddArg(VoidPtr _pArg)
	{
		bool bResult = (s_uMaxArgs >  m_uCount);
		if (false != bResult)
		{
			m_pArgs[m_uCount] = _pArg;
			++m_uCount;
		}
		return bResult;
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

	}

	bool CoreObject::UpdateRecord()
	{
		return false;
	}

	bool CoreObject::RecordCommand(CoreCommandPtr _pCommand)
	{
		return false;
	}

	bool CoreObject::ReplayCommand(CoreCommandRef _pCommand)
	{
		return false;
	}

}
