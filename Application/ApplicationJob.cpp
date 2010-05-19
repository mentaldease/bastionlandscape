#include "stdafx.h"
#include "../Application/ApplicationJob.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	AppTestJob::AppTestJob(JobManagerRef _rJobManager)
	:	Job(_rJobManager)
	{

	}
	
	AppTestJob::~AppTestJob()
	{

	}

	void AppTestJob::Update()
	{
		UInt uCount = 0;
		while (true)
		{
			++uCount;
		}
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	PathfindJob::PathfindJob(JobManagerRef _rJobManager)
	:	Job(_rJobManager)
	{

	}

	PathfindJob::~PathfindJob()
	{

	}

	bool PathfindJob::Create(const boost::any& _rConfig)
	{
		bool bResult = false;
		return bResult;
	}

	void PathfindJob::Update()
	{
		UInt uCount = 0;
		while (true)
		{
			++uCount;
		}
	}
}
