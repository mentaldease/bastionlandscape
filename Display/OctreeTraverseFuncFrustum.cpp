#include "stdafx.h"
#include "../Display/OctreeTraverseFuncFrustum.h"
#include "../Display/Camera.h"
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
		DisplayCamera::ECollision eCollision = Display::GetInstance()->GetCurrentCamera()->CollisionWithAABB(_rNode.GetAABB());
		if (DisplayCamera::ECollision_IN == eCollision)
		{
			return EOctreeTraverseResult_FULL;
		}
		else if (DisplayCamera::ECollision_INTERSECT == eCollision)
		{
			return EOctreeTraverseResult_PARTIAL;
		}
		return EOctreeTraverseResult_NONE;
	}
}
