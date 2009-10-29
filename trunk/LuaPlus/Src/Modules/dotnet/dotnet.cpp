#include "../../LuaPlus/LuaPlus.h"
#include <windows.h>
#include <assert.h>

using namespace System;
using namespace System::IO;
using namespace System::Collections;
using namespace System::Reflection;
using System::Runtime::InteropServices::GCHandle;

using namespace LuaPlus;

struct lua_State {};


static int GCHandle_gc(LuaState* state)
{
	LuaStack args(state);
	if ( !args[1].IsUserData() )
		assert( 0 );		// How did we get here?
	GCHandle::op_Explicit((int)args[1].GetUserData()).Free();

	System::GC::Collect();
	System::GC::WaitForPendingFinalizers();

	return 0;
}

extern "C" LUAMODULE_API int luaopen_dotnet(lua_State* L)
{
	LuaState* state = LuaState::CastState(L);
	LuaObject obj = state->GetGlobals()["dotnet"];
	if (obj.IsTable())
		return 0;

	obj = state->GetGlobals().CreateTable( "dotnet" );

	char path[ _MAX_PATH ];
#ifdef _DEBUG
	const char* dllName = "ldotnet.dlld";
#else !_DEBUG
	const char* dllName = "ldotnet.dll";
#endif _DEBUG
	GetModuleFileName(GetModuleHandle(dllName), path, _MAX_PATH);
	char* slashPtr = strrchr( path, '\\' );
	slashPtr++;
	*slashPtr = 0;
#ifdef _DEBUG
	strcat(path, "Debug\\");
#endif _DEBUG
	strcat(path, "dotnetinterface.dll");

	Assembly* assembly = Assembly::LoadFrom(path);
	Type* type = assembly->GetType("LuaInterface.Lua");
	Object* parms[] = new Object*[1];
	parms[0] = __box((IntPtr)state->GetCState());
	Object* lua = Activator::CreateInstance(type, parms);

	LuaObject metaTableObj;
	metaTableObj.AssignNewTable(state);
	metaTableObj.Register("__gc", GCHandle_gc);

	LuaObject dotNetObj = state->NewUserDataBox((void*)(GCHandle::op_Explicit(GCHandle::Alloc(lua))).ToInt32());
	dotNetObj.SetMetaTable(metaTableObj);

	obj.SetObject("__dotNetUserData", dotNetObj);

	return 0;
}


