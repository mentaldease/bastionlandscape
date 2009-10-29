namespace LuaInterface 
{
	using System;
	using System.IO;
	using System.Collections;
	using System.Reflection;

	/*
	 * Passes objects from the CLR to Lua and vice-versa
	 * 
	 * Author: Fabio Mascarenhas
	 * Version: 1.0
	 */
	public class ObjectTranslator 
	{
		internal CheckType typeChecker;
		public ArrayList objects;
		private Lua interpreter;
		private MetaFunctions metaFunctions;
		private ArrayList assemblies;
		private LuaCSFunction registerTableFunction,unregisterTableFunction,getMethodSigFunction,
			getConstructorSigFunction,importTypeFunction,importType__indexFunction,loadAssemblyFunction, loadAssemblyFromFunction;

		public ObjectTranslator(Lua interpreter,IntPtr luaState) 
		{
			this.interpreter=interpreter;
			typeChecker=new CheckType(this);
			metaFunctions=new MetaFunctions(this);
			objects=new ArrayList();
			assemblies=new ArrayList();

			importTypeFunction=new LuaCSFunction(this.importType);
			importType__indexFunction=new LuaCSFunction(this.importType__index);
			loadAssemblyFunction=new LuaCSFunction(this.loadAssembly);
			loadAssemblyFromFunction=new LuaCSFunction(this.loadAssemblyFrom);
			registerTableFunction=new LuaCSFunction(this.registerTable);
			unregisterTableFunction=new LuaCSFunction(this.unregisterTable);
			getMethodSigFunction=new LuaCSFunction(this.getMethodSignature);
			getConstructorSigFunction=new LuaCSFunction(this.getConstructorSignature);

			createLuaObjectList(luaState);
			createIndexingMetaFunction(luaState);
			createBaseClassMetatable(luaState);
			createClassMetatable(luaState);
			createFunctionMetatable(luaState);
			setGlobalFunctions(luaState);

			LuaDLL.lua_dostring(luaState, "load_assembly('mscorlib')");
		}
		/*
		 * Sets up the list of objects in the Lua side
		 */
		private void createLuaObjectList(IntPtr luaState) 
		{
			LuaDLL.lua_pushstring(luaState,"luaNet_objects");
			LuaDLL.lua_newtable(luaState);
			LuaDLL.lua_newtable(luaState);
			LuaDLL.lua_pushstring(luaState,"__mode");
			LuaDLL.lua_pushstring(luaState,"v");
			LuaDLL.lua_settable(luaState,-3);
			LuaDLL.lua_setmetatable(luaState,-2);
			LuaDLL.lua_settable(luaState,LuaIndexes.LUA_REGISTRYINDEX);
		}
		/*
		 * Registers the indexing function of CLR objects
		 * passed to Lua
		 */
		private void createIndexingMetaFunction(IntPtr luaState) 
		{
			LuaDLL.lua_pushstring(luaState,"luaNet_indexfunction");
			LuaDLL.lua_dostring(luaState,MetaFunctions.luaIndexFunction);
			//LuaDLL.lua_pushstdcallcfunction(luaState,indexFunction);
			LuaDLL.lua_rawset(luaState,LuaIndexes.LUA_REGISTRYINDEX);
		}
		/*
		 * Creates the metatable for superclasses (the base
		 * field of registered tables)
		 */
		private void createBaseClassMetatable(IntPtr luaState) 
		{
			LuaDLL.luaL_newmetatable(luaState,"luaNet_searchbase");
			LuaDLL.lua_pushstring(luaState,"__gc");
			LuaDLL.lua_pushstdcallcfunction(luaState,metaFunctions.gcFunction);
			LuaDLL.lua_settable(luaState,-3);
			LuaDLL.lua_pushstring(luaState,"__tostring");
			LuaDLL.lua_pushstdcallcfunction(luaState,metaFunctions.toStringFunction);
			LuaDLL.lua_settable(luaState,-3);
			LuaDLL.lua_pushstring(luaState,"__index");
			LuaDLL.lua_pushstdcallcfunction(luaState,metaFunctions.baseIndexFunction);
			LuaDLL.lua_settable(luaState,-3);
			LuaDLL.lua_pushstring(luaState,"__newindex");
			LuaDLL.lua_pushstdcallcfunction(luaState,metaFunctions.newindexFunction);
			LuaDLL.lua_settable(luaState,-3);
			LuaDLL.lua_settop(luaState,-2);
		}
		/*
		 * Creates the metatable for type references
		 */
		private void createClassMetatable(IntPtr luaState) 
		{
			LuaDLL.luaL_newmetatable(luaState,"luaNet_class");
			LuaDLL.lua_pushstring(luaState,"__gc");
			LuaDLL.lua_pushstdcallcfunction(luaState,metaFunctions.gcFunction);
			LuaDLL.lua_settable(luaState,-3);
			LuaDLL.lua_pushstring(luaState,"__tostring");
			LuaDLL.lua_pushstdcallcfunction(luaState,metaFunctions.toStringFunction);
			LuaDLL.lua_settable(luaState,-3);
			LuaDLL.lua_pushstring(luaState,"__index");
			LuaDLL.lua_pushstdcallcfunction(luaState,metaFunctions.classIndexFunction);
			LuaDLL.lua_settable(luaState,-3);
			LuaDLL.lua_pushstring(luaState,"__newindex");
			LuaDLL.lua_pushstdcallcfunction(luaState,metaFunctions.classNewindexFunction);
			LuaDLL.lua_settable(luaState,-3);
			LuaDLL.lua_pushstring(luaState,"__call");
			LuaDLL.lua_pushstdcallcfunction(luaState,metaFunctions.callConstructorFunction);
			LuaDLL.lua_settable(luaState,-3);
			LuaDLL.lua_settop(luaState,-2);
		}
		/*
		 * Registers the global functions used by LuaInterface
		 */
		private void setGlobalFunctions(IntPtr luaState)
		{
			LuaDLL.lua_pushstdcallcfunction(luaState,metaFunctions.indexFunction);
			LuaDLL.lua_setglobal(luaState,"get_object_member");
			LuaDLL.lua_pushstdcallcfunction(luaState,importTypeFunction);
			LuaDLL.lua_setglobal(luaState,"import_type");
			LuaDLL.lua_pushstdcallcfunction(luaState,loadAssemblyFunction);
			LuaDLL.lua_setglobal(luaState,"load_assembly");
			LuaDLL.lua_pushstdcallcfunction(luaState,loadAssemblyFromFunction);
			LuaDLL.lua_setglobal(luaState,"load_assemblyfrom");
			LuaDLL.lua_pushstdcallcfunction(luaState,registerTableFunction);
			LuaDLL.lua_setglobal(luaState,"make_object");
			LuaDLL.lua_pushstdcallcfunction(luaState,unregisterTableFunction);
			LuaDLL.lua_setglobal(luaState,"free_object");
			LuaDLL.lua_pushstdcallcfunction(luaState,getMethodSigFunction);
			LuaDLL.lua_setglobal(luaState,"get_method_bysig");
			LuaDLL.lua_pushstdcallcfunction(luaState,getConstructorSigFunction);
			LuaDLL.lua_setglobal(luaState,"get_constructor_bysig");

			LuaDLL.lua_getglobal(luaState, "dotnet");
			LuaDLL.luaL_newmetatable(luaState, "dotnet_metatable");
			LuaDLL.lua_pushstring(luaState,"__index");
			LuaDLL.lua_pushstdcallcfunction(luaState, importType__indexFunction);
			LuaDLL.lua_settable(luaState,-3);
			LuaDLL.lua_setmetatable(luaState, -2);

		}
		/*
		 * Creates the metatable for delegates
		 */
		private void createFunctionMetatable(IntPtr luaState) 
		{
			LuaDLL.luaL_newmetatable(luaState,"luaNet_function");
			LuaDLL.lua_pushstring(luaState,"__gc");
			LuaDLL.lua_pushstdcallcfunction(luaState,metaFunctions.gcFunction);
			LuaDLL.lua_settable(luaState,-3);
			LuaDLL.lua_pushstring(luaState,"__call");
			LuaDLL.lua_pushstdcallcfunction(luaState,metaFunctions.execDelegateFunction);
			LuaDLL.lua_settable(luaState,-3);
			LuaDLL.lua_settop(luaState,-2);
		}
		/*
		 * Passes errors (argument e) to the Lua interpreter
		 */
		internal void throwError(IntPtr luaState,object e) 
		{
			push(luaState,e);
			LuaDLL.lua_error(luaState);
		}
		/*
		 * Implementation of load_assembly. Throws an error
		 * if the assembly is not found.
		 */
		private int loadAssembly(IntPtr luaState) 
		{
			string assemblyName=LuaDLL.lua_tostring(luaState,1);
			try 
			{
				Assembly assembly=Assembly.LoadWithPartialName(assemblyName);
				if(assemblies.IndexOf(assembly)==-1)
					assemblies.Add(assembly);
			} 
			catch(Exception e) 
			{
				throwError(luaState,e);
			}
			return 0;
		}
		/*
		 * Implementation of load_assembly. Throws an error
		 * if the assembly is not found.
		 */
		private int loadAssemblyFrom(IntPtr luaState) 
		{
			string assemblyName=LuaDLL.lua_tostring(luaState,1);
			try 
			{
				Assembly assembly=Assembly.LoadFrom(assemblyName);
				if(assemblies.IndexOf(assembly)==-1)
					assemblies.Add(assembly);
			} 
			catch(Exception e) 
			{
				throwError(luaState,e);
			}
			return 0;
		}
		/*
		 * Implementation of import_type. Returns nil if the
		 * type is not found.
		 */
		private int importType(IntPtr luaState) 
		{
			string className=LuaDLL.lua_tostring(luaState,1);
			foreach(Assembly assembly in assemblies) 
			{
				Type klass=assembly.GetType(className);
				if(klass!=null) 
				{
					pushType(luaState,klass);
					return 1;
				}
			}
			LuaDLL.lua_pushnil(luaState);
			return 1;
		}

		private int importType__index(IntPtr luaState) 
		{
			string className=LuaDLL.lua_tostring(luaState,2);
			foreach(Assembly assembly in assemblies) 
			{
				Type klass=assembly.GetType(className);
				if(klass!=null) 
				{
					pushType(luaState,klass);
					return 1;
				}
			}
			LuaDLL.lua_pushnil(luaState);
			return 1;
		}
		
		/*
		 * Implementation of make_object. Registers a table (first
		 * argument in the stack) as an object subclassing the
		 * type passed as second argument in the stack.
		 */
		private int registerTable(IntPtr luaState) 
		{
			if(LuaDLL.lua_type(luaState,1)==LuaTypes.LUA_TTABLE) 
			{
				LuaTable luaTable=getTable(luaState,1);
				Type klass=(Type)getRawNetObject(luaState,2);
				if(klass!=null) 
				{
					// Creates and pushes the object in the stack, setting
					// it as the  metatable of the first argument
					object obj=CodeGeneration.Instance.GetClassInstance(klass,luaTable);
					pushObject(luaState,obj,"luaNet_metatable");
					LuaDLL.lua_newtable(luaState);
					LuaDLL.lua_pushstring(luaState,"__index");
					LuaDLL.lua_pushvalue(luaState,-3);
					LuaDLL.lua_settable(luaState,-3);
					LuaDLL.lua_pushstring(luaState,"__newindex");
					LuaDLL.lua_pushvalue(luaState,-3);
					LuaDLL.lua_settable(luaState,-3);
					LuaDLL.lua_setmetatable(luaState,1);
					// Pushes the object again, this time as the base field
					// of the table and with the luaNet_searchbase metatable
					LuaDLL.lua_pushstring(luaState,"base");
					int index=-1;
					for(int i=objects.IndexOf(obj)+1;i<objects.Count;i++) 
					{
						if(objects[i]==null) 
						{
							objects[i]=obj;
							index=i;
							break;
						}
					}
					if(index==-1) index=objects.Add(obj);
					pushNewObject(luaState,obj,index,"luaNet_searchbase");
					LuaDLL.lua_rawset(luaState,1);
				} 
				else throwError(luaState,"register_table: second arg is not a valid type reference");
			} 
			else throwError(luaState,"register_table: first arg is not a table");
			return 0;
		}
		/*
		 * Implementation of free_object. Clears the metatable and the
		 * base field, freeing the created object for garbage-collection
		 */
		private int unregisterTable(IntPtr luaState) 
		{
			try 
			{
				if(LuaDLL.lua_getmetatable(luaState,1)!=0) 
				{
					LuaDLL.lua_pushstring(luaState,"__index");
					LuaDLL.lua_gettable(luaState,-2);
					object obj=getRawNetObject(luaState,-1);
					if(obj==null) throwError(luaState,"unregister_table: arg is not valid table");
					FieldInfo luaTableField=obj.GetType().GetField("__luaInterface_luaTable");
					if(luaTableField==null) throwError(luaState,"unregister_table: arg is not valid table");
					luaTableField.SetValue(obj,null);
					LuaDLL.lua_pushnil(luaState);
					LuaDLL.lua_setmetatable(luaState,1);
					LuaDLL.lua_pushstring(luaState,"base");
					LuaDLL.lua_pushnil(luaState);
					LuaDLL.lua_settable(luaState,1);
				} 
				else throwError(luaState,"unregister_table: arg is not valid table");
			} 
			catch(Exception e) 
			{
				throwError(luaState,e.Message);
			}
			return 0;
		}
		/*
		 * Implementation of get_method_bysig. Returns nil
		 * if no matching method is not found.
		 */
		private int getMethodSignature(IntPtr luaState) 
		{
			Type klass; object target;
			IntPtr udata=LuaDLL.luaL_checkudata(luaState,1,"luaNet_class");
			if(udata!=IntPtr.Zero) 
			{
				klass=(Type)objects[readPointer(udata)];
				target=null;
			}
			else 
			{
				target=getRawNetObject(luaState,1);
				if(target==null) 
				{
					throwError(luaState,"get_method_bysig: first arg is not type or object reference");
					LuaDLL.lua_pushnil(luaState);
					return 1;
				}
				klass=target.GetType();
			}
			string methodName=LuaDLL.lua_tostring(luaState,2);
			Type[] signature=new Type[LuaDLL.lua_gettop(luaState)-2];
			for(int i=0;i<signature.Length;i++)
				signature[i]=(Type)getRawNetObject(luaState,i+3);
			try 
			{
				MethodInfo method=klass.GetMethod(methodName,signature);
				pushFunction(luaState,new LuaCSFunction((new LuaMethodWrapper(this,target,klass,method)).call));
			} 
			catch(Exception e) 
			{
				throwError(luaState,e);
				LuaDLL.lua_pushnil(luaState);
			}
			return 1;
		}
		/*
		 * Implementation of get_constructor_bysig. Returns nil
		 * if no matching constructor is found.
		 */
		private int getConstructorSignature(IntPtr luaState) 
		{
			Type klass=null;
			IntPtr udata=LuaDLL.luaL_checkudata(luaState,1,"luaNet_class");
			if(udata!=IntPtr.Zero) 
			{
				klass=(Type)objects[readPointer(udata)];
			}
			if(klass==null) 
			{
				throwError(luaState,"get_constructor_bysig: first arg is invalid type reference");
			}
			Type[] signature=new Type[LuaDLL.lua_gettop(luaState)-1];
			for(int i=0;i<signature.Length;i++)
				signature[i]=(Type)getRawNetObject(luaState,i+2);
			try 
			{
				ConstructorInfo constructor=klass.GetConstructor(signature);
				pushFunction(luaState,new LuaCSFunction((new LuaMethodWrapper(this,null,klass,constructor)).call));
			} 
			catch(Exception e) 
			{
				throwError(luaState,e);
				LuaDLL.lua_pushnil(luaState);
			}
			return 1;
		}
		/*
		 * Pushes a type reference into the stack
		 */
		internal void pushType(IntPtr luaState, Type t) 
		{
			pushObject(luaState,t,"luaNet_class");
		}
		/*
		 * Pushes a delegate into the stack
		 */
		internal void pushFunction(IntPtr luaState, LuaCSFunction func) 
		{
			pushObject(luaState,func,"luaNet_function");
		}
		/*
		 * Pushes a CLR object into the Lua stack as an userdata
		 * with the provided metatable
		 */
		internal void pushObject(IntPtr luaState, object o, string metatable) 
		{
			int index;
			// Pushes nil
			if(o==null) 
			{
				LuaDLL.lua_pushnil(luaState);
				return;
			}
			// Object already in the list of Lua objects? Push the stored reference.
			if((index=objects.IndexOf(o))!=-1) 
			{
				LuaDLL.luaL_getmetatable(luaState,"luaNet_objects");
				LuaDLL.lua_rawgeti(luaState,-1,index);
				LuaDLL.lua_remove(luaState,-2);
				return;
			}
			// New object: inserts it in the list
			for(int i=0;i<objects.Count;i++) 
				if(objects[i]==null) 
				{
					objects[i]=o;
					index=i;
					break;
				} 
			if(index==-1) index=objects.Add(o);
			pushNewObject(luaState,o,index,metatable);
		}
		/*
		 * Pushes a new object into the Lua stack with the provided
		 * metatable
		 */
		private void pushNewObject(IntPtr luaState,object o,int index,string metatable) 
		{
			if(metatable=="luaNet_metatable") 
			{
				// Gets or creates the metatable for the object's type
				LuaDLL.luaL_getmetatable(luaState,o.GetType().AssemblyQualifiedName);
				if(LuaDLL.lua_isnil(luaState,-1))
				{
					LuaDLL.lua_settop(luaState,-2);
					LuaDLL.luaL_newmetatable(luaState,o.GetType().AssemblyQualifiedName);
					LuaDLL.lua_pushstring(luaState,"cache");
					LuaDLL.lua_newtable(luaState);
					LuaDLL.lua_rawset(luaState,-3);
					LuaDLL.lua_pushstring(luaState,"__index");
					LuaDLL.lua_pushstring(luaState,"luaNet_indexfunction");
					LuaDLL.lua_rawget(luaState,LuaIndexes.LUA_REGISTRYINDEX);
					LuaDLL.lua_rawset(luaState,-3);
					LuaDLL.lua_pushstring(luaState,"__gc");
					LuaDLL.lua_pushstdcallcfunction(luaState,metaFunctions.gcFunction);
					LuaDLL.lua_rawset(luaState,-3);
					LuaDLL.lua_pushstring(luaState,"__tostring");
					LuaDLL.lua_pushstdcallcfunction(luaState,metaFunctions.toStringFunction);
					LuaDLL.lua_rawset(luaState,-3);
					LuaDLL.lua_pushstring(luaState,"__newindex");
					LuaDLL.lua_pushstdcallcfunction(luaState,metaFunctions.newindexFunction);
					LuaDLL.lua_rawset(luaState,-3);
				}
			}
			else
			{
				LuaDLL.luaL_getmetatable(luaState,metatable);
			}
			// Stores the object index in the Lua list and pushes the
			// index into the Lua stack
			LuaDLL.luaL_getmetatable(luaState,"luaNet_objects");
			newUserData(luaState,index);
			LuaDLL.lua_pushvalue(luaState,-3);
			LuaDLL.lua_remove(luaState,-4);
			LuaDLL.lua_setmetatable(luaState,-2);
			LuaDLL.lua_pushvalue(luaState,-1);
			LuaDLL.lua_rawseti(luaState,-3,index);
			LuaDLL.lua_remove(luaState,-2);
		}
		/*
		 * Unsafe code (does pointer manipulation). Creates a new userdata
		 * in the Lua stack and sets its value.
		 */
		private unsafe void newUserData(IntPtr luaState, int val) 
		{
			IntPtr pointer=LuaDLL.lua_newuserdata(luaState,sizeof(int));
			int* ptr=(int*)pointer.ToPointer();
			*ptr=val;
		}
		/*
		 * Removes the object from the list, freeing it for garbage-collection.
		 */
		internal void collectObject(IntPtr udata) 
		{
			objects[readPointer(udata)]=null;
		}
		/*
		 * Unsafe code (does pointer manipulation). Returns the integer
		 * addressed by the pointer (the value of the userdata).
		 */
		internal unsafe int readPointer(IntPtr pointer)
		{
			int* ptr=(int*)pointer.ToPointer();
			return *ptr;
		}
		/*
		 * Gets an object from the Lua stack with the desired type, if it matches, otherwise
		 * returns null.
		 */
		internal object getAsType(IntPtr luaState,int stackPos,Type paramType) 
		{
			ExtractValue extractor=typeChecker.checkType(luaState,stackPos,paramType);
			if(extractor!=null) return extractor(luaState,stackPos);
			return null;
		}
		/*
		 * Gets an object from the Lua stack according to its Lua type.
		 */
		internal object getObject(IntPtr luaState,int index) 
		{
			if(LuaDLL.lua_type(luaState,index)==LuaTypes.LUA_TNUMBER) 
			{
				return LuaDLL.lua_tonumber(luaState,index);
			} 
			else if(LuaDLL.lua_type(luaState,index)==LuaTypes.LUA_TSTRING) 
			{
				return LuaDLL.lua_tostring(luaState,index);
			} 
			else if(LuaDLL.lua_type(luaState,index)==LuaTypes.LUA_TBOOLEAN) 
			{
				return LuaDLL.lua_toboolean(luaState,index);
			} 
			else if(LuaDLL.lua_type(luaState,index)==LuaTypes.LUA_TTABLE) 
			{
				return getTable(luaState,index);
			} 
			else if(LuaDLL.lua_type(luaState,index)==LuaTypes.LUA_TFUNCTION) 
			{
				return getFunction(luaState,index);
			} 
			else if(LuaDLL.lua_type(luaState,index)==LuaTypes.LUA_TUSERDATA) 
			{
				IntPtr udata=LuaDLL.luaL_checkudata(luaState,index,"luaNet_class");
				if(!(udata==IntPtr.Zero)) return objects[readPointer(udata)];
				udata=LuaDLL.luaL_checkudata(luaState,index,"luaNet_searchbase");
				if(!(udata==IntPtr.Zero)) return objects[readPointer(udata)];
				udata=LuaDLL.luaL_checkudata(luaState,index,"luaNet_function");
				if(!(udata==IntPtr.Zero)) return getFunction(luaState,index);
				if(LuaDLL.luaL_checkmetatable(luaState,index)) 
					return getRawNetObject(luaState,index);
				return getUserData(luaState,index);
			}
			else return null;
		}
		/*
		 * Gets the table in the index positon of the Lua stack.
		 */
		internal LuaTable getTable(IntPtr luaState,int index) 
		{
			LuaDLL.lua_pushvalue(luaState,index);
			return new LuaTable(LuaDLL.lua_ref(luaState,1),interpreter);
		}
		/*
		 * Gets the userdata in the index positon of the Lua stack.
		 */
		internal LuaUserData getUserData(IntPtr luaState,int index) 
		{
			LuaDLL.lua_pushvalue(luaState,index);
			return new LuaUserData(LuaDLL.lua_ref(luaState,1),interpreter);
		}
		/*
		 * Gets the function in the index positon of the Lua stack.
		 */
		internal LuaFunction getFunction(IntPtr luaState,int index) 
		{
			LuaDLL.lua_pushvalue(luaState,index);
			return new LuaFunction(LuaDLL.lua_ref(luaState,1),interpreter);
		}
		/*
		 * Gets the CLR object in the index positon of the Lua stack. Returns
		 * delegates as Lua functions.
		 */
		internal object getNetObject(IntPtr luaState,int index) 
		{
			if(LuaDLL.lua_type(luaState,index)==LuaTypes.LUA_TUSERDATA) 
			{
				if(LuaDLL.luaL_checkmetatable(luaState,index)) 
					return getRawNetObject(luaState,index);
				IntPtr udata=LuaDLL.luaL_checkudata(luaState,index,"luaNet_class");
				if(udata!=IntPtr.Zero) return objects[readPointer(udata)];
				udata=LuaDLL.luaL_checkudata(luaState,index,"luaNet_searchbase");
				if(udata!=IntPtr.Zero) return objects[readPointer(udata)];
				udata=LuaDLL.luaL_checkudata(luaState,index,"luaNet_function");
				if(udata!=IntPtr.Zero) return getFunction(luaState,index);
				return null;
			}
			return null;
		}
		/*
		 * Gets the CLR object in the index positon of the Lua stack. Returns
		 * delegates as is.
		 */
		internal object getRawNetObject(IntPtr luaState,int index) 
		{
			IntPtr udata=LuaDLL.lua_touserdata(luaState,index);
			if(udata!=IntPtr.Zero) 
			{
				int i=readPointer(udata);
				try 
				{
					return objects[i];
				} 
				catch 
				{
					return null;
				}
			}
			return null;
		}
		/*
		 * Pushes the entire array into the Lua stack and returns the number
		 * of elements pushed.
		 */
		internal int returnValues(IntPtr luaState, object[] returnValues) 
		{
			if(LuaDLL.lua_checkstack(luaState,returnValues.Length+5)) 
			{
				for(int i=0;i<returnValues.Length;i++) 
				{
					push(luaState,returnValues[i]);
				}
				return returnValues.Length;
			} else
				return 0;
		}
		/*
		 * Gets the values from the provided index to
		 * the top of the stack and returns them in an array.
		 */
		internal object[] popValues(IntPtr luaState,int oldTop) 
		{
			int newTop=LuaDLL.lua_gettop(luaState);
			if(oldTop==newTop) 
			{
				return null;
			} 
			else 
			{
				ArrayList returnValues=new ArrayList();
				for(int i=oldTop+1;i<=newTop;i++) 
				{
					returnValues.Add(getObject(luaState,i));
				}
				LuaDLL.lua_settop(luaState,oldTop);
				return returnValues.ToArray();
			}
		}
		/*
		 * Gets the values from the provided index to
		 * the top of the stack and returns them in an array, casting
		 * them to the provided types.
		 */
		internal object[] popValues(IntPtr luaState,int oldTop,Type[] popTypes) 
		{
			int newTop=LuaDLL.lua_gettop(luaState);
			if(oldTop==newTop) 
			{
				return null;
			} 
			else 
			{
				int iTypes;
				ArrayList returnValues=new ArrayList();
				if(popTypes[0].Equals(typeof(void)))
					iTypes=1;
				else
					iTypes=0;
				for(int i=oldTop+1;i<=newTop;i++) 
				{
					returnValues.Add(getAsType(luaState,i,popTypes[iTypes]));
					iTypes++;
				}
				LuaDLL.lua_settop(luaState,oldTop);
				return returnValues.ToArray();
			}
		}
		/*
		 * Pushes the object into the Lua stack according to its type.
		 */
		internal void push(IntPtr luaState, object o) 
		{
			if(o==null) 
			{
				LuaDLL.lua_pushnil(luaState);
			}
			else if(o is ILuaGeneratedType) 
			{
				(((ILuaGeneratedType)o).__luaInterface_getLuaTable()).push(luaState);
			}
			else if(o is LuaTable) 
			{
				((LuaTable)o).push(luaState);
			} 
			else if(o is LuaCSFunction) 
			{
				pushFunction(luaState,(LuaCSFunction)o);
			} 
			else if(o is LuaFunction)
			{
				((LuaFunction)o).push(luaState);
			}
			else if(o is string)
			{
				string str=(string)o;
				LuaDLL.lua_pushstring(luaState,str);
			}
			else if(o is bool)
			{
				bool b=(bool)o;
				LuaDLL.lua_pushboolean(luaState,b);
			}
			else if(o is sbyte || o is byte || o is short || o is ushort ||
				o is int || o is uint || o is long || o is char || o is float ||
				o is ulong || o is decimal || o is double) 
			{
				double d=Convert.ToDouble(o);
				LuaDLL.lua_pushnumber(luaState,d);
			}
			else 
			{
				pushObject(luaState,o,"luaNet_metatable");
			}
		}
		/*
		 * Checks if the method matches the arguments in the Lua stack, getting
		 * the arguments if it does.
		 */
		internal bool matchParameters(IntPtr luaState,MethodBase method,ref MethodCache methodCache) 
		{
			return metaFunctions.matchParameters(luaState,method,ref methodCache);
		}
	}
}