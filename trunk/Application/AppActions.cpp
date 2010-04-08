#include "stdafx.h"
#include "../Application/AppActions.h"
#include "../Application/Application.h"
#include "../Application/ActionKeybinding.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	bool Application::InitActions()
	{
		LuaObject lActions = Scripting::Lua::GetGlobalTable(string("Actions"), true);
		bool bResult = (false == lActions.IsNil());

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

			#undef REGISTER_ACTION

			m_pKeybinds->LoadBindings("Data/keybinds.lua");
		}

		return bResult;
	}
}