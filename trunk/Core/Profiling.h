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
	#define PROFILING(TAG) ElixirEngine::Profiling oProfiling(TAG)
	#endif

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class Profiling : public CoreObject
	{
	public:
		Profiling(const string& _strTag);
		virtual ~Profiling();

		static void SetTimer(const UInt _uTimerID);
		static void OutputInfo(FilePtr _pFile);
		static void Reset();
		static void Clear();

	protected:
		struct TagInfo
		{
			TagInfo();

			UInt	m_uCount;
			float	m_fTotal;
			float	m_fLongest;
			float	m_fShortest;
		};
		typedef TagInfo* TagInfoPtr;
		typedef TagInfo& TagInfoRef;
		typedef map<string, TagInfo> TagInfoMap;

		static TagInfoMap	s_mTags;
		static UInt			s_uTimerID;

	protected:
		string	m_strTag;
		float	m_fStartTime;
	};
}

#endif // __PROFILING_H__
