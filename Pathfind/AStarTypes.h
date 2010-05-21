#ifndef __ASTARTYPES_H__
#define __ASTARTYPES_H__

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class AStarNode;
	typedef AStarNode* AStarNodePtr;
	typedef AStarNode& AStarNodeRef;
	typedef vector<AStarNodePtr> AStarNodePtrVec;

	class AStarNodeContainer;
	typedef AStarNodeContainer* AStarNodeContainerPtr;
	typedef AStarNodeContainer& AStarNodeContainerRef;

	class AStar;
	typedef AStar* AStarPtr;
	typedef AStar& AStarRef;
}

#endif // __ASTARTYPES_H__
