#ifndef __NORMALPROCESS_H__
#define __NORMALPROCESS_H__

#include "../Core/Core.h"
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
			LuaObjectPtr	m_pLuaObject;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInfoRef;

	public:
		DisplayNormalProcess(DisplayRef _rDisplay);
		virtual ~DisplayNormalProcess();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		void RenderBegin();
		void RenderEnd();
		Key GetNameKey();
		bool ClearRequired();

	protected:
		bool CreateFromLuaConfig(CreateInfoRef _rInfo);
		void RenderBegin(const bool _bSetViewport);

	protected:
		static Key s_uTypeTex2DKey;
		static Key s_uTypeGBufferKey;

	protected:
		DisplayRef					m_rDisplay;
		DisplayRenderTargetChainPtr	m_pRTChain;
		Key							m_uNameKey;
		Key							m_uViewportNameKey;
		KeyVec						m_vRTTypes;
		KeyVec						m_vRTNames;
		KeyVec						m_vRTIndexes;
		DisplayTexturePtrMap		m_mTextures;
		bool						m_bClear;

	private:
	};
}

#endif // __NORMALPROCESS_H__
