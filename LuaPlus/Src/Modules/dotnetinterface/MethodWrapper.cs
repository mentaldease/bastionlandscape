namespace LuaInterface 
{
	using System;
	using System.IO;
	using System.Collections;
	using System.Reflection;

	/*
	 * Cached method
	 */
	struct MethodCache 
	{
		public MethodBase cachedMethod;
		// List or arguments
		public object[] args;
		// Positions of out parameters
		public int[] outList;
		// Types of parameters
		public MethodArgs[] argTypes;
	}

	/*
	 * Parameter information
	 */
	struct MethodArgs 
	{
		// Position of parameter
		public int index;
		// Type-conversion function
		public ExtractValue extractValue;
	}

	/*
	 * Argument extraction with type-conversion function
	 */
	delegate object ExtractValue(IntPtr luaState, int stackPos);

	/*
	 * Wrapper class for methods/constructors accessed from Lua.
	 * 
	 * Author: Fabio Mascarenhas
	 * Version: 1.0
	 */
	class LuaMethodWrapper 
	{
		ObjectTranslator translator;
		MethodBase method;
		MethodCache lastCalledMethod=new MethodCache();
		string methodName;
		MemberInfo[] members;
		Type targetType;
		object target;
		BindingFlags bindingType;
		/*
		 * Constructs the wrapper for a known MethodBase instance
		 */
		public LuaMethodWrapper(ObjectTranslator translator, object target, Type targetType, MethodBase method) 
		{
			this.translator=translator;
			this.target=target;
			this.targetType=targetType;
			this.method=method;
			this.methodName=method.Name;
			if(method.IsStatic) { bindingType=BindingFlags.Static; } 
			else { bindingType=BindingFlags.Instance; }
		}
		/*
		 * Constructs the wrapper for a known method name
		 */
		public LuaMethodWrapper(ObjectTranslator translator, Type targetType, string methodName, BindingFlags bindingType) 
		{
			this.translator=translator;
			this.methodName=methodName;
			this.targetType=targetType;
			this.bindingType=bindingType;
			members=targetType.GetMember(methodName,MemberTypes.Method,bindingType|BindingFlags.Public|BindingFlags.NonPublic);
		}
		/*
		 * Calls the method. Receives the arguments from the Lua stack
		 * and returns values in it.
		 */
		public int call(IntPtr luaState) 
		{
			MethodBase methodToCall=method;
			object targetObject=target;
			bool failedCall=true;
			int nReturnValues=1;
			if(!LuaDLL.lua_checkstack(luaState,5))
				throw new Exception("Lua stack overflow");
			if(methodToCall==null) // Method from name
			{
				if(bindingType==BindingFlags.Static) 
					targetObject=null;
				else
					targetObject=translator.typeChecker.getAsObject(luaState,1);
				LuaDLL.lua_remove(luaState,1); // Pops the receiver
				if(lastCalledMethod.cachedMethod!=null) // Cached?
				{
					if(LuaDLL.lua_gettop(luaState)==lastCalledMethod.argTypes.Length) // No. of args match?
					{
						if(!LuaDLL.lua_checkstack(luaState,lastCalledMethod.outList.Length+6))
							throw new Exception("Lua stack overflow");
						try 
						{
							for(int i=0;i<lastCalledMethod.argTypes.Length;i++) 
							{
								lastCalledMethod.args[lastCalledMethod.argTypes[i].index]=
									lastCalledMethod.argTypes[i].extractValue(luaState,i+1);
								if(lastCalledMethod.args[lastCalledMethod.argTypes[i].index]==null &&
									!LuaDLL.lua_isnil(luaState,i+1)) 
								{
									throw new Exception("argument number "+(i+1)+" is invalid"); 
								}
							}
							if(bindingType==BindingFlags.Static) 
							{
								translator.push(luaState,lastCalledMethod.cachedMethod.Invoke(null,lastCalledMethod.args));
							} 
							else 
							{
								if(lastCalledMethod.cachedMethod.IsConstructor)
									translator.push(luaState,((ConstructorInfo)lastCalledMethod.cachedMethod).Invoke(lastCalledMethod.args));
								else
									translator.push(luaState,lastCalledMethod.cachedMethod.Invoke(targetObject,lastCalledMethod.args));
							}
							failedCall=false;
						} 
						catch(TargetInvocationException e) 
						{
							// Failure of method invocation
							failedCall=false;
							translator.throwError(luaState,e.GetBaseException());
							LuaDLL.lua_pushnil(luaState);
							return 1;
						}
						catch(Exception e)
						{
							if(members.Length==1) // Is the method overloaded?
							{
								// No, throw error
								translator.throwError(luaState,e);
								LuaDLL.lua_pushnil(luaState);
								return 1;
							}
						}
					}
				} 
				// Cache miss
				if(failedCall) 
				{
					// Tries to match all versions
					bool hasMatch=false;
					foreach(MemberInfo member in members) 
					{
						MethodBase m=(MethodInfo)member;
						bool isMethod=translator.matchParameters(luaState,m,ref lastCalledMethod);
						if(isMethod) 
						{
							hasMatch=true;
							break;
						}
					}
					if(!hasMatch) 
					{
						translator.throwError(luaState,"invalid arguments to method call"); 
						LuaDLL.lua_pushnil(luaState);
						return 1;
					}
				}
			} 
			else // Method from MethodBase instance 
			{
				if(!methodToCall.IsStatic && !methodToCall.IsConstructor && targetObject==null) 
				{
					targetObject=translator.typeChecker.getAsObject(luaState,1);
					LuaDLL.lua_remove(luaState,1); // Pops the receiver
				}
				if(!translator.matchParameters(luaState,methodToCall,ref lastCalledMethod)) 
				{
					translator.throwError(luaState,"invalid arguments to method call"); 
					LuaDLL.lua_pushnil(luaState);
					return 1;
				}
			}
			if(failedCall) 
			{
				if(!LuaDLL.lua_checkstack(luaState,lastCalledMethod.outList.Length+6))
					throw new Exception("Lua stack overflow");
				try 
				{
					if(bindingType==BindingFlags.Static) 
					{
						translator.push(luaState,lastCalledMethod.cachedMethod.Invoke(null,lastCalledMethod.args));
					} 
					else 
					{
						if(lastCalledMethod.cachedMethod.IsConstructor)
							translator.push(luaState,((ConstructorInfo)lastCalledMethod.cachedMethod).Invoke(lastCalledMethod.args));
						else
							translator.push(luaState,lastCalledMethod.cachedMethod.Invoke(targetObject,lastCalledMethod.args));
					}
				} 
				catch(TargetInvocationException e) 
				{
					translator.throwError(luaState,e.GetBaseException());
					LuaDLL.lua_pushnil(luaState);
					return 1;
				}
				catch(Exception e)
				{
					translator.throwError(luaState,e);
					LuaDLL.lua_pushnil(luaState);
					return 1;
				}
			}
			// Pushes out and ref return values
			for(int index=0;index<lastCalledMethod.outList.Length;index++)
			{
				nReturnValues++;
				for(int i=0;i<lastCalledMethod.outList.Length;i++)
					translator.push(luaState,lastCalledMethod.args[lastCalledMethod.outList[i]]);
			}
			return nReturnValues;
		}
	}

	/*
	 * Wrapper class for events that does registration/deregistration
	 * of event handlers.
	 * 
	 * Author: Fabio Mascarenhas
	 * Version: 1.0
	 */
	class RegisterEventHandler 
	{
		public object target;
		public EventInfo eventInfo;
		public RegisterEventHandler(object target, EventInfo eventInfo) 
		{
			this.target=target;
			this.eventInfo=eventInfo;
		}
		/*
		 * Adds a new event handler
		 */
		public Delegate Add(LuaFunction function) 
		{
			MethodInfo mi = eventInfo.EventHandlerType.GetMethod("Invoke");
			ParameterInfo[] pi = mi.GetParameters();
			LuaEventHandler handler=CodeGeneration.Instance.GetEvent(pi[1].ParameterType,function);
			Delegate handlerDelegate=Delegate.CreateDelegate(eventInfo.EventHandlerType,handler,"HandleEvent");
			eventInfo.AddEventHandler(target,handlerDelegate);
			return handlerDelegate;
		}
		/*
		 * Removes an existing event handler
		 */
		public void Remove(Delegate handlerDelegate) 
		{
			eventInfo.RemoveEventHandler(target,handlerDelegate);
		}
	}

	/*
	 * Base wrapper class for Lua function event handlers.
	 * Subclasses that do actual event handling are created
	 * at runtime.
	 * 
	 * Author: Fabio Mascarenhas
	 * Version: 1.0
	 */
	public class LuaEventHandler 
	{
		public LuaFunction handler;
		public LuaEventHandler() 
		{
			handler=null;
		}
		public void handleEvent(object sender,object data) 
		{
			handler.call(new object[] { sender,data },new Type[0]);
		}
	}

	/*
	 * Wrapper class for Lua functions as delegates
	 * Subclasses with correct signatures are created
	 * at runtime.
	 * 
	 * Author: Fabio Mascarenhas
	 * Version: 1.0
	 */
	public class LuaDelegate
	{
		public Type[] returnTypes;
		public LuaFunction function;
		public LuaDelegate() 
		{
			function=null;
			returnTypes=null;
		}
		public object callFunction(object[] args,object[] inArgs,int[] outArgs) 
		{
			// args is the return array of arguments, inArgs is the actual array
			// of arguments passed to the function (with in parameters only), outArgs
			// has the positions of out parameters
			object returnValue;
			int iRefArgs;
			object[] returnValues=function.call(inArgs,returnTypes);
			if(returnTypes[0].Equals(typeof(void))) 
			{
				returnValue=null;
				iRefArgs=0;
			}
			else 
			{
				returnValue=returnValues[0];
				iRefArgs=1;
			}
			// Sets the value of out and ref parameters (from
			// the values returned by the Lua function).
			for(int i=0;i<outArgs.Length;i++) 
			{
				args[outArgs[i]]=returnValues[iRefArgs];
				iRefArgs++;
			}
			return returnValue;
		}
	}

	/*
	 * Static helper methods for Lua tables acting as CLR objects.
	 * 
	 * Author: Fabio Mascarenhas
	 * Version: 1.0
	 */
	public class LuaClassHelper
	{
		/*
		 *  Gets the function called name from the provided table,
		 * returning null if it does not exist
		 */
		public static LuaFunction getTableFunction(LuaTable luaTable,string name) 
		{
			object funcObj=luaTable.rawget(name);
			if(funcObj is LuaFunction)
				return (LuaFunction)funcObj;
			else
				return null;
		}
		/*
		 * Calls the provided function with the provided parameters
		 */
		public static object callFunction(LuaFunction function,object[] args,Type[] returnTypes,object[] inArgs,int[] outArgs) 
		{
			// args is the return array of arguments, inArgs is the actual array
			// of arguments passed to the function (with in parameters only), outArgs
			// has the positions of out parameters
			object returnValue;
			int iRefArgs;
			object[] returnValues=function.call(inArgs,returnTypes);
			if(returnTypes[0].Equals(typeof(void))) 
			{
				returnValue=null;
				iRefArgs=0;
			}
			else 
			{
				returnValue=returnValues[0];
				iRefArgs=1;
			}
			for(int i=0;i<outArgs.Length;i++) 
			{
				args[outArgs[i]]=returnValues[iRefArgs];
				iRefArgs++;
			}
			return returnValue;
		}
	}
}
