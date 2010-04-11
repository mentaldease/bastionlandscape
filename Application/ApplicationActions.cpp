#include "stdafx.h"
#include "../Core/Core.h"
#include "../Application/Application.h"
#include "../Application/ApplicationActions.h"
#include "../Application/ActionKeybinding.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	bool Application::CreateActions()
	{
		LuaObject lActions = Scripting::Lua::GetGlobalTable(string("Actions"), true);
		ActionKeybindingManager::CreateInfo oAKMCInfo = { m_aKeysInfo, m_aKeysInfoOld };
		m_pKeybinds = new ActionKeybindingManager;
		m_pActionDispatcher = new ActionDispatcher;
		bool bResult = (false == lActions.IsNil())
			&& ActionKeybindingManager::InitScripting()
			&& m_pKeybinds->Create(boost::any(&oAKMCInfo))
			&& m_pActionDispatcher->Create(boost::any(0));

		if (false != bResult)
		{
			struct RegisterActionLua
			{
				RegisterActionLua(LuaObjectRef _rlTable)
				:	m_rlTable(_rlTable)
				{

				}
				void Process(const CharPtr _pName, const int _sValue)
				{
					m_rlTable.SetInteger(_pName, _sValue);
				}

				LuaObjectRef	m_rlTable;
			};
			RegisterActionLua funcRegister(lActions);

			#define REGISTER_ACTION(ACTION) funcRegister.Process(#ACTION, EAppAction_##ACTION)

			REGISTER_ACTION(CAMERA_INC_SPEED);
			REGISTER_ACTION(CAMERA_DEC_SPEED);
			REGISTER_ACTION(CAMERA_STRAFE_FRONT);
			REGISTER_ACTION(CAMERA_STRAFE_BACK);
			REGISTER_ACTION(CAMERA_MOVE_FRONT);
			REGISTER_ACTION(CAMERA_MOVE_BACK);
			REGISTER_ACTION(CAMERA_MOVE_RIGHT);
			REGISTER_ACTION(CAMERA_MOVE_LEFT);
			REGISTER_ACTION(CAMERA_MOVE_UP);
			REGISTER_ACTION(PATH_CREATE);

			#undef REGISTER_ACTION

			m_pActionDispatcher->SetBindings(m_pKeybinds);

			m_pActionDispatcher->RegisterActionCallback(EAppAction_CAMERA_INC_SPEED, m_uProcessAction, m_pActionCallback);
			m_pActionDispatcher->RegisterActionCallback(EAppAction_CAMERA_DEC_SPEED, m_uProcessAction, m_pActionCallback);
			m_pActionDispatcher->RegisterActionCallback(EAppAction_CAMERA_STRAFE_FRONT, m_uProcessAction, m_pActionCallback);
			m_pActionDispatcher->RegisterActionCallback(EAppAction_CAMERA_STRAFE_BACK, m_uProcessAction, m_pActionCallback);
			m_pActionDispatcher->RegisterActionCallback(EAppAction_CAMERA_MOVE_FRONT, m_uProcessAction, m_pActionCallback);
			m_pActionDispatcher->RegisterActionCallback(EAppAction_CAMERA_MOVE_BACK, m_uProcessAction, m_pActionCallback);
			m_pActionDispatcher->RegisterActionCallback(EAppAction_CAMERA_MOVE_RIGHT, m_uProcessAction, m_pActionCallback);
			m_pActionDispatcher->RegisterActionCallback(EAppAction_CAMERA_MOVE_LEFT, m_uProcessAction, m_pActionCallback);
			m_pActionDispatcher->RegisterActionCallback(EAppAction_CAMERA_MOVE_UP, m_uProcessAction, m_pActionCallback);
			m_pActionDispatcher->RegisterActionCallback(EAppAction_PATH_CREATE, m_uProcessAction, m_pActionCallback);

			bResult = m_pKeybinds->LoadBindings(MakeKey(string("default")), "Data/keybinds.lua", true);
		}

		return bResult;
	}

	void Application::ProcessActions(UInt _uActionID)
	{
		switch (_uActionID)
		{
			case EAppAction_CAMERA_INC_SPEED:
			{
				m_fCameraMoveSpeed *= 2.0f;
				m_oCameraParams.m_fMoveSpeed *= 2.0f;
				break;
			}
			case EAppAction_CAMERA_DEC_SPEED:
			{
				m_fCameraMoveSpeed /= 2.0f;
				m_oCameraParams.m_fMoveSpeed /= 2.0f;
				if (1.0f > m_fCameraMoveSpeed)
				{
					m_fCameraMoveSpeed = 1.0f;
					m_oCameraParams.m_fMoveSpeed = 1.0f;
				}
				break;
			}
			case EAppAction_CAMERA_STRAFE_FRONT:
			{
				m_oCameraParams.m_fFront2 += 1.0f;
				break;
			}
			case EAppAction_CAMERA_STRAFE_BACK:
			{
				m_oCameraParams.m_fFront2 -= 1.0f;
				break;
			}
			case EAppAction_CAMERA_MOVE_FRONT:
			{
				m_oCameraParams.m_fFront += 1.0f;
				break;
			}
			case EAppAction_CAMERA_MOVE_BACK:
			{
				m_oCameraParams.m_fFront -= 1.0f;
				break;
			}
			case EAppAction_CAMERA_MOVE_RIGHT:
			{
				m_oCameraParams.m_fStrafe += 1.0f;
				break;
			}
			case EAppAction_CAMERA_MOVE_LEFT:
			{
				m_oCameraParams.m_fStrafe -= 1.0f;
				break;
			}
			case EAppAction_CAMERA_MOVE_UP:
			{
				m_oCameraParams.m_fUp += 1.0f;
				break;
			}
			case EAppAction_PATH_CREATE:
			{
				m_pActionDispatcher->UnregisterActionCallback(EAppAction_PATH_CREATE, m_uProcessAction);
				break;
			}
		}
	}

	void Application::ReleaseActions()
	{
		if (NULL != m_pActionDispatcher)
		{
			m_pActionDispatcher->Release();
			delete m_pActionDispatcher;
			m_pActionDispatcher = NULL;
		}

		if (NULL != m_pKeybinds)
		{
			m_pKeybinds->Release();
			delete m_pKeybinds;
			m_pKeybinds = NULL;
		}
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	ActionDispatcher::ActionDispatcher()
	:	CoreObject(),
		m_pBindingManager(NULL),
		m_mCallbacks()
	{

	}

	ActionDispatcher::~ActionDispatcher()
	{

	}

	bool ActionDispatcher::Create(const boost::any& _rConfig)
	{
		bool bResult = true;
		return bResult;
	}

	void ActionDispatcher::Update()
	{
		assert(NULL != m_pBindingManager);

		m_pBindingManager->Update();

		const UIntVec& rActions = m_pBindingManager->GetActiveActions();
		UIntVec::const_iterator iAction = rActions.begin();
		UIntVec::const_iterator iEnd = rActions.end();
		ActionCallbackFuncMultiMap::iterator iCallbacksEnd = m_mCallbacks.end();
		while (iEnd != iAction)
		{
			const UInt uAction = *iAction;
			ActionCallbackFuncMultiMap::iterator iPair = m_mCallbacks.find(uAction);
			if (iCallbacksEnd != iPair)
			{
				ActionCallbackFuncMapRef rCallback = iPair->second;
				ActionCallbackFuncMap::iterator iCallback = rCallback.begin();
				ActionCallbackFuncMap::iterator iCallbackEnd = rCallback.end();
				while (iCallbackEnd != iCallback)
				{
					ActionCallbackFunc& pCallback = iCallback->second;
					pCallback(uAction);
					++iCallback;
				}
			}
			++iAction;
		}
	}

	void ActionDispatcher::Release()
	{
		m_pBindingManager = NULL;
		m_mCallbacks.clear();
	}

	void ActionDispatcher::SetBindings(ActionKeybindingManagerPtr _pBindingManager)
	{
		m_pBindingManager = _pBindingManager;
	}

	bool ActionDispatcher::RegisterActionCallback(UInt _uActionID, const Key _uCallbackID, ActionCallbackFunc _pCallback)
	{
		ActionCallbackFuncMapRef rCallback = m_mCallbacks[_uActionID];
		ActionCallbackFuncMap::iterator iPair = rCallback.find(_uCallbackID);
		bool bResult = (rCallback.end() == iPair);

		if (false != bResult)
		{
			rCallback[_uCallbackID] = _pCallback;
		}

		return bResult;
	}

	void ActionDispatcher::UnregisterActionCallback(UInt _uActionID, const Key _uCallbackID)
	{
		ActionCallbackFuncMapRef rCallback = m_mCallbacks[_uActionID];
		ActionCallbackFuncMap::iterator iPair = rCallback.find(_uCallbackID);
		if (rCallback.end() != iPair)
		{
			rCallback.erase(iPair);
		}
	}
}