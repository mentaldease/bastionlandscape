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

	struct VertexDefault;
	typedef VertexDefault*	VertexDefaultPtr;
	typedef VertexDefault&	VertexDefaultRef;

	struct VertexLiquid;
	typedef VertexLiquid*	VertexLiquidPtr;
	typedef VertexLiquid&	VertexLiquidRef;

	struct VertexIndependent;
	typedef VertexIndependent*				VertexIndependentPtr;
	typedef VertexIndependent&				VertexIndependentRef;
	typedef vector<VertexIndependentPtr>	VertexIndependentPtrVec;
}

#endif // __LANDSCAPETYPES_H__
