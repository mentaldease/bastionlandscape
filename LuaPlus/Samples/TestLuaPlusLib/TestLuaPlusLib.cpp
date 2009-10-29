#include "LuaPlus/LuaPlus.h"
using namespace LuaPlus;
#include "Timer.h"

/**
	The LuaState class is able to make Lua callbacks look like natural extensions
	of the LuaState class.
**/
static int LS_PrintNumber(LuaState* state)
{
	// Verify it is a number and print it.
	LuaStack args(state);
	if (args[1].IsNumber())
		printf("%f\n", args[1].GetNumber());

	// No return values.
	return 0;
}


/**																		  
**/
static int LS_Add(LuaState* state)
{
	// Verify it is a number and print it.
	LuaStack args(state);
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


/**
**/
void DoScriptFormatTest()
{
	LuaStateOwner state;
	state->DoFile("ScriptVectorDump.lua");
}


/**
	Demonstrate registering callback functions for the Lua script.
**/
void DoScriptCallbackTest()
{
	LuaStateOwner state;

	state->GetGlobals().Register("PrintNumber", LS_PrintNumber);
	state->GetGlobals().Register("Add", LS_Add);

	state->DoFile("ScriptCallbackTest.lua");
}


/**
	Demonstrate reading and saving a script.
**/
void DoScriptSaveTest()
{
	LuaStateOwner state;

	state->DoFile( "ScriptSaveTest.lua" );
//	state->DumpGlobalsFile("ScriptSaveTest.dmp");
}


/**
	Demonstrates walking an array table.
**/
void DoScriptArrayTest()
{
	LuaStateOwner state;
	state->DoFile("ScriptArrayTest.lua");
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
	return 0;
}


void TestPointer()
{
	LuaStateOwner state;

	state->GetGlobals().Register("LightUserDataCall", LS_LightUserDataCall);
	LuaCall func = state->GetGlobal("LightUserDataCall");
	func << (void*)0xfedcba98 << LuaRun();
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


void CloneTest()
{
	LuaStateOwner state;

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

void TestGCObject()
{
	LuaStateOwner state(false);
	state->CollectGarbage();

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


int __cdecl main(int argc, char* argv[])
{
	TestGCObject();
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
