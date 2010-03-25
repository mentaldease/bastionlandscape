#include "stdafx.h"
#include "../Display/NormalProcess.h"
#include "../Display/Display.h"
#include "../Display/Effect.h"
#include "../Display/Texture.h"
#include "../Display/RenderTarget.h"
#include "../Display/camera.h"
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
		m_uViewportNameKey(0),
		m_vRTTypes(),
		m_vRTNames(),
		m_vRTIndexes(),
		m_mTextures(),
		m_bClear(true)
	{

	}

	DisplayNormalProcess::~DisplayNormalProcess()
	{

	}

	bool DisplayNormalProcess::Create(const boost::any& _rConfig)
	{
		CreateInfoPtr pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
		bool bResult = (NULL != pInfo) && (NULL != pInfo->m_pLuaObject);

		if (false != bResult)
		{
			bResult = CreateFromLuaConfig(*pInfo);
		}
		if (false != bResult)
		{
			m_pRTChain = m_rDisplay.GetRenderTargetChain();
		}

		return bResult;
	}

	void DisplayNormalProcess::Update()
	{
	}

	void DisplayNormalProcess::Release()
	{
		m_uNameKey = 0;
		m_vRTTypes.clear();
		m_vRTNames.clear();
		m_mTextures.clear();
	}

	void DisplayNormalProcess::RenderBegin()
	{
		// prepare render targets
		m_pRTChain->DisableAllRenderTargets();
		const UInt uCount = UInt(m_vRTNames.size());
		for (UInt i = 0 ; uCount > i ; ++i)
		{
			const Key uRTTypeKey = m_vRTTypes[i];
			if (s_uTypeTex2DKey == uRTTypeKey)
			{
				const Key uRTIndex = m_vRTIndexes[i];
				const Key uRTName = m_vRTNames[i];
				DisplayRenderTargetPtr pRT = m_pRTChain->GetRenderTarget(UInt(uRTIndex));
				pRT->SetEnabled(true);
				pRT->SetIndex(i);
				pRT->SetRTOverride(m_rDisplay.GetTextureManager()->Get(uRTName));
			}
			else if (s_uTypeGBufferKey == uRTTypeKey)
			{
				const Key uRTIndex = m_vRTIndexes[i];
				DisplayRenderTargetPtr pRT = m_pRTChain->GetRenderTarget(UInt(uRTIndex));
				pRT->SetEnabled(true);
				pRT->SetIndex(i);
				pRT->SetRTOverride(NULL);
			}
		}

		// set view port
		m_rDisplay.GetCurrentCamera()->SetViewport(m_uViewportNameKey);

		// start scene render
		m_pRTChain->SetImmediateWrite(0 == uCount);
		m_pRTChain->RenderBegin(DisplayRenderTarget::ERenderMode_NORMALPROCESS);
		if (false != ClearRequired())
		{
			const UInt uBlack = D3DCOLOR_XRGB(0, 0, 0);
			const UInt uBlue = D3DCOLOR_XRGB(16, 32, 64);
			const UInt uClearColor = uBlack;

			m_pRTChain->RenderBeginPass(0);
			m_rDisplay.GetDevicePtr()->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, uClearColor, 1.0f, 0L);
			m_pRTChain->RenderEndPass();
		}
	}

	void DisplayNormalProcess::RenderEnd()
	{
		m_pRTChain->RenderEnd();
	}

	bool DisplayNormalProcess::RenderBeginRecord()
	{
		m_rDisplay.GetCurrentCamera()->SetViewport(m_uViewportNameKey);
		return true;
	}

	bool DisplayNormalProcess::RenderEndRecord()
	{
		return true;
	}

	Key DisplayNormalProcess::GetNameKey()
	{
		return m_uNameKey;
	}

	bool DisplayNormalProcess::ClearRequired()
	{
		return m_bClear;
	}

	bool DisplayNormalProcess::CreateFromLuaConfig(CreateInfoRef _rInfo)
	{
		LuaObjectRef rLuaObject = *_rInfo.m_pLuaObject;
		LuaObject oRenderTargets = rLuaObject["render_targets"];
		const string strName = rLuaObject["name"].GetString();
		const UInt uRTCount = UInt(oRenderTargets.GetCount());
		bool bResult = (false == strName.empty());// && (0 < uRTCount);

		if (false != bResult)
		{
			m_uNameKey = MakeKey(strName);

			const string strViewportName = rLuaObject["viewport"].GetString();
			m_uViewportNameKey = MakeKey(strViewportName);

			if (0 < uRTCount)
			{
				m_vRTTypes.resize(uRTCount);
				m_vRTNames.resize(uRTCount);
				m_vRTIndexes.resize(uRTCount);

				for (UInt i = 0 ; uRTCount > i ; ++i)
				{
					LuaObject oRenderTarget = oRenderTargets[i + 1];
					const string strRTType = oRenderTarget["type"].GetString();
					const Key uRTTypeKey = MakeKey(strRTType);
					if (s_uTypeTex2DKey == uRTTypeKey)
					{
						const UInt uRTIndex = UInt(oRenderTarget["index"].GetInteger());
						const string strRTName = oRenderTarget["name"].GetString();
						const Key uRTNameKey = MakeKey(strRTName);
						DisplayTexturePtr pTexture = m_rDisplay.GetTextureManager()->Get(uRTNameKey);
						m_mTextures[uRTNameKey] = pTexture;
						bResult = (NULL != pTexture);
						m_vRTTypes[i] = uRTTypeKey;
						m_vRTNames[i] = uRTNameKey;
						m_vRTIndexes[i] = uRTIndex;
					}
					else if (s_uTypeGBufferKey == uRTTypeKey)
					{
						const UInt uRTIndex = UInt(oRenderTarget["index"].GetInteger());
						m_vRTTypes[i] = uRTTypeKey;
						m_vRTNames[i] = uRTIndex;
						m_vRTIndexes[i] = uRTIndex;
					}
					if (false == bResult)
					{
						break;
					}
				}
			}

			Scripting::Lua::Get(rLuaObject, "clear", m_bClear, m_bClear);
		}

		return bResult;
	}
}
