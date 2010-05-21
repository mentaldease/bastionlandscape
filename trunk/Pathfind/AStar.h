#ifndef __ASTAR_H__
#define __ASTAR_H__

#include "../Core/Core.h"
#include "../Pathfind/AStarTypes.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class AStarNode : public CoreObject
	{
	public:
		 void GetNeightboors(AStarNodePtrVec& _rvNodes);
		 float GetH();
		 float GetG();
		 void SetG(const float _fValue);
		 void SetParent(AStarNodePtr _pParent);
		 float MovementCost(AStarNodePtr _pTarget);
		 float GetLowestPriority();
		 void SetLowestPriority(const float _fValue);

	protected:
		AStarNodePtr	m_pParent;
		float			m_fH;
		float			m_fG;
		float			m_fPriority;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class AStarNodeContainer : public CoreObject
	{
	public:
		AStarNodeContainer();
		virtual ~AStarNodeContainer();

		bool Contains(AStarNodePtr _pNode);
		AStarNodePtr GetLowestPriority();
		void Add(AStarNodePtr _pNode);
		void Remove(AStarNodePtr _pNode);

	protected:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class AStar : public CoreObject
	{
	public:
		AStar();
		virtual ~AStar();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

	protected:
	};
}

#endif // __ASTAR_H__
