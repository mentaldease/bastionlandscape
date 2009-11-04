#include "stdafx.h"
#include "../Display/Display.h"
#include "../Core/Scripting.h"

namespace ElixirEngine
{
	namespace Scripting
	{
		template<>
		bool Lua::Get(LuaObjectRef _rLuaObject, const CharPtr _pszFieldName, const Vector2 _tDefault, Vector2& _tResult)
		{
			LuaObject oLuaObject = _rLuaObject[_pszFieldName];
			bool bResult = (false == oLuaObject.IsNil());
			if (false != bResult)
			{
				_tResult.x = oLuaObject[1].GetFloat();
				_tResult.y = oLuaObject[2].GetFloat();
			}
			else
			{
				_tResult = _tDefault;
			}
			return bResult;
		}

		template<>
		bool Lua::Get(LuaObjectRef _rLuaObject, const CharPtr _pszFieldName, const Vector3 _tDefault, Vector3& _tResult)
		{
			LuaObject oLuaObject = _rLuaObject[_pszFieldName];
			bool bResult = (false == oLuaObject.IsNil());
			if (false != bResult)
			{
				_tResult.x = oLuaObject[1].GetFloat();
				_tResult.y = oLuaObject[2].GetFloat();
				_tResult.z = oLuaObject[3].GetFloat();
			}
			else
			{
				_tResult = _tDefault;
			}
			return bResult;
		}

		template<>
		bool Lua::Get(LuaObjectRef _rLuaObject, const CharPtr _pszFieldName, const Vector4 _tDefault, Vector4& _tResult)
		{
			LuaObject oLuaObject = _rLuaObject[_pszFieldName];
			bool bResult = (false == oLuaObject.IsNil());
			if (false != bResult)
			{
				_tResult.x = oLuaObject[1].GetFloat();
				_tResult.y = oLuaObject[2].GetFloat();
				_tResult.z = oLuaObject[3].GetFloat();
				_tResult.w = oLuaObject[4].GetFloat();
			}
			else
			{
				_tResult = _tDefault;
			}
			return bResult;
		}
	}
}