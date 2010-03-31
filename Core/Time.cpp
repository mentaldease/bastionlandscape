#include "stdafx.h"
#include "../Core/Time.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	TimePtr	Time::s_pTime = NULL;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Time::Timer::Timer(LARGE_INTEGER& _rTicksPerSeconds)
	:	m_rTicksPerSeconds(_rTicksPerSeconds)
	{
		Reset();
	}

	bool Time::Timer::Reset()
	{
		m_bIsActive = true;
		m_bIsPaused = false;

		m_lStart.HighPart = 0;
		m_lStart.LowPart = 0;
		m_lStart.QuadPart = 0;
		m_lLastPause.HighPart = 0;
		m_lLastPause.LowPart = 0;
		m_lLastPause.QuadPart = 0;
		m_lCurrentCounter.HighPart = 0;
		m_lCurrentCounter.LowPart = 0;
		m_lCurrentCounter.QuadPart = 0;
		m_lTotalPause.HighPart = 0;
		m_lTotalPause.LowPart = 0;
		m_lTotalPause.QuadPart = 0;

		const bool bResult = (FALSE != QueryPerformanceCounter(&m_lStart));
		return bResult;
	}

	bool Time::Timer::Pause()
	{
		bool bResult = true;
		if (false == m_bIsPaused)
		{
			m_bIsPaused = true;
			bResult = (FALSE != QueryPerformanceCounter(&m_lLastPause));
		}
		return bResult;
	}

	bool Time::Timer::Resume()
	{
		bool bResult = true;
		if (false != m_bIsPaused)
		{
			m_bIsPaused = false;
			bResult = (FALSE != QueryPerformanceCounter(&m_lCurrentCounter));
			if (false != bResult)
			{
				m_lTotalPause.QuadPart += (m_lCurrentCounter.QuadPart - m_lLastPause.QuadPart);
			}
		}
		return bResult;
	}

	void Time::Timer::Release()
	{
		m_bIsActive = false;
	}

	bool Time::Timer::GetElapsedTime(float& _fElapsedMilliseconds)
	{
		bool bResult = true;

		if (false == m_bIsPaused)
		{
			bResult = (FALSE != QueryPerformanceCounter(&m_lCurrentCounter));
			if (false != bResult)
			{
				LARGE_INTEGER lTemp;
				lTemp.QuadPart = m_lCurrentCounter.QuadPart - m_lStart.QuadPart - m_lTotalPause.QuadPart;
				_fElapsedMilliseconds = ((float)lTemp.QuadPart / (float)m_rTicksPerSeconds.QuadPart) * 1000.0f;
			}
		}
		else
		{
			LARGE_INTEGER lTemp;
			lTemp.QuadPart = m_lLastPause.QuadPart - m_lStart.QuadPart - m_lTotalPause.QuadPart;
			_fElapsedMilliseconds = ((float)lTemp.QuadPart / (float)m_rTicksPerSeconds.QuadPart) * 1000.0f;
		}

		return bResult;
	}

	Time::TimerRef Time::Timer::operator = (Time::Timer _oTimer)
	{
		if (this != &_oTimer)
		{
			m_rTicksPerSeconds	= _oTimer.m_rTicksPerSeconds;
			m_lStart			= _oTimer.m_lStart;
			m_lLastPause		= _oTimer.m_lLastPause;
			m_lTotalPause		= _oTimer.m_lTotalPause;
			m_lCurrentCounter	= _oTimer.m_lCurrentCounter;
			m_bIsPaused			= _oTimer.m_bIsPaused;
			m_bIsActive			= _oTimer.m_bIsActive;
		}
		return *this;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Time::Time()
	:	CoreObject(),
		m_vTimers()
	{

	}

	Time::~Time()
	{

	}

	void Time::SetRoot(TimePtr _pTime)
	{
		s_pTime = _pTime;
	}

	TimePtr Time::GetRoot()
	{
		return s_pTime;
	}

	bool Time::Create(const boost::any& _rConfig)
	{
		Release();
		const bool bResult = (FALSE != QueryPerformanceFrequency(&m_lTicksPerSeconds));
		return bResult;
	}

	void Time::Update()
	{
	}

	void Time::Release()
	{
		//struct TimerReleaseFunction
		//{
		//	void operator() (TimerRef _rTimer)
		//	{
		//		_rTimer.Release();
		//	}
		//};
		//for_each(m_vTimers.begin(), m_vTimers.end(), TimerReleaseFunction());
		m_vTimers.clear();
	}

	unsigned int Time::CreateTimer(const bool _bStart)
	{
		bool bResult = true;

		TimerVec::iterator iTimer = m_vTimers.begin();
		TimerVec::iterator iEnd = m_vTimers.end();
		TimerPtr pTimer = NULL;
		unsigned int uResult = 0;

		while (iEnd != iTimer)
		{
			if (false == (*iTimer).m_bIsActive)
			{
				pTimer = &(*iTimer);
				break;
			}
			++iTimer;
			++uResult;
		}

		if (NULL == pTimer)
		{
			uResult = m_vTimers.size();
			Timer oTimer(m_lTicksPerSeconds);
			m_vTimers.push_back(oTimer);
			pTimer = &(m_vTimers.back());
		}

		if (false == _bStart)
		{
			float fTemp;
			bResult = ResetTimer(uResult, fTemp) && PauseTimer(uResult);
		}

		if (false == bResult)
		{
			pTimer->Release();
			uResult = 0xffffffff;
		}

		return uResult;
	}

	bool Time::ReleaseTimer(const unsigned int& _uTimerID, float& _fElapsedMilliseconds)
	{
		TimerPtr pTimer = GetTimer(_uTimerID);
		bool bResult = (NULL != pTimer);

		if (false != bResult)
		{
			bResult = pTimer->GetElapsedTime(_fElapsedMilliseconds);
			pTimer->Release();
		}

		return bResult;
	}

	bool Time::ResetTimer(const unsigned int& _uTimerID, float& _fElapsedMilliseconds)
	{
		TimerPtr pTimer = GetTimer(_uTimerID);
		bool bResult = (NULL != pTimer);

		if (false != bResult)
		{
			bResult = pTimer->GetElapsedTime(_fElapsedMilliseconds) && pTimer->Reset();
		}

		return bResult;
	}

	bool Time::PauseTimer(const unsigned int& _uTimerID)
	{
		TimerPtr pTimer = GetTimer(_uTimerID);
		bool bResult = (NULL != pTimer);

		if (false != bResult)
		{
			bResult = pTimer->Pause();
		}

		return bResult;
	}

	bool Time::ResumeTimer(const unsigned int& _uTimerID)
	{
		TimerPtr pTimer = GetTimer(_uTimerID);
		bool bResult = (NULL != pTimer);

		if (false != bResult)
		{
			bResult = pTimer->Resume();
		}

		return bResult;
	}

	bool Time::GetElapsedTime(const unsigned int& _uTimerID, float& _fElapsedMilliseconds)
	{
		TimerPtr pTimer = GetTimer(_uTimerID);
		bool bResult = (NULL != pTimer) && (false != pTimer->GetElapsedTime(_fElapsedMilliseconds));
		return bResult;
	}

	Time::TimerPtr Time::GetTimer(const unsigned int& _uTimerID)
	{
		TimerPtr pResult = (_uTimerID < m_vTimers.size()) ? &(m_vTimers[_uTimerID]) : NULL;
		return pResult;
	}
}
