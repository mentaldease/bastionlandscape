#include "stdafx.h"
#undef LoadString // avoid collision with Win32 LoadString
#include "../Core/Scripting.h"
#include "../Core/File.h"

namespace ElixirEngine
{
	namespace Scripting
	{
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		LuaStatePtr Lua::s_pState = NULL;

		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		Lua::Lua()
		:	CoreObject()
		{

		}

		Lua::~Lua()
		{

		}

		LuaStatePtr Lua::CreateState()
		{
			LuaStatePtr pLuaState = LuaState::Create();
			return pLuaState;
		}

		void Lua::ReleaseState(LuaStatePtr _pState)
		{
			s_pState = (s_pState == _pState) ? NULL : s_pState;
			LuaState::Destroy(_pState);
		}

		void Lua::SetStateInstance(LuaStatePtr _pState)
		{
			s_pState = _pState;
		}

		LuaStatePtr Lua::GetStateInstance()
		{
			return s_pState;
		}

		bool Lua::Loadfile(const string& _strFileName, LuaStatePtr _pState)
		{
			FilePtr pFile = FS::GetRoot()->OpenFile(_strFileName, FS::EOpenMode_READTEXT);
			bool bResult = (NULL != pFile);
			if (false != bResult)
			{
				int sSize = pFile->Size();
				char* pSourceCode = new char[sSize + 1];
				sSize = pFile->Read(pSourceCode, sSize);
				FS::GetRoot()->CloseFile(pFile);
				pSourceCode[sSize] = '\0';
				_pState = (NULL == _pState) ? s_pState : _pState;
				bResult = (0 == _pState->DoString(pSourceCode));
			}
			return bResult;
		}
	}
}
