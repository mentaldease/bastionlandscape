namespace LuaInterface 
{

	using System;
	using System.Runtime.InteropServices;
	using System.Reflection;
	using System.Collections;
	using System.Text;

	/*
	 * Lua types for the API, returned by lua_type function
	 */
	public enum LuaTypes 
	{
		LUA_TNONE=-1,
		LUA_TNIL=0,
		LUA_TBOOLEAN=1,
		LUA_TLIGHTUSERDATA=2,
		LUA_TNUMBER=3,
		LUA_TSTRING=4,
		LUA_TTABLE=5,
		LUA_TFUNCTION=6,
		LUA_TUSERDATA=7,
		LUA_TTHREAD=8,
		LUA_TWSTRING=9
	}

	/*
	 * Special stack indexes
	 */
	sealed class LuaIndexes 
	{
		public static int LUA_REGISTRYINDEX=-10000;
		public static int LUA_GLOBALSINDEX=-10001;
	}

	/*
	 * Structure used by the chunk reader
	 */
	[ StructLayout( LayoutKind.Sequential )]
	public struct ReaderInfo
	{
		public String chunkData;
		public bool finished;
	}

	/*
	 * Delegate for functions passed to Lua as function pointers
	 */
	public delegate int LuaCSFunction(IntPtr luaState);

	/*
	 * Delegate for chunk readers used with lua_load
	 */
	public delegate string LuaChunkReader(IntPtr luaState,ref ReaderInfo data,ref uint size);

	/*
	 * P/Invoke wrapper of the Lua API
	 *
	 * Author: Fabio Mascarenhas
	 * Version: 1.0
	 */
	public class LuaDLL 
	{
#if (DEBUG)
		private const string LUADLL = "LuaPlusD_1082.dll";
#else
		private const string LUADLL = "LuaPlus_1082.dll";
#endif

		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern IntPtr lua_open();
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_close(IntPtr luaState);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void luaopen_base(IntPtr luaState);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void luaopen_io(IntPtr luaState);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern int lua_strlen(IntPtr luaState, int stackPos);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern int lua_dostring(IntPtr luaState, string chunk);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_newtable(IntPtr luaState);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern int lua_dofile(IntPtr luaState, string fileName);
		public static void lua_getglobal(IntPtr luaState, string name) 
		{
			LuaDLL.lua_pushstring(luaState,name);
			LuaDLL.lua_gettable(luaState,LuaIndexes.LUA_GLOBALSINDEX);
		}
		public static void lua_setglobal(IntPtr luaState, string name)
		{
			LuaDLL.lua_pushstring(luaState,name);
			LuaDLL.lua_insert(luaState,-2);
			LuaDLL.lua_settable(luaState,LuaIndexes.LUA_GLOBALSINDEX);
		}
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_settop(IntPtr luaState, int newTop);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_insert(IntPtr luaState, int newTop);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_remove(IntPtr luaState, int index);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_gettable(IntPtr luaState, int index);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_rawget(IntPtr luaState, int index);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_settable(IntPtr luaState, int index);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_rawset(IntPtr luaState, int index);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_setmetatable(IntPtr luaState, int objIndex);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern int lua_getmetatable(IntPtr luaState, int objIndex);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern int lua_equal(IntPtr luaState, int index1, int index2);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_pushvalue(IntPtr luaState, int index);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern int lua_gettop(IntPtr luaState);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern LuaTypes lua_type(IntPtr luaState, int index);
		public static bool lua_isnil(IntPtr luaState, int index)
		{
			return (LuaDLL.lua_type(luaState,index)==LuaTypes.LUA_TNIL);
		}
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern bool lua_isnumber(IntPtr luaState, int index);
		public static bool lua_isboolean(IntPtr luaState, int index) 
		{
			return LuaDLL.lua_type(luaState,index)==LuaTypes.LUA_TBOOLEAN;
		}
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern int luaL_ref(IntPtr luaState, int registryIndex);
		public static int lua_ref(IntPtr luaState, int lockRef)
		{
			if(lockRef!=0) 
			{
				return LuaDLL.luaL_ref(luaState,LuaIndexes.LUA_REGISTRYINDEX);
			} 
			else return 0;
		}
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_rawgeti(IntPtr luaState, int tableIndex, int index);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_rawseti(IntPtr luaState, int tableIndex, int index);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern IntPtr lua_newuserdata(IntPtr luaState, int size);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern IntPtr lua_touserdata(IntPtr luaState, int index);
		public static void lua_getref(IntPtr luaState, int reference)
		{
			LuaDLL.lua_rawgeti(luaState,LuaIndexes.LUA_REGISTRYINDEX,reference);
		}
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void luaL_unref(IntPtr luaState, int registryIndex, int reference);
		public static void lua_unref(IntPtr luaState, int reference) 
		{
			LuaDLL.luaL_unref(luaState,LuaIndexes.LUA_REGISTRYINDEX,reference);
		}
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern bool lua_isstring(IntPtr luaState, int index);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern bool lua_iscfunction(IntPtr luaState, int index);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_pushnil(IntPtr luaState);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_pushstdcallcfunction(IntPtr luaState, [MarshalAs(UnmanagedType.FunctionPtr)]LuaCSFunction function);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern int lua_call(IntPtr luaState, int nArgs, int nResults);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern int lua_pcall(IntPtr luaState, int nArgs, int nResults, int errfunc);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern int lua_rawcall(IntPtr luaState, int nArgs, int nResults);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern IntPtr lua_tocfunction(IntPtr luaState, int index);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern double lua_tonumber(IntPtr luaState, int index);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern bool lua_toboolean(IntPtr luaState, int index);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_safetostring(IntPtr luaState, int index, StringBuilder buffer);
		public static string lua_tostring(IntPtr luaState, int index)
		{
			StringBuilder buffer=new StringBuilder(lua_strlen(luaState,index));
			LuaDLL.lua_safetostring(luaState,index,buffer);
			return buffer.ToString();
		}
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_pushnumber(IntPtr luaState, double number);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_pushboolean(IntPtr luaState, bool value);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_pushlstring(IntPtr luaState, string str, int size);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_pushstring(IntPtr luaState, string str);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern int luaL_newmetatable(IntPtr luaState, string meta);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void luaL_getmetatable(IntPtr luaState, string meta);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern IntPtr luaL_checkudata(IntPtr luaState, int stackPos, string meta);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern bool luaL_getmetafield(IntPtr luaState, int stackPos, string field);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern int lua_load(IntPtr luaState, LuaChunkReader chunkReader, ref ReaderInfo data, string chunkName);
		public static bool luaL_checkmetatable(IntPtr luaState,int obj) 
		{
			if(LuaDLL.lua_getmetatable(luaState,obj)!=0) 
			{
				LuaDLL.lua_rawget(luaState,LuaIndexes.LUA_REGISTRYINDEX);
				bool retVal=!LuaDLL.lua_isnil(luaState,-1);
				LuaDLL.lua_settop(luaState,-2);
				return retVal;
			} else
				return false;
		}
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern void lua_error(IntPtr luaState);
		[DllImport(LUADLL,CallingConvention=CallingConvention.Cdecl)]
		public static extern bool lua_checkstack(IntPtr luaState,int extra);
	}

}
