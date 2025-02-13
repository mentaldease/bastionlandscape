///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2005 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#define BUILDING_LUAPLUS
#include "LuaLink.h"
LUA_EXTERN_C_BEGIN
#define LUA_CORE
#include "src/lobject.h"
LUA_EXTERN_C_END
#include "LuaPlus.h"
#include "LuaCall.h"
#include <string.h>
#ifdef WIN32
#if defined(WIN32) && !defined(_XBOX) && !defined(_WIN32_WCE)
#include <windows.h>
#elif defined(_XBOX)
#include <xtl.h>
#endif // WIN32
#undef GetObject
#endif // WIN32
#if defined(__GNUC__)
	#include <new>
#else
	#include <new.h>
#endif

#include <stdlib.h>
#include <wchar.h>
#include <assert.h>

#include "LuaPlusFunctions.h"

#ifdef _MSC_VER
#pragma warning(disable: 4100)
#endif // _MSC_VER

//-----------------------------------------------------------------------------
LUA_EXTERN_C_BEGIN
#include "src/lualib.h"
#include "src/lfunc.h"
#include "src/lgc.h"
#include "src/lstate.h"
#include "src/lua.h"
#include "src/lauxlib.h"

static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize, const char* allocName, unsigned int flags) {
  (void)osize;
  (void)ud;
  (void)allocName;
  (void)flags;
  if (nsize == 0) {
    free(ptr);
    return NULL;
  }
  else
    return realloc(ptr, nsize);
}


static lua_Alloc luaHelper_defaultAlloc = l_alloc;
static void* luaHelper_ud = NULL;


void lua_getdefaultallocfunction(lua_Alloc* allocFunc, void** ud)
{
	*allocFunc = luaHelper_defaultAlloc;
	*ud = luaHelper_ud;
}


void lua_setdefaultallocfunction(lua_Alloc allocFunc, void* ud)
{
	luaHelper_defaultAlloc = allocFunc ? allocFunc : l_alloc;
	luaHelper_ud = ud;
}

void reallymarkobject (global_State *g, GCObject *o);

LUA_EXTERN_C_END


namespace LuaPlus
{

USING_NAMESPACE_LUA


LuaException::LuaException(const char* message)
	: m_message(NULL)
{
	if (message)
	{
		m_message = new char[strlen(message) + 1];
		strcpy(m_message, message);
	}
}


LuaException::LuaException(const LuaException& src)
{
    m_message = new char[strlen(src.m_message) + 1];
    strcpy(m_message, src.m_message);
}


LuaException& LuaException::operator=(const LuaException& src)
{
    delete[] m_message;
    m_message = new char[strlen(src.m_message) + 1];
    strcpy(m_message, src.m_message);
    return *this;
}


LuaException::~LuaException()
{
    delete[] m_message;
}

//static void LSLock(void* data);
//static void LSUnlock(void* data);

void LuaPlusGCFunction(void*);

#undef api_check
#define api_check(L, o)		(luaplus_assert(o))

#define api_checknelems(L, n)	api_check(L, (n) <= (L->top - L->ci->base))

//#define api_incr_top(L)   (api_check(L, L->top<L->ci->top), L->top++)

#define api_incr_top(L) \
	{if (L->top >= L->ci->top) lua_checkstack(L, 1); L->top++;}

LUAPLUS_API void lua_pushtobject(lua_State *L, void* tobject)
{
	TValue* tobj = (TValue*)tobject;
	lua_lock(L);
	setobj(L, L->top, tobj);
	api_incr_top(L);
	lua_unlock(L);
}


void LuaState::newthread_handler(lua_State *L, lua_State *L1)
{
	void* data;
	lua_Alloc reallocFunc = lua_getallocf(L, &data);
	LuaState* state = (LuaState*)(*reallocFunc)(data, NULL, 0, sizeof(LuaState), "CoLuaState", 0);
	::new(state) LuaState(L, L1);

//    luastateopen_thread(L);
}


void LuaState::freethread_handler(lua_State * /*L*/, lua_State *L1)
{
    LuaState::Destroy(LuaState::CastState(L1));
}


/*static*/ LuaState* LuaState::Create( bool initStandardLibrary, bool multithreaded )
{
	lua_Alloc reallocFunc;
	void* data;
	lua_getdefaultallocfunction(&reallocFunc, &data);
	LuaState* state = (LuaState*)(*reallocFunc)(data, NULL, 0, sizeof(LuaState), "LuaState", 0);
	::new(state) LuaState(initStandardLibrary, multithreaded);
	return state;
}


/*static*/ LuaState* LuaState::CastState( lua_State* L )
{
	LuaState* state = (LuaState*)lua_getstateuserdata( L );
	return state;
}


/*static*/ void LuaState::Destroy( LuaState* state )
{
	void* data;
	lua_Alloc reallocFunc = lua_getallocf(state->GetCState(), &data);
	state->~LuaState();
	(*reallocFunc)(data, state, sizeof(LuaState), 0, NULL, 0);
}


void LuaState_UserStateThread_Internal(lua_State* L, lua_State* L1)
{
	lua_Alloc reallocFunc;
	void* data;
	lua_getdefaultallocfunction(&reallocFunc, &data);
	LuaState* state = (LuaState*)(*reallocFunc)(data, NULL, 0, sizeof(LuaState), "LuaState", 0);
	::new(state) LuaState(L, L1);
}


extern "C" void LuaState_UserStateThread(lua_State* L, lua_State* L1)
{
	LuaState_UserStateThread_Internal(L, L1);
}


extern "C" void LuaState_UserStateFree(lua_State* L)
{
	LuaState* state = (LuaState*)lua_getstateuserdata(L);
	if (state)
		LuaState::Destroy(state);
}


LuaObject LuaState::CreateThread(LuaState* parentState)
{
    lua_State* L1 = lua_newthread(parentState->GetCState());
    lua_TValue tobject;
    setnilvalue2n(L1, &tobject);
    setthvalue(parentState->GetCState(), &tobject, L1);

	LuaObject retObj = LuaObject((LuaState*)lua_getstateuserdata(L1), &tobject);
    setnilvalue(&tobject);
    lua_pop(parentState->GetCState(), 1);
    return retObj;
}


/**
**/
LuaState::LuaState(bool initStandardLibrary, bool multithreaded)
{
	m_state = lua_newstate(luaHelper_defaultAlloc, luaHelper_ud);
	m_ownState = true;

	lua_setusergcfunction(m_state, LuaPlusGCFunction);
//jj    luaX_setnewthreadhandler( newthread_handler );
//jj    luaX_setfreethreadhandler( freethread_handler );

	SetupStateEx();

	GetHeadObject()->m_prev = NULL;
	GetHeadObject()->m_next = (LuaObject*)GetTailObject();
	GetTailObject()->m_prev = (LuaObject*)GetHeadObject();
	GetTailObject()->m_next = NULL;

#ifdef LUA_MTSUPPORT
	if (multithreaded)
	{
/*#ifdef WIN32
		// What about clean up?
		CRITICAL_SECTION* cs = new CRITICAL_SECTION;
		::InitializeCriticalSection(cs);
		lua_setlockfunctions(m_state, LSLock, LSUnlock, cs);
#endif // WIN32*/
	}
#endif // LUA_MTSUPPORT

//    luastateopen_thread(m_state);

    Init(initStandardLibrary);
}


/**
**/
LuaState::LuaState( lua_State* state )
{
	m_state = state;

	m_ownState = false;

//jj    luaX_setnewthreadhandler( newthread_handler );
//jj    luaX_setfreethreadhandler( freethread_handler );

    GetHeadObject()->m_prev = NULL;
	GetHeadObject()->m_next = (LuaObject*)GetTailObject();
	GetTailObject()->m_prev = (LuaObject*)GetHeadObject();
	GetTailObject()->m_next = NULL;

    SetupStateEx();

//    luastateopen_thread(m_state);
}


LuaState::LuaState(lua_State* L, lua_State* L1)
{
	m_state = L1;

	m_ownState = false;

    GetHeadObject()->m_prev = NULL;
	GetHeadObject()->m_next = (LuaObject*)GetTailObject();
	GetTailObject()->m_prev = (LuaObject*)GetHeadObject();
	GetTailObject()->m_next = NULL;

    SetupStateEx();
}


/**
	The LuaState class assumes ownership of the lua_State pointer and
	destroys it when destroyed.
**/
LuaState::~LuaState()
{
	if ( m_state  &&  m_ownState )
	{
		lua_close( m_state );

        MiniLuaObject* headObject = GetHeadObject();	(void)headObject;
		MiniLuaObject* tailObject = GetTailObject();	(void)tailObject;
		assert((MiniLuaObject*)headObject->m_next == tailObject && "There are still active LuaObjects referencing this state");
		luaplus_assert((MiniLuaObject*)headObject->m_next == tailObject);
	}
}


/**
**/
void LuaState::SetupStateEx()
{
	lua_setstateuserdata( m_state, this );
}


static int LS_LOG( lua_State* L )
{
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "towstring");
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s = NULL;
		const lua_WChar *ws = NULL;
		if (lua_type(L, i) == LUA_TSTRING)
		{
			s = lua_tostring(L, -1);
		}
		else if (lua_type(L, i) != LUA_TWSTRING)
		{
			lua_pushvalue(L, -1);  /* function to be called */
			lua_pushvalue(L, i);   /* value to print */
			lua_call(L, 1, 1);
			s = lua_tostring(L, -1);  /* get result */
			if (s == NULL)
				return luaL_error(L, "`tostring' must return a string to `print'");
		}
		else
		{
			lua_pushvalue(L, -2);  /* function to be called */
			lua_pushvalue(L, i);   /* value to print */
			lua_call(L, 1, 1);
			ws = lua_towstring(L, -1);  /* get result */
			if (ws == NULL)
				return luaL_error(L, "`tostring' must return a string to `print'");
		}
		if (i>1)
		{
#ifdef WIN32
			OutputDebugStringA("\t");
#else
			fputs("\t", stdout);
#endif
		}
		if (s)
		{
#ifdef WIN32
			OutputDebugStringA(s);
#else
			fputs(s, stdout);
#endif
		}
		else if (ws)
		{
            wchar_t out[512];
            wchar_t* outEnd = out + sizeof(out) - 2;
            while (*ws) {
                wchar_t* outPos = out;
                while (*ws && outPos != outEnd) {
                *outPos++ = *ws++;
                }
                *outPos++ = 0;
#ifdef WIN32
			    OutputDebugStringW(out);
#else
    			fputws(out, stdout);
#endif
            }
		}
		lua_pop(L, 1);  /* pop result */
	}

#ifdef WIN32
	OutputDebugStringA("\n");
#else
	fputs("\n", stdout);
#endif

	return 0;
}


static int LS_ALERT( lua_State* L )
{
	const char* err = lua_tostring(L, 1);
#ifdef WIN32
	OutputDebugString(err);
    OutputDebugString("\n");
#else // !WIN32
	puts(err);
#endif // WIN32

	return 0;
}


#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4702)
#endif /* _MSC_VER */

/**
**/
static int FatalError( lua_State* state )
{
	const char* err = lua_tostring(state, 1);
#ifdef WIN32
	if (err)
		OutputDebugString(err);
#else // !WIN32
	if (err)
		puts(err);
#endif // WIN32

#ifndef _WIN32_WCE
	luaplus_throw(err);
#endif // _WIN32_WCE

	return -1;
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif /* _MSC_VER */


#if 0

/**
**/
static void LSLock(void* data)
{
#ifdef WIN32
	CRITICAL_SECTION* cs = (CRITICAL_SECTION*)data;
	::EnterCriticalSection(cs);
#endif // WIN32
}


/**
**/
static void LSUnlock(void* data)
{
#ifdef WIN32
	CRITICAL_SECTION* cs = (CRITICAL_SECTION*)data;
	::LeaveCriticalSection(cs);
#endif // WIN32
}

#endif

class LuaStateOutString : public LuaStateOutFile
{
public:
	LuaStateOutString(size_t growBy = 10000) :
		m_buffer(NULL),
		m_growBy(growBy),
		m_curPos(0),
		m_size(0)
	{
	}

	virtual ~LuaStateOutString()
	{
		free(m_buffer);
	}

	virtual void Print(const char* str, ...)
	{
		char message[ 800 ];
		va_list arglist;

		va_start( arglist, str );
		vsprintf( message, str, arglist );
		va_end( arglist );

		size_t len = strlen(message);
		if (len != 0)
		{
			if (m_curPos + len + 1 > m_size)
			{
				size_t newSize = m_size;
				while (newSize < m_curPos + len + 1)
					newSize += m_growBy;
				m_buffer = (char*)realloc(m_buffer, newSize);
				m_size = newSize;
			}

			strncpy(m_buffer + m_curPos, message, len);
			m_curPos += len;
			m_buffer[m_curPos] = 0;
		}
	}

	const char* GetBuffer() const
	{
		return m_buffer;
	}

protected:
	char* m_buffer;
	size_t m_growBy;
	size_t m_curPos;
	size_t m_size;
};


// LuaDumpObject(file, key, value, alphabetical, indentLevel, maxIndentLevel, writeAll)
static int LS_LuaDumpObject( LuaState* state )
{
	LuaStateOutFile file;
	LuaStateOutString stringFile;

	LuaStack args(state);
	LuaStackObject fileObj = args[1];
	const char* fileName = NULL;
	if ( fileObj.IsUserData() )
	{
		FILE* stdioFile = (FILE *)fileObj.GetUserData();
		file.Assign( stdioFile );
	}
	else if ( fileObj.IsString() )
	{
		fileName = fileObj.GetString();
	}

	LuaObject nameObj = args[2];
	LuaObject valueObj = args[3];
	LuaStackObject alphabeticalObj = args[4];
	LuaStackObject indentLevelObj = args[5];
	LuaStackObject maxIndentLevelObj = args[6];
	LuaStackObject writeAllObj = args[7];
	bool writeAll = writeAllObj.IsBoolean() ? writeAllObj.GetBoolean() : false;
	bool alphabetical = alphabeticalObj.IsBoolean() ? alphabeticalObj.GetBoolean() : true;
	unsigned int maxIndentLevel = maxIndentLevelObj.IsInteger() ? (unsigned int)maxIndentLevelObj.GetInteger() : 0xFFFFFFFF;

	unsigned int flags = (alphabetical ? LuaState::DUMP_ALPHABETICAL : 0) | (writeAll ? LuaState::DUMP_WRITEALL : 0);

	if (fileName)
	{
		if (strcmp(fileName, ":string") == 0)
		{
			state->DumpObject(stringFile, nameObj, valueObj, flags, indentLevelObj.GetInteger(), maxIndentLevel);
			state->PushString(stringFile.GetBuffer());
			return 1;
		}
		else
		{
			state->DumpObject(fileName, nameObj, valueObj, flags, (unsigned int)indentLevelObj.GetInteger(), maxIndentLevel);
		}
	}
	else
	{
		state->DumpObject(file, nameObj, valueObj, flags, (unsigned int)indentLevelObj.GetInteger(), maxIndentLevel);
	}

	return 0;
}


// LuaDumpFile(file, key, value, alphabetical, indentLevel, maxIndentLevel, writeAll)
static int LS_LuaDumpFile( LuaState* state )
{
	return LS_LuaDumpObject(state);
}


// LuaDumpGlobals(file, alphabetical, maxIndentLevel, writeAll)
static int LS_LuaDumpGlobals(LuaState* state)
{
	LuaStateOutFile file;

	LuaStack args(state);
	LuaStackObject fileObj = args[1];
	const char* fileName = NULL;
	if ( fileObj.IsUserData() )
	{
		FILE* stdioFile = (FILE *)fileObj.GetUserData();
		file.Assign( stdioFile );
	}
	else if ( fileObj.IsString() )
	{
		fileName = fileObj.GetString();
	}

	LuaStackObject alphabeticalObj = args[2];
	LuaStackObject maxIndentLevelObj = args[3];
	LuaStackObject writeAllObj = args[4];
	bool alphabetical = alphabeticalObj.IsBoolean() ? alphabeticalObj.GetBoolean() : true;
	unsigned int maxIndentLevel = maxIndentLevelObj.IsInteger() ? (unsigned int)maxIndentLevelObj.GetInteger() : 0xFFFFFFFF;
	bool writeAll = writeAllObj.IsBoolean() ? writeAllObj.GetBoolean() : false;

	unsigned int flags = (alphabetical ? LuaState::DUMP_ALPHABETICAL : 0) | (writeAll ? LuaState::DUMP_WRITEALL : 0);

	if (fileName)
	{
		state->DumpGlobals(fileName, flags, maxIndentLevel);
	}
	else
	{
		state->DumpGlobals(file, flags, maxIndentLevel);
	}

	return 0;
}


static const luaL_Reg lualibs[] = {
  {"", luaopen_base},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_TABLIBNAME, luaopen_table},
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_DBLIBNAME, luaopen_debug},
  {NULL, NULL}
};


static int pmain (lua_State *L)
{
	const luaL_reg *lib = lualibs;
	for (; lib->func; lib++) {
		lua_pushcfunction(L, lib->func);
		lua_pushstring(L, lib->name);
		lua_call(L, 1, 0);
	}
#if 0
	luaopen_wstring(L);
	lua_settop(L, 0);
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	lua_replace(L, LUA_ENVIRONINDEX);  /* restore environment */
#endif

	return 0;
}

	
/**
**/
void LuaState::Init( bool initStandardLibrary )
{
#if LUAPLUS_INCLUDE_STANDARD_LIBRARY
	// Register some basic functions with Lua.
	if (initStandardLibrary)
	{
		LuaAutoBlock autoBlock(this);

		lua_cpcall(m_state, &pmain, NULL);

#ifdef LUA_MTSUPPORT
//        luaopen_thread(m_state);
#endif // LUA_MTSUPPORT

		ScriptFunctionsRegister( this );

        GetGlobals().Register("LuaDumpGlobals", LS_LuaDumpGlobals);
		GetGlobals().Register("LuaDumpObject", LS_LuaDumpObject);
		GetGlobals().Register("LuaDumpFile", LS_LuaDumpFile);

		GetGlobals().Register("LOG", LS_LOG);
		GetGlobals().Register("_ALERT", LS_ALERT);
	}
#endif // LUAPLUS_INCLUDE_STANDARD_LIBRARY

	lua_atpanic( m_state, FatalError );
}


LuaObject LuaState::GetGlobals() throw()
{
	return LuaObject( this, gt(m_state) );
}


} // namespace LuaPlus

LUA_EXTERN_C_BEGIN
#include "src/ltable.h"

LUA_API int luaH_findindex (lua_State *L, Table *t, StkId key);
LUA_EXTERN_C_END

namespace LuaPlus {

LUAPLUS_API bool LuaPlusH_next(LuaState* state, LuaObject* table, LuaObject* key, LuaObject* value)
{
	Table* t = hvalue(table->GetTObject());
	int i = luaH_findindex(*state, t, key->GetTObject());  /* find original element */
    for (i++; i < t->sizearray; i++) {  /* try first array part */
      if (!ttisnil(&t->array[i])) {  /* a non-nil value? */
        key->AssignInteger(state, i + 1);
        value->AssignTObject(state, &t->array[i]);
        return true;
	  }
	}
    for (i -= t->sizearray; i < sizenode(t); i++) {  /* then hash part */
      if (!ttisnil(gval(gnode(t, i)))) {  /* a non-nil value? */
        key->AssignTObject(state, key2tval(gnode(t, i)));
        value->AssignTObject(state, gval(gnode(t, i)));
        return true;
      }
	}
	return false;  /* no more elements */
}


#define markvalue(g,o) { checkconsistency(o); \
  if (iscollectable(o) && iswhite(gcvalue(o))) reallymarkobject(g,gcvalue(o)); }

#define markobject(g,t) { if (iswhite(obj2gco(t))) \
		reallymarkobject(g, obj2gco(t)); }

void LuaState::LuaPlusGCFunction(void* s)
{
	lua_State* L = (lua_State*)s;
	LuaState* state = (LuaState*)lua_getstateuserdata(L);
	if (!state)
		return;

    global_State* g = G(L);

	LuaObject* curObj = state->GetHeadObject()->m_next;
	while (curObj != (LuaObject*)state->GetTailObject())
	{
		markvalue(g, curObj->GetTObject());
		curObj = ((MiniLuaObject*)curObj)->m_next;
	}
}


/**
	\param tableObj The table to iterate the contents of.
	\param doReset If true, the Reset() function is called at constructor
		initialization time, allowing the iterator to be used immediately.
		If false, then Reset() must be called before iterating.
**/
LuaTableIterator::LuaTableIterator( LuaObject& tableObj, bool doReset ) :
	m_keyObj(tableObj.GetState()),
	m_valueObj(tableObj.GetState()),
	m_tableObj(tableObj),
	m_isDone(false)
{
	luaplus_assert(tableObj.IsTable());

	// If the user requested it, perform the automatic reset.
	if ( doReset )
		Reset();
}


/**
	The destructor does nothing.
**/
LuaTableIterator::~LuaTableIterator()
{
};


/**
	Start iteration at the beginning of the table.
**/
void LuaTableIterator::Reset()
{
	// Start afresh...
	LuaState* state = m_tableObj.GetState();

	// Start at the beginning.
	m_keyObj.AssignNil(state);

	// Start the iteration.  If the return value is 0, then the iterator
	// will be invalid.
	if ( !LuaPlusH_next(state, &m_tableObj, &m_keyObj, &m_valueObj) )
		m_isDone = true;
	else
		m_isDone = false;
}


/**
	Invalidates the iterator.  Call this function if you early abort from
	an iteration loop (such as before it hits the end).
**/
void LuaTableIterator::Invalidate()
{
	// This is a local helper variable so we don't waste space in the class
	// definition.
	LuaState* state = m_tableObj.GetState();
	m_keyObj.AssignNil(state);
	m_valueObj.AssignNil(state);
}

/**
	Go to the next entry in the table.

	\return Returns true if the iteration is done.
**/
bool LuaTableIterator::Next()
{
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	// This is a local helper variable so we don't waste space in the class
	// definition.
	LuaState* state = m_tableObj.GetState();

	// Do the Lua table iteration.
	if ( !LuaPlusH_next(state, &m_tableObj, &m_keyObj, &m_valueObj) )
	{
		m_isDone = true;
		return false;
	}

	// The iterator is still valid.
	return true;
}


/**
	\return Returns true if the iterator is valid (there is a current element).
**/
bool LuaTableIterator::IsValid() const
{
	return !m_isDone;
}


/**
	We can easily allow a prefix operator++.  Postfix would be a stack
	management nightmare.
**/
LuaTableIterator& LuaTableIterator::operator++()
{
	Next();
	return *this;
}


/**
	\return Returns true if the iterator is valid (there is a current element).
**/
LuaTableIterator::operator bool() const
{
	// If the iterator is valid, then we're good.
	return IsValid();
}


/**
	\return Returns a LuaObject describing the current key.
**/
LuaObject& LuaTableIterator::GetKey()
{
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	return m_keyObj;
}


/**
	\return Returns a LuaObject describing the current value.
**/
LuaObject& LuaTableIterator::GetValue()
{
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	return m_valueObj;
}


/**
	\param tableObj The table to iterate the contents of.
	\param doReset If true, the Reset() function is called at constructor
		initialization time, allowing the iterator to be used immediately.
		If false, then Reset() must be called before iterating.
	\param autoStackManagement If true, then for every Next() pass through
		the iterator, the stack is cleared to the iteration position (as if
		a LuaAutoBlock was run every single time).
**/
LuaStackTableIterator::LuaStackTableIterator( LuaStackObject tableObj, bool doReset,
		bool autoStackManagement ) :
	m_tableObj( tableObj ),
	m_startStackIndex( -1 ),
	m_autoStackManagement( autoStackManagement )
{
	// If the user requested it, perform the automatic reset.
	if ( doReset )
		Reset();
}


/**
	The destructor does nothing.
**/
LuaStackTableIterator::~LuaStackTableIterator()
{
};


/**
	Start iteration at the beginning of the table.
**/
void LuaStackTableIterator::Reset()
{
	// Start afresh...
	LuaState* state = m_tableObj.GetState();
	m_startStackIndex = state->GetTop();

	// Push the head stack entry.
	state->PushNil();

	// Start the iteration.  If the return value is 0, then the iterator
	// will be invalid.
	if ( state->Next( m_tableObj ) == 0 )
		m_startStackIndex = -1;
}


/**
	Invalidates the iterator.  Call this function if you early abort from
	an iteration loop (such as before it hits the end).
**/
void LuaStackTableIterator::Invalidate()
{
	// See if the iterator is already invalid.
	if ( !IsValid() )
		return;

	// This is a local helper variable so we don't waste space in the class
	// definition.
	LuaState* state = m_tableObj.GetState();

	if ( !m_autoStackManagement )
	{
		luaplus_assert( state->GetTop() <= m_startStackIndex + 1 );
	}

	// Set the stack back.
	state->SetTop( m_startStackIndex );

	// Invalidate the iterator.
	m_startStackIndex = -1;
}

/**
	Go to the next entry in the table.

	\return Returns true if the iteration is done.
**/
bool LuaStackTableIterator::Next()
{
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	// This is a local helper variable so we don't waste space in the class
	// definition.
	LuaState* state = m_tableObj.GetState();

	// Do any stack management operations.
	if ( m_autoStackManagement )
	{
		state->SetTop( m_startStackIndex + 1 );
	}
	else
	{
		// If this luaplus_assert fires, then you left something on the stack.
		luaplus_assert( state->GetTop() == m_startStackIndex + 1 );
	}

	// Do the Lua table iteration.
	if ( state->Next( m_tableObj ) == 0 )
	{
		// Invalidate the iterator.
		m_startStackIndex = -1;
		return false;
	}

	// The iterator is still valid.
	return true;
}


/**
	\return Returns true if the iterator is valid (there is a current element).
**/
bool LuaStackTableIterator::IsValid() const
{
	return m_startStackIndex != -1;
}


/**
	We can easily allow a prefix operator++.  Postfix would be a stack
	management nightmare.
**/
LuaStackTableIterator& LuaStackTableIterator::operator++()
{
	Next();
	return *this;
}


/**
	\return Returns true if the iterator is valid (there is a current element).
**/
LuaStackTableIterator::operator bool() const
{
	// If the iterator is valid, then we're good.
	return IsValid();
}


/**
	\return Returns a LuaStackObject describing the current key.
**/
LuaStackObject LuaStackTableIterator::GetKey()
{
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	return LuaStackObject( m_tableObj.GetState(), m_startStackIndex + 1 );
}


/**
	\return Returns a LuaStackObject describing the current value.
**/
LuaStackObject LuaStackTableIterator::GetValue()
{
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	return LuaStackObject( m_tableObj.GetState(), m_startStackIndex + 2 );
}


LuaCall::LuaCall(LuaObject& functionObj) :
	m_functionObj(functionObj),
	m_numArgs(0)
{
	luaplus_assert(m_functionObj.IsFunction());
	m_state = m_functionObj.GetState();
	m_startResults = m_state->GetTop() + 1;
	m_functionObj.Push();
}

LuaCall& operator<<(LuaCall& call, const LuaArgNil& /*value*/)
{
	call.m_state->PushNil();
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, float value)
{
	call.m_state->PushNumber(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, double value)
{
	call.m_state->PushNumber(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, int value)
{
	call.m_state->PushInteger(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, unsigned int value)
{
	call.m_state->PushInteger(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, const char* value)
{
	call.m_state->PushString(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, const lua_WChar* value)
{
	call.m_state->PushWString(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, lua_CFunction value)
{
	call.m_state->PushCClosure(value, 0);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, int (*value)(LuaState*))
{
	call.m_state->PushCClosure(value, 0);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, bool value)
{
	call.m_state->PushBoolean(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, void* value)
{
	call.m_state->PushLightUserData(value);
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, LuaStackObject& value)
{
	value.Push();
	call.m_numArgs++;
	return call;
}

LuaCall& operator<<(LuaCall& call, LuaObject& value)
{
	value.Push();
	call.m_numArgs++;
	return call;
}

LuaStackObject LuaCall::operator<<(const LuaRun& run)
{
    int resultsStackPos = m_state->GetTop() - m_numArgs;
	int err = m_state->PCall(m_numArgs, run.m_numResults, run.m_alertStackPos);
	if (err != 0)
	{
		LuaStackObject errObj(m_state, -1);
        if (errObj.IsString())
        {
            // Does this string persist long enough?
            luaplus_throw(errObj.GetString());
        }
        else
        {
            char buf[200];
            sprintf(buf, "unknown lua error, code: %d", err);
            luaplus_throw(buf);
        }
	}
	return LuaStackObject(m_state, resultsStackPos);
}

LuaCall& LuaCall::operator=(const LuaCall& src)
{
	m_state = src.m_state;
	m_functionObj = src.m_functionObj;
	m_numArgs = src.m_numArgs;
	m_startResults = src.m_startResults;

	return *this;
}


LuaStateOutFile::LuaStateOutFile() :
    m_file( NULL ),
    m_fileOwner( false )
{
}


LuaStateOutFile::LuaStateOutFile(const char* fileName) :
    m_file( NULL ),
    m_fileOwner( false )
{
	Open(fileName);
}


LuaStateOutFile::~LuaStateOutFile()
{
	if ( m_file  &&  m_fileOwner )
		fclose( m_file );
}


bool LuaStateOutFile::Open( const char* fileName )
{
	Close();

    if (fileName[0] == '+')
	    m_file = fopen( fileName + 1, "a+b" );
    else
	    m_file = fopen( fileName, "wb" );
	m_fileOwner = true;

	return m_file != NULL;
}


void LuaStateOutFile::Close()
{
	if ( m_file  &&  m_fileOwner )
		fclose( m_file );
}


void LuaStateOutFile::Print( const char* str, ... )
{
	char message[ 800 ];
	va_list arglist;

	va_start( arglist, str );
	vsprintf( message, str, arglist );
	va_end( arglist );

	fputs( message, m_file );
}


bool LuaStateOutFile::Assign( FILE* file )
{
	m_file = file;
	m_fileOwner = false;

	return true;
}

} // namespace LuaPlus
