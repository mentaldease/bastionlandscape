#ifndef __POSTPROCESS_H__
#define __POSTPROCESS_H__

#include "../Core/Core.h"
#include "../Display/DisplayTypes.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayPostProcess : public CoreObject
	{
	public:
		struct CreateInfo
		{
			LuaObjectPtr	m_pLuaObject;
		};
		typedef CreateInfo* CreateInfoPtr;
		typedef CreateInfo& CreateInfoRef;

	public:
		DisplayPostProcess(DisplayRef _rDisplay);
		virtual ~DisplayPostProcess();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		void RenderBegin();
		void RenderEnd();

	protected:
		bool CreateFromLuaConfig(CreateInfoRef _rInfo);

	protected:
		static Key s_uTypeTex2DKey;
		static Key s_uTypeGBufferKey;

	protected:
		string						m_strName;
		DisplayRef					m_rDisplay;
		DisplayRenderTargetChainPtr	m_pRTChain;
		DisplayMaterialPtr			m_pMaterial;
		DisplayObjectPtr			m_pDisplayObject;
		Key							m_uMaterialNameKey;
		KeyVec						m_vRTTypes;
		KeyVec						m_vRTNames;
		KeyVec						m_vRTIndexes;
		DisplayTexturePtrMap		m_mTextures;
		bool						m_bImmediateWrite;

	private:
	};
}

#endif // __POSTPROCESS_H__
