#include "stdafx.h"
#include "../Application/Application.h"
#include "../Application/DebugTextOverlay.h"

namespace BastionGame
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DebugTextOverlay::DebugTextOverlay()
	:	CoreObject(),
		m_vDrawInfoPool(),
		m_f3ScreenOffset(0.0f, 0.0f, 0.0f),
		m_uRenderPassKey(0),
		m_uMaxText(0),
		m_uDrawCount(0)
	{

	}

	DebugTextOverlay::~DebugTextOverlay()
	{

	}

	bool DebugTextOverlay::Create(const boost::any& _rConfig)
	{
		CreateInfoPtr pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
		bool bResult = (NULL != pInfo);

		if (false != bResult)
		{
			Release();
			m_uMaxText = pInfo->m_uMaxText;
			m_f3ScreenOffset = pInfo->m_f3ScreenOffset;
			m_uRenderPassKey = pInfo->m_uRenderPassKey;

			m_vDrawInfoPool.resize(m_uMaxText);
			for (UInt i = 0 ; m_uMaxText > i ; ++i)
			{
				m_vDrawInfoPool[i].m_pText = NULL;
			}

			DisplayPtr pDisplay = Display::GetInstance();
			DisplayFontManagerPtr pFontManager = pDisplay->GetFontManager();
			DisplayMaterialManagerPtr pMaterialManager = pDisplay->GetMaterialManager();

			map<Key, string>::iterator iPair = pInfo->m_mFontNameList.begin();
			map<Key, string>::iterator iEnd = pInfo->m_mFontNameList.end();
			while (iEnd != iPair)
			{
				const Key uFontNameKey = iPair->first;
				const string strFileName = iPair->second;
				bResult = pFontManager->Load(uFontNameKey, strFileName);
				if (false == bResult)
				{
					break;
				}
				m_mFonts[uFontNameKey] = pFontManager->Get(uFontNameKey);

				const Key uMaterialNameKey = MakeKey(pInfo->m_mFontMaterialList[uFontNameKey]);
				DisplayMaterialPtr pMaterial = pMaterialManager->GetMaterial(uMaterialNameKey);
				bResult = (NULL != pMaterial);
				if (false == bResult)
				{
					break;
				}
				m_mMaterials[uFontNameKey] = pMaterial;

				++iPair;
			}
		}

		return bResult;
	}

	void DebugTextOverlay::Update()
	{
		DisplayPtr pDisplay = Display::GetInstance();
		if (m_uRenderPassKey == pDisplay->GetCurrentRenderStage()->GetNameKey())
		{
			for (UInt i = 0 ; m_uDrawCount > i ; ++i)
			{
				DrawInfoRef rDrawInfo = m_vDrawInfoPool[i];
				pDisplay->RenderRequest(m_uRenderPassKey, rDrawInfo.m_pText);
			}
			m_uDrawCount = 0;
		}
	}

	void DebugTextOverlay::Release()
	{
		for (UInt i = 0 ; m_uMaxText > i ; ++i)
		{
			DrawInfoRef rDrawInfo = m_vDrawInfoPool[i];
			if (NULL != rDrawInfo.m_pText)
			{
				DisplayFontPtr pFont = m_mFonts[rDrawInfo.m_uFontName];
				pFont->ReleaseText(rDrawInfo.m_pText);
				rDrawInfo.m_pText = NULL;
			}
		}
		m_vDrawInfoPool.clear();

		DisplayPtr pDisplay = Display::GetInstance();
		DisplayFontManagerPtr pFontManager = pDisplay->GetFontManager();
		FontPtrMap::iterator iPair = m_mFonts.begin();
		FontPtrMap::iterator iEnd = m_mFonts.end();
		while (iEnd != iPair)
		{
			pFontManager->Unload(iPair->first);
			++iPair;
		}
		m_mFonts.clear();

		m_mMaterials.clear();
	}

	bool DebugTextOverlay::DrawRequest(const float _fX, const float _fY, const Key& _uFontName, const wstring& _wstrText, const Vector4& _f4Color)
	{
		bool bResult = (m_uMaxText > m_uDrawCount);

		if (false != bResult)
		{
			DisplayFontPtr pFont = m_mFonts[_uFontName];
			bResult = (NULL != pFont);

			if (false != bResult)
			{
				DrawInfoRef rDrawInfo = m_vDrawInfoPool[m_uDrawCount];
				rDrawInfo.m_f4Color = _f4Color;
				rDrawInfo.m_uFontName = _uFontName;
				if (NULL == rDrawInfo.m_pText)
				{
					rDrawInfo.m_pText = pFont->CreateText();
					bResult = (NULL != rDrawInfo.m_pText);
					if (false != bResult)
					{
						rDrawInfo.m_pText->SetMaterial(m_mMaterials[_uFontName]);
					}
				}
				if (false != bResult)
				{
					Vector3 oVPos(_fX + m_f3ScreenOffset.x, _fY + m_f3ScreenOffset.y, 0.0f + m_f3ScreenOffset.y);
					MatrixPtr pWorld = rDrawInfo.m_pText->GetWorldMatrix();
					D3DXMatrixTransformation(pWorld, NULL, NULL, NULL, NULL, NULL, &oVPos);
					rDrawInfo.m_pText->SetText(_wstrText);
					rDrawInfo.m_pText->SetColor(_f4Color);
					++m_uDrawCount;
				}
			}
		}

		return bResult;
	}
}
