#ifndef __OCTREETRAVERSEFUNCFRUSTUM_H__
#define __OCTREETRAVERSEFUNCFRUSTUM_H__

#include "../Display/Display.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class OctreeTraverseFuncFrustum : public CoreObject
	{
	public:
		OctreeTraverseFuncFrustum();
		virtual ~OctreeTraverseFuncFrustum();

		EOctreeTraverseResult Do(OctreeNodeRef _rNode);

	protected:
	};
}

#endif // __OCTREETRAVERSEFUNCFRUSTUM_H__
