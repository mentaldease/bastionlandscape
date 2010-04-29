#include "stdafx.h"
#undef LoadString // avoid collision with Win32 LoadString
#include "../Core/Scripting.h"
#include "../Core/File.h"
#include "../Core/File.h"
#include "../Core/Util.h"

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
				const int sResult = _pState->DoString(pSourceCode);
				bResult = (0 == sResult);
				delete[] pSourceCode;
				if (false == bResult)
				{
					OutputError(sResult, _pState);
				}
			}
			return bResult;
		}


		LuaObject Lua::GetGlobalTable(const string& _strTableName, const bool _bCreate, LuaStatePtr _pState)
		{
			_pState = (NULL == _pState) ? s_pState : _pState;
			LuaObject lTable = _pState->GetGlobal(_strTableName.c_str());
			if ((false != lTable.IsNil()) && (false != _bCreate))
			{
				lTable = _pState->GetGlobals().CreateTable(_strTableName.c_str());
			}
			return lTable;
		}

		void Lua::OutputError(const int _sErrorCode, LuaStatePtr _pState)
		{
			const char* szMsg = lua_tostring(_pState->GetCState(), -1);
			if (NULL == szMsg) 
			{
				szMsg = "(error with no message)"; 
			}

			switch (_sErrorCode)
			{
				case LUA_ERRERR:
				{
					vsoutput("LUA_ERRERR : error while running the error handler function (%s)\n", szMsg);
					break;
				}
				case LUA_ERRFILE:
				{
					vsoutput("LUA_ERRFILE : file error (%s)\n", szMsg);
					break;		
				}
				case LUA_ERRMEM:
				{
					vsoutput("LUA_ERRMEM : memory allocation error (%s)\n", szMsg);
					break;
				}
				case LUA_ERRRUN:
				{
					vsoutput("LUA_ERRRUN : runtime error (%s)\n", szMsg);
					break;
				}
				case LUA_ERRSYNTAX:
				{
					vsoutput("LUA_ERRSYNTAX : syntax error during pre-compilation (%s)\n", szMsg);
					break;
				}
				default:
				{
					vsoutput("%s\n", szMsg);
					break;
				}
			}
		}

		template<typename T>
		bool Lua::Get(LuaObjectRef _rLuaObject, const CharPtr _pszFieldName, const T _tDefault, T& _tResult)
		{
			_tResult = _tDefault;
			return false;
		}

		template<>
		bool Lua::Get(LuaObjectRef _rLuaObject, const CharPtr _pszFieldName, const float _tDefault, float& _tResult)
		{
			LuaObject oLuaObject = _rLuaObject[_pszFieldName];
			bool bResult = (false == oLuaObject.IsNil()) && (false != oLuaObject.IsNumber());
			_tResult = (false != bResult) ? oLuaObject.GetFloat() : _tDefault;
			return bResult;
		}

		template<>
		bool Lua::Get(LuaObjectRef _rLuaObject, const CharPtr _pszFieldName, const UInt _tDefault, UInt& _tResult)
		{
			LuaObject oLuaObject = _rLuaObject[_pszFieldName];
			bool bResult = (false == oLuaObject.IsNil()) && (false != oLuaObject.IsNumber());
			_tResult = (false != bResult) ? UInt(oLuaObject.GetInteger()) : _tDefault;
			return bResult;
		}

		template<>
		bool Lua::Get(LuaObjectRef _rLuaObject, const CharPtr _pszFieldName, const int _tDefault, int& _tResult)
		{
			LuaObject oLuaObject = _rLuaObject[_pszFieldName];
			bool bResult = (false == oLuaObject.IsNil()) && (false != oLuaObject.IsNumber());
			_tResult = (false != bResult) ? oLuaObject.GetInteger() : _tDefault;
			return bResult;
		}

		template<>
		bool Lua::Get(LuaObjectRef _rLuaObject, const CharPtr _pszFieldName, const string _tDefault, string& _tResult)
		{
			LuaObject oLuaObject = _rLuaObject[_pszFieldName];
			bool bResult = (false == oLuaObject.IsNil()) && (false != oLuaObject.IsString());
			_tResult = (false != bResult) ? oLuaObject.GetString() : _tDefault;
			return bResult;
		}

		template<>
		bool Lua::Get(LuaObjectRef _rLuaObject, const CharPtr _pszFieldName, const bool _tDefault, bool& _tResult)
		{
			LuaObject oLuaObject = _rLuaObject[_pszFieldName];
			bool bResult = (false == oLuaObject.IsNil()) && (false != oLuaObject.IsBoolean());
			_tResult = (false != bResult) ? oLuaObject.GetBoolean() : _tDefault;
			return bResult;
		}
	}
}
