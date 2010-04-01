#include "stdafx.h"
#undef LoadString // avoid collision with Win32 LoadString
#include "../Core/Scripting.h"
#include "../Core/File.h"
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
				delete[] pSourceCode;
			}
			return bResult;
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
