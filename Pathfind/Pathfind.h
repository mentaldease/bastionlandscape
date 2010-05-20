#ifndef __PATHFIND_H__
#define __PATHFIND_H__

#include "../Core/Core.h"
#include "../Pathfind/PathfindTypes.h"

namespace ElixirEngine
{
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

#endif // __PATHFIND_H__
