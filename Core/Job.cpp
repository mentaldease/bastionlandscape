#include "stdafx.h"
#include "../Core/Core.h"
#include "../Core/Job.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	unsigned __stdcall JobWorkerThread(VoidPtr _pData)
	{
		JobThreadPtr pJobThread = (JobThreadPtr)_pData;
		unsigned uExitCode = (NULL != pJobThread) ? 0 : 1;

		if (0 == uExitCode)
		{
			pJobThread->Update();
		}

		_endthreadex(uExitCode);

		return 0;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Job::Job(JobManagerRef _rJobManager)
	:	CoreObject(),
		m_rJobManager(_rJobManager)
	{

	}

	Job::~Job()
	{

	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	JobThread::JobThread(JobManagerRef _rJobManager)
	:	CoreObject(),
		m_rJobManager(_rJobManager),
		m_hThread(NULL),
		m_hBeginEvent(NULL),
		m_hStopEvent(NULL),
		m_hEndEvent(NULL)
	{

	}

	JobThread::~JobThread()
	{

	}

	bool JobThread::Create(const boost::any& _rConfig)
	{
		m_hBeginEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hEndEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		bool bResult = (NULL != m_hBeginEvent)
			&& (NULL != m_hStopEvent)
			&& (NULL != m_hEndEvent);

		if (false != bResult)
		{
			m_hThread = (HANDLE)_beginthreadex(NULL, 0, &JobWorkerThread, this, 0, NULL);
			bResult = (NULL != m_hThread);
		}

		return bResult;
	}

	void JobThread::Update()
	{
		SetEvent(m_hBeginEvent);

		while (WAIT_OBJECT_0 != WaitForSingleObject(m_hStopEvent, 16))
		{
			JobPtr pJob = m_rJobManager.PopJob();
			if (NULL != pJob)
			{
				pJob->Update();
			}
		}

		SetEvent(m_hEndEvent);
	}

	void JobThread::Release()
	{
		const bool bSuccessfulCreate = (NULL != m_hBeginEvent)
			&& (NULL != m_hStopEvent)
			&& (NULL != m_hEndEvent)
			&& (NULL != m_hThread);

		if (false != bSuccessfulCreate)
		{
			SetEvent(m_hStopEvent);
			while (WAIT_OBJECT_0 != WaitForSingleObject(m_hEndEvent, 16))
			{
			}
		}

		if (NULL != m_hThread)
		{
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
		if (NULL != m_hEndEvent)
		{
			CloseHandle(m_hEndEvent);
			m_hEndEvent = NULL;
		}
		if (NULL != m_hStopEvent)
		{
			CloseHandle(m_hStopEvent);
			m_hStopEvent = NULL;
		}
		if (NULL != m_hBeginEvent)
		{
			CloseHandle(m_hBeginEvent);
			m_hBeginEvent = NULL;
		}
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	JobManager::JobManager()
	:	CoreObject(),
		m_vThreads(),
		m_cJobs(100)
	{

	}

	JobManager::~JobManager()
	{

	}

	bool JobManager::Create(const boost::any& _rConfig)
	{
		CreateInfoPtr pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
		bool bResult = (NULL != pInfo);

		if (false != bResult)
		{
			for (UInt i = 0 ; pInfo->m_uPoolSize > i ; ++i)
			{
				JobThreadPtr pJobThread = new JobThread(*this);
				bResult = pJobThread->Create(boost::any(0));
				if (false == bResult)
				{
					pJobThread->Release();
					delete pJobThread;
					break;
				}
				m_vThreads.push_back(pJobThread);
			}
		}

		return bResult;
	}

	void JobManager::Release()
	{
		if (false == m_vThreads.empty())
		{
			JobThreadPtrVec::iterator iJobThread = m_vThreads.begin();
			JobThreadPtrVec::iterator iEnd = m_vThreads.end();
			while (iEnd != iJobThread)
			{
				JobThreadPtr pJobThread = *iJobThread;
				pJobThread->Release();
				delete pJobThread;
				++iJobThread;
			}
			m_vThreads.clear();
		}
	}

	void JobManager::PushJob(JobPtr _pJob)
	{
		// push back the job in the lock-less queue
		m_cJobs.Push(_pJob);
	}

	JobPtr JobManager::PopJob()
	{
		JobPtr pJob = NULL;
		if (false != m_cJobs.Pop(pJob))
		{
			return pJob;
		}
		return NULL;
	}
}
