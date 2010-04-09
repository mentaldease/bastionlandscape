#ifndef __CORETYPES_H__
#define __CORETYPES_H__

#include "../Core/VMath.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	typedef vector<fsVector2> fsVector2Vec;
	typedef vector<fsVector3> fsVector3Vec;
	typedef vector<fsVector4> fsVector4Vec;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	typedef std::size_t Key;
	typedef Key* KeyPtr;
	typedef Key& KeyRef;
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
	typedef UIntVec::iterator	UIntVecIt;

	typedef unsigned char		Byte;
	typedef Byte*				BytePtr;
	typedef Byte&				ByteRef;

	typedef float*				FloatPtr;
	typedef float&				FloatRef;
	typedef map<Key, FloatPtr>	FloatPtrMap;

	typedef bool*				BoolPtr;
	typedef bool&				BoolRef;

	typedef char*				CharPtr;
	typedef char&				CharRef;
	typedef vector<CharPtr>		CharPtrVec;

	typedef unsigned short		Word;
	typedef Word*				WordPtr;
	typedef Word&				WordRef;

	typedef vector<string>		StringVec;

	typedef wchar_t*			WCharPtr;

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

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class OctreeObject;
	typedef OctreeObject* OctreeObjectPtr;
	typedef OctreeObject& OctreeObjectRef;
	typedef vector<OctreeObjectPtr> OctreeObjectPtrVec;
	typedef OctreeObjectPtrVec* OctreeObjectPtrVecPtr;
	typedef OctreeObjectPtrVec& OctreeObjectPtrVecRef;
	typedef map<Key, OctreeObjectPtr> OctreeObjectPtrMap;

	class OctreeNode;
	typedef OctreeNode* OctreeNodePtr;
	typedef OctreeNode& OctreeNodeRef;
	typedef vector<OctreeNode> OctreeNodeVec;
	typedef vector<OctreeNodePtr> OctreeNodePtrVec;
	typedef OctreeNodePtrVec* OctreeNodePtrVecPtr;
	typedef OctreeNodePtrVec& OctreeNodePtrVecRef;
	typedef map<Key, OctreeNodePtr> OctreeNodePtrMap;

	class Octree;
	typedef Octree* OctreePtr;
	typedef Octree& OctreeRef;

	enum EOctreeTraverseResult
	{
		EOctreeTraverseResult_UNKNOWN,
		EOctreeTraverseResult_NONE,
		EOctreeTraverseResult_PARTIAL,
		EOctreeTraverseResult_FULL
	};
	typedef boost::function<EOctreeTraverseResult (OctreeNodeRef _rNode)> OctreeTraverseFunc;
	typedef OctreeTraverseFunc* OctreeTraverseFuncPtr;
	typedef OctreeTraverseFunc& OctreeTraverseFuncRef;
	typedef map<Key, OctreeTraverseFunc> OctreeTraverseFuncMap;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Profiling;
	typedef Profiling* ProfilingPtr;
	typedef Profiling& ProfilingRef;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Job;
	typedef Job* JobPtr;
	typedef Job& JobRef;

	class JobManager;
	typedef JobManager* JobManagerPtr;
	typedef JobManager& JobManagerRef;
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
