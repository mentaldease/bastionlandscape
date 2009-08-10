#ifndef __LANDSCAPETYPES_H__
#define __LANDSCAPETYPES_H__

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Landscape;
	typedef Landscape* LandscapePtr;
	typedef Landscape& LandscapeRef;
	typedef vector<LandscapePtr> LandscapePtrVec;
	typedef map<Key, LandscapePtr> LandscapePtrMap;

	class LandscapeChunk;
	typedef LandscapeChunk*				LandscapeChunkPtr;
	typedef vector<LandscapeChunkPtr>	LandscapeChunkPtrVec;
	typedef LandscapeChunkPtrVec&		LandscapeChunkPtrVecRef;
}

#endif // __LANDSCAPETYPES_H__
