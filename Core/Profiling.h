#ifndef __PROFILING_H__
#define __PROFILING_H__

#include "../Core/Core.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	#if _FULL_SPEED
	#define PROFILING(TAG)
	#else
	#define PROFILING(TAG) \
		static const Key uTagKey = MakeKey(string(TAG)); \
		ElixirEngine::Profiling oProfiling(uTagKey, TAG);
	#endif

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Profiling : public CoreObject
	{
	public:
		Profiling(const Key _uTagKey, const string& _strName);
		virtual ~Profiling();

		static void SetTimer(const UInt _uTimerID);
		static void OutputInfo(FilePtr _pFile);
		static void Reset();
		static void Clear();

	protected:
		struct TagInfo
		{
			TagInfo();

			string	m_strName;
			UInt	m_uCount;
			float	m_fTotal;
			float	m_fLongest;
			float	m_fShortest;
		};
		typedef TagInfo* TagInfoPtr;
		typedef TagInfo& TagInfoRef;
		typedef map<Key, TagInfo> TagInfoMap;

		struct CallInfo;
		typedef CallInfo* CallInfoPtr;
		typedef CallInfo& CallInfoRef;
		typedef vector<CallInfo> CallInfoVec;
		typedef map<Key, CallInfo> CallInfoMap;
		typedef CallInfoMap& CallInfoMapRef;
		struct CallInfo
		{
			CallInfo();

			CallInfoMap	m_mChildrenCalls;
			UInt		m_uCount;
			float		m_fTotal;
			float		m_fLongest;
			float		m_fShortest;
		};

		static TagInfoMap	s_mTags;
		static CallInfoMap	s_mCallStacks;
		static KeyVec		s_vCallStack;
		static CallInfoPtr	s_pCurrentCallStack;
		static UInt			s_uTimerID;
		static const size_t	s_uBufferSize = 4 * 1024;
		static char			s_szBuffer[s_uBufferSize];
		static float		s_fStartTime;
		static float		s_fStopTime;

	protected:
		static void OutputInfoCallStack(CallInfoMapRef _rmCallStack, UInt _uDepth, FilePtr _pFile);
		static void ResetCallStack(CallInfoMapRef _rmCallStack);

	protected:
		string		m_strTag;
		Key			m_uTagKey;
		float		m_fStartTime;
		CallInfoPtr	m_pPreviousCallStack;
		CallInfoPtr	m_pCallStack;
	};
}

#endif // __PROFILING_H__
