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
		m_pCurrentContext(NULL),
		m_pKeysInfo(NULL),
		m_pKeysInfoOld(NULL)
	{

	}

	ActionKeybindingManager::~ActionKeybindingManager()
	{

	}

	bool ActionKeybindingManager::Create(const boost::any& _rConfig)
	{
		CreateInfoPtr pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
		bool bResult = (NULL != pInfo) && (NULL != pInfo->m_pKeysInfo) && (NULL != pInfo->m_pKeysInfoOld);

		if (false != bResult)
		{
			m_pKeysInfo = pInfo->m_pKeysInfo;
			m_pKeysInfoOld = pInfo->m_pKeysInfoOld;
		}

		return bResult;
	}

	void ActionKeybindingManager::Update()
	{
		const UINT uBaseModifiers = (((0 != m_pKeysInfo[DIK_RSHIFT]) || (0 != m_pKeysInfo[DIK_LSHIFT])) ? s_uShiftModifier : 0)
			+ (((0 != m_pKeysInfo[DIK_RCONTROL]) || (0 != m_pKeysInfo[DIK_LCONTROL])) ? s_uControlModifier : 0)
			+ (((0 != m_pKeysInfo[DIK_RALT]) || (0 != m_pKeysInfo[DIK_LALT])) ? s_uAltModifier : 0);

		m_pCurrentContext->m_mActions.clear();

		for (int i = 0 ; 256 > i ; ++i)
		{
			if (((0 != m_pKeysInfoOld[i]) && (0 == m_pKeysInfo[i])) || (0 != m_pKeysInfo[i]))
			{
				const UINT uKey = i + uBaseModifiers + (((0 != m_pKeysInfoOld[i]) && (0 == m_pKeysInfo[i])) ? s_uOnceModifier : 0);
				KeyActionMap::iterator iPair = m_pCurrentContext->m_mKeyActions.find(uKey);
				if (m_pCurrentContext->m_mKeyActions.end() != iPair)
				{
					const UInt uAction = iPair->second;
					m_pCurrentContext->m_mActions[uAction] = m_pCurrentContext->m_mActionRights[uAction];
				}
			}
		}
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

			#define REGISTER_KEY(KEY) funcRegister.Process(#KEY, KEY)

			#define SHIFT s_uShiftModifier
			#define CONTROL s_uControlModifier
			#define ALT s_uAltModifier
			#define ONCE s_uOnceModifier

			REGISTER_KEY(SHIFT);
			REGISTER_KEY(CONTROL);
			REGISTER_KEY(ALT);
			REGISTER_KEY(ONCE);

			REGISTER_KEY(DIK_ESCAPE);
			REGISTER_KEY(DIK_1);
			REGISTER_KEY(DIK_2);
			REGISTER_KEY(DIK_3);
			REGISTER_KEY(DIK_4);
			REGISTER_KEY(DIK_5);
			REGISTER_KEY(DIK_6);
			REGISTER_KEY(DIK_7);
			REGISTER_KEY(DIK_8);
			REGISTER_KEY(DIK_9);
			REGISTER_KEY(DIK_0);
			REGISTER_KEY(DIK_MINUS);
			REGISTER_KEY(DIK_EQUALS);
			REGISTER_KEY(DIK_BACK);
			REGISTER_KEY(DIK_TAB);
			REGISTER_KEY(DIK_Q);
			REGISTER_KEY(DIK_W);
			REGISTER_KEY(DIK_E);
			REGISTER_KEY(DIK_R);
			REGISTER_KEY(DIK_T);
			REGISTER_KEY(DIK_Y);
			REGISTER_KEY(DIK_U);
			REGISTER_KEY(DIK_I);
			REGISTER_KEY(DIK_O);
			REGISTER_KEY(DIK_P);
			REGISTER_KEY(DIK_LBRACKET);
			REGISTER_KEY(DIK_RBRACKET);
			REGISTER_KEY(DIK_RETURN);
			REGISTER_KEY(DIK_LCONTROL);
			REGISTER_KEY(DIK_A);
			REGISTER_KEY(DIK_S);
			REGISTER_KEY(DIK_D);
			REGISTER_KEY(DIK_F);
			REGISTER_KEY(DIK_G);
			REGISTER_KEY(DIK_H);
			REGISTER_KEY(DIK_J);
			REGISTER_KEY(DIK_K);
			REGISTER_KEY(DIK_L);
			REGISTER_KEY(DIK_SEMICOLON);
			REGISTER_KEY(DIK_APOSTROPHE);
			REGISTER_KEY(DIK_GRAVE);
			REGISTER_KEY(DIK_LSHIFT);
			REGISTER_KEY(DIK_BACKSLASH);
			REGISTER_KEY(DIK_Z);
			REGISTER_KEY(DIK_X);
			REGISTER_KEY(DIK_C);
			REGISTER_KEY(DIK_V);
			REGISTER_KEY(DIK_B);
			REGISTER_KEY(DIK_N);
			REGISTER_KEY(DIK_M);
			REGISTER_KEY(DIK_COMMA);
			REGISTER_KEY(DIK_PERIOD);
			REGISTER_KEY(DIK_SLASH);
			REGISTER_KEY(DIK_RSHIFT);
			REGISTER_KEY(DIK_MULTIPLY);
			REGISTER_KEY(DIK_LMENU);
			REGISTER_KEY(DIK_SPACE);
			REGISTER_KEY(DIK_CAPITAL);
			REGISTER_KEY(DIK_F1);
			REGISTER_KEY(DIK_F2);
			REGISTER_KEY(DIK_F3);
			REGISTER_KEY(DIK_F4);
			REGISTER_KEY(DIK_F5);
			REGISTER_KEY(DIK_F6);
			REGISTER_KEY(DIK_F7);
			REGISTER_KEY(DIK_F8);
			REGISTER_KEY(DIK_F9);
			REGISTER_KEY(DIK_F10);
			REGISTER_KEY(DIK_NUMLOCK);
			REGISTER_KEY(DIK_SCROLL);
			REGISTER_KEY(DIK_NUMPAD7);
			REGISTER_KEY(DIK_NUMPAD8);
			REGISTER_KEY(DIK_NUMPAD9);
			REGISTER_KEY(DIK_SUBTRACT);
			REGISTER_KEY(DIK_NUMPAD4);
			REGISTER_KEY(DIK_NUMPAD5);
			REGISTER_KEY(DIK_NUMPAD6);
			REGISTER_KEY(DIK_ADD);
			REGISTER_KEY(DIK_NUMPAD1);
			REGISTER_KEY(DIK_NUMPAD2);
			REGISTER_KEY(DIK_NUMPAD3);
			REGISTER_KEY(DIK_NUMPAD0);
			REGISTER_KEY(DIK_DECIMAL);
			REGISTER_KEY(DIK_OEM_102);
			REGISTER_KEY(DIK_F11);
			REGISTER_KEY(DIK_F12);
			REGISTER_KEY(DIK_F13);
			REGISTER_KEY(DIK_F14);
			REGISTER_KEY(DIK_F15);
			REGISTER_KEY(DIK_KANA);
			REGISTER_KEY(DIK_ABNT_C1);
			REGISTER_KEY(DIK_CONVERT);
			REGISTER_KEY(DIK_NOCONVERT);
			REGISTER_KEY(DIK_YEN);
			REGISTER_KEY(DIK_ABNT_C2);
			REGISTER_KEY(DIK_NUMPADEQUALS);
			REGISTER_KEY(DIK_PREVTRACK);
			REGISTER_KEY(DIK_AT);
			REGISTER_KEY(DIK_COLON);
			REGISTER_KEY(DIK_UNDERLINE);
			REGISTER_KEY(DIK_KANJI);
			REGISTER_KEY(DIK_STOP);
			REGISTER_KEY(DIK_AX);
			REGISTER_KEY(DIK_UNLABELED);
			REGISTER_KEY(DIK_NEXTTRACK);
			REGISTER_KEY(DIK_NUMPADENTER);
			REGISTER_KEY(DIK_RCONTROL);
			REGISTER_KEY(DIK_MUTE);
			REGISTER_KEY(DIK_CALCULATOR);
			REGISTER_KEY(DIK_PLAYPAUSE);
			REGISTER_KEY(DIK_MEDIASTOP);
			REGISTER_KEY(DIK_VOLUMEDOWN);
			REGISTER_KEY(DIK_VOLUMEUP);
			REGISTER_KEY(DIK_WEBHOME);
			REGISTER_KEY(DIK_NUMPADCOMMA);
			REGISTER_KEY(DIK_DIVIDE);
			REGISTER_KEY(DIK_SYSRQ);
			REGISTER_KEY(DIK_RMENU);
			REGISTER_KEY(DIK_PAUSE);
			REGISTER_KEY(DIK_HOME);
			REGISTER_KEY(DIK_UP);
			REGISTER_KEY(DIK_PRIOR);
			REGISTER_KEY(DIK_LEFT);
			REGISTER_KEY(DIK_RIGHT);
			REGISTER_KEY(DIK_END);
			REGISTER_KEY(DIK_DOWN);
			REGISTER_KEY(DIK_NEXT);
			REGISTER_KEY(DIK_INSERT);
			REGISTER_KEY(DIK_DELETE);
			REGISTER_KEY(DIK_LWIN);
			REGISTER_KEY(DIK_RWIN);
			REGISTER_KEY(DIK_APPS);
			REGISTER_KEY(DIK_POWER);
			REGISTER_KEY(DIK_SLEEP);
			REGISTER_KEY(DIK_WAKE);
			REGISTER_KEY(DIK_WEBSEARCH);
			REGISTER_KEY(DIK_WEBFAVORITES);
			REGISTER_KEY(DIK_WEBREFRESH);
			REGISTER_KEY(DIK_WEBSTOP);
			REGISTER_KEY(DIK_WEBFORWARD);
			REGISTER_KEY(DIK_WEBBACK);
			REGISTER_KEY(DIK_MYCOMPUTER);
			REGISTER_KEY(DIK_MAIL);
			REGISTER_KEY(DIK_MEDIASELECT);
			REGISTER_KEY(DIK_BACKSPACE);
			REGISTER_KEY(DIK_NUMPADSTAR);
			REGISTER_KEY(DIK_LALT);
			REGISTER_KEY(DIK_CAPSLOCK);
			REGISTER_KEY(DIK_NUMPADMINUS);
			REGISTER_KEY(DIK_NUMPADPLUS);
			REGISTER_KEY(DIK_NUMPADPERIOD);
			REGISTER_KEY(DIK_NUMPADSLASH);
			REGISTER_KEY(DIK_RALT);
			REGISTER_KEY(DIK_UPARROW);
			REGISTER_KEY(DIK_PGUP);
			REGISTER_KEY(DIK_LEFTARROW);
			REGISTER_KEY(DIK_RIGHTARROW);
			REGISTER_KEY(DIK_DOWNARROW);
			REGISTER_KEY(DIK_PGDN);
			REGISTER_KEY(DIK_CIRCUMFLEX);

			#undef REGISTER_KEY
		}

		return bResult;
	}
}
