#ifndef __APPLICATIONJOB_H__
#define __APPLICATIONJOB_H__

#include "../Core/Job.h"
using namespace ElixirEngine;

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class AppTestJob : public Job
	{
	public:
		AppTestJob(JobManagerRef _rJobManager);
		virtual ~AppTestJob();

		virtual void Update();

	protected:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class PathfindJob : public Job
	{
	public:
		PathfindJob(JobManagerRef _rJobManager);
		virtual ~PathfindJob();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();

	protected:
	};
}

#endif // __APPLICATIONJOB_H__
