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

		protected:
			static LuaStatePtr s_pState;

		private:
		};
	}
}

#endif // __SCRIPTING_H__