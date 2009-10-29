
namespace LuaInterface 
{

	using System;
	using System.IO;
	using System.Collections;
	using System.Reflection;

	/*
	 * Main class of LuaInterface
	 * Object-oriented wrapper to Lua API
	 *
	 * Author: Fabio Mascarenhas
	 * Version: 1.0
	 */
	public class Lua 
	{
		public IntPtr luaState;
		public ObjectTranslator translator;

		public Lua(IntPtr _luaState) 
		{
			luaState = _luaState;
			translator=new ObjectTranslator(this,luaState);
		}
		/*
		 * Excutes a Lua chunk and returns all the chunk's return
		 * values in an array
		 */
		public object[] DoString(string chunk) 
		{
			int oldTop=LuaDLL.lua_gettop(luaState);
			if(LuaDLL.lua_dostring(luaState,chunk)==0) 
			{
				return translator.popValues(luaState,oldTop);
			} 
			else 
			{
				return null;
			}
		}
		/*
		 * Excutes a Lua file and returns all the chunk's return
		 * values in an array
		 */
		public object[] DoFile(string fileName) 
		{
			int oldTop=LuaDLL.lua_gettop(luaState);
			if(LuaDLL.lua_dofile(luaState,fileName)==0) 
			{
				return translator.popValues(luaState,oldTop);
			} 
			else 
			{
				return null;
			}
		}
		/*
		 * Indexer for global variables from the LuaInterpreter
		 * Supports navigation of tables by using . operator
		 */
		public object this[string fullPath]
		{
			get 
			{
				object returnValue=null;
				int oldTop=LuaDLL.lua_gettop(luaState);
				string[] path=fullPath.Split(new char[] { '.' });
				LuaDLL.lua_getglobal(luaState,path[0]);
				returnValue=translator.getObject(luaState,-1);
				if(path.Length>1) 
				{
					string[] remainingPath=new string[path.Length-1];
					Array.Copy(path,1,remainingPath,0,path.Length-1);
					returnValue=getObject(remainingPath);
				}
				LuaDLL.lua_settop(luaState,oldTop);
				return returnValue;
			}
			set 
			{
				int oldTop=LuaDLL.lua_gettop(luaState);
				string[] path=fullPath.Split(new char[] { '.' });
				if(path.Length==1) 
				{
					translator.push(luaState,value);
					LuaDLL.lua_setglobal(luaState,fullPath);
				} 
				else 
				{
					LuaDLL.lua_getglobal(luaState,path[0]);
					string[] remainingPath=new string[path.Length-1];
					Array.Copy(path,1,remainingPath,0,path.Length-1);
					setObject(remainingPath,value);
				}
				LuaDLL.lua_settop(luaState,oldTop);
			}
		}
		/*
		 * Navigates a table in the top of the stack, returning
		 * the value of the specified field
		 */
		internal object getObject(string[] remainingPath) 
		{
			object returnValue=null;
			for(int i=0;i<remainingPath.Length;i++) 
			{
				LuaDLL.lua_pushstring(luaState,remainingPath[i]);
				LuaDLL.lua_gettable(luaState,-2);
				returnValue=translator.getObject(luaState,-1);
				if(returnValue==null) break;	
			}
			return returnValue;    
		}
		/*
		 * Gets a numeric global variable
		 */
		public double GetNumber(string fullPath) 
		{
			return (double)this[fullPath];
		}
		/*
		 * Gets a string global variable
		 */
		public string GetString(string fullPath) 
		{
			return (string)this[fullPath];
		}
		/*
		 * Gets a table global variable
		 */
		public LuaTable GetTable(string fullPath) 
		{
			return (LuaTable)this[fullPath];
		}
		/*
		 * Gets a table global variable as an object implementing
		 * the interfaceType interface
		 */
		public object GetTable(Type interfaceType, string fullPath) 
		{
			return CodeGeneration.Instance.GetClassInstance(interfaceType,GetTable(fullPath));
		}
		/*
		 * Gets a function global variable
		 */
		public LuaFunction GetFunction(string fullPath) 
		{
			return (LuaFunction)this[fullPath];
		}
		/*
		 * Gets a function global variable as a delegate of
		 * type delegateType
		 */
		public Delegate GetFunction(Type delegateType,string fullPath) 
		{
			return CodeGeneration.Instance.GetDelegate(delegateType,GetFunction(fullPath));
		}
		/*
		 * Calls the object as a function with the provided arguments,
		 * returning the function's returned values inside an array
		 */
		internal object[] callFunction(object function,object[] args) 
		{
			int nArgs=0;
			int oldTop=LuaDLL.lua_gettop(luaState);
			if(args!=null && !LuaDLL.lua_checkstack(luaState,args.Length+6))
				throw new Exception("Lua stack overflow");
			translator.push(luaState,function);
			if(args!=null) 
			{
				nArgs=args.Length;
				for(int i=0;i<args.Length;i++) 
				{
					translator.push(luaState,args[i]);
				}
			}
			int error=LuaDLL.lua_pcall(luaState,nArgs,-1,0);
			if(error!=0) 
			{
				throw new Exception(LuaDLL.lua_tostring(luaState,-1));
			}
			return translator.popValues(luaState,oldTop);
		}
		/*
		 * Calls the object as a function with the provided arguments and
		 * casting returned values to the types in returnTypes before returning
		 * them in an array
		 */
		internal object[] callFunction(LuaFunction function,object[] args,Type[] returnTypes) 
		{
			int nArgs=0;
			int oldTop=LuaDLL.lua_gettop(luaState);
			if(!LuaDLL.lua_checkstack(luaState,args.Length+6))
				throw new Exception("Lua stack overflow");
			translator.push(luaState,function);
			if(args!=null) 
			{
				nArgs=args.Length;
				for(int i=0;i<args.Length;i++) 
				{
					translator.push(luaState,args[i]);
				}
			}
			int error=LuaDLL.lua_pcall(luaState,nArgs,-1,0);
			if(error!=0) 
			{
				LuaDLL.lua_error(luaState);
			}
			return translator.popValues(luaState,oldTop,returnTypes);
		}
		/*
		 * Navigates a table to set the value of one of its fields
		 */
		internal void setObject(string[] remainingPath, object val) 
		{
			for(int i=0; i<remainingPath.Length-1;i++) 
			{
				LuaDLL.lua_pushstring(luaState,remainingPath[i]);
				LuaDLL.lua_gettable(luaState,-2);
			}
			LuaDLL.lua_pushstring(luaState,remainingPath[remainingPath.Length-1]);
			translator.push(luaState,val);
			LuaDLL.lua_settable(luaState,-3);
		}
		/*
		 * Creates a new table as a global variable or as a field
		 * inside an existing table
		 */
		public void NewTable(string fullPath) 
		{
			string[] path=fullPath.Split(new char[] { '.' });
			int oldTop=LuaDLL.lua_gettop(luaState);
			if(path.Length==1) 
			{
				LuaDLL.lua_newtable(luaState);
				LuaDLL.lua_setglobal(luaState,fullPath);
			} 
			else 
			{
				LuaDLL.lua_getglobal(luaState,path[0]);
				for(int i=1; i<path.Length-1;i++) 
				{
					LuaDLL.lua_pushstring(luaState,path[i]);
					LuaDLL.lua_gettable(luaState,-2);
				}
				LuaDLL.lua_pushstring(luaState,path[path.Length-1]);
				LuaDLL.lua_newtable(luaState);
				LuaDLL.lua_settable(luaState,-3);
			}
			LuaDLL.lua_settop(luaState,oldTop);
		}
		/*
		 * Lets go of a previously allocated reference to a table, function
		 * or userdata
		 */
		internal void dispose(int reference) 
		{
			LuaDLL.lua_unref(luaState,reference);
		}
		/*
		 * Gets a field of the table corresponding to the provided reference
		 * using rawget (do not use metatables)
		 */
		internal object rawGetObject(int reference,string field) 
		{
			int oldTop=LuaDLL.lua_gettop(luaState);
			LuaDLL.lua_getref(luaState,reference);
			LuaDLL.lua_pushstring(luaState,field);
			LuaDLL.lua_rawget(luaState,-2);
			object obj=translator.getObject(luaState,-1);
			LuaDLL.lua_settop(luaState,oldTop);
			return obj;
		}
		/*
		 * Gets a field of the table or userdata corresponding to the provided reference
		 */
		internal object getObject(int reference,string field) 
		{
			int oldTop=LuaDLL.lua_gettop(luaState);
			LuaDLL.lua_getref(luaState,reference);
			object returnValue=getObject(field.Split(new char[] {'.'}));
			LuaDLL.lua_settop(luaState,oldTop);
			return returnValue;
		}
		/*
		 * Gets a numeric field of the table or userdata corresponding the the provided reference
		 */
		internal object getObject(int reference,double field) 
		{
			int oldTop=LuaDLL.lua_gettop(luaState);
			LuaDLL.lua_getref(luaState,reference);
			LuaDLL.lua_pushnumber(luaState,field);
			LuaDLL.lua_gettable(luaState,-2);
			object returnValue=translator.getObject(luaState,-1);
			LuaDLL.lua_settop(luaState,oldTop);
			return returnValue;
		}
		/*
		 * Sets a field of the table or userdata corresponding the the provided reference
		 * to the provided value
		 */
		internal void setObject(int reference, string field, object val) 
		{
			int oldTop=LuaDLL.lua_gettop(luaState);
			LuaDLL.lua_getref(luaState,reference);
			setObject(field.Split(new char[] {'.'}),val);
			LuaDLL.lua_settop(luaState,oldTop);
		}
		/*
		 * Sets a numeric field of the table or userdata corresponding the the provided reference
		 * to the provided value
		 */
		internal void setObject(int reference, double field, object val) 
		{
			int oldTop=LuaDLL.lua_gettop(luaState);
			LuaDLL.lua_getref(luaState,reference);
			LuaDLL.lua_pushnumber(luaState,field);
			translator.push(luaState,val);
			LuaDLL.lua_settable(luaState,-3);
			LuaDLL.lua_settop(luaState,oldTop);
		}
		/*
		 * Registers an object's method as a Lua function (global or table field)
		 * The method may have any signature
		 */
    	public LuaFunction RegisterFunction(string path, object target,MethodInfo function) 
		{
			LuaMethodWrapper wrapper=new LuaMethodWrapper(translator,target,function.DeclaringType,function);
			int oldTop=LuaDLL.lua_gettop(luaState);
			translator.push(luaState,new LuaCSFunction(wrapper.call));
			this[path]=translator.getObject(luaState,-1);
			return GetFunction(path);
		}
		/*
		 * Compares the two values referenced by ref1 and ref2 for equality
		 */
		internal bool compareRef(int ref1, int ref2) 
		{
			int top=LuaDLL.lua_gettop(luaState);
			LuaDLL.lua_getref(luaState,ref1);
			LuaDLL.lua_getref(luaState,ref2);
            int equal=LuaDLL.lua_equal(luaState,-1,-2);
			LuaDLL.lua_settop(luaState,top);
			return (equal!=0);
		}
	}

	/*
	 * Wrapper class for Lua tables
	 *
	 * Author: Fabio Mascarenhas
	 * Version: 1.0
	 */
	public class LuaTable
	{
		internal int reference;
		private Lua interpreter;
		public LuaTable(int reference, Lua interpreter) 
		{
			this.reference=reference;
			this.interpreter=interpreter;
		}
		~LuaTable() 
		{
			interpreter.dispose(reference);
		}
		/*
		 * Indexer for string fields of the table
		 */
		public object this[string field] 
		{
			get 
			{
				return interpreter.getObject(reference,field);
			}
			set 
			{
				interpreter.setObject(reference,field,value);
			}
		}
		/*
		 * Indexer for numeric fields of the table
		 */
		public object this[double field] 
		{
			get 
			{
				return interpreter.getObject(reference,field);
			}
			set 
			{
				interpreter.setObject(reference,field,value);
			}
		}
		/*
		 * Gets an string fields of a table ignoring its metatable,
		 * if it exists
		 */
		internal object rawget(string field) 
		{
			return interpreter.rawGetObject(reference,field);
		}
		/*
		 * Pushes this table into the Lua stack
		 */
		internal void push(IntPtr luaState) 
		{
			LuaDLL.lua_getref(luaState,reference);
		}
		public override string ToString() 
		{
			return "table";
		}
		public override bool Equals(object o) 
		{
			if(o is LuaTable) 
			{
				LuaTable l=(LuaTable)o;
				return interpreter.compareRef(l.reference,this.reference);
			} else return false;
		}
		public override int GetHashCode() 
		{
			return reference;
		}
	}

	public class LuaFunction 
	{
		private Lua interpreter;
		internal int reference;
		public LuaFunction(int reference, Lua interpreter) 
		{
			this.reference=reference;
			this.interpreter=interpreter;
		}
		~LuaFunction() 
		{
			interpreter.dispose(reference);
		}
		/*
		 * Calls the function casting return values to the types
		 * in returnTypes
		 */
		internal object[] call(object[] args, Type[] returnTypes) 
		{
			return interpreter.callFunction(this,args,returnTypes);
		}
		/*
		 * Calls the function and returns its return values inside
		 * an array
		 */
		public object[] Call(params object[] args) 
		{
			return interpreter.callFunction(this,args);
		}
		/*
		 * Pushes the function into the Lua stack
		 */
		internal void push(IntPtr luaState) 
		{
			LuaDLL.lua_getref(luaState,reference);
		}
		public override string ToString() 
		{
			return "function";
		}
		public override bool Equals(object o) 
		{
			if(o is LuaFunction) 
			{
				LuaFunction l=(LuaFunction)o;
				return interpreter.compareRef(l.reference,this.reference);
			} 
			else return false;
		}
		public override int GetHashCode() 
		{
			return reference;
		}
	}

	public class LuaUserData
	{
		internal int reference;
		private Lua interpreter;
		public LuaUserData(int reference, Lua interpreter) 
		{
			this.reference=reference;
			this.interpreter=interpreter;
		}
		~LuaUserData() 
		{
			interpreter.dispose(reference);
		}
		/*
		 * Indexer for string fields of the userdata
		 */
		public object this[string field] 
		{
			get 
			{
				return interpreter.getObject(reference,field);
			}
			set 
			{
				interpreter.setObject(reference,field,value);
			}
		}
		/*
		 * Indexer for numeric fields of the userdata
		 */
		public object this[double field] 
		{
			get 
			{
				return interpreter.getObject(reference,field);
			}
			set 
			{
				interpreter.setObject(reference,field,value);
			}
		}
		/*
		 * Calls the userdata and returns its return values inside
		 * an array
		 */
		public object[] Call(params object[] args) 
		{
			return interpreter.callFunction(this,args);
		}
		/*
		 * Pushes the userdata into the Lua stack
		 */
		internal void push(IntPtr luaState) 
		{
			LuaDLL.lua_getref(luaState,reference);
		}
		public override string ToString() 
		{
			return "userdata";
		}
		public override bool Equals(object o) 
		{
			if(o is LuaUserData) 
			{
				LuaUserData l=(LuaUserData)o;
				return interpreter.compareRef(l.reference,this.reference);
			} 
			else return false;
		}
		public override int GetHashCode() 
		{
			return reference;
		}
	}

}