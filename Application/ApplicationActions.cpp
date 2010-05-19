#include "stdafx.h"
#include "../Core/Core.h"
#include "../Application/Application.h"
#include "../Application/ApplicationActions.h"
#include "../Application/ActionKeybinding.h"
#include "../Application/ApplicationJob.h"
#include "../Application/Scene.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	bool Application::CreateActions()
	{
		LuaObject lActions = Scripting::Lua::GetGlobalTable(string("Actions"), true);
		ActionKeybindingManager::CreateInfo oAKMCInfo = { m_aKeysInfo, m_aKeysInfoOld, &m_oMouseInfo, &m_oMouseInfoOld };
		m_pKeybinds = new ActionKeybindingManager;
		m_pActionDispatcher = new ActionDispatcher;
		bool bResult = (false == lActions.IsNil())
			&& ActionKeybindingManager::InitScripting()
			&& m_pKeybinds->Create(boost::any(&oAKMCInfo))
			&& m_pActionDispatcher->Create(boost::any(0));

		if (false != bResult)
		{
			struct RegisterAction
			{
				RegisterAction(LuaObjectRef _rlTable, ActionDispatcherPtr _pActionDispatcher, const Key _uProcessAction, ActionCallbackFunc _pActionCallback)
				:	m_rlTable(_rlTable),
					m_pActionDispatcher(_pActionDispatcher),
					m_uProcessAction(_uProcessAction),
					m_pActionCallback(_pActionCallback),
					m_bResult(true)
				{

				}
				void Process(const CharPtr _pName, const int _sValue)
				{
					m_rlTable.SetInteger(_pName, _sValue);
					m_bResult &= m_pActionDispatcher->RegisterActionCallback(_sValue, m_uProcessAction, m_pActionCallback);
				}

				LuaObjectRef		m_rlTable;
				ActionDispatcherPtr	m_pActionDispatcher;
				Key					m_uProcessAction;
				ActionCallbackFunc	m_pActionCallback;
				bool				m_bResult;
			};
			RegisterAction funcRegister(lActions, m_pActionDispatcher, m_uProcessAction, m_pActionCallback);

			#define REGISTER_ACTION(ACTION) funcRegister.Process(#ACTION, EAppAction_##ACTION)

			REGISTER_ACTION(CANCEL);
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
			REGISTER_ACTION(ENTITY_CREATE);
			REGISTER_ACTION(POINTERCLICK1);
			REGISTER_ACTION(POINTERCLICK2);
			REGISTER_ACTION(POINTERCLICK3);
			REGISTER_ACTION(POINTERCLICK4);
			REGISTER_ACTION(POINTERCLICK5);
			REGISTER_ACTION(POINTERCLICK6);
			REGISTER_ACTION(POINTERCLICK7);
			REGISTER_ACTION(POINTERCLICK8);
			REGISTER_ACTION(POINTERMOVEX);
			REGISTER_ACTION(POINTERMOVEY);
			REGISTER_ACTION(POINTERMOVEZ);

			#undef REGISTER_ACTION

			bResult = funcRegister.m_bResult;

			if (false != bResult)
			{
				m_uPendingAction = EAppAction_UNKNOWN;
				m_pActionDispatcher->SetBindings(m_pKeybinds);
				bResult = m_pKeybinds->LoadBindings(MakeKey(string("default")), "Data/keybinds.lua", true);
			}
			else
			{
				vsoutput(__FUNCTION__" : could not register one or more action.\n");
			}
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
				m_pSelectedEntity = NULL;
				m_uPendingAction = _uActionID;
				m_pScene->ActivatePicking(true);
				m_pActionDispatcher->UnregisterActionCallback(_uActionID, m_uProcessAction);
				//if ((NULL == m_pOneJob) && (NULL != m_pJobManager))
				//{
				//	m_pOneJob = new AppTestJob(*m_pJobManager);
				//	if (false != m_pOneJob->Create(boost::any(0)))
				//	{
				//		m_pJobManager->PushJob(m_pOneJob);
				//	}
				//}
				break;
			}
			case EAppAction_ENTITY_CREATE:
			{
				m_uPendingAction = _uActionID;
				m_pScene->ActivatePicking(true);
				m_pActionDispatcher->UnregisterActionCallback(_uActionID, m_uProcessAction);
				break;
			}
			default:
			{
				ProcessPendingAction(_uActionID);
				break;
			}
		}
	}

	void Application::ProcessPendingAction(UInt _uActionID)
	{
		switch (m_uPendingAction)
		{
			case EAppAction_ENTITY_CREATE:
			{
				switch (_uActionID)
				{
					case EAppAction_POINTERCLICK1:
					{
						if (false != m_pScene->GetPicker().IsPickOnWater())
						{
							m_pScene->NewDummy();
							m_pScene->ActivatePicking(false);
							m_pActionDispatcher->RegisterActionCallback(m_uPendingAction, m_uProcessAction, m_pActionCallback);
							m_uPendingAction = EAppAction_UNKNOWN;
						}
						break;
					}
					case EAppAction_CANCEL:
					{
						m_pScene->ActivatePicking(false);
						m_pActionDispatcher->RegisterActionCallback(m_uPendingAction, m_uProcessAction, m_pActionCallback);
						m_uPendingAction = EAppAction_UNKNOWN;
						break;
					}
				}
				break;
			}
			case EAppAction_PATH_CREATE:
			{
				switch (_uActionID)
				{
					case EAppAction_POINTERCLICK1:
					{
						if (NULL == m_pSelectedEntity)
						{
							m_pSelectedEntity = m_pScene->GetSelectedEntity();
						}
						else if (false != m_pScene->GetPicker().IsPickOnWater())
						{
							m_pScene->ActivatePicking(false);
							m_pActionDispatcher->RegisterActionCallback(m_uPendingAction, m_uProcessAction, m_pActionCallback);
							m_uPendingAction = EAppAction_UNKNOWN;
						}
						break;
					}
					case EAppAction_CANCEL:
					{
						m_pScene->ActivatePicking(false);
						m_pActionDispatcher->RegisterActionCallback(m_uPendingAction, m_uProcessAction, m_pActionCallback);
						m_uPendingAction = EAppAction_UNKNOWN;
						break;
					}
				}
				break;
			}
			case EAppAction_UNKNOWN:
			{
				switch (_uActionID)
				{
					case EAppAction_CANCEL:
					{
						m_eStateMode = EStateMode_QUIT;
						break;
					}
				}
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