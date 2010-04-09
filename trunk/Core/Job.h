#ifndef __JOB_H__
#define __JOB_H__

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

	class JobManager : public CoreObject
	{
	public:
		struct CreateInfo
		{
			UInt	m_uPoolSize;
		};

	public:
		JobManager();
		virtual ~JobManager();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Release();

		void Schedule(JobPtr _pJob);

	protected:
	};
}

#endif // __JOB_H__
