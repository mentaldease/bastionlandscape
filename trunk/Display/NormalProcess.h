#ifndef __NORMALPROCESS_H__
#define __NORMALPROCESS_H__

#include "../Core/Core.h"
#include "../Core/Config.h"
#include "../Display/DisplayTypes.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayNormalProcess : public CoreObject
	{
	public:
		struct CreateInfo
		{
			ConfigPtr			m_pConfig;
			ConfigShortcutPtr	m_pShortcut;
			LuaObjectPtr		m_pLuaObject;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInfoRef;

	public:
		DisplayNormalProcess(DisplayRef _rDisplay);
		virtual ~DisplayNormalProcess();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		Key GetNameKey();

	protected:
		bool CreateFromLibConfig(CreateInfoRef _rInfo);
		bool CreateFromLuaConfig(CreateInfoRef _rInfo);

	protected:
		static Key s_uTypeTex2DKey;
		static Key s_uTypeGBufferKey;

	protected:
		DisplayRef				m_rDisplay;
		Key						m_uNameKey;
		KeyVec					m_vRTTypes;
		KeyVec					m_vRTNames;
		DisplayTexturePtrMap	m_mTextures;

	private:
	};
}

#endif // __NORMALPROCESS_H__
