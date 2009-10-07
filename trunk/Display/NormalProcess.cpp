#include "stdafx.h"
#include "../Display/NormalProcess.h"
#include "../Display/Display.h"
#include "../Display/Effect.h"
#include "../Display/Texture.h"
#include "../Display/RenderTarget.h"

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
		string strName = "";
		UInt uRTCount = 0;
		bool bResult = false;

		bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "name", strName)
			&& pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, "render_target_count", uRTCount)
			&& (false == strName.empty())
			&& (0 < uRTCount);

		if (false != bResult)
		{
			m_uNameKey = MakeKey(strName);
			m_vRTTypes.resize(uRTCount);
			m_vRTNames.resize(uRTCount);

			for (UInt i = 0 ; uRTCount > i ; ++i)
			{
				string strRTType = boost::str(boost::format("render_target_type_%1%") % i);
				bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, strRTType, strRTType);

				if (false == bResult)
				{
					break;
				}
				bResult = false;
				const Key uRTTypeKey = MakeKey(strRTType);
				if (s_uTypeTex2DKey == uRTTypeKey)
				{
					string strRTName = boost::str(boost::format("render_target_name_%1%") % i);
					bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, strRTName, strRTName);
					if (false != bResult)
					{
						const Key uRTNameKey = MakeKey(strRTName);
						DisplayTexturePtr pTexture = m_rDisplay.GetTextureManager()->Get(uRTNameKey);
						m_mTextures[uRTNameKey] = pTexture;
						bResult = (NULL != pTexture);
						m_vRTTypes[i] = uRTTypeKey;
						m_vRTNames[i] = uRTNameKey;
					}
				}
				else if (s_uTypeGBufferKey == uRTTypeKey)
				{
					UInt uRTIndex;
					const string strRTName = boost::str(boost::format("render_target_index_%1%") % i);
					bResult = pInfo->m_pConfig->GetValue(pInfo->m_pShortcut, strRTName, uRTIndex);
					if (false != bResult)
					{
						m_vRTTypes[i] = uRTTypeKey;
						m_vRTNames[i] = uRTIndex;
					}
				}
				if (false == bResult)
				{
					break;
				}
			}
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
}
