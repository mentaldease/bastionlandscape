#ifndef __CORETYPES_H__
#define __CORETYPES_H__

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	typedef std::size_t Key;
	typedef vector<Key> KeyVec;

	template<typename T>
	inline Key MakeKey(const T& _rData)
	{
		boost::hash<T> T_hash;
		return T_hash(_rData);
	}

	template<typename T>
	inline Key MakeKey(const T* _pData)
	{
		boost::hash<T> T_hash;
		return T_hash(*_pData);
	}

	class CoreObject;
	typedef CoreObject*				CoreObjectPtr;
	typedef CoreObject&				CoreObjectRef;
	typedef vector<CoreObjectPtr>	CoreObjectPtrVec;
	typedef map<CoreObjectPtr, int>	CoreObjectPtrCounterMap;
	typedef map<Key, CoreObjectPtr>	CoreObjectPtrMap;

	typedef void*				VoidPtr;
	typedef vector<VoidPtr>		VoidPtrVec;

	typedef unsigned int		UInt;
	typedef UInt*				UIntPtr;
	typedef UInt&				UIntRef;
	typedef vector<UInt>		UIntVec;

	typedef unsigned char		Byte;
	typedef Byte*				BytePtr;
	typedef Byte&				ByteRef;

	typedef float*				FloatPtr;
	typedef float&				FloatRef;
	typedef map<Key, FloatPtr>	FloatPtrMap;

	typedef bool*				BoolPtr;
	typedef bool&				BoolRef;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Config;
	typedef Config* ConfigPtr;
	typedef Config& ConfigRef;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class File;
	typedef File* FilePtr;
	typedef File& FileRef;

	class FS;
	typedef FS* FSPtr;
	typedef FS& FSRef;
	typedef vector<FSPtr> FSPtrVec;
	typedef map<Key, FSPtr> FSPtrMap;

	typedef map<FilePtr, FSPtr> OpenFileMap;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Time;
	typedef Time* TimePtr;
	typedef Time& TimeRef;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class NoiseGenerator;
	typedef NoiseGenerator*	NoiseGeneratorPtr;
	typedef NoiseGenerator&	NoiseGeneratorRef;
}

namespace LuaPlus
{
	class LuaState;
	typedef LuaState*	LuaStatePtr;
	typedef LuaState&	LuaStateRef;

	class LuaObject;
	typedef LuaObject*	LuaObjectPtr;
	typedef LuaObject&	LuaObjectRef;
}
using namespace LuaPlus;

#endif // __CORETYPES_H__
