#include "stdafx.h"
#include "../Application/ApplicationIncludes.h"
#include "../Application/ActionKeybinding.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	void ActionKeybindingManager::Context::Clear()
	{
		m_mKeyActions.clear();
		m_mActions.clear();
		m_mActionRights.clear();
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	ActionKeybindingManager::ActionKeybindingManager()
	:	CoreObject(),
		m_mContextes(),
		m_vActions(),
		m_pCurrentContext(NULL),
		m_pKeysInfo(NULL),
		m_pKeysInfoOld(NULL),
		m_pMouseInfo(NULL),
		m_pMouseInfoOld(NULL)
	{

	}

	ActionKeybindingManager::~ActionKeybindingManager()
	{

	}

	bool ActionKeybindingManager::Create(const boost::any& _rConfig)
	{
		CreateInfoPtr pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
		bool bResult = (NULL != pInfo)
			&& (NULL != pInfo->m_pKeysInfo)
			&& (NULL != pInfo->m_pKeysInfoOld)
			&& (NULL != pInfo->m_pMouseInfo)
			&& (NULL != pInfo->m_pMouseInfoOld);

		if (false != bResult)
		{
			m_pKeysInfo = pInfo->m_pKeysInfo;
			m_pKeysInfoOld = pInfo->m_pKeysInfoOld;
			m_pMouseInfo = pInfo->m_pMouseInfo;
			m_pMouseInfoOld = pInfo->m_pMouseInfoOld;
		}
		else
		{
			vsoutput(__FUNCTION__" : missing keyboard and/or mouse info data\n");
		}

		return bResult;
	}

	void ActionKeybindingManager::Update()
	{
		UINT uBaseModifiers = (((0 != m_pKeysInfo[DIK_RSHIFT]) || (0 != m_pKeysInfo[DIK_LSHIFT])) ? s_uShiftModifier : 0)
			+ (((0 != m_pKeysInfo[DIK_RCONTROL]) || (0 != m_pKeysInfo[DIK_LCONTROL])) ? s_uControlModifier : 0)
			+ (((0 != m_pKeysInfo[DIK_RALT]) || (0 != m_pKeysInfo[DIK_LALT])) ? s_uAltModifier : 0);

		m_pCurrentContext->m_mActions.clear();
		m_vActions.clear();

		for (int i = 0 ; 256 > i ; ++i)
		{
			if (((0 != m_pKeysInfoOld[i]) && (0 == m_pKeysInfo[i])) || (0 != m_pKeysInfo[i]))
			{
				const bool bOnce = ((0 != m_pKeysInfoOld[i]) && (0 == m_pKeysInfo[i]));
				const UINT uKey = i + uBaseModifiers + (((0 != m_pKeysInfoOld[i]) && (0 == m_pKeysInfo[i])) ? s_uOnceModifier : 0);
				KeyActionMap::iterator iPair = m_pCurrentContext->m_mKeyActions.find(uKey);
				if (m_pCurrentContext->m_mKeyActions.end() != iPair)
				{
					const UInt uAction = iPair->second;
					const bool bActive = m_pCurrentContext->m_mActionRights[uAction];
					m_pCurrentContext->m_mActions[uAction] = bActive;
					if (false != bActive)
					{
						m_vActions.push_back(uAction);
					}
				}
			}
		}

#if 0
		uBaseModifiers += s_uMouseModifier;
		BYTE* pMouseButtons = &m_pMouseInfo->rgbButtons[0];
		BYTE* pMouseButtonsOld = &m_pMouseInfoOld->rgbButtons[0];

		for (int i = 0 ; 8 > i ; ++i)
		{
			if (((0 != pMouseButtonsOld[i]) && (0 == pMouseButtons[i])) || (0 != pMouseButtons[i]))
			{
				const UINT uKey = DIM_BUTTONLEFT + i + uBaseModifiers + (((0 != pMouseButtonsOld[i]) && (0 == pMouseButtons[i])) ? s_uOnceModifier : 0);
				KeyActionMap::iterator iPair = m_pCurrentContext->m_mKeyActions.find(uKey);
				if (m_pCurrentContext->m_mKeyActions.end() != iPair)
				{
					const UInt uAction = iPair->second;
					const bool bActive = m_pCurrentContext->m_mActionRights[uAction];
					m_pCurrentContext->m_mActions[uAction] = bActive;
					if (false != bActive)
					{
						m_vActions.push_back(uAction);
					}
				}
			}
		}

		LONG aMove[] = { m_pMouseInfo->lX, m_pMouseInfo->lY, m_pMouseInfo->lZ };
		LONG aMoveOld[] = { m_pMouseInfoOld->lX, m_pMouseInfoOld->lY, m_pMouseInfoOld->lZ };
		for (int i = 0 ; 3 > i ; ++i)
		{
			if (((0 != aMoveOld[i]) && (0 == aMove[i])) || (0 != aMove[i]))
			{
				const UINT uKey = DIM_MOVEX + i + uBaseModifiers + (((0 != aMoveOld[i]) && (0 == aMove[i])) ? s_uOnceModifier : 0);
				KeyActionMap::iterator iPair = m_pCurrentContext->m_mKeyActions.find(uKey);
				if (m_pCurrentContext->m_mKeyActions.end() != iPair)
				{
					const UInt uAction = iPair->second;
					const bool bActive = m_pCurrentContext->m_mActionRights[uAction];
					m_pCurrentContext->m_mActions[uAction] = bActive;
					if (false != bActive)
					{
						m_vActions.push_back(uAction);
					}
				}
			}
		}
#endif
	}

	void ActionKeybindingManager::Release()
	{
		ContextPtrMap::iterator iPair = m_mContextes.begin();
		ContextPtrMap::iterator iEnd = m_mContextes.end();
		while (iEnd != iPair)
		{
			delete iPair->second;
			++iPair;
		}
		m_mContextes.clear();
		m_vActions.clear();
	}

	bool ActionKeybindingManager::LoadBindings(const Key _uContextID, const string& _strFileName, const bool _bSetCurrentContext)
	{
		bool bResult = Scripting::Lua::Loadfile(_strFileName);
		LuaObject lBindinds;

		if (false != bResult)
		{
			string strTableName;
			FS::GetFileNameWithoutExt(_strFileName, strTableName);
			strTableName = strtolower(strTableName);
			lBindinds = Scripting::Lua::GetGlobalTable(strTableName.c_str());
			bResult = (false == lBindinds.IsNil());
		}

		if (false != bResult)
		{
			ContextPtr pContext = m_mContextes[_uContextID];
			if (NULL == pContext)
			{
				pContext = new Context;
				m_mContextes[_uContextID] = pContext;
			}
			else
			{
				pContext->Clear();
			}
			for (LuaTableIterator it(lBindinds) ; it ; it.Next())
			{
				const UInt uKey = it.GetKey().GetInteger();
				const UInt uAction = it.GetValue().GetInteger();
				bResult = (pContext->m_mKeyActions.end() == pContext->m_mKeyActions.find(uKey));
				if (false == bResult)
				{
					vsoutput(__FUNCTION__" : colliding bindings\n");
					break;
				}
				pContext->m_mKeyActions[uKey] = uAction;
				pContext->m_mActionRights[uAction] = true;
			}

			if (false != bResult)
			{
				m_pCurrentContext = (false != _bSetCurrentContext) ? pContext : m_pCurrentContext;
			}
		}

		return bResult;
	}

	bool ActionKeybindingManager::SetCurrentContext(const Key _uContextID)
	{
		ContextPtrMap::iterator iPair = m_mContextes.find(_uContextID);
		bool bResult = (m_mContextes.end() != iPair);

		if (false != bResult)
		{
			m_pCurrentContext = iPair->second;
			bResult = (NULL != m_pCurrentContext);
		}

		return bResult;
	}

	bool ActionKeybindingManager::TestAction(const UInt _uAction)
	{
		return m_pCurrentContext->m_mActions[_uAction];
	}

	void ActionKeybindingManager::DisableAction(const UInt _uAction)
	{
		m_pCurrentContext->m_mActionRights[_uAction] = false;
	}

	void ActionKeybindingManager::EnableAction(const UInt _uAction)
	{
		m_pCurrentContext->m_mActionRights[_uAction] = true;
	}

	const UIntVec& ActionKeybindingManager::GetActiveActions() const
	{
		return m_vActions;
	}

	bool ActionKeybindingManager::InitScripting()
	{
		LuaObject lKeys =  Scripting::Lua::GetGlobalTable(string("Keys"), true);
		bool bResult = (false == lKeys.IsNil());

		if (false != bResult)
		{
			struct RegisterKeyLua
			{
				RegisterKeyLua(LuaObjectRef _rlTable)
				:	m_rlTable(_rlTable)
				{

				}
				void Process(const CharPtr _pName, const int _sValue)
				{
					m_rlTable.SetInteger(_pName, _sValue);
				}

				LuaObjectRef	m_rlTable;
			};
			RegisterKeyLua funcRegister(lKeys);

			#define REGISTER_INPUTEVENT(EVENT) funcRegister.Process(#EVENT, EVENT)

			#define SHIFT s_uShiftModifier
			#define CONTROL s_uControlModifier
			#define ALT s_uAltModifier
			#define ONCE s_uOnceModifier

			REGISTER_INPUTEVENT(SHIFT);
			REGISTER_INPUTEVENT(CONTROL);
			REGISTER_INPUTEVENT(ALT);
			REGISTER_INPUTEVENT(ONCE);

			REGISTER_INPUTEVENT(DIK_ESCAPE);
			REGISTER_INPUTEVENT(DIK_1);
			REGISTER_INPUTEVENT(DIK_2);
			REGISTER_INPUTEVENT(DIK_3);
			REGISTER_INPUTEVENT(DIK_4);
			REGISTER_INPUTEVENT(DIK_5);
			REGISTER_INPUTEVENT(DIK_6);
			REGISTER_INPUTEVENT(DIK_7);
			REGISTER_INPUTEVENT(DIK_8);
			REGISTER_INPUTEVENT(DIK_9);
			REGISTER_INPUTEVENT(DIK_0);
			REGISTER_INPUTEVENT(DIK_MINUS);
			REGISTER_INPUTEVENT(DIK_EQUALS);
			REGISTER_INPUTEVENT(DIK_BACK);
			REGISTER_INPUTEVENT(DIK_TAB);
			REGISTER_INPUTEVENT(DIK_Q);
			REGISTER_INPUTEVENT(DIK_W);
			REGISTER_INPUTEVENT(DIK_E);
			REGISTER_INPUTEVENT(DIK_R);
			REGISTER_INPUTEVENT(DIK_T);
			REGISTER_INPUTEVENT(DIK_Y);
			REGISTER_INPUTEVENT(DIK_U);
			REGISTER_INPUTEVENT(DIK_I);
			REGISTER_INPUTEVENT(DIK_O);
			REGISTER_INPUTEVENT(DIK_P);
			REGISTER_INPUTEVENT(DIK_LBRACKET);
			REGISTER_INPUTEVENT(DIK_RBRACKET);
			REGISTER_INPUTEVENT(DIK_RETURN);
			REGISTER_INPUTEVENT(DIK_LCONTROL);
			REGISTER_INPUTEVENT(DIK_A);
			REGISTER_INPUTEVENT(DIK_S);
			REGISTER_INPUTEVENT(DIK_D);
			REGISTER_INPUTEVENT(DIK_F);
			REGISTER_INPUTEVENT(DIK_G);
			REGISTER_INPUTEVENT(DIK_H);
			REGISTER_INPUTEVENT(DIK_J);
			REGISTER_INPUTEVENT(DIK_K);
			REGISTER_INPUTEVENT(DIK_L);
			REGISTER_INPUTEVENT(DIK_SEMICOLON);
			REGISTER_INPUTEVENT(DIK_APOSTROPHE);
			REGISTER_INPUTEVENT(DIK_GRAVE);
			REGISTER_INPUTEVENT(DIK_LSHIFT);
			REGISTER_INPUTEVENT(DIK_BACKSLASH);
			REGISTER_INPUTEVENT(DIK_Z);
			REGISTER_INPUTEVENT(DIK_X);
			REGISTER_INPUTEVENT(DIK_C);
			REGISTER_INPUTEVENT(DIK_V);
			REGISTER_INPUTEVENT(DIK_B);
			REGISTER_INPUTEVENT(DIK_N);
			REGISTER_INPUTEVENT(DIK_M);
			REGISTER_INPUTEVENT(DIK_COMMA);
			REGISTER_INPUTEVENT(DIK_PERIOD);
			REGISTER_INPUTEVENT(DIK_SLASH);
			REGISTER_INPUTEVENT(DIK_RSHIFT);
			REGISTER_INPUTEVENT(DIK_MULTIPLY);
			REGISTER_INPUTEVENT(DIK_LMENU);
			REGISTER_INPUTEVENT(DIK_SPACE);
			REGISTER_INPUTEVENT(DIK_CAPITAL);
			REGISTER_INPUTEVENT(DIK_F1);
			REGISTER_INPUTEVENT(DIK_F2);
			REGISTER_INPUTEVENT(DIK_F3);
			REGISTER_INPUTEVENT(DIK_F4);
			REGISTER_INPUTEVENT(DIK_F5);
			REGISTER_INPUTEVENT(DIK_F6);
			REGISTER_INPUTEVENT(DIK_F7);
			REGISTER_INPUTEVENT(DIK_F8);
			REGISTER_INPUTEVENT(DIK_F9);
			REGISTER_INPUTEVENT(DIK_F10);
			REGISTER_INPUTEVENT(DIK_NUMLOCK);
			REGISTER_INPUTEVENT(DIK_SCROLL);
			REGISTER_INPUTEVENT(DIK_NUMPAD7);
			REGISTER_INPUTEVENT(DIK_NUMPAD8);
			REGISTER_INPUTEVENT(DIK_NUMPAD9);
			REGISTER_INPUTEVENT(DIK_SUBTRACT);
			REGISTER_INPUTEVENT(DIK_NUMPAD4);
			REGISTER_INPUTEVENT(DIK_NUMPAD5);
			REGISTER_INPUTEVENT(DIK_NUMPAD6);
			REGISTER_INPUTEVENT(DIK_ADD);
			REGISTER_INPUTEVENT(DIK_NUMPAD1);
			REGISTER_INPUTEVENT(DIK_NUMPAD2);
			REGISTER_INPUTEVENT(DIK_NUMPAD3);
			REGISTER_INPUTEVENT(DIK_NUMPAD0);
			REGISTER_INPUTEVENT(DIK_DECIMAL);
			REGISTER_INPUTEVENT(DIK_OEM_102);
			REGISTER_INPUTEVENT(DIK_F11);
			REGISTER_INPUTEVENT(DIK_F12);
			REGISTER_INPUTEVENT(DIK_F13);
			REGISTER_INPUTEVENT(DIK_F14);
			REGISTER_INPUTEVENT(DIK_F15);
			REGISTER_INPUTEVENT(DIK_KANA);
			REGISTER_INPUTEVENT(DIK_ABNT_C1);
			REGISTER_INPUTEVENT(DIK_CONVERT);
			REGISTER_INPUTEVENT(DIK_NOCONVERT);
			REGISTER_INPUTEVENT(DIK_YEN);
			REGISTER_INPUTEVENT(DIK_ABNT_C2);
			REGISTER_INPUTEVENT(DIK_NUMPADEQUALS);
			REGISTER_INPUTEVENT(DIK_PREVTRACK);
			REGISTER_INPUTEVENT(DIK_AT);
			REGISTER_INPUTEVENT(DIK_COLON);
			REGISTER_INPUTEVENT(DIK_UNDERLINE);
			REGISTER_INPUTEVENT(DIK_KANJI);
			REGISTER_INPUTEVENT(DIK_STOP);
			REGISTER_INPUTEVENT(DIK_AX);
			REGISTER_INPUTEVENT(DIK_UNLABELED);
			REGISTER_INPUTEVENT(DIK_NEXTTRACK);
			REGISTER_INPUTEVENT(DIK_NUMPADENTER);
			REGISTER_INPUTEVENT(DIK_RCONTROL);
			REGISTER_INPUTEVENT(DIK_MUTE);
			REGISTER_INPUTEVENT(DIK_CALCULATOR);
			REGISTER_INPUTEVENT(DIK_PLAYPAUSE);
			REGISTER_INPUTEVENT(DIK_MEDIASTOP);
			REGISTER_INPUTEVENT(DIK_VOLUMEDOWN);
			REGISTER_INPUTEVENT(DIK_VOLUMEUP);
			REGISTER_INPUTEVENT(DIK_WEBHOME);
			REGISTER_INPUTEVENT(DIK_NUMPADCOMMA);
			REGISTER_INPUTEVENT(DIK_DIVIDE);
			REGISTER_INPUTEVENT(DIK_SYSRQ);
			REGISTER_INPUTEVENT(DIK_RMENU);
			REGISTER_INPUTEVENT(DIK_PAUSE);
			REGISTER_INPUTEVENT(DIK_HOME);
			REGISTER_INPUTEVENT(DIK_UP);
			REGISTER_INPUTEVENT(DIK_PRIOR);
			REGISTER_INPUTEVENT(DIK_LEFT);
			REGISTER_INPUTEVENT(DIK_RIGHT);
			REGISTER_INPUTEVENT(DIK_END);
			REGISTER_INPUTEVENT(DIK_DOWN);
			REGISTER_INPUTEVENT(DIK_NEXT);
			REGISTER_INPUTEVENT(DIK_INSERT);
			REGISTER_INPUTEVENT(DIK_DELETE);
			REGISTER_INPUTEVENT(DIK_LWIN);
			REGISTER_INPUTEVENT(DIK_RWIN);
			REGISTER_INPUTEVENT(DIK_APPS);
			REGISTER_INPUTEVENT(DIK_POWER);
			REGISTER_INPUTEVENT(DIK_SLEEP);
			REGISTER_INPUTEVENT(DIK_WAKE);
			REGISTER_INPUTEVENT(DIK_WEBSEARCH);
			REGISTER_INPUTEVENT(DIK_WEBFAVORITES);
			REGISTER_INPUTEVENT(DIK_WEBREFRESH);
			REGISTER_INPUTEVENT(DIK_WEBSTOP);
			REGISTER_INPUTEVENT(DIK_WEBFORWARD);
			REGISTER_INPUTEVENT(DIK_WEBBACK);
			REGISTER_INPUTEVENT(DIK_MYCOMPUTER);
			REGISTER_INPUTEVENT(DIK_MAIL);
			REGISTER_INPUTEVENT(DIK_MEDIASELECT);
			REGISTER_INPUTEVENT(DIK_BACKSPACE);
			REGISTER_INPUTEVENT(DIK_NUMPADSTAR);
			REGISTER_INPUTEVENT(DIK_LALT);
			REGISTER_INPUTEVENT(DIK_CAPSLOCK);
			REGISTER_INPUTEVENT(DIK_NUMPADMINUS);
			REGISTER_INPUTEVENT(DIK_NUMPADPLUS);
			REGISTER_INPUTEVENT(DIK_NUMPADPERIOD);
			REGISTER_INPUTEVENT(DIK_NUMPADSLASH);
			REGISTER_INPUTEVENT(DIK_RALT);
			REGISTER_INPUTEVENT(DIK_UPARROW);
			REGISTER_INPUTEVENT(DIK_PGUP);
			REGISTER_INPUTEVENT(DIK_LEFTARROW);
			REGISTER_INPUTEVENT(DIK_RIGHTARROW);
			REGISTER_INPUTEVENT(DIK_DOWNARROW);
			REGISTER_INPUTEVENT(DIK_PGDN);
			REGISTER_INPUTEVENT(DIK_CIRCUMFLEX);

			REGISTER_INPUTEVENT(DIM_MOVEX);
			REGISTER_INPUTEVENT(DIM_MOVEY);
			REGISTER_INPUTEVENT(DIM_MOVEZ);
			REGISTER_INPUTEVENT(DIM_BUTTONLEFT);
			REGISTER_INPUTEVENT(DIM_BUTTONRIGHT);
			REGISTER_INPUTEVENT(DIM_BUTTONMIDDLE);
			REGISTER_INPUTEVENT(DIM_BUTTON4);
			REGISTER_INPUTEVENT(DIM_BUTTON5);
			REGISTER_INPUTEVENT(DIM_BUTTON6);
			REGISTER_INPUTEVENT(DIM_BUTTON7);
			REGISTER_INPUTEVENT(DIM_BUTTON8);

			#undef REGISTER_INPUTEVENT
		}

		return bResult;
	}
}
