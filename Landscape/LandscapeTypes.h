#ifndef __LANDSCAPETYPES_H__
#define __LANDSCAPETYPES_H__

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Landscape;
	typedef Landscape*				LandscapePtr;
	typedef Landscape&				LandscapeRef;
	typedef vector<LandscapePtr>	LandscapePtrVec;
	typedef map<Key, LandscapePtr>	LandscapePtrMap;

	class LandscapeChunk;
	typedef LandscapeChunk*				LandscapeChunkPtr;
	typedef vector<LandscapeChunkPtr>	LandscapeChunkPtrVec;
	typedef LandscapeChunkPtrVec&		LandscapeChunkPtrVecRef;

	class LandscapeLayering;
	typedef LandscapeLayering*				LandscapeLayeringPtr;
	typedef LandscapeLayering&				LandscapeLayeringRef;
	typedef map<Key, LandscapeLayeringPtr>	LandscapeLayeringPtrMap;

	class LandscapeLayerManager;
	typedef LandscapeLayerManager*	LandscapeLayerManagerPtr;
	typedef LandscapeLayerManager&	LandscapeLayerManagerRef;

	struct LandscapeVertexDefault;
	typedef LandscapeVertexDefault*	LandscapeVertexDefaultPtr;
	typedef LandscapeVertexDefault&	LandscapeVertexDefaultRef;

	struct LandscapeVertexLiquid;
	typedef LandscapeVertexLiquid*	LandscapeVertexLiquidPtr;
	typedef LandscapeVertexLiquid&	LandscapeVertexLiquidRef;

	struct LandscapeVertexIndependent;
	typedef LandscapeVertexIndependent*				LandscapeVertexIndependentPtr;
	typedef LandscapeVertexIndependent&				LandscapeVertexIndependentRef;
	typedef vector<LandscapeVertexIndependentPtr>	LandscapeVertexIndependentPtrVec;
}

#endif // __LANDSCAPETYPES_H__
