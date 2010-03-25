#include "stdafx.h"
#include "../Display/PostProcess.h"
#include "../Display/Display.h"
#include "../Display/Effect.h"
#include "../Display/RenderTarget.h"
#include "../Core/Scripting.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Key DisplayPostProcess::s_uTypeTex2DKey = MakeKey(string("tex2d"));
	Key DisplayPostProcess::s_uTypeGBufferKey = MakeKey(string("gbuffer"));

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayPostProcess::DisplayPostProcess(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_pRTChain(NULL),
		m_pMaterial(NULL),
		m_pDisplayObject(NULL),
		m_bImmediateWrite(false)
	{

	}

	DisplayPostProcess::~DisplayPostProcess()
	{

	}

	bool DisplayPostProcess::Create(const boost::any& _rConfig)
	{
		CreateInfoPtr pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
		bool bResult = (NULL != pInfo) && (NULL != pInfo->m_pLuaObject);

		if (false != bResult)
		{
			bResult = CreateFromLuaConfig(*pInfo);
		}

		if (false != bResult)
		{
			m_pMaterial = m_rDisplay.GetMaterialManager()->GetMaterial(m_uMaterialNameKey);
			bResult = (NULL != m_pMaterial);
		}

		if (false != bResult)
		{
			m_pDisplayObject = m_rDisplay.GetPostProcessGeometry();
			m_pRTChain = m_rDisplay.GetRenderTargetChain();
		}

		return bResult;
	}

	void DisplayPostProcess::Update()
	{
		RenderObjectFunction oROF(m_pMaterial);
		oROF(m_pDisplayObject);
	}

	void DisplayPostProcess::Release()
	{
		if (NULL != m_pMaterial)
		{
			m_rDisplay.GetMaterialManager()->UnloadMaterial(m_uMaterialNameKey);
			m_pMaterial = NULL;
		}
	}

	void DisplayPostProcess::RenderBegin()
	{
		m_pRTChain->SetImmediateWrite(m_bImmediateWrite);
		if (false == m_bImmediateWrite)
		{
			// prepare render targets
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
		}
	}

	void DisplayPostProcess::RenderEnd()
	{

	}

	bool DisplayPostProcess::RenderBeginRecord()
	{
		return true;
	}

	bool DisplayPostProcess::RenderEndRecord()
	{
		return true;
	}

	bool DisplayPostProcess::CreateFromLuaConfig(CreateInfoRef _rInfo)
	{
		LuaObjectRef rLuaObject = *_rInfo.m_pLuaObject;
		LuaObject oRenderTargets = rLuaObject["render_targets"];
		const UInt uRTCount = UInt(oRenderTargets.GetCount());
		string strMaterialName;
		Scripting::Lua::Get(rLuaObject, "name", m_strName, m_strName);
		Scripting::Lua::Get(rLuaObject, "immediate_write", m_bImmediateWrite, m_bImmediateWrite);
		Scripting::Lua::Get(rLuaObject, "material", strMaterialName, strMaterialName);
		m_uMaterialNameKey = MakeKey(strMaterialName);

		bool bResult = true;

		if (false != bResult)
		{
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
		}

		return bResult;
	}
}
