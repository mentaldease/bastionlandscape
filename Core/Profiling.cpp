#include "stdafx.h"
#include "../Core/Profiling.h"
#include "../Core/Time.h"
#include "../Core/File.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Profiling::TagInfoMap	Profiling::s_mTags;
	UInt					Profiling::s_uTimerID = 0;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Profiling::TagInfo::TagInfo()
	:	m_uCount(0),
		m_fTotal(0.0f),
		m_fLongest(-FLT_MAX),
		m_fShortest(FLT_MAX)
	{

	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Profiling::Profiling(const string& _strTag)
	:	CoreObject(),
		m_strTag(_strTag),
		m_fStartTime(0.0f)
	{
		Time::GetRoot()->GetElapsedTime(s_uTimerID, m_fStartTime);
	}

	Profiling::~Profiling()
	{
		float fStopTime;
		Time::GetRoot()->GetElapsedTime(s_uTimerID, fStopTime);
		TagInfoRef rTagInfo = s_mTags[m_strTag];
		const float fDelta = fStopTime - m_fStartTime;
		rTagInfo.m_fTotal += fDelta;
		rTagInfo.m_fLongest = (rTagInfo.m_fLongest < fDelta) ? fDelta : rTagInfo.m_fLongest;
		rTagInfo.m_fShortest = (rTagInfo.m_fShortest > fDelta) ? fDelta : rTagInfo.m_fShortest;
		++rTagInfo.m_uCount;
	}

	void Profiling::SetTimer(const UInt _uTimerID)
	{
		s_uTimerID = _uTimerID;
	}

	void Profiling::OutputInfo(FilePtr _pFile)
	{
		const size_t uBufferSize = 4 * 1024;
		char szBuffer[uBufferSize];

		TagInfoMap::iterator iPair = s_mTags.begin();
		TagInfoMap::iterator iEnd = s_mTags.end();
		while (iEnd != iPair)
		{
			TagInfoRef rTagInfo = iPair->second;
			const float fAverage = (0 < rTagInfo.m_uCount) ? rTagInfo.m_fTotal / float(rTagInfo.m_uCount) : 0.0f;
			const int sSize = _snprintf_s(szBuffer, uBufferSize, uBufferSize, "%s : %u calls total=%f min=%f max=%f avg=%f\n", iPair->first.c_str(), rTagInfo.m_uCount, rTagInfo.m_fTotal, rTagInfo.m_fShortest, rTagInfo.m_fLongest, fAverage);

			_pFile->Write(szBuffer, sSize);
	
			++iPair;
		}
	}

	void Profiling::Reset()
	{
		TagInfoMap::iterator iPair = s_mTags.begin();
		TagInfoMap::iterator iEnd = s_mTags.end();
		while (iEnd != iPair)
		{
			TagInfoRef rTagInfo = iPair->second;
			rTagInfo.m_fTotal = 0.0f;
			rTagInfo.m_uCount = 0;
			++iPair;
		}
	}

	void Profiling::Clear()
	{
		s_mTags.clear();
	}
}