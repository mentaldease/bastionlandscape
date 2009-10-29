#include "LuaPlus/LuaPlus.h"
using namespace LuaPlus;
#include "SimpleHeap.h"
#include "Timer.h"
#include <math.h>
#include <assert.h>
#include <string>
#include <crtdbg.h>

class Foo
{
public:
	int check;
	Foo() { check = 0x12345678; }
	void bar() { assert(check == 0x12345678); }
};

void NewUserDataBoxTest()
{
	Foo f;

	LuaStateOwner lua;

	LuaObject obj = lua->NewUserDataBox(&f);
	lua->NewTable();
    LuaObject meta = lua->StackTop();
	meta.RegisterObjectDirect("bar", (Foo*)0, &Foo::bar);
	obj.SetMetaTable(meta);

	lua->GetGlobals().SetObject("f", obj);

	f.bar();   // OK
	lua->DoString("f:bar()");   // ASSERT
};

	
namespace LPCD
{
    inline bool	Match(TypeWrapper<std::string>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TSTRING;  }
	inline bool	Match(TypeWrapper<std::string&>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TSTRING;  }
	inline bool	Match(TypeWrapper<const std::string&>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TSTRING;  }
    inline std::string Get(TypeWrapper<std::string>, lua_State* L, int idx)
		{  return static_cast<const char*>(lua_tostring(L, idx));  }
    inline std::string Get(TypeWrapper<std::string&>, lua_State* L, int idx)
		{  return static_cast<const char*>(lua_tostring(L, idx));  }
    inline std::string Get(TypeWrapper<const std::string&>, lua_State* L, int idx)
		{  return static_cast<const char*>(lua_tostring(L, idx));  }
	inline void Push(lua_State* L, std::string& value)								{  lua_pushstring(L, value.c_str());  }
	inline void Push(lua_State* L, const std::string& value)						{  lua_pushstring(L, value.c_str());  }
}


std::string TestStdString1(std::string str)
{
    printf("%s\n", str.c_str());
    return "Hello";
}


using namespace LuaPlus;

void TestStdString()
{
    LuaStateOwner state;

	LuaObject globalsObj = state->GetGlobals();
	globalsObj.RegisterDirect("TestStdString1", TestStdString1);
    LuaFunction<std::string> funcCall(state, "TestStdString1");
    std::string value = funcCall("World");
    printf("%s\n", value.c_str());
}


/**
	The LuaState class is able to make Lua callbacks look like natural extensions
	of the LuaState class.
**/
static int LS_PrintNumber(LuaState* state)
{
	LuaStack args(state);

	// Verify it is a number and print it.
	if (args[1].IsNumber())
		printf("%f\n", args[1].GetNumber());

	// No return values.
	return 0;
}


/**
**/
class ObjectWrapper
{
public:
	void Zero()
	{
	}

	void Print(const char* str)
	{
		printf("%s\n", str);
	}

	float Add(float num1, float num2)
	{
		return num1 + num2;
	}

	float Add3(float num1, float num2, float num3)
	{
		return num1 + num2 + num3;
	}

	float Add4(float num1, float num2, float num3, float num4)
	{
		return num1 + num2 + num3 + num4;
	}

	int LS_Add(LuaState* state)
	{
		LuaStack args(state);

		// Verify it is a number and print it.
		if (args[1].IsNumber()  &&  args[2].IsNumber())
		{
			state->PushNumber(args[1].GetNumber() + args[2].GetNumber() );
		}
		else
		{
			state->PushNumber( 0.0 );
		}

		// 1 return value.
		return 1;
	}

	char buffer[1000];
};


void Add4NoReturn(float num1, float num2, float num3, float num4)
{
}

/**
**/
void DoScriptFormatTest()
{
	LuaStateOwner state;
	state->DoFile("ScriptVectorDump.lua");
}


int Test(int num)
{
	num;
	return 5;
}


/**
	Demonstrate registering callback functions for the Lua script.
**/
void DoScriptCallbackTest()
{
/*	LuaStateOwner state;

	state->GetGlobals().Register("PrintNumber", LS_PrintNumber);

	ObjectWrapper obj;
	state->GetGlobals().Register("Add", obj, ObjectWrapper::LS_Add);

	state->DoFile("ScriptCallbackTest.lua");
*/

	LuaStateOwner state;
/*
	state->GetGlobals().Register("PrintNumber", LS_PrintNumber);

	ObjectWrapper obj;
	state->GetGlobals().Register("Add", obj, ObjectWrapper::LS_Add);

	state->DoFile("ScriptCallbackTest.lua");

	// Now call it ourselves.
	LuaFunction<int> function(state, "PrintNumber");
	function(22);

	LuaFunction<float> add(state, "Add");
	float ret = add(22, 11);
*/
	LuaObject globalsObj = state->GetGlobals();

	LuaObject testTableObj = globalsObj.CreateTable("TestTable");
	testTableObj.Register("PrintNumber", LS_PrintNumber);
	state->DoString("TestTable.PrintNumber(1000)");

	// Test auto thing.
	int top = state->GetTop();
	globalsObj.RegisterDirect("Test", Test);
	int top2 = state->GetTop();
	globalsObj.RegisterDirect("sin", (double (*)(double))&sin);
	LuaFunction<int> funcCall(state, "Test");
	int value = funcCall(10);

	LuaFunction<float> funcCall2(state, "sin");
	float value2 = funcCall2(5);

	ObjectWrapper obj;
	globalsObj.RegisterDirect("Add", obj, &ObjectWrapper::Add);
	globalsObj.RegisterDirect("Add3", obj, &ObjectWrapper::Add3);
	globalsObj.RegisterDirect("Add4", obj, &ObjectWrapper::Add4);
	globalsObj.RegisterDirect("Add4NoReturn", Add4NoReturn);
	globalsObj.RegisterDirect("Zero", obj, &ObjectWrapper::Zero);
	globalsObj.RegisterDirect("Print", obj, &ObjectWrapper::Print);
	LuaFunction<double> addCall(state, "Add");
	LuaFunctionVoid printCall(state, "Print");
	LuaFunctionVoid zeroCall(state, "Zero");
	zeroCall();
	printCall("Stuff");
	double result = addCall(10, 2);

}


/**
	Demonstrate reading and saving a script.
**/
void DoScriptSaveTest()
{
	LuaStateOwner state;

	try
	{
		state->DoFile( "ScriptSaveTest.lua" );
	}
	catch (LuaException&)
	{
	}

//	state->DumpGlobalsFile("ScriptSaveTest.dmp");
}


/**
	Demonstrates walking an array table.
**/
void DoScriptArrayTest()
{
	LuaStateOwner state;
	if (state->DoFile("ScriptArrayTest.lua") != 0)
	{
		printf("%s\n", LuaStackObject(state, -1).GetString());
		printf("Unable to read ScriptArrayTest.lua.\n");
	}
	LuaObject testTableObj = state->GetGlobals()[ "TestArray" ];
	for (int i = 1; ; ++i)
	{
		LuaObject entryObj = testTableObj[ i ];
		if (entryObj.IsNil())
			break;
		if (entryObj.IsNumber())
			printf("%f\n", entryObj.GetNumber());
		else if (entryObj.IsString())
			printf("%s\n", entryObj.GetString());
	}
}


static int LS_LightUserDataCall( LuaState* state )
{
	LuaStack args(state);
	bool isLightUserData = args[ 1 ].IsLightUserData();
	const void* ptr = args[ 1 ].GetUserData();
    state->PushNumber(10000);
	return 1;
}


void TestPointer()
{
	LuaStateOwner state;

	state->GetGlobals().Register("LightUserDataCall", LS_LightUserDataCall);
	LuaObject funcObj = state->GetGlobal("LightUserDataCall");
	LuaCall call = funcObj;
	LuaStackObject retObj = call << (void*)0xfedcba98 << LuaRun();
    printf("%f\n", retObj.GetNumber());
}


/*void LuaStackTableIteratorTest()
{
	LuaStateOwner state;

	state->DoString( "Table = { Hi = 5, Hello = 10, Yo = 6 }" );

	int origTop = state->GetTop();

	LuaStackObject obj = state->GetGlobal("Table");

	for ( LuaStackTableIterator it( obj ); it; it.Next() )
	{
		const char* key = it.GetKey().GetString();
		int num = it.GetValue().GetInteger();
	}

	LuaStackObject obj2 = state->GetGlobal("Table");

	for ( it.Reset(); it; ++it )
	{
		const char* key = it.GetKey().GetString();
	}
}
*/

void LuaTableIteratorTest()
{
	LuaStateOwner state;

	state->DoString( "Table = { Hi = 5, Hello = 10, Yo = 6 }" );

	int origTop = state->GetTop();

	LuaObject obj = state->GetGlobal("Table");

	for ( LuaTableIterator it( obj ); it; it.Next() )
	{
		const char* key = it.GetKey().GetString();
		int num = it.GetValue().GetInteger();
	}
}


void TestNewCall()
{
	LuaStateOwner state;

	state->DoString("function Add(x, y) return x + y end");
	LuaObject funcObj = state->GetGlobal("Add");
	int top = state->GetTop();
	LuaCall call = funcObj;
	LuaObject retObj = call << 2 << 7 << LuaRun();
	int top2 = state->GetTop();
//	funcObj() << LuaRun();
}

using namespace SimpleMem;

SimpleHeap* heap;

void* ReallocFunction(void* ud, void* ptr, size_t osize, size_t nsize, const char* allocName, unsigned int flags)
{
	SimpleHeap* heap = (SimpleHeap*)ud;

	if (nsize == 0)
	{
		heap->Free(ptr);
		return NULL;
	}

	if (flags == LUA_ALLOC_TEMP)
	{
		return heap->Realloc(ptr, nsize, SimpleHeap::FIRSTFIT_TOP,
			allocName, __FILE__, __LINE__);
	}

	return heap->Realloc(ptr, nsize, SimpleHeap::FIRSTFIT_BOTTOM,
		allocName, __FILE__, __LINE__);
}


int checkpoint = 0;

class MemLogFile
{
public:
	MemLogFile()
	{
		Create("c:\\MemoryDump.txt");
	}

	~MemLogFile()
	{
		Close();
	}

	void Close()
	{
		if (m_file)
		{
			fclose(m_file);
			m_file = NULL;
		}
	}

	void Create(const char* fileName)
	{
		Close();

		strcpy(m_fileName, fileName);
		m_file = fopen(m_fileName, "wt");
		assert(m_file);
		Write("Heap Name\tBlock Type\tAddress\tRequested Size\tTotal Size\tIndex\tName\tFile Name\tLine Number\n");
	}

	void Open()
	{
		Close();
		m_file = fopen(m_fileName, "at");
		assert(m_file);
	}

	void Write(const char* msg, ...)
	{
		va_list args;
		va_start(args, msg);
		vfprintf(m_file, msg, args);
		va_end(args);
	}

	void Flush()
	{
		fflush(m_file);
	}

	operator FILE*()				{  return m_file;  }

protected:
	FILE* m_file;
	char m_fileName[_MAX_PATH];
};


MemLogFile& GetMemLogFile()
{
	static MemLogFile autoFile;
	return autoFile;
}


void DumpIt()
{
	heap->HeapDump(GetMemLogFile(), checkpoint, checkpoint);
}


int Chunkwriter(lua_State *L, const void* p,
                size_t sz, void* ud)
{
	FILE* file = (FILE*)ud;
	fwrite(p, sz, 1, file);
	return sz;
}


void TestHeap()
{
	SimpleHeap mainHeap("Test");
	mainHeap.Initialize(1 * 1024 * 1024);
	heap = &mainHeap;

	GetMemLogFile().Create("c:\\dump.txt");

	heap->SetAlignment(1);

	heap->Alloc(1);
	DumpIt();
	void* p0 = heap->Alloc(1);
	DumpIt();
	void* p1 = heap->Alloc(1);
	DumpIt();
	heap->Free(p0);
	DumpIt();
	heap->Free(p1);
	DumpIt();

	heap->Alloc(100);
	DumpIt();
	heap->Alloc(100);
	DumpIt();
	heap->Alloc(100);
	DumpIt();
	heap->Alloc(100, SimpleHeap::FIRSTFIT_TOP);
	DumpIt();
	heap->Alloc(100, SimpleHeap::FIRSTFIT_TOP);
	DumpIt();
	heap->Free(p1);
	DumpIt();
	heap->Alloc(50);
	DumpIt();
	heap->Alloc(50);
	DumpIt();
	heap->Alloc(1047300);
	DumpIt();
	heap->Alloc(2, SimpleHeap::FIRSTFIT_TOP);
	DumpIt();
}


void TestGCObject()
{
	TestHeap();
	SimpleHeap mainHeap("Test");
	mainHeap.Initialize(1 * 1024 * 1024);
	heap = &mainHeap;

	GetMemLogFile().Create("c:\\dump.txt");

	lua_setdefaultallocfunction(ReallocFunction, &mainHeap);
	LuaStateOwner state(false);
	lua_setdefaultallocfunction(NULL, NULL);
	state->CollectGarbage();

	heap->LogStats();
	DumpIt();

//	checkpoint = heap->SetCheckpoint();

/*	{
		LuaObject stringObj(state);
		stringObj.AssignString("Hello, world!");
		LuaObject globalsObj = state->GetGlobals();
	}

	state->CollectGarbage();
*/
//	state->DoString("Table = { 0, 1, 2, 'Hello', nil, 'Hi', Yo = 'My Stuff', NobodysHome = 5, NestedTable = { 1, 2, 3, { 'String', }, { 'Table2' } }, { 'String1' } }");
//	state->DoString("a = 5");
/*	FILE* inFile = fopen("c:\\dump.dat", "rb");
	fseek(inFile, 0, SEEK_END);
	int size = ftell(inFile);
	fseek(inFile, 0, SEEK_SET);
	BYTE* buf = new BYTE[size];
	fread(buf, size, 1, inFile);
	fclose(inFile);

	state->DoBuffer((const char*)buf, size, NULL);
	state->CollectGarbage();
*/
	LPCSTR strbuf = "a = 5";
//	state->CheckStack(1);
//	state->LoadBuffer(strbuf, strlen(strbuf), "Stuff");
	state->DoString(strbuf);
/*  FILE* file = fopen("c:\\dump.dat", "wb");
	state->CheckStack(1);
	state->Dump(Chunkwriter, file);
	fclose(file);*/
	state->CollectGarbage();

	{
		LuaObject obj;

	}
}


void SetUserData()
{
	SimpleHeap mainHeap("Test");
	mainHeap.Initialize(1 * 1024 * 1024);
	heap = &mainHeap;

	GetMemLogFile().Create("c:\\dump.txt");

	{
		lua_setdefaultallocfunction(ReallocFunction, &mainHeap);
		LuaStateOwner state(false);
		lua_setdefaultallocfunction(NULL, NULL);
		{
			LuaObject obj(state);
			obj.AssignNewTable(state);
			assert(obj.IsTable());

			// number
			obj.SetUserData(1, (void*)0x12345678);
			assert(obj[1].GetUserData() == (void*)0x12345678);

			// string
			obj.SetUserData("Hello", (void*)0x87654321);
			assert(obj["Hello"].GetUserData() == (void*)0x87654321);

			// object
			LuaObject stringObj(state);
			stringObj.AssignString(state, "Test");
			obj.SetUserData(stringObj, (void*)0x02468024);
			assert(obj[stringObj].GetUserData() == (void*)0x02468024);
		}
		state->CollectGarbage();
		DumpIt();
	}
}


void CloneTest()
{
	SimpleHeap mainHeap("Test");
	mainHeap.Initialize(1 * 1024 * 1024);
	heap = &mainHeap;

	GetMemLogFile().Create("c:\\dump.txt");

	lua_setdefaultallocfunction(ReallocFunction, &mainHeap);
	LuaStateOwner state;
	lua_setdefaultallocfunction(NULL, NULL);

	LuaObject valueObj(state);
	valueObj.AssignBoolean(state, true);

	LuaObject cloneObj = valueObj.Clone();

	Timer timer;
	timer.Start();
	state->DoString("Table = { 0, 1, 2, 'Hello', nil, 'Hi', Yo = 'My Stuff', NobodysHome = 5, NestedTable = { 1, 2, 3, { 'String', }, { 'Table2' } }, { 'String1' } }");
	timer.Stop();
	printf("DoString: %f\n", timer.GetMillisecs());

	LuaObject tableObj = state->GetGlobal("Table");
	timer.Reset();
	timer.Start();
	LuaObject clonedTableObj = tableObj.Clone();
	timer.Stop();
	printf("Clone: %f\n", timer.GetMillisecs());

	clonedTableObj.SetNil("Yo");

	state->DumpObject("c:\\test1.lua", "Table", tableObj, false);
	state->DumpObject("c:\\test2.lua", "Table", clonedTableObj, false);
}


int CallbackA(LuaState* state)
{
	for (int i = 0; i < 10; ++i)
	{
		printf("Hello%d\n", i);
//		state->CoYield(0);
	}

	return 0;
}


int CallbackB(LuaState* state)
{
	for (int i = 0; i < 15; ++i)
	{
		printf("Hi%d\n", i);
//		state->CoYield(0);
	}

	return 0;
}


void TestThreeThreads()
{
	LuaStateOwner state;
	LuaObject thread1(LuaState::CreateThread(state));
	LuaObject thread2(LuaState::CreateThread(state));

	LuaObject obj(thread1);
	obj.AssignNewTable(state);

	LuaObject obj2(thread2);
	obj2.AssignString(state, "Hi");

	LuaObject obj3(thread2);
	obj3.AssignWString(state, (const lua_WChar*)L"Hello");

	state->GetGlobals().Register("CallbackA", CallbackA);
	state->GetGlobals().Register("CallbackB", CallbackB);

	LuaFunctionVoid callbackA = thread1.GetState()->GetGlobal("CallbackA");
	LuaFunctionVoid callbackB = thread2.GetState()->GetGlobal("CallbackB");

	callbackA();
	callbackB();

	state->CollectGarbage();
}


void TestDumpGlobals()
{
	LuaStateOwner state(false);
	state->DoString("GlobalTable = { 1, 2, 3, 4 }");
	state->DoString("GlobalValue = 5");
	state->DumpGlobals("c:\\dump.lua");
}


void DumpTest()
{
	LuaStateOwner state(false);
	LuaObject complexObj = state->GetGlobals().CreateTable("Complex");
	complexObj.SetString("d:\\Test\\Stuff\\\xff\xfe", "An entry");
	state->DumpObject("c:\\test.lua", "Complex", complexObj);
}


void TestANSIFile()
{
	LuaStateOwner state(true);
	state->DoFile("TestANSI.lua");

	LuaObject sObj = state->GetGlobal("s");
	const char* str = sObj.GetString();
}


void TestUnicodeFile()
{
	LuaStateOwner state(true);
	state->DoFile("TestUnicode.lua");

	LuaObject sObj = state->GetGlobal("s");
	const lua_WChar* str = sObj.GetWString();
}


void ReadUnicodeFile()
{
    LuaStateOwner state(true);
    state->DoFile("ReadUnicodeFile.lua");
}


void IntegerTest()
{
	LuaStateOwner state(true);
	state->DoString("i1 = 10");
	state->DoString("i2 = 20; f1 = 15.5; f2 = .5");
	state->DoString("i3 = -10");
	state->DoString("print(type(i1), type(i2), type(i3), type(f1), type(f2))");
	state->DoString("res1 = i1 + i2");
	state->DoString("print(type(res1), res1)");
	state->DoString("res1 = i1 + f1");
	state->DoString("print(type(res1), res1)");
	state->DoString("res = -i1");
	state->DoString("print(type(res), res)");
	state->DoString("print(i1 | 3)");
	state->DoString("print(i1 & 2)");
	state->DoString("print(i1 ^ 12)");
	state->DoString("print(1 << 4)");
	state->DoString("print(32 >> 4)");

	state->DoString("i1 = 0x1000");
	state->DoString("print(i1, type(i1))");
	state->DoString("print(i1 | 0x80000000)");

	state->DoString("for index = 1, 10, 0.5 do print(index) end");
	state->DoString("for index = 1, 10, 2 do print(index) end");
}


void LookupTest()
{
	LuaStateOwner state;
    state->DoString("s = { { Hi = { abc = 10 } }, { Hello = { xyz = 20 } } }");

    LuaObject obj = state->GetGlobals().Lookup("s.1.Hi.x");
    obj = state->GetGlobals().Lookup("s.1.Hi.abc");
    obj = state->GetGlobals().Lookup("s.2.Hello.xyz");
}


class MultiObject
{
public:
	MultiObject(int num) :
		m_num(num)
	{
	}

	int Print(LuaState* state)
	{
		printf("%d\n", m_num);
		return 0;
	}

	void Print2(int num)
	{
		printf("%d %d\n", m_num, num);
	}

protected:
	int m_num;
};


void MultiObjectTest()
{
	LuaStateOwner state;

	LuaObject metaTableObj = state->GetGlobals().CreateTable("MultiObjectMetaTable");
	metaTableObj.SetObject("__index", metaTableObj);
	metaTableObj.RegisterObjectFunctor("Print", &MultiObject::Print);
	metaTableObj.RegisterObjectDirect("Print2", (MultiObject*)0, &MultiObject::Print2);

	MultiObject obj1(10);
	LuaObject obj1Obj = state->BoxPointer(&obj1);
	obj1Obj.SetMetaTable(metaTableObj);
	state->GetGlobals().SetObject("obj1", obj1Obj);

	MultiObject obj2(20);
	LuaObject obj2Obj = state->BoxPointer(&obj2);
	obj2Obj.SetMetaTable(metaTableObj);
	state->GetGlobals().SetObject("obj2", obj2Obj);

	state->DoString("obj1:Print()");
	state->DoString("obj2:Print()");
	state->DoString("obj1:Print2(5)");
	state->DoString("obj2:Print2(15)");

	LuaObject table1Obj = state->GetGlobals().CreateTable("table1");
	table1Obj.SetLightUserData("__object", &obj1);
	table1Obj.SetMetaTable(metaTableObj);

	LuaObject table2Obj = state->GetGlobals().CreateTable("table2");
	table2Obj.SetLightUserData("__object", &obj2);
	table2Obj.SetMetaTable(metaTableObj);

	state->DoString("table1:Print()");
	state->DoString("table2:Print()");
	state->DoString("table1:Print2(5)");
	state->DoString("table2:Print2(15)");
}


class lua_StateObjectHelper
{
public:
	lua_StateObjectHelper() :
		m_str("Me")
	{
	}

	int PrintSomething(lua_State* L)
	{
		printf("Hello, world! %s\n", m_str);
		return 0;
	}

protected:
	const char* m_str;
};


int PrintSomethingGlobal(lua_State* L)
{
	printf("In global PrintSomething.\n");
	return 0;
}


void lua_StateCallbackTest()
{
	lua_State* L = lua_open();

	lua_StateObjectHelper helper;

	lua_pushstring(L, "PrintSomething");
	lua_pushfunctorclosureex(L, helper, &lua_StateObjectHelper::PrintSomething, 0);
	lua_settable(L, LUA_GLOBALSINDEX);

	lua_pushstring(L, "PrintSomethingGlobal");
	lua_pushfunctorclosure(L, PrintSomethingGlobal, 0);
	lua_settable(L, LUA_GLOBALSINDEX);

	luaL_dostring(L, "PrintSomething()");
	luaL_dostring(L, "PrintSomethingGlobal()");

	lua_close(L);
}


void MemoryTest()
{
	SimpleHeap mainHeap("Test");
	mainHeap.Initialize(1 * 1024 * 1024);
	heap = &mainHeap;

	GetMemLogFile().Create("c:\\dump.txt");

	{
		lua_setdefaultallocfunction(ReallocFunction, &mainHeap);
		LuaStateOwner state(true);
		lua_setdefaultallocfunction(NULL, NULL);
		{
//			state->DoFile("s:\\svndb\\lua-5.0.1\\test\\sieve.lua");
			DumpIt();
		}
		state->CollectGarbage();
		DumpIt();
	}
}


void RCTest()
{
	LuaStateOwner state(false);
	LuaObject stringObj(state);
	stringObj.AssignString(state, "Test");
	stringObj.AssignWString(state, (const lua_WChar*)L"Wide String");
}


void ExceptionTest()
{
	LuaStateOwner state;
	state->DoString("MyTable = { 1, 2, 3, 4 }");

	try
	{
		void* data = state->GetGlobal("MyTable")[1].GetUserData();
	}
	catch (const LuaException& /*e*/)
	{
		int hi = 5; (void)hi;
	}

	try
	{
		LuaObject obj = state->GetGlobals()["MyTable"][1][2];
	}
	catch (const LuaException& /*e*/)
	{
		int hi = 5; (void)hi;
	}
}


void BisectTest()
{
	LuaStateOwner state;
	state->DoFile("../../Test/bisect.lua");
}


class MyPropertyTest
{
public:
	MyPropertyTest()
	{
	}

	void SetVar(int index)
	{
		m_var = index;
	}

	int m_var;
	int m_var2;
};


int aGlobalVariable = 10;


void PropertyTest()
{
	LuaStateOwner state;
	
	MyPropertyTest test;

	LuaObject metaTableObj = state->GetRegistry().CreateTable("MyPropertyClassMetaTable");
	LPCD::MetaTable_IntegratePropertySupport(metaTableObj);
	metaTableObj.RegisterObjectDirect("SetVar", (MyPropertyTest*)0, &MyPropertyTest::SetVar);

	LuaObject table1Obj = state->GetGlobals().CreateTable("table1");
	table1Obj.SetLightUserData("__object", &test);
	table1Obj.SetMetaTable(metaTableObj);

	LPCD::PropertyCreate(metaTableObj, "var", &MyPropertyTest::m_var);

	state->DoString("table1.var = 20");
	state->DoString("print(table1.var)");

	test.m_var2 = 100;

	LPCD::Register_MemberPropertyGetFunction(table1Obj, "GetTheVar2", &MyPropertyTest::m_var2);
	LPCD::Register_MemberPropertySetFunction(table1Obj, "SetTheVar2", &MyPropertyTest::m_var2);
	state->DoString("print(table1:GetTheVar2())");
	state->DoString("print(table1:SetTheVar2(50))");

	LPCD::PropertyCreate(metaTableObj, "var2", &MyPropertyTest::m_var2);

	state->DoString("print(table1.var2)");
	state->DoString("table1.var2 = 60");
	state->DoString("print(table1.var2)");

	try
	{
		state->DoString("table1.var3 = 60");
	}
	catch (LuaException&)
	{
	}

	try
	{
		state->DoString("print(table1.var3)");
	}
	catch (LuaException&)
	{
	}

	state->DoString("table1:SetVar(600)");

	LPCD::Register_GlobalPropertyGetFunction(state->GetGlobals(), "GetAGlobalVariable", &aGlobalVariable);
	LPCD::Register_GlobalPropertySetFunction(state->GetGlobals(), "SetAGlobalVariable", &aGlobalVariable);

	state->DoString("print(GetAGlobalVariable())");
	state->DoString("SetAGlobalVariable(50)");
	state->DoString("print(GetAGlobalVariable())");

	int end = 5; (void)end;
}


int PropertyMetaTable_newindex(LuaState* state)
{
	LuaStack args(state);
	LuaObject propsObj = args[1].GetMetaTable()["__props"];
	if (propsObj.IsTable())
	{
		LuaObject propObj = propsObj[args[2]];
		if (propObj.IsNil())
		{
			luaL_argerror(*state, 1, "The property is not available.");
		}

		LuaObject funcObj = propObj[2];
		args[1].Push();
		args[3].Push();
		state->Call(2, 1);
		return 1;
	}

	return 0;
}


void SetTest()
{
	LuaStateOwner state;

	state->DoString("abs = 5");
#if _MSC_VER > 1300
	state->GetGlobals().Set("abs", 10);
#endif
	state->GetGlobals().SetNil("abs");
}


void BadDoString()
{
	LuaStateOwner state;

	try
	{
		state->DoString("MyTable = { Name1 = 5, Name2 = 10 Name3 = 15 }");
	}
	catch (LuaException&)
	{
	}
}


void LoadCompiledScript()
{
	LuaStateOwner state;
	int ret = state->DoFile("CompileMe.lc");
	ret;
}



class VECTOR 
{
public:
	double x,y,z; 
}; 


class MONSTER 
{
public:
	int alive;
	void *mesh; 
	VECTOR position; 
	char name[32];
}; 

namespace LPCD
{
	template <int NUM_CHARS>
	inline void Push(lua_State* L, char value[NUM_CHARS])
	{
		lua_pushstring(L, value);
	}

	template <int NUM_CHARS>
	inline bool	Match(TypeWrapper<char [NUM_CHARS]>, lua_State* L, int idx)
	{
		return lua_type(L, idx) == LUA_TSTRING;
	}

/*	template <int NUM_CHARS>
	inline const char* Get(TypeWrapper<char [NUM_CHARS]>, lua_State* L, int idx)
	{
		return lua_tostring(L, idx);
	}
*/
	inline void Push(lua_State* L, const VECTOR& value)
	{
		LuaState* state = LuaState::CastState(L);
		LuaObject obj = state->BoxPointer((void*)&value);
		obj.SetMetaTable(state->GetRegistry()["VECTOR"]);
	}

	inline bool	Match(TypeWrapper<VECTOR>, lua_State* L, int idx)
	{
		LuaState* state = LuaState::CastState(L);
		LuaObject obj = state->Stack(idx);
		return obj.GetMetaTable() == state->GetRegistry()["VECTOR"];
	}

	inline bool	Match(TypeWrapper<VECTOR&>, lua_State* L, int idx)
	{
		LuaState* state = LuaState::CastState(L);
		LuaObject obj = state->Stack(idx);
		return obj.GetMetaTable() == state->GetRegistry()["VECTOR"];
	}

	inline VECTOR Get(TypeWrapper<VECTOR>, lua_State* L, int idx)
	{
		return *(VECTOR*)lua_unboxpointer(L, idx);
	}

	inline VECTOR& Get(TypeWrapper<VECTOR&>, lua_State* L, int idx)
	{
		return *(VECTOR*)lua_unboxpointer(L, idx);
	}
}


void PassVector(VECTOR& vec)
{
	printf("Vector: %f, %f, %f\n", vec.x, vec.y, vec.z);
}

using namespace LuaPlus;

void VectorMonsterMetatableTest()
{
	LuaStateOwner state(true);

	LuaObject vectorMetaTableObj = state->GetRegistry().CreateTable("VECTOR"); 
	LPCD::MetaTable_IntegratePropertySupport(vectorMetaTableObj); 

	LPCD::PropertyCreate(vectorMetaTableObj, "x", &VECTOR::x); 
	LPCD::PropertyCreate(vectorMetaTableObj, "y", &VECTOR::y); 
	LPCD::PropertyCreate(vectorMetaTableObj, "z", &VECTOR::z); 

	LuaObject monsterMetaTableObj = state->GetRegistry().CreateTable("MONSTER"); 
	LPCD::MetaTable_IntegratePropertySupport(monsterMetaTableObj); 

	LPCD::PropertyCreate(monsterMetaTableObj, "alive", &MONSTER::alive); 
	LPCD::PropertyCreate(monsterMetaTableObj, "mesh", &MONSTER::mesh); 

	LPCD::PropertyCreate(monsterMetaTableObj, "position", &MONSTER::position);
//	LPCD::PropertyCreate(monsterMetaTableObj, "name", &MONSTER::name);

	MONSTER monster;

	LuaObject monsterObj = state->GetGlobals().CreateTable("Monster");
	monsterObj.SetLightUserData("__object", &monster);
	monsterObj.SetMetaTable(monsterMetaTableObj);

	state->DoString("Monster.alive = 1");
	state->DoString("Monster.position.x = 5");
	state->DoString("Monster.position.y = 10");
	state->DoString("Monster.position.z = 15");
	state->DoString("print(Monster.position.x)");

	state->GetGlobals().RegisterDirect("PassVector", &PassVector);
	state->DoString("PassVector(Monster.position)");
}


///////////////////////////////////////////////////////////////////////////////
// issue_1
//
// Cleanarray is not validating the range before accessing the stack elements.
// Lua allows setting the stack pointer to an invalid position without
// accessing the elements, although any subsequent stack operation would crash
// as well; this is minor however the API should behave similarly.
//////////////////////////////////////////////////////////////////////////////
void issue_1( lua_State *L )
{
    lua_pop(L, 2);
    printf( "Issue 1 passed\n" );
}

///////////////////////////////////////////////////////////////////////////////
// issue_2
//
// Related to the LuaRC cleanarray modification to ldo.c luaD_reallocstack when
// growing the stack. The elements within L->old_stack_last < L->stack_last
// (within the new stack) are assumed to be initialized by cleanarray. These
// are uninitialized by realloc and causes the (i_o2->tt >= LUA_TSTRING) test
// to pass depending on the contents of that uninitialized memory. It then
// crashes in ((--gcvalue(i_o2)->gch.ref)<=0)) dereferencing i_o2->value.
//
//     L->old_stack --------  L->stack --------
//                  |  D1  |           |  D1  |
//                  --------           --------
//                  |  E   |           |  E   |
//                  --------           --------
//                  |  D2  |           |  D2  |
//                  --------           --------
//       L->old_top |      |    L->top |      |
//                  --------           --------
//                  |  D3  |           |  D3  | properly cleaned
//                  --------           --------
//L->old_stack_last | XXXX |      tpos | ???? | cleanvalue called on this
//                  --------           --------
//                                     | ???? | cleanvalue called on this
//                                     --------
//                       L->stack_last | XXXX |
//                                     --------
//
// It should probably cleararray from L->top to tpos, then call
// setnilvalue2n on (tpos < L->stack_last).  The following seems to work.
//
//    void luaD_reallocstack (lua_State *L, int newsize) {
//        // DAP BEGIN -- realloc temp fix
//        TObject* tpos;
//        int remaining = L->stack_last - L->top; // used to proper<snip>
//        // DAP END -- realloc temp fix
//
//        TObject *oldstack = L->stack;
//        luaM_reallocvector(L, L->stack, L->stacksize, newsize, TObject);
//        L->stacksize = newsize;
//        L->stack_last = L->stack+newsize-1-EXTRA_STACK;
//        correctstack(L, oldstack);
//
//        //cleanarray(L->top, L->stack_last);
//        // DAP BEGIN -- realloc temp fix
//        tpos = L->top+remaining;
//        cleanarray(L->top, tpos);
//        while (tpos < L->stack_last)
//            setnilvalue2n(tpos++);
//        // DAP END -- realloc temp fix
//    }
//////////////////////////////////////////////////////////////////////////////
void issue_2( lua_State *L )
{
    for (int i = 0; i < 2000; ++i)
    {
        lua_checkstack(L, 1);
        lua_pushstring(L, "Hello");
    }

    lua_pop(L, 1999);
    lua_pop(L, 1);

//jj    lua_setgcthreshold(L, 0);
}

///////////////////////////////////////////////////////////////////////////////
// issue_3
//
// The collection of userdata with __gc metatables can corrupt existing stack
// elements.
//
//-----------------------------------------------------------------------------
// Assertion failed: i_o->value.gc->gch.tt == 4, file c:\dev\luarc\src\lapi.c,
// line 416
//-----------------------------------------------------------------------------
//
// If you have a userdata with __gc metatable at the stack top (L->top) with a
// RC of 1, then push anything on the stack LuaRC asserts. The following
// happens in the process of replacing the userdata with a string from
// lua_pushstring:
//
//  * userdata info is copied into a temp variable via lua_makeobjectbackup
//  * string object is created and placed at stack top (L->top) and its RC
//    incremented.
//  * userdata RC (through temp var) is decremented because string is in the
//    process of replacing it.
//  * lua_releaseobject (a LuaRC call) is called on userdata (through temp var)
//    because RC = 0.
//  * lua_releaseobject calls do1gcTM which checks if the userdata contains a
//    metatable (which it does).
//  * the metatable is placed at the stack top (replacing the string we just
//    put there <--- BAD) and userdata goes at top+1.
//  * the stack top is adjusted by +2 and the metatable __gc is called popping
//    the arg and function off and setting stack top back -2.
//  * the type of the object at stack top is LUA_TNIL (due to cleanup), but the
//    gc still points to the __gc function object.
//  * we resume with the rest of the string move and assert because the gc type
//    is not LUA_TSTRING.
//
// A quick and dirty workaround increments L->top before calling do1gcTM in
// lua_releaseobject, then decrementing it afterwards which seems to prevent
// the corruption but it'll break the first time something puts two or more
// elements there so I don't trust it at all :P
//
//    void lua_releaseobject(lua_State *L, GCObject* o) {
//        ...
//        case LUA_TUSERDATA: {
//          /* we're freeing this object, but fake ref count it, <snip>
//			o->gch.ref = 1;
//
//          // DAP BEGIN -- __gc userdata corruption workaround
//          L->top++;
//          // DAP END -- __gc userdata corruption workaround
//
//            do1gcTM(L, gcotou(o));
//
//          // DAP BEGIN -- __gc userdata corruption workaround
//          L->top--;
//          // DAP END -- __gc userdata corruption workaround
//
//			Unlink((GCObject*)gcotou(o));
//
//            lua_releasetable(L, gcotou(o)->uv.metatable);
//
//			luaM_free(L, o, sizeudata(gcotou(o)->uv.len));
//			break;
//		}
//      ...
//////////////////////////////////////////////////////////////////////////////
int issue_3_gc( lua_State* L )
{
	printf( "issue_3 gc\n");
	return 0;
}

void issue_3( lua_State *L )
{
    // create a userdata
	lua_newuserdata(L, sizeof(unsigned int));

	// create a table to become the metatable
	lua_newtable(L);

	// add a gc function
	lua_pushstring(L, "__gc");
	lua_pushcclosure(L, issue_3_gc, 0);
	lua_settable(L, -3);

    // add the table as a metatable
    lua_setmetatable(L, -2);

    // put a copy of the userdata at L->top
    lua_insert(L, -1);

    // pop the original userdata off L->top-1 to get the ref of the one at L->top down to 1.
    lua_pop(L,1);
    // push something on stack to get the userdata back to L->top
    lua_pushnumber(L,1);

    // now push anything on the stack and it will die.
    //lua_pushnil(L);
    //lua_pushboolean( L, true );
    //lua_pushnumber( L, 5 );
    lua_pushstring( L, "test" );

    printf( "Issue 3 passed\n" );
}

//////////////////////////////////////////////////////////////////////////////
// issue_4
//
// When resuming the caller of a function, during cleaning up
// (cleanarray(res, L->top) in luaD_poscall), LuaRC attempts to re-enter
// luaD_call via do1gcTM while collecting a userdata with __gc metatable.
//
//-----------------------------------------------------------------------------
// Assertion failed: !(L->ci->state & (1<<2)), file c:\dev\luarc\src\ldo.c,
// line 330
//-----------------------------------------------------------------------------
//
//While cleaning the stack elements from the function call before adjusting
// L->top back to res.
//
//void luaD_poscall (lua_State *L, int wanted, StkId firstResult) {
//    ...
//    cleanarray(res, L->top)
//       // visits (L->top - 2) and (L->top -1) calling cleanvalue on them
//       // which calls lua_releaseobject in the case they are collectable and
//       // decrements RC == 0.
//       //
//       // In the stack dump below, -2 is in the process of being released.
//       // lua_release has already decremented the RC and is about to
//       // call lua_releaseobject
//       // ---> YOU ARE HERE
//       lua_releaseobject(L,gcvalue(i_o2));
//
//// stack dump as soon as we enter lua_releaseobject
//-------------------------------------------------------------------------------
//Stack top [9] addr[03336F44] last[033373A4] size[90] base[03336EB4]
//   [-5] (03336EF4) (number)
//   [-4] (03336F04) (string) { gc[033EB9CC] ref[3] tt[string] mark[0] }
//   [-3] (03336F14) (table) { gc[0331635C] ref[2] tt[table] mark[0] }
//   [-2] (03336F24) (table) { gc[033163A0] ref[0] tt[table] mark[0] }
//   [-1] (03336F34) (table) { gc[0331635C] ref[2] tt[table] mark[0] }
//-->[ 0] (03336F44) (nil)
//   [ 1] (03336F54) (nil)
//   [ 2] (03336F64) (nil)
//   [ 3] (03336F74) (nil)
//   [ 4] (03336F84) (nil)
//   [ 5] (03336F94) (nil)
//-------------------------------------------------------------------------------
//void lua_releaseobject(lua_State *L, GCObject* o) {
//    ...
//    case LUA_TTABLE: {
//        Table *h = gcotoh(o);
//        Unlink((GCObject*)h);
//
//        // now we're in lua_releaseobject and know we're dealing with a table.
//        // we call traversetable
//        // ---> YOU ARE HERE
//        traversetable(L, h);
//        ...
//
//static void traversetable (lua_State *L, Table *h) {
//    ...
//    if (!weakvalue) {
//        i = h->sizearray;
//        // i = 2 here
//        while (i--)
//        {
//            // i = 1
//            // ---> YOU ARE HERE
//            lua_release(L, &h->array[i]);
//            ...
//
//void lua_releaseobject(lua_State *L, GCObject* o) {
//    ...
//    case LUA_TUSERDATA: {
//        /* we're freeing this object, but fake ref count it, because <snip>
//       o->gch.ref = 1;
//
//       // ---> YOU ARE HERE
//       do1gcTM(L, gcotou(o));
//       ...
//
//
//// redump of stack to illustrate L->top++ hack as soon as we enter do1gcTM
//-------------------------------------------------------------------------------
//Stack top [10] addr[03336F54] last[033373A4] size[90] base[03336EB4]
//   [-5] (03336F04) (string) { gc[033EB9CC] ref[3] tt[string] mark[0] }
//   [-4] (03336F14) (table) { gc[0331635C] ref[2] tt[table] mark[0] }
//   [-3] (03336F24) (table) { gc[033163A0] ref[0] tt[table] mark[0] }
//   [-2] (03336F34) (table) { gc[0331635C] ref[2] tt[table] mark[0] }
//   [-1] (03336F44) (nil)
//-->[ 0] (03336F54) (nil)
//   [ 1] (03336F64) (nil)
//   [ 2] (03336F74) (nil)
//   [ 3] (03336F84) (nil)
//   [ 4] (03336F94) (nil)
//   [ 5] (03336FA4) (table) { gc[03316538] ref[2] tt[table] mark[0] }
//-------------------------------------------------------------------------------
//void do1gcTM (lua_State *L, Udata *udata) {
//    ...
//    const TObject *tm = fasttm(L, udata->uv.metatable, TM_GC);
//    // tm != NULL here as we have a metatable
//    if (tm != NULL) {
//        setobj2s(L->top, tm);
//        setuvalue(L->top+1, udata);
//        L->top += 2;
//        // ---> YOU ARE HERE
//        luaD_call(L, L->top - 2, 0);
//    }
//}
//
//// metatable function and userdata have been pushed on stack and we're
//// about to call luaD_call. stack dump as soon as we enter luaD_call
//-------------------------------------------------------------------------------
//Stack top [12] addr[03336F74] last[033373A4] size[90] base[03336EB4]
//   [-5] (03336F24) (table) { gc[033163A0] ref[0] tt[table] mark[0] }
//   [-4] (03336F34) (table) { gc[0331635C] ref[2] tt[table] mark[0] }
//   [-3] (03336F44) (nil)
//   [-2] (03336F54) (function) { gc[03911350] ref[2] tt[function] mark[0] }
//   [-1] (03336F64) (userdata) { gc[03911398] ref[2] tt[userdata] mark[2] }
//-->[ 0] (03336F74) (nil)
//   [ 1] (03336F84) (nil)
//   [ 2] (03336F94) (nil)
//   [ 3] (03336FA4) (table) { gc[03316538] ref[2] tt[table] mark[0] }
//   [ 4] (03336FB4) (number) value[0]
//   [ 5] (03336FC4) (nil)
//-------------------------------------------------------------------------------
//void luaD_call (lua_State *L, StkId func, int nResults) {
//  StkId firstResult;
//  // L->ci->state == 12 here (CF_SAVEDPC | CI_CALLING)
//  lua_assert(!(L->ci->state & CI_CALLING)); ---> BOOM!
//
//////////////////////////////////////////////////////////////////////////////
int issue_4_gc( lua_State* L )
{
	printf( "issue_4 gc\n");
	return 0;
}

void lua_issue_4_userdata_new( lua_State *L )
{
    // create a userdata
	lua_newuserdata( L, sizeof( unsigned int ) );

	// create a table to become the metatable
    lua_newtable(L);

	// add a gc function
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, issue_4_gc );
    lua_settable(L, -3);

    // add the table as a metatable
    lua_setmetatable(L, -2);
}

int lua_issue_4_create_userdata( lua_State *L )
{
    lua_issue_4_userdata_new( L );
    return 1;
}

int lua_issue_4_test( lua_State *L )
{
    // Assertion happens shortly after we return from here.
    return 0;
}

int lua_issue_4_new( lua_State *L )
{
    lua_newtable(L);

    lua_pushstring(L, "create_userdata");
    lua_pushcfunction(L, lua_issue_4_create_userdata);
    lua_rawset(L, 1);

    lua_pushstring(L, "test");
    lua_pushcfunction(L, lua_issue_4_test);
    lua_rawset(L, 1);

    return 1;
}

static const struct luaL_reg lua_issue_4_lib[] = {
    {"issue_4", lua_issue_4_new },
    {NULL,  NULL}
};

int issue_4_error( lua_State *L )
{
    printf( "Error [%s]\n", lua_tostring(L, -1) );
    lua_pop(L,1);
    return 0;
}

void issue_4()
{
    LuaStateOwner state;
    lua_State* L = state->GetCState();

    // hook for creating our userdata from script
	luaL_register(L, "Issue_4", lua_issue_4_lib);

    // ensure stack is clean
    lua_settop(L, 0);
//jj    lua_setgcthreshold(L, 0);

    // call script to return obj to call on stack
  	luaL_dofile( L, "issue_4.lua" );

    // lookup func on table
    lua_pushstring(L, "doit");
    lua_gettable(L, -2);

    // put func infront of obj
    lua_insert(L, -2);

    // create arg to function
    lua_issue_4_userdata_new(L);

    // insert fb under self and args
    int base = lua_gettop( L ) - 2;
    lua_pushcclosure( L, issue_4_error, 0 );
    lua_insert( L, base );

     // call the function passing in self and userdata arg
    lua_pcall(L, 2, 0, base );

    printf( "Issue 4 passed\n" );
}


void IssuesTest()
{
    lua_State* L = lua_open();

//    issue_1(L);
//	issue_2(L);
    // no issue 2 repro
//    issue_3(L);
//    issue_4();

    printf( "End\n" );
    lua_close(L);
//    getch();
}


void GlobalErrorTest()
{
    LuaStateOwner state(false);
    try
    {
        state->DoString("Foo = nil; Foo.x = 9");
    }
    catch (LuaException&)
    {
    }
}


void DoStringErrorTest()
{
    // init luastate with all libs open
    LuaStateOwner state;
    if (state->DoString("invalid code:") != 0)
	{
		printf("%s\n", state->Stack(-1).GetString());
	}
}


void RCBlowup()
{
	LuaStateOwner state;
	state->DoString("function Test()\nlocal file = io.open('TestScript.cpp')\nend\nTest()\n");
}


void ForBlowup()
{
    LuaStateOwner state;
    state->DoFile("ForBlowup.lua");
}


void TestClass()
{
    LuaStateOwner state;
    state->DoString("package.path='../../test/?.lua;' .. package.path");
    int ret = state->DoFile("../../test/animal.lua");
    assert(ret == 0);
    
    LuaObject createFunctionObj = state->GetGlobal("Dog");
    createFunctionObj.PushStack();
    state->PushString("Golden Retriever");
    state->PCall(1, 1, 0);
    LuaObject myObject(state, -1);
    state->GetGlobals().SetObject("GoldenRetriever", myObject);

    state->DoString("print(GoldenRetriever)");
}


static int delay(LuaState* pState)
{
    // delay (length)
    LuaStack tStack(pState);

    if (tStack[1].IsInteger())
    {
        printf("Delaying for %d seconds...\n", tStack[1].GetInteger());
    }

    return 0;
}


void CoroutineTest()
{
    LuaStateOwner pState;
    LuaObject tGlobals = pState->GetGlobals();
    tGlobals.Register("delay", delay);

    pState->DoString("function TestCoroutine()\n"
"    delay(4)\n"
"end\n");

    pState->DoString("co = coroutine.create(TestCoroutine)");
    pState->DoString("coroutine.resume(co)");
}

void MemoryKeyTest()
{
	SimpleHeap mainHeap("Test");
	mainHeap.Initialize(1 * 1024 * 1024);
	heap = &mainHeap;

	{
		lua_setdefaultallocfunction(ReallocFunction, &mainHeap);
		LuaStateOwner state(false);
		mainHeap.IntegrityCheck();
//		lua_dostring(state->GetCState(), "require 'compat-5.1'");
		lua_setdefaultallocfunction(NULL, NULL);

		LuaObject string1;
		LuaObject string2;
		LuaObject string3;
		LuaObject string4;
		LuaObject string5;
		LuaObject string6;
		LuaObject string7;
		LuaObject string8;
		LuaObject string1_1;
		LuaObject string2_1;
		LuaObject string3_1;
		LuaObject string4_1;
		LuaObject string5_1;
		LuaObject string6_1;
		LuaObject string7_1;
		LuaObject string8_1;
		string1.AssignString(state, "String1");
		string2.AssignString(state, "String2");
		string3.AssignString(state, "String3");
		string4.AssignString(state, "String4");
		string5.AssignString(state, "String5");
		string6.AssignString(state, "String6");
		string7.AssignString(state, "String7");
		string8.AssignString(state, "String8");
		string1_1.AssignString(state, "String1_1");
		string2_1.AssignString(state, "String2_1");
		string3_1.AssignString(state, "String3_1");
		string4_1.AssignString(state, "String4_1");
		string5_1.AssignString(state, "String5_1");
		string6_1.AssignString(state, "String6_1");
		string7_1.AssignString(state, "String7_1");
		string8_1.AssignString(state, "String8_1");

		LuaObject globalsObj = state->GetGlobals();
		globalsObj.SetString(string1, "Stuff1");
		globalsObj.SetString(string2, "Stuff2");
		globalsObj.SetString(string3, "Stuff3");
		globalsObj.SetString(string4, "Stuff4");
		globalsObj.SetString(string5, "Stuff5");
		globalsObj.SetString(string6, "Stuff6");
		globalsObj.SetString(string7, "Stuff7");
		globalsObj.SetString(string8, "Stuff8");
		globalsObj.SetString(string1_1, "Stuff1_1");
		globalsObj.SetString(string2_1, "Stuff2_1");
		globalsObj.SetString(string3_1, "Stuff3_1");
		globalsObj.SetString(string4_1, "Stuff4_1");
		globalsObj.SetString(string5_1, "Stuff5_1");
		globalsObj.SetString(string6_1, "Stuff6_1");
		globalsObj.SetString(string7_1, "Stuff7_1");
		globalsObj.SetString(string8_1, "Stuff8_1");

		globalsObj.SetNil(string8_1);
		globalsObj.SetNil(string7_1);
		globalsObj.SetNil(string6_1);
		globalsObj.SetNil(string5_1);
		globalsObj.SetNil(string4_1);
		globalsObj.SetNil(string3_1);
		globalsObj.SetNil(string2_1);
		globalsObj.SetNil(string1_1);
		globalsObj.SetNil(string8);
		globalsObj.SetNil(string7);
		globalsObj.SetNil(string6);
		globalsObj.SetNil(string5);
		globalsObj.SetNil(string4);
		globalsObj.SetNil(string3);
		globalsObj.SetNil(string2);
		globalsObj.SetNil(string1);

		globalsObj.SetString(string1, "Stuff1");
		globalsObj.SetString(string2, "Stuff2");
		globalsObj.SetString(string3, "Stuff3");
		globalsObj.SetString(string4, "Stuff4");
		globalsObj.SetString(string5, "Stuff5");
		globalsObj.SetString(string6, "Stuff6");
		globalsObj.SetString(string7, "Stuff7");
		globalsObj.SetString(string8, "Stuff8");
		globalsObj.SetString(string1_1, "Stuff1_1");
		globalsObj.SetString(string2_1, "Stuff2_1");
		globalsObj.SetString(string3_1, "Stuff3_1");
		globalsObj.SetString(string4_1, "Stuff4_1");
		globalsObj.SetString(string5_1, "Stuff5_1");
		globalsObj.SetString(string6_1, "Stuff6_1");
		globalsObj.SetString(string7_1, "Stuff7_1");
		globalsObj.SetString(string8_1, "Stuff8_1");

		globalsObj.SetNil(string8_1);
		globalsObj.SetNil(string7_1);
		globalsObj.SetNil(string6_1);
		globalsObj.SetNil(string5_1);
		globalsObj.SetNil(string4_1);
		globalsObj.SetNil(string3_1);
		globalsObj.SetNil(string2_1);
		globalsObj.SetNil(string1_1);
		globalsObj.SetNil(string8);
		globalsObj.SetNil(string7);
		globalsObj.SetNil(string6);
		globalsObj.SetNil(string5);
		globalsObj.SetNil(string4);
		globalsObj.SetNil(string3);
		globalsObj.SetNil(string2);
		globalsObj.SetNil(string1);
	}
}


void WeakTableTest()
{
	LuaStateOwner state;

	LuaObject stuffObj = state->GetGlobals().CreateTable("Stuff");
	{
		LuaObject numObj = state->GetGlobals().CreateTable("Num");

		{
			LuaObject obj;
			obj.AssignNewTable(state);
	//		stuffObj.SetObject("Hi", obj);

			numObj.Insert(obj);
		}

	//	stuffObj.SetNil("Hi");
		numObj.Remove(1);
	}
	state->GetGlobals().SetNil("Num");
	state->CollectGarbage();
}

#include "LuaPlus/src/lualib.h"

void MiniTest()
{
	LuaStateOwner state(true);
	int ret = state->DoString("a = 5");

	LuaObject obj = state->GetGlobals()["a"];
	printf("%g\n", obj.GetNumber());
}


void TestParser()
{
    {
        LuaStateOwner state;
        assert(_CrtCheckMemory());
        state->DoString("x = 0x12345678; print(x);");
        assert(_CrtCheckMemory());
    }
    {
        LuaStateOwner state;
        assert(_CrtCheckMemory());
        state->DoString("str = L'ab\\x1234\\xabcdef'");
        assert(_CrtCheckMemory());
    }
    {
        LuaStateOwner state;
        assert(_CrtCheckMemory());
        state->DoString("str = L'Wide string'");
        assert(_CrtCheckMemory());
    }
    {
        LuaStateOwner state;
        assert(_CrtCheckMemory());
        state->DoString("str2 = L'Wide string'");
        assert(_CrtCheckMemory());
    }
}


void TestGC()
{
    LuaStateOwner state(false);
    lua_State* L = state->GetCState();
    lua_pushstring(L, "Hello");
    while (state->GC(LUA_GCSTEP, 1) != 1)
    {
    }
    lua_pop(L, 1);
    state->GC(LUA_GCRESTART, 1);
    while (state->GC(LUA_GCSTEP, 1) != 1)
    {
    }
    state->GC(LUA_GCRESTART, 1);
    lua_pushstring(L, "SomeKey");
    lua_pushstring(L, "SomeValue");
    lua_rawset(L, LUA_GLOBALSINDEX);
    while (state->GC(LUA_GCSTEP, 1) != 1)
    {
    }
    lua_pushstring(L, "SomeKey");
    lua_pushnil(state->GetCState());
    lua_rawset(L, LUA_GLOBALSINDEX);
    while (state->GC(LUA_GCSTEP, 1) != 1)
    {
    }

    state->GC(LUA_GCRESTART, 1);
    LuaObject strObj;
    strObj.AssignString(state, "Hello");
    while (state->GC(LUA_GCSTEP, 1) != 1)
    {
    }
    strObj.Reset();
    state->GC(LUA_GCRESTART, 1);
    while (state->GC(LUA_GCSTEP, 1) != 1)
    {
    }
}


#if _WIN32_WCE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
#else // _WIN32_WCE
int __cdecl main(int argc, char* argv[])
#endif // _WIN32_WCE
{
	WeakTableTest();
    TestGC();
    TestParser();
    MiniTest();
	MemoryKeyTest();
//    CoroutineTest();
//    TestClass();
    ForBlowup();
	RCBlowup();
	NewUserDataBoxTest();
	DoStringErrorTest();
    GlobalErrorTest();
    TestStdString();
    IssuesTest();
	VectorMonsterMetatableTest();

	TestANSIFile();
	TestUnicodeFile();
    ReadUnicodeFile();
	LoadCompiledScript();
	BadDoString();
	SetTest();

	PropertyTest();
	BisectTest();
	RCTest();
	ExceptionTest();

	int i;

	{
		LuaStateOwner state(false);
		state->DoString("s = \"12345678901234567890123456789012345678901234567890\"");
		state->DoString("s2 = L\"abcdefghijklmnopqrstuvwxyzzyxwvutsrqponmlkjihgfedcba\"");
	}
	LookupTest();
	MemoryTest();
	lua_StateCallbackTest();
	MultiObjectTest();
	IntegerTest();

	LuaState* state = LuaState::Create(true);

	DWORD count = GetTickCount();
	luaL_dostring(*state, "a = true;  if a then print(\"Hi\") end");

	luaL_dostring(*state, "theGlobal = 5; function ReturnTrue() if theGlobal == 5 then return true else return false end end");

	LuaObject obj = state->GetGlobals().CreateTable("Stuff");
	for (i = 0; i < 100000; ++i)
	{
//		obj.CreateTable(i);
	}

		lua_pushstring(*state, "ReturnTrue");
		lua_gettable(*state, LUA_GLOBALSINDEX);
	for (i = 0; i < 1000000; ++i)
	{
		lua_pushvalue(*state, -1);
//		char s[80];
//		sprintf(s, "%d", i);
//		lua_pushstring(*state, s);
		lua_pushnumber(*state, i);
		lua_call(*state, 1, 1);
		lua_pop(*state, 1);
	}

	state->CollectGarbage();

	count = GetTickCount() - count;
	printf("%d\n", count);


	DumpTest();
	TestDumpGlobals();
	SetUserData();
	TestGCObject();
	TestThreeThreads();
	TestNewCall();
	CloneTest();

	TestPointer();
	{
		LuaStateOwner state;
		state->DoString("Table = { 0, 1, 2, 'Hello', nil, 'Hi', Yo = 'My Stuff', NobodysHome = 5, NestedTable = { 1, 2, 3, { 'String', }, { 'Table2' } }, { 'String1' } }");
		LuaObject globalsObj = state->GetGlobals();
		LuaObject tableObj = state->GetGlobal("Table");
		LuaObject numObj = tableObj[2];
	}
	LuaTableIteratorTest();
//	LuaStackTableIteratorTest();

	DoScriptFormatTest();

	DoScriptCallbackTest();
	DoScriptSaveTest();
	DoScriptArrayTest();
	TestPointer();

	return 0;
}
