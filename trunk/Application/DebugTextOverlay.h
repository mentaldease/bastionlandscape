#ifndef __DEBUGTEXTOVERLAY_H__
#define __DEBUGTEXTOVERLAY_H__

#include "../Application/ApplicationIncludes.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DebugTextOverlay : public CoreObject
	{
	public:
		struct CreateInfo
		{
			map<Key, string>	m_mFontList;
			Vector3				m_fScreenOffset;
			Key					m_uRenderPassKey;
			UInt				m_uMaxText;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInfoRef;

	public:
		DebugTextOverlay();
		virtual ~DebugTextOverlay();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		bool Draw(const float _fX, const float _fY, const Key& _uFontName, const wstring& _wstrText, const Vector4& _oColor);

	protected:
		struct DrawInfo
		{
			float	m_fX;
			float	m_fY;
			wstring	m_wstrText;
			Vector4	m_oColor;
			Key		m_uFontName;
		};
		typedef DrawInfo* DrawInfoPtr;
		typedef DrawInfo& DrawInfoRef;
		typedef vector<DrawInfo> DrawInfoVec;
		typedef vector<DrawInfoPtr> DrawInfoPtrVec;

	protected:
		DrawInfoVec		m_vDrawInfoPool;
		DrawInfoPtrVec	m_vpDrawInfo;
		Vector3			m_fScreenOffset;
		Key				m_uRenderPassKey;
		UInt			m_uMaxText;
	}
}

#endif // __DEBUGTEXTOVERLAY_H__
