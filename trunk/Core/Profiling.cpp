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
	Profiling::CallInfoMap	Profiling::s_mCallStacks;
	KeyVec					Profiling::s_vCallStack;
	Profiling::CallInfoPtr	Profiling::s_pCurrentCallStack = NULL;
	UInt					Profiling::s_uTimerID = 0;
	char					Profiling::s_szBuffer[s_uBufferSize];
	float					Profiling::s_fStartTime = 0.0f;
	float					Profiling::s_fStopTime = 0.0f;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Profiling::TagInfo::TagInfo()
	:	m_strName(),
		m_uCount(0),
		m_fTotal(0.0f),
		m_fLongest(-FLT_MAX),
		m_fShortest(FLT_MAX)
	{

	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Profiling::CallInfo::CallInfo()
	:	m_mChildrenCalls(),
		m_uCount(0),
		m_fTotal(0.0f),
		m_fLongest(-FLT_MAX),
		m_fShortest(FLT_MAX)
	{

	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Profiling::Profiling(const Key _uTagKey, const string& _strName)
	:	CoreObject(),
		m_strTag(_strName),
		m_uTagKey(_uTagKey),
		m_fStartTime(0.0f),
		m_pPreviousCallStack(s_pCurrentCallStack),
		m_pCallStack(NULL)
	{
		Time::GetRoot()->GetElapsedTime(s_uTimerID, m_fStartTime);
		s_fStartTime = ((0.0f == s_fStartTime) && (NULL == s_pCurrentCallStack)) ? m_fStartTime : s_fStartTime;
		CallInfoRef rCallInfo = (NULL == s_pCurrentCallStack) ? s_mCallStacks[_uTagKey] : s_pCurrentCallStack->m_mChildrenCalls[_uTagKey];
		m_pCallStack = &rCallInfo;
		s_pCurrentCallStack = m_pCallStack;
		s_vCallStack.push_back(_uTagKey);
	}

	Profiling::~Profiling()
	{
		float fStopTime;
		Time::GetRoot()->GetElapsedTime(s_uTimerID, fStopTime);

		s_vCallStack.pop_back();
		s_fStopTime = fStopTime;
		s_pCurrentCallStack = m_pPreviousCallStack;

		TagInfoRef rTagInfo = s_mTags[m_uTagKey];
		const bool bRecursion = (s_vCallStack.end() != find(s_vCallStack.begin(), s_vCallStack.end(), m_uTagKey));
		const float fDelta = fStopTime - m_fStartTime;
		rTagInfo.m_fTotal += (false == bRecursion) ? fDelta : 0.0f;
		rTagInfo.m_fLongest = (rTagInfo.m_fLongest < fDelta) ? fDelta : rTagInfo.m_fLongest;
		rTagInfo.m_fShortest = (rTagInfo.m_fShortest > fDelta) ? fDelta : rTagInfo.m_fShortest;
		rTagInfo.m_strName = m_strTag;
		++rTagInfo.m_uCount;

		m_pCallStack->m_fTotal += fDelta;
		m_pCallStack->m_fLongest = (m_pCallStack->m_fLongest < fDelta) ? fDelta : m_pCallStack->m_fLongest;
		m_pCallStack->m_fShortest = (m_pCallStack->m_fShortest > fDelta) ? fDelta : m_pCallStack->m_fShortest;
		++m_pCallStack->m_uCount;
	}

	void Profiling::SetTimer(const UInt _uTimerID)
	{
		s_uTimerID = _uTimerID;
	}

	void Profiling::OutputInfo(FilePtr _pFile)
	{
		const float fTotal = s_fStopTime - s_fStartTime;
		if (0.0f < fTotal)
		{
			const int sSize = _snprintf_s(s_szBuffer, s_uBufferSize, s_uBufferSize, "calls\n");
			_pFile->Write(s_szBuffer, sSize);
			TagInfoMap::iterator iPair = s_mTags.begin();
			TagInfoMap::iterator iEnd = s_mTags.end();
			while (iEnd != iPair)
			{
				TagInfoRef rTagInfo = iPair->second;
				const float fAverage = (0 < rTagInfo.m_uCount) ? rTagInfo.m_fTotal / float(rTagInfo.m_uCount) : 0.0f;
				const int sSize = _snprintf_s(s_szBuffer, s_uBufferSize, s_uBufferSize,
					"%s : %u calls total=%f(%5.2f%%) min=%f max=%f avg=%f\n",
					rTagInfo.m_strName.c_str(),
					rTagInfo.m_uCount,
					rTagInfo.m_fTotal,
					(rTagInfo.m_fTotal / fTotal) * 100.0f,
					rTagInfo.m_fShortest,
					rTagInfo.m_fLongest,
					fAverage);

				_pFile->Write(s_szBuffer, sSize);

				++iPair;
			}

			{
				const int sSize = _snprintf_s(s_szBuffer, s_uBufferSize, s_uBufferSize, "callstack\n");
				_pFile->Write(s_szBuffer, sSize);
				OutputInfoCallStack(s_mCallStacks, 0, _pFile);
			}
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
			rTagInfo.m_fLongest = -FLT_MAX;
			rTagInfo.m_fShortest = FLT_MAX;
			++iPair;
		}

		ResetCallStack(s_mCallStacks);

		s_vCallStack.clear();

		s_pCurrentCallStack = NULL;
		s_fStartTime = 0.0f;
		s_fStopTime = 0.0f;
	}

	void Profiling::Clear()
	{
		s_mTags.clear();
		s_mCallStacks.clear();
		Reset();
	}

	void Profiling::OutputInfoCallStack(CallInfoMapRef _rmCallStack, UInt _uDepth, FilePtr _pFile)
	{
		const float fTotal = s_fStopTime - s_fStartTime;
		if (0.0f < fTotal)
		{
			CallInfoMap::iterator iPair = _rmCallStack.begin();
			CallInfoMap::iterator iEnd = _rmCallStack.end();
			while (iEnd != iPair)
			{
				CallInfoRef rCallInfo = iPair->second;
				TagInfoRef rTagInfo = s_mTags[iPair->first];
				const float fAverage = (0 < rCallInfo.m_uCount) ? rCallInfo.m_fTotal / float(rCallInfo.m_uCount) : 0.0f;

				for (UInt i = 0 ; (_uDepth > i) && (s_uBufferSize > i) ; ++i)
				{
					s_szBuffer[i] = '\t';
				}

				const int sSize = _snprintf_s(s_szBuffer + _uDepth, s_uBufferSize - _uDepth, s_uBufferSize - _uDepth,
					"%s : %u calls total=%f(%5.2f%%) min=%f max=%f avg=%f\n",
					rTagInfo.m_strName.c_str(),
					rCallInfo.m_uCount,
					rCallInfo.m_fTotal,
					(rCallInfo.m_fTotal / fTotal) * 100.0f,
					rCallInfo.m_fShortest,
					rCallInfo.m_fLongest,
					fAverage);

				_pFile->Write(s_szBuffer, sSize + _uDepth);

				if (false == rCallInfo.m_mChildrenCalls.empty())
				{
					OutputInfoCallStack(rCallInfo.m_mChildrenCalls, _uDepth + 1, _pFile);
				}

				++iPair;
			}
		}
	}

	void Profiling::ResetCallStack(CallInfoMapRef _rmCallStack)
	{
		CallInfoMap::iterator iPair = _rmCallStack.begin();
		CallInfoMap::iterator iEnd = _rmCallStack.end();
		while (iEnd != iPair)
		{
			CallInfoRef rCallInfo = iPair->second;
			rCallInfo.m_fTotal = 0.0f;
			rCallInfo.m_uCount = 0;
			rCallInfo.m_fLongest = -FLT_MAX;
			rCallInfo.m_fShortest = FLT_MAX;
			if (false != rCallInfo.m_mChildrenCalls.empty())
			{
				ResetCallStack(rCallInfo.m_mChildrenCalls);
			}
			++iPair;
		}
	}
}