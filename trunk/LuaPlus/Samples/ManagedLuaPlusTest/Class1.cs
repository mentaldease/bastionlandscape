using System;
using System.IO;
using ManagedLuaPlus;

namespace ManagedLuaPlusTest
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class Class1
	{
		private static int MyCallback(LuaState state)
		{
			LuaStackObject strObj = state.Stack(1);
			Console.WriteLine(strObj.GetString());
			return 0;
		}

		private static void Func(int num)
		{
			Console.WriteLine(num);
		}

		public delegate void FuncDelegate(int num);

		public void Func2(int num)
		{
			Console.WriteLine(num);
		}

		static void TestGC()
		{
			LuaState state = new LuaState();
			LuaObject obj = state.GetGlobals().CreateTable("Hello, world!");
		}

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			TestGC();
			
			Class1 cl1 = new Class1();
			LuaState state = new LuaState();
			state.GetGlobals().Register("MyCallback", new LuaPlusCallback(MyCallback));
			state.DoString( "MyVar = 5" );
//			System.Reflection.MethodInfo info = typeof(Class1).GetMethod("Func2");
//			state.GetGlobals().Register("Func", null, typeof(Class1).GetMethod("Func")); //typeof(Class1).GetMethod("Func"));
//			state.DoString("Func(5)");
			LuaObject obj = state.GetGlobal( "MyVar" );
			state.DoString("MyCallback('Hello')");
			LuaObject funcObj = state.GetGlobal("print");
//			funcObj.Call("Stuff", 0);

			try
			{
//				state.DoString( "print('Hello, world!')" );
			}
			catch (Exception e)
			{
			}
			state.DoString( "MyTable = { 1, 2, [3] = { \"Hi\" } }" );

			double num = obj.GetNumber();
			LuaObject obj3 = state.GetGlobals()["MyTable"];
			String s = obj3.TypeName();
			LuaObject obj2 = new LuaObject(obj);
			obj2.AssignWString(state, "Yo baby!");

			state.DoString("MyTable = { 1, 2, 3, A = 5, 4 }");

			foreach (LuaNode node in state.Globals["MyTable"])
			{
				Console.WriteLine(node);
			}

			try
			{
				LuaObject globObj = state.Globals["MyTable"][1][2];
			}
			catch (LuaException e)
			{
				int hi = 5;
			}

			System.IO.FileStream fileStream = new System.IO.FileStream("Out.lua", FileMode.Create);
			LuaStateStreamOutFile outFile = new LuaStateStreamOutFile(fileStream);
			state.DumpObject(outFile, "MyTable", state.Globals["MyTable"]);
		}
	}
}
