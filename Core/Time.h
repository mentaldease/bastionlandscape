#ifndef __TIME_H__
#define __TIME_H__

#include "../Core/Core.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Time : public CoreObject
	{
	public:
		Time();
		virtual ~Time();

		static void SetRoot(TimePtr _pTime);
		static TimePtr GetRoot();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		UInt CreateTimer(const bool _bStart);
		bool ReleaseTimer(const UInt& _uTimerID, float& _fElapsedMilliseconds);
		bool ResetTimer(const UInt& _uTimerID, float& _fElapsedMilliseconds);
		bool PauseTimer(const UInt& _uTimerID);
		bool ResumeTimer(const UInt& _uTimerID);
		bool GetElapsedTime(const UInt& _uTimerID, float& _fElapsedMilliseconds);

	protected:
		struct Timer;
		typedef Timer* TimerPtr;
		typedef Timer& TimerRef;
		typedef vector<Timer> TimerVec;
		typedef vector<TimerPtr> TimerPtrVec;

		struct Timer
		{
			Timer(LARGE_INTEGER& _rTicksPerSeconds);

			bool Reset();
			bool Pause();
			bool Resume();
			void Release();
			bool GetElapsedTime(float& _fElapsedMilliseconds);

			TimerRef operator = (Timer _oTimer);

			LARGE_INTEGER&	m_rTicksPerSeconds;
			LARGE_INTEGER	m_lStart;
			LARGE_INTEGER	m_lLastPause;
			LARGE_INTEGER	m_lTotalPause;
			LARGE_INTEGER	m_lCurrentCounter;
			bool			m_bIsPaused;
			bool			m_bIsActive;
		};

		TimerPtr GetTimer(const UInt& _uTimerID);

	protected:
		static TimePtr	s_pTime;

		TimerPtrVec		m_vpTimers;
		LARGE_INTEGER	m_lTicksPerSeconds;
	};
}

#endif // __TIME_H__
