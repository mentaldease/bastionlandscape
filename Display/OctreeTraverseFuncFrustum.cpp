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
#if 0
		DisplayCamera::ECollision eCollision = Display::GetInstance()->GetCurrentCamera()->CollisionWithAABB(_rNode.GetAABB());
#else
		const fsVector3 fs3Center = _rNode.GetCenter();
		const Vector3 f3Center(fs3Center.x(), fs3Center.y(), fs3Center.z());
		DisplayCamera::ECollision eCollision = Display::GetInstance()->GetCurrentCamera()->CollisionWithSphere(f3Center, _rNode.GetRadius());
#endif
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
