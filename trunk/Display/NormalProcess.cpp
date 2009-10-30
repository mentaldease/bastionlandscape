#include "stdafx.h"
#include "../Display/NormalProcess.h"
#include "../Display/Display.h"
#include "../Display/Effect.h"
#include "../Display/Texture.h"
#include "../Display/RenderTarget.h"
#include "../Core/Scripting.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Key DisplayNormalProcess::s_uTypeTex2DKey = MakeKey(string("tex2d"));
	Key DisplayNormalProcess::s_uTypeGBufferKey = MakeKey(string("gbuffer"));

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayNormalProcess::DisplayNormalProcess(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_uNameKey(0),
		m_vRTTypes(),
		m_vRTNames(),
		m_mTextures()
	{

	}

	DisplayNormalProcess::~DisplayNormalProcess()
	{

	}

	bool DisplayNormalProcess::Create(const boost::any& _rConfig)
	{
		CreateInfoPtr pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
		bool bResult = false;

		if (NULL != pInfo->m_pLuaObject)
		{
			bResult = CreateFromLuaConfig(*pInfo);
		}

		return bResult;
	}

	void DisplayNormalProcess::Update()
	{
		DisplayRenderTargetChainPtr pRTChain = m_rDisplay.GetRenderTargetChain();
		pRTChain->DisableAllRenderTargets();
		const UInt uCount = m_vRTNames.size();
		for (UInt i = 0 ; uCount > i ; ++i)
		{
			const Key uRTTypeKey = m_vRTTypes[i];
			if (s_uTypeTex2DKey == uRTTypeKey)
			{
			}
			else if (s_uTypeGBufferKey == uRTTypeKey)
			{
				const Key uRTIndex = m_vRTNames[i];
				DisplayRenderTargetPtr pRT = pRTChain->GetRenderTarget(UInt(uRTIndex));
				pRT->SetEnabled(true);
				pRT->SetIndex(i);
			}
		}
	}

	void DisplayNormalProcess::Release()
	{
		m_uNameKey = 0;
		m_vRTTypes.clear();
		m_vRTNames.clear();
		m_mTextures.clear();
	}

	Key DisplayNormalProcess::GetNameKey()
	{
		return m_uNameKey;
	}

	bool DisplayNormalProcess::CreateFromLuaConfig(CreateInfoRef _rInfo)
	{
		LuaObjectRef rLuaObject = *_rInfo.m_pLuaObject;
		LuaObject oRenderTargets = rLuaObject["render_targets"];
		const string strName = rLuaObject["name"].GetString();
		const UInt uRTCount = UInt(oRenderTargets.GetCount());
		bool bResult = (false == strName.empty()) && (0 < uRTCount);

		if (false != bResult)
		{
			m_uNameKey = MakeKey(strName);
			m_vRTTypes.resize(uRTCount);
			m_vRTNames.resize(uRTCount);

			for (UInt i = 0 ; uRTCount > i ; ++i)
			{
				LuaObject oRenderTarget = oRenderTargets[i + 1];
				const string strRTType = oRenderTarget["type"].GetString();
				const Key uRTTypeKey = MakeKey(strRTType);
				if (s_uTypeTex2DKey == uRTTypeKey)
				{
					const string strRTName = oRenderTarget["name"].GetString();
					const Key uRTNameKey = MakeKey(strRTName);
					DisplayTexturePtr pTexture = m_rDisplay.GetTextureManager()->Get(uRTNameKey);
					m_mTextures[uRTNameKey] = pTexture;
					bResult = (NULL != pTexture);
					m_vRTTypes[i] = uRTTypeKey;
					m_vRTNames[i] = uRTNameKey;
				}
				else if (s_uTypeGBufferKey == uRTTypeKey)
				{
					UInt uRTIndex = UInt(oRenderTarget["index"].GetInteger());
					m_vRTTypes[i] = uRTTypeKey;
					m_vRTNames[i] = uRTIndex;
				}
				if (false == bResult)
				{
					break;
				}
			}
		}

		return bResult;
	}
}
