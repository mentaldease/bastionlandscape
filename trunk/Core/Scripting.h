#ifndef __SCRIPTING_H__
#define __SCRIPTING_H__

#include "../Core/Core.h"

#include "../LuaPlus/Src/LuaPlus/LuaPlus.h"
using namespace LuaPlus;

namespace ElixirEngine
{
	namespace Scripting
	{
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------

		class Lua : public CoreObject
		{
		public:
			Lua();
			virtual ~Lua();

			static LuaStatePtr CreateState();
			static void ReleaseState(LuaStatePtr _pState);
			static void SetStateInstance(LuaStatePtr _pState);
			static LuaStatePtr GetStateInstance();
			static bool Loadfile(const string& _strFileName, LuaStatePtr _pState = NULL);

			static LuaObject GetGlobalTable(const string& _strTableName, const bool _bCreate = false, LuaStatePtr _pState = NULL);

			template<typename T>
			static bool Get(LuaObjectRef _rLuaObject, const CharPtr _pszFieldName, const T _tDefault, T& _tResult);

		protected:
			static void OutputError(const int _sErrorCode, LuaStatePtr _pState);

			static LuaStatePtr s_pState;
		};
	}
}

#endif // __SCRIPTING_H__