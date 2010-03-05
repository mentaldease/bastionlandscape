#include "stdafx.h"
#include "../Display/OctreeTraverseFuncFrustum.h"
#include "../Core/Octree.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	OctreeTraverseFuncFrustum::OctreeTraverseFuncFrustum()
	{

	}

	OctreeTraverseFuncFrustum::~OctreeTraverseFuncFrustum()
	{

	}

	EOctreeTraverseResult OctreeTraverseFuncFrustum::Do(OctreeNodeRef _rNode)
	{
		return EOctreeTraverseResult_NONE;
	}
}
