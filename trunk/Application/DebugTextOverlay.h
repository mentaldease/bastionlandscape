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
			map<Key, string>	m_mFontNameList;
			map<Key, string>	m_mFontMaterialList;
			Vector3				m_f3ScreenOffset;
			Key					m_uRenderStageKey;
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

		bool DrawRequest(const float _fX, const float _fY, const Key& _uFontName, const wstring& _wstrText, const Vector4& _f4Color);

	protected:
		struct DrawInfo
		{
			Vector4				m_f4Color;
			Key					m_uFontName;
			DisplayFontTextPtr	m_pText;
		};
		typedef DrawInfo* DrawInfoPtr;
		typedef DrawInfo& DrawInfoRef;
		typedef vector<DrawInfo> DrawInfoVec;
		typedef vector<DrawInfoPtr> DrawInfoPtrVec;

		typedef map<Key, DisplayFontPtr> FontPtrMap;

	protected:
		DrawInfoVec				m_vDrawInfoPool;
		FontPtrMap				m_mFonts;
		DisplayMaterialPtrMap	m_mMaterials;
		Vector3					m_f3ScreenOffset;
		Key						m_uRenderStageKey;
		UInt					m_uMaxText;
		UInt					m_uDrawCount;
	};
}

#endif // __DEBUGTEXTOVERLAY_H__
