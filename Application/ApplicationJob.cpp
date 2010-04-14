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
}
