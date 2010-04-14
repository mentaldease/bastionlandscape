#ifndef __JOB_H__
#define __JOB_H__

#include <process.h>
#include "../Core/Core.h"
#include "../Core/AtomicQueue.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Job : public CoreObject
	{
	public:
		Job(JobManagerRef _rJobManager);
		virtual ~Job();

	protected:
		JobManagerRef	m_rJobManager;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class JobThread : public CoreObject
	{
	public:
		JobThread(JobManagerRef _rJobManager);
		~JobThread();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

	protected:
		JobManagerRef	m_rJobManager;
		HANDLE			m_hThread;
		HANDLE			m_hBeginEvent;
		HANDLE			m_hStopEvent;
		HANDLE			m_hEndEvent;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class JobManager : public CoreObject
	{
	public:
		struct CreateInfo
		{
			UInt	m_uPoolSize;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInfoRef;

	public:
		JobManager();
		virtual ~JobManager();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Release();

		LONG GetReleaseMarker();
		void PushJob(JobPtr _pJob);
		JobPtr PopJob();

	protected:
		typedef vector<HANDLE> ThreadHandleVec;

	protected:
		JobThreadPtrVec		m_vThreads;
		AtomicQueue<JobPtr>	m_cJobs;
		volatile LONG*		m_pReleaseMarker;
	};
}

#endif // __JOB_H__
