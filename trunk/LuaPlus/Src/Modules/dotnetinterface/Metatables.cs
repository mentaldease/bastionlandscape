namespace LuaInterface 
{
	using System;
	using System.IO;
	using System.Collections;
	using System.Reflection;

	/*
	 * Functions used in the metatables of userdata representing
	 * CLR objects
	 * 
	 * Author: Fabio Mascarenhas
	 * Version: 1.0
	 */
	class MetaFunctions 
	{
		/*
		 * __index metafunction for CLR objects. Implemented in Lua.
		 */
		internal static string luaIndexFunction=
			"local function index(obj,name)\n"+
			"  local meta=getmetatable(obj)\n"+
			"  local cached=meta.cache[name]\n"+
			"  if cached~=nil  then\n"+
			"    return cached\n"+
			"  else\n"+
			"    value,isFunc=get_object_member(obj,name)\n"+
			"    if isFunc then\n"+
			"      meta.cache[name]=value\n"+
			"    end\n"+
			"    return value\n"+
			"  end\n"+
			"end\n"+
			"return index";
		
		private ObjectTranslator translator;
		private Hashtable memberCache;
		internal LuaCSFunction gcFunction,indexFunction,newindexFunction,
			baseIndexFunction,classIndexFunction,classNewindexFunction,
			execDelegateFunction,callConstructorFunction,toStringFunction;
			
		public MetaFunctions(ObjectTranslator translator) 
		{
			this.translator=translator;
			memberCache=new Hashtable();
			gcFunction=new LuaCSFunction(this.collectObject);
			toStringFunction=new LuaCSFunction(this.toString);
			indexFunction=new LuaCSFunction(this.getMethod);
			newindexFunction=new LuaCSFunction(this.setFieldOrProperty);
			baseIndexFunction=new LuaCSFunction(this.getBaseMethod);
			callConstructorFunction=new LuaCSFunction(this.callConstructor);
			classIndexFunction=new LuaCSFunction(this.getClassMethod);
			classNewindexFunction=new LuaCSFunction(this.setClassFieldOrProperty);
			execDelegateFunction=new LuaCSFunction(this.runFunctionDelegate);
		}

		/*
		 * __call metafunction of CLR delegates, retrieves and calls the delegate.
		 */
		private int runFunctionDelegate(IntPtr luaState) 
		{
			LuaCSFunction func=(LuaCSFunction)translator.getRawNetObject(luaState,1);
			LuaDLL.lua_remove(luaState,1);
			return func(luaState);
		}
		/*
		 * __gc metafunction of CLR objects.
		 */
		private int collectObject(IntPtr luaState) 
		{
			IntPtr udata=LuaDLL.lua_touserdata(luaState,1);
			if(udata!=IntPtr.Zero) 
			{
				translator.collectObject(udata);
			}
			return 0;
		}
		/*
		 * __tostring metafunction of CLR objects.
		 */
		private int toString(IntPtr luaState)
		{
			object obj=translator.getRawNetObject(luaState,1);
			if(obj!=null) {
				translator.push(luaState,obj.ToString());
			} else LuaDLL.lua_pushnil(luaState);
			return 1;
		}
		/*
		 * Called by the __index metafunction of CLR objects in case the
		 * method is not cached or it is a field/property/event.
		 * Receives the object and the member name as arguments and returns
		 * either the value of the member or a delegate to call it.
		 * If the member does not exist returns nil.
		 */
		private int getMethod(IntPtr luaState)
		{
			object obj=translator.getRawNetObject(luaState,1);
			if(obj==null) 
			{
				translator.throwError(luaState,"trying to index an invalid object reference");
				LuaDLL.lua_pushnil(luaState);
				return 1;
			}
			if(LuaDLL.lua_isnumber(luaState,2)) 
			{
				int index=(int)LuaDLL.lua_tonumber(luaState,2);
				try 
				{
					object[] arr=(object[])obj;
					try 
					{
						translator.push(luaState,arr[index]);
					} 
					catch 
					{
						LuaDLL.lua_pushnil(luaState);
					}
				} 
				catch 
				{
					LuaDLL.lua_pushnil(luaState);
				}
				return 1;
			} 
			else 
			{
				string methodName=LuaDLL.lua_tostring(luaState,2);
				if(methodName==null) 
				{
					LuaDLL.lua_pushnil(luaState);
					return 1;
				}
				return getMember(luaState,obj.GetType(),obj,methodName,BindingFlags.Instance);
			}
		}
		/*
		 * __index metafunction of base classes (the base field of Lua tables).
		 * Adds a prefix to the method name to call the base version of the method.
		 */
		private int getBaseMethod(IntPtr luaState)
		{
			object obj=translator.getRawNetObject(luaState,1);
			if(obj==null) 
			{
				translator.throwError(luaState,"trying to index an invalid object reference");
				LuaDLL.lua_pushnil(luaState);
				return 1;
			}
			string methodName=LuaDLL.lua_tostring(luaState,2);
			if(methodName==null) 
			{
				LuaDLL.lua_pushnil(luaState);
				return 1;
			}
			getMember(luaState,obj.GetType(),obj,"__luaInterface_base_"+methodName,BindingFlags.Instance);
			LuaDLL.lua_settop(luaState,-2);
			if(LuaDLL.lua_type(luaState,-1)==LuaTypes.LUA_TNIL) 
			{
				LuaDLL.lua_settop(luaState,-2);
				return getMember(luaState,obj.GetType(),obj,methodName,BindingFlags.Instance);
			}
			return 1;
		}
		/*
		 * Pushes the value of a member or a delegate to call it, depending on the type of
		 * the member. Works with static or instance members.
		 * Uses reflection to find members, and stores the reflected MemberInfo object in
		 * a cache (indexed by the type of the object and the name of the member).
		 */
		private int getMember(IntPtr luaState, Type objType, object obj, string methodName, BindingFlags bindingType)
		{
			MemberInfo member=null;
			object cachedMember=checkMemberCache(memberCache,objType,methodName);
			//object cachedMember=null;
			if(cachedMember is LuaCSFunction) 
			{
				translator.pushFunction(luaState,(LuaCSFunction)cachedMember);
				translator.push(luaState,true);
				return 2;
			} 
			else if(cachedMember is MemberInfo) 
			{
				member=(MemberInfo)cachedMember;
			} 
			else 
			{
				MemberInfo[] members=objType.GetMember(methodName,bindingType|BindingFlags.Public|BindingFlags.NonPublic);
				if(members.Length>0) member=members[0];
			}
			if(member!=null)
			{
				if(member.MemberType==MemberTypes.Field) 
				{
					FieldInfo field=(FieldInfo)member;
					if(cachedMember==null) setMemberCache(memberCache,objType,methodName,member);
					try 
					{
						translator.push(luaState,field.GetValue(obj));
					}
					catch 
					{
						LuaDLL.lua_pushnil(luaState);
					}
				} 
				else if(member.MemberType==MemberTypes.Property) 
				{
					PropertyInfo property=(PropertyInfo)member;
					if(cachedMember==null) setMemberCache(memberCache,objType,methodName,member);
					try 
					{
						translator.push(luaState,property.GetValue(obj,null));
					} 
					catch 
					{
						LuaDLL.lua_pushnil(luaState);
					}
				} 
				else if(member.MemberType==MemberTypes.Event) 
				{
					EventInfo eventInfo=(EventInfo)member;
					if(cachedMember==null) setMemberCache(memberCache,objType,methodName,member);
					translator.push(luaState,new RegisterEventHandler(obj,eventInfo));
				}
				else 
				{
					LuaCSFunction wrapper=new LuaCSFunction((new LuaMethodWrapper(translator,objType,methodName,bindingType)).call);
					if(cachedMember==null) setMemberCache(memberCache,objType,methodName,wrapper);
					translator.pushFunction(luaState,wrapper);
					translator.push(luaState,true);
					return 2;
				}
			} 
			else
				LuaDLL.lua_pushnil(luaState);
			return 1;
		}
		/*
		 * Checks if a MemberInfo object is cached, returning it or null.
		 */
		private object checkMemberCache(Hashtable memberCache,Type objType,string memberName)
		{
			Hashtable members=(Hashtable)memberCache[objType];
			if(members!=null) 
				return members[memberName];
			else
				return null;
		}
		/*
		 * Stores a MemberInfo object in the member cache.
		 */
		private void setMemberCache(Hashtable memberCache,Type objType,string memberName,object member)
		{
			Hashtable members=(Hashtable)memberCache[objType];
			if(members==null) 
			{
				members=new Hashtable();
				memberCache[objType]=members;
			}
			members[memberName]=member;
		}
		/*
		 * __newindex metafunction of CLR objects. Receives the object,
		 * the member name and the value to be stored as arguments. Throws
		 * and error if the assignment is invalid.
		 */
		private int setFieldOrProperty(IntPtr luaState) 
		{
			if(LuaDLL.lua_isnumber(luaState,2)) 
			{
				object target=translator.getRawNetObject(luaState,1);
				if(target==null) 
				{
					translator.throwError(luaState,"trying to index and invalid object reference");
					return 0;
				}
				int index=(int)LuaDLL.lua_tonumber(luaState,2);
				try 
				{
					object[] arr=(object[])target;
					object val=translator.getAsType(luaState,3,arr.GetType().GetElementType());
					arr[index]=val;
				} 
				catch(Exception e) 
				{
					translator.throwError(luaState,e);
				}
				return 0;
			} 
			else 
			{
				object target=translator.getRawNetObject(luaState,1);
				if(target==null) 
				{
					translator.throwError(luaState,"trying to index an invalid object reference");
					return 0;
				}
				return setMember(luaState,target.GetType(),target,BindingFlags.Instance);
			}
		}
		/*
		 * Writes to fields or properties, either static or instance. Throws an error
		 * if the operation is invalid.
		 */
		private int setMember(IntPtr luaState, Type targetType, object target, BindingFlags bindingType) 
		{
			string fieldName=LuaDLL.lua_tostring(luaState,2);
			if(fieldName==null) 
			{
				translator.throwError(luaState,"field or property does not exist");
				return 0;
			}
			MemberInfo member=(MemberInfo)checkMemberCache(memberCache,targetType,fieldName);
			if(member==null) 
			{
				MemberInfo[] members=targetType.GetMember(fieldName,bindingType|BindingFlags.Public|BindingFlags.NonPublic);
				if(members.Length>0) 
				{
					member=members[0];
					setMemberCache(memberCache,targetType,fieldName,member);
				} 
				else 
				{
					translator.throwError(luaState,"field or property does not exist");
					return 0;
				}
			}
			if(member.MemberType==MemberTypes.Field) 
			{
				FieldInfo field=(FieldInfo)member;
				object val=translator.getAsType(luaState,3,field.FieldType);
				try 
				{
					field.SetValue(target,val);
				} 
				catch(Exception e) 
				{
					translator.throwError(luaState,e);
				}
				return 0;
			} 
			else if(member.MemberType==MemberTypes.Property) 
			{
				PropertyInfo property=(PropertyInfo)member;
				object val=translator.getAsType(luaState,3,property.PropertyType);
				try 
				{
					property.SetValue(target,val,null);
				} 
				catch(Exception e) 
				{
					translator.throwError(luaState,e);
				}
				return 0;
			}
			translator.throwError(luaState,"field or property does not exist");
			return 0;
		}
		/*
		 * __index metafunction of type references, works on static members.
		 */
		private int getClassMethod(IntPtr luaState)
		{
			Type klass;
			object obj=translator.getRawNetObject(luaState,1);
			if(obj==null || !(obj is Type)) 
			{
				translator.throwError(luaState,"trying to index an invalid type reference");
				LuaDLL.lua_pushnil(luaState);
				return 1;
			} 
			else klass=(Type)obj;
			if(LuaDLL.lua_isnumber(luaState,2)) 
			{
				int size=(int)LuaDLL.lua_tonumber(luaState,2);
				translator.push(luaState,Array.CreateInstance(klass,size));
				return 1;
			} 
			else 
			{
				string methodName=LuaDLL.lua_tostring(luaState,2);
				if(methodName==null) 
				{
					LuaDLL.lua_pushnil(luaState);
					return 1;
				} 
				else return getMember(luaState,klass,null,methodName,BindingFlags.Static);
			}
		}
		/*
		 * __newindex function of type references, works on static members.
		 */
		private int setClassFieldOrProperty(IntPtr luaState) 
		{
			Type target;
			object obj=translator.getRawNetObject(luaState,1);
			if(obj==null || !(obj is Type)) 
			{
				translator.throwError(luaState,"trying to index an invalid type reference");
				return 0;
			} 
			else target=(Type)obj;
			return setMember(luaState,target,null,BindingFlags.Static);
		}
		/*
		 * __call metafunction of type references. Searches for and calls
		 * a constructor for the type. Returns nil if the constructor is not
		 * found or if the arguments are invalid. Throws an error if the constructor
		 * generates an exception.
		 */
		private int callConstructor(IntPtr luaState)
		{
			MethodCache validConstructor=new MethodCache();
			Type klass;
			object obj=translator.getRawNetObject(luaState,1);
			if(obj==null || !(obj is Type)) 
			{
				translator.throwError(luaState,"trying to call constructor on an invalid type reference");
				LuaDLL.lua_pushnil(luaState);
				return 1;
			} 
			else klass=(Type)obj;
			ArrayList paramList=new ArrayList();
			ArrayList outList=new ArrayList();
			LuaDLL.lua_remove(luaState,1);
			ConstructorInfo[] constructors=klass.GetConstructors();
			foreach(ConstructorInfo constructor in constructors) 
			{
				bool isConstructor=matchParameters(luaState,constructor,ref validConstructor);
				if(isConstructor) 
				{
					object[] args=paramList.ToArray();
					try 
					{
						translator.push(luaState,constructor.Invoke(validConstructor.args));
					} 
					catch(TargetInvocationException e) 
					{
						translator.throwError(luaState,e.InnerException);
						LuaDLL.lua_pushnil(luaState);
					} 
					catch 
					{
						LuaDLL.lua_pushnil(luaState);
					}
					return 1;
				}
			}
			LuaDLL.lua_pushnil(luaState);
			return 1;
		}
		/*
		 * Matches a method against its arguments in the Lua stack. Returns
		 * if the match was succesful. It it was also returns the information
		 * necessary to invoke the method.
		 */
		internal bool matchParameters(IntPtr luaState,MethodBase method,ref MethodCache methodCache) 
		{
			ExtractValue extractValue;
			bool isMethod=true;
			ParameterInfo[] paramInfo=method.GetParameters();
			int currentLuaParam=1;
			int nLuaParams=LuaDLL.lua_gettop(luaState);
			ArrayList paramList=new ArrayList();
			ArrayList outList=new ArrayList();
			ArrayList argTypes=new ArrayList();
			foreach(ParameterInfo currentNetParam in paramInfo) 
			{
				if(!currentNetParam.IsIn && currentNetParam.IsOut)  // Skips out params
				{
					outList.Add(paramList.Add(null));
				}
				else if(currentLuaParam>nLuaParams) // Adds optional parameters
				{
					if(currentNetParam.IsOptional) 
					{
						paramList.Add(currentNetParam.DefaultValue);
					}
					else 
					{
						isMethod=false;
						break;
					}
				}
				else if((extractValue=translator.typeChecker.checkType(luaState,currentLuaParam,currentNetParam.ParameterType))!=null)  // Type checking
				{
					int index=paramList.Add(extractValue(luaState,currentLuaParam));
					MethodArgs methodArg=new MethodArgs();
					methodArg.index=index;
					methodArg.extractValue=extractValue;
					argTypes.Add(methodArg);
					if(currentNetParam.ParameterType.IsByRef)
						outList.Add(index);
					currentLuaParam++;
				}  // Type does not match, ignore if the parameter is optional
				else if(currentNetParam.IsOptional) 
				{
					paramList.Add(currentNetParam.DefaultValue);
				}
				else  // No match
				{ 
					isMethod=false;
					break;
				}
			}
			if(currentLuaParam!=nLuaParams+1) // Number of parameters does not match
				isMethod=false;
			if(isMethod) 
			{
				methodCache.args=paramList.ToArray();
				methodCache.cachedMethod=method;
				methodCache.outList=(int[])outList.ToArray(typeof(int));
				methodCache.argTypes=(MethodArgs[])argTypes.ToArray(typeof(MethodArgs));
			}
			return isMethod;
		}
	}
}