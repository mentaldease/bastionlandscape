#include "stdafx.h"
#include "../Display/RenderTarget.h"
#include "../Display/Camera.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	VertexElement	DisplayRenderTargetGeometry::Vertex::s_aDecl[6] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
		{ 0, 16, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0 },
		{ 0, 28, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  1 },
		{ 0, 40, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  2 },
		{ 0, 52, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  3 },
		D3DDECL_END()
	};

	DisplayRenderTargetGeometry::DisplayRenderTargetGeometry(DisplayRef _rDisplay)
	:	DisplayObject(),
		m_rDisplay(*Display::GetInstance()),
		m_pPreviousVertexBuffer(NULL),
		m_pPreviousVertexDecl(NULL),
		m_uVertexDecl(0),
		m_uPreviousVBOffset(0),
		m_uPreviousVBStride(0)
	{

	}

	DisplayRenderTargetGeometry::~DisplayRenderTargetGeometry()
	{

	}

	bool DisplayRenderTargetGeometry::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = (NULL != pInfo);

		if (false != bResult)
		{
			Release();

			m_fFullWidth = float(pInfo->m_uWidth);
			m_fFullHeight = float(pInfo->m_uHeight);

			const float fWidth = pInfo->m_uWidth * 0.5f;
			const float fHeight = pInfo->m_uHeight * 0.5f;
			const Vertex aQuad[4] =
			{
				{ -0.5f,					-0.5f,						1.0f,	1.0f,	0.0f,	0.0f,	float(DisplayCamera::EFrustumCorner_FARTOPLEFT),		0.0f,	0.0f,	float(DisplayCamera::EFrustumCorner_FARTOPLEFT)		},
				{ -0.5f,					fHeight - 0.5f,				1.0f,	1.0f,	0.0f,	0.5f,	float(DisplayCamera::EFrustumCorner_FARBOTTOMLEFT),		0.0f,	1.0f,	float(DisplayCamera::EFrustumCorner_FARBOTTOMLEFT)	},
				{ fWidth - 0.5f,			-0.5f,						1.0f,	1.0f,	0.5f,	0.0f,	float(DisplayCamera::EFrustumCorner_FARTOPRIGHT),		1.0f,	0.0f,	float(DisplayCamera::EFrustumCorner_FARTOPRIGHT)	},
				{ fWidth - 0.5f,			fHeight - 0.5f,				1.0f,	1.0f,	0.5f,	0.5f,	float(DisplayCamera::EFrustumCorner_FARBOTTOMRIGHT),	1.0f,	1.0f,	float(DisplayCamera::EFrustumCorner_FARBOTTOMRIGHT)	}
			};
			memcpy(m_aQuad, aQuad, 4 * sizeof(Vertex));

			m_uVertexDecl = m_rDisplay.CreateVertexDeclaration(DisplayRenderTargetGeometry::Vertex::s_aDecl);
			bResult = (0 != m_uVertexDecl);
		}

		return bResult;
	}

	void DisplayRenderTargetGeometry::Update()
	{

	}

	void DisplayRenderTargetGeometry::Release()
	{
		if (0 != m_uVertexDecl)
		{
			m_rDisplay.ReleaseVertexDeclaration(m_uVertexDecl);
			m_uVertexDecl = 0;
		}
		m_pPreviousVertexBuffer = NULL;
		m_pPreviousVertexDecl = NULL;
		m_uPreviousVBOffset = 0;
		m_uPreviousVBStride = 0;
	}

	void DisplayRenderTargetGeometry::RenderBegin()
	{
		//DevicePtr pDevice = m_rDisplay.GetDevicePtr();
		//pDevice->GetStreamSource(0, &m_pPreviousVertexBuffer, &m_uPreviousVBOffset, &m_uPreviousVBStride);
		//pDevice->GetVertexDeclaration(&m_pPreviousVertexDecl);
		//if (m_pPreviousVertexDecl != m_uVertexDecl)
		//{
		//	pDevice->SetVertexDeclaration(m_uVertexDecl);
		//}
		DisplayPtr pDisplay = Display::GetInstance();
		if (m_uVertexDecl != pDisplay->GetCurrentVertexDeclaration())
		{
			pDisplay->SetVertexDeclaration(m_uVertexDecl);
		}
	}

	void DisplayRenderTargetGeometry::Render()
	{
		// use current view port to map geometry size and texture coordinates.
		ViewportPtr pViewport = m_rDisplay.GetCurrentCamera()->GetCurrentViewport();
		const float fWidth = float(pViewport->Width);
		const float fHeight = float(pViewport->Height);
		const float fX = float(pViewport->X);
		const float fY = float(pViewport->Y);
		m_aQuad[0].x = fX - 0.5f;				m_aQuad[0].y = fY - 0.5f;
		m_aQuad[1].x = fX - 0.5f;				m_aQuad[1].y = fY + fHeight - 0.5f;
		m_aQuad[2].x = fX + fWidth - 0.5f;		m_aQuad[2].y = fY - 0.5f;
		m_aQuad[3].x = fX + fWidth - 0.5f;		m_aQuad[3].y = fY + fHeight - 0.5f;
		m_aQuad[0].tu = 0.0f;					m_aQuad[0].tv = 0.0f;
		m_aQuad[1].tu = 0.0f;					m_aQuad[1].tv = fHeight / m_fFullHeight;
		m_aQuad[2].tu = fWidth / m_fFullWidth;	m_aQuad[2].tv = 0.0f;
		m_aQuad[3].tu = fWidth / m_fFullWidth;	m_aQuad[3].tv = fHeight / m_fFullHeight;

		// update frustum corners values.
		Vector3Ptr pFrustumCorners = m_rDisplay.GetCurrentCamera()->GetFrustumCorners();
		for (UInt i = 0 ; 4 > i ; ++i)
		{
			Vertex& rVertex = m_aQuad[i];
			Vector3Ptr pFarCorner = &pFrustumCorners[UInt(rVertex.tw)];
			Vector3Ptr pNearCorner = &pFrustumCorners[UInt(rVertex.tw) + 4];
			rVertex.tu3 = pFarCorner->x;
			rVertex.tv3 = pFarCorner->y;
			rVertex.tw3 = pFarCorner->z;
			rVertex.tu4 = pNearCorner->x;
			rVertex.tv4 = pNearCorner->y;
			rVertex.tw4 = pNearCorner->z;
		}

		m_rDisplay.GetDevicePtr()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, m_aQuad, sizeof(Vertex));
	}

	void DisplayRenderTargetGeometry::RenderEnd()
	{
		//if ((m_pPreviousVertexDecl != m_uVertexDecl) && (NULL != m_pPreviousVertexDecl))
		//{
		//	DevicePtr pDevice = m_rDisplay.GetDevicePtr();
		//	pDevice->SetStreamSource(0, m_pPreviousVertexBuffer, m_uPreviousVBOffset, m_uPreviousVBStride);
		//	pDevice->SetVertexDeclaration(m_pPreviousVertexDecl);
		//}
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayRenderTarget::DisplayRenderTarget(DisplayRef _rDisplay)
	:	CoreObject(),
		m_strName(),
		m_rDisplay(_rDisplay),
		m_pPreviousBufferSurf(NULL),
		m_pCurrentBufferTex(NULL),
		m_pRTOverrideTex(NULL),
		m_pRTOverrideSurf(NULL),
		m_uCurrentBuffer(0),
		m_uRTIndex(0),
		m_uPassIndex(0),
		m_uRTSemanticNameKey(0),
		m_uORTSemanticNameKey(0),
		m_eRenderState(ERenderState_UNKNOWN),
		m_eMode(ERenderMode_UNKNOWNPROCESS),
		m_bFirstRender(true),
		m_bImmediateWrite(false),
		m_bEnabled(false),
		m_bSwap(false)
	{
		for (UInt i = 0 ; c_uBufferCount > i ; ++i)
		{
			m_pDoubleBufferTex[i] = NULL;
			m_pDoubleBufferSurf[i] = NULL;
		}
	}

	DisplayRenderTarget::~DisplayRenderTarget()
	{

	}

	bool DisplayRenderTarget::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = (NULL != pInfo);

		if (false != bResult)
		{
			Release();

			m_strName = pInfo->m_strName;
			m_uRTIndex = pInfo->m_uIndex;
			m_bImmediateWrite = pInfo->m_bImmediateWrite;
			m_eRenderState = ERenderState_UNKNOWN;
			m_eMode = ERenderMode_UNKNOWNPROCESS;
			m_pCurrentBufferTex = NULL;
			m_uCurrentBuffer = 0;

			string strSemanticName = boost::str(boost::format("RT2D0%1%") % m_uRTIndex);
			m_uRTSemanticNameKey = MakeKey(strSemanticName);
			strSemanticName = boost::str(boost::format("ORT2D0%1%") % m_uRTIndex);
			m_uORTSemanticNameKey = MakeKey(strSemanticName);

			for (UInt i = 0 ; c_uBufferCount > i ; ++i)
			{
				string strTexName = boost::str(boost::format("%1%_%2%") % m_strName % i);
				bResult = m_rDisplay.GetTextureManager()->New(strTexName, pInfo->m_uWidth, pInfo->m_uHeight, pInfo->m_uFormat, false, DisplayTexture::EType_2D, DisplayTexture::EUsage_RENDERTARGET);
				if (false == bResult)
				{
					break;
				}
				m_pDoubleBufferTex[i] = m_rDisplay.GetTextureManager()->Get(strTexName);
				TexturePtr pTexture = static_cast<TexturePtr>(m_pDoubleBufferTex[i]->GetBase());
				pTexture->GetSurfaceLevel(0, &m_pDoubleBufferSurf[i]);
			}
		}

		return bResult;
	}

	void DisplayRenderTarget::Update()
	{

	}

	void DisplayRenderTarget::Release()
	{
		SetRTOverride(NULL);
		for (UInt i = 0 ; c_uBufferCount > i ; ++i)
		{
			if (NULL != m_pDoubleBufferSurf[i])
			{
				m_pDoubleBufferSurf[i]->Release();
				m_pDoubleBufferSurf[i] = NULL;
			}
			if (NULL != m_pDoubleBufferTex[i])
			{
				string strTexName = boost::str(boost::format("%1%_%2%") % m_strName % i);
				m_rDisplay.GetTextureManager()->Unload(strTexName);
				m_pDoubleBufferTex[i] = NULL;
			}
		}
	}

	void DisplayRenderTarget::RenderBegin(const ERenderMode& _eMode)
	{
		m_bSwap = false;
		if ((false != m_bEnabled) && ((ERenderState_UNKNOWN == m_eRenderState) || (ERenderState_RENDEREND == m_eRenderState)))
		{
			m_eRenderState = ERenderState_RENDERBEGIN;
			//if (false == m_bImmediateWrite)
			if (0 == m_uRTIndex)
			{
				m_rDisplay.GetDevicePtr()->GetRenderTarget(m_uRTIndex, &m_pPreviousBufferSurf);
			}
			m_eMode = _eMode;
			if (ERenderMode_NORMALPROCESS == m_eMode)
			{
				m_uCurrentBuffer = 0;
				m_bFirstRender = true;
				m_pCurrentBufferTex = NULL;
			}
		}
	}

	void DisplayRenderTarget::RenderBeginPass(const UInt _uIndex)
	{
		m_bSwap = false;
		if ((false != m_bEnabled) && ((ERenderState_RENDERBEGIN == m_eRenderState) || (ERenderState_RENDERENDPASS == m_eRenderState)))
		{
			m_uPassIndex = _uIndex;
			m_eRenderState = ERenderState_RENDERBEGINPASS;
			if ((ERenderMode_POSTPROCESS == m_eMode) || (NULL == m_pCurrentBufferTex))
			{
				if (ERenderMode_POSTPROCESS == m_eMode)
				{
					m_rDisplay.GetTextureManager()->SetBySemantic(m_uRTSemanticNameKey, m_pCurrentBufferTex);
					m_rDisplay.GetTextureManager()->SetBySemantic(m_uORTSemanticNameKey, m_pDoubleBufferTex[c_uOriginalBuffer]);
				}
				if (false == m_bImmediateWrite)
				{
					if (NULL == m_pRTOverrideSurf)
					{
						const UInt uNewIndex = (false != m_bFirstRender) ? c_uOriginalBuffer : m_uCurrentBuffer;
						if (m_pDoubleBufferTex[uNewIndex] != m_pCurrentBufferTex)
						{
							m_pCurrentBufferTex = m_pDoubleBufferTex[uNewIndex];
							m_rDisplay.GetDevicePtr()->SetRenderTarget(m_uRTIndex, m_pDoubleBufferSurf[uNewIndex]);
							m_bSwap = true;
						}
					}
					else if (m_pCurrentBufferTex != m_pRTOverrideTex)
					{
						m_pCurrentBufferTex = m_pRTOverrideTex;
						m_rDisplay.GetDevicePtr()->SetRenderTarget(m_uRTIndex, m_pRTOverrideSurf);
						m_bSwap = true;
					}
				}
			}
		}
	}

	void DisplayRenderTarget::RenderEndPass()
	{
		m_bSwap = false;
		if ((false != m_bEnabled) && (ERenderState_RENDERBEGINPASS == m_eRenderState))
		{
			m_eRenderState = ERenderState_RENDERENDPASS;
			if (ERenderMode_POSTPROCESS == m_eMode)
			{
				m_rDisplay.GetTextureManager()->SetBySemantic(m_uRTSemanticNameKey, NULL);
				m_rDisplay.GetTextureManager()->SetBySemantic(m_uORTSemanticNameKey, NULL);
			}
			if (false == m_bImmediateWrite)
			{
				m_uCurrentBuffer = 1 - m_uCurrentBuffer;
			}
		}
	}

	void DisplayRenderTarget::RenderEnd()
	{
		m_bSwap = false;
		if ((false != m_bEnabled) && ((ERenderState_RENDERBEGIN == m_eRenderState) || (ERenderState_RENDERENDPASS == m_eRenderState)))
		{
			m_eRenderState = ERenderState_RENDEREND;
			m_bFirstRender = false;
			if ((0 != m_uRTIndex) || (NULL != m_pPreviousBufferSurf))
			{
				m_rDisplay.GetDevicePtr()->SetRenderTarget(m_uRTIndex, m_pPreviousBufferSurf);
			}
			if (NULL != m_pPreviousBufferSurf)
			{
				m_pPreviousBufferSurf->Release();
				m_pPreviousBufferSurf = NULL;
			}
		}
	}

	DisplayTexturePtr DisplayRenderTarget::GetTexture()
	{
		return m_pCurrentBufferTex;
	}

	void DisplayRenderTarget::SetEnabled(const bool _bState)
	{
		m_bEnabled = _bState;
		SetRTOverride(NULL);
	}

	bool DisplayRenderTarget::IsEnabled()
	{
		return m_bEnabled;
	}

	void DisplayRenderTarget::SetIndex(const UInt _uIndex)
	{
		m_uRTIndex = _uIndex;
	}

	UInt DisplayRenderTarget::GetIndex()
	{
		return m_uRTIndex;
	}

	bool DisplayRenderTarget::SwapOccured()
	{
		return m_bSwap;
	}

	void DisplayRenderTarget::SetRTOverride(DisplayTexturePtr _RTOverride)
	{
		if (m_pRTOverrideTex != _RTOverride)
		{
			if (NULL != m_pRTOverrideSurf)
			{
				m_pRTOverrideSurf->Release();
				m_pRTOverrideSurf = NULL;
			}
			m_pRTOverrideTex = _RTOverride;
			if (NULL != m_pRTOverrideTex)
			{
				TexturePtr pTexture = static_cast<TexturePtr>(m_pRTOverrideTex->GetBase());
				pTexture->GetSurfaceLevel(0, &m_pRTOverrideSurf);
			}
		}
	}

	void DisplayRenderTarget::SetImmediateWrite(const bool& _bState)
	{
		if (m_bImmediateWrite != _bState)
		{
			m_bImmediateWrite = _bState;
			if ((false != m_bImmediateWrite)
				&& (NULL != m_pPreviousBufferSurf))
			{
				m_rDisplay.GetDevicePtr()->SetRenderTarget(m_uRTIndex, m_pPreviousBufferSurf);
				m_pPreviousBufferSurf->Release();
				m_pPreviousBufferSurf = NULL;
			}
			else if ((false == m_bImmediateWrite) && (NULL == m_pPreviousBufferSurf) && ( 0 == m_uRTIndex))
			{
				m_rDisplay.GetDevicePtr()->GetRenderTarget(m_uRTIndex, &m_pPreviousBufferSurf);
			}
		}
	}

	bool DisplayRenderTarget::GetImmediateWrite()
	{
		return m_bImmediateWrite;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayRenderTargetChain::DisplayRenderTargetChain(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_vGBuffer(),
		m_bImmediateWrite(false)
	{

	}

	DisplayRenderTargetChain::~DisplayRenderTargetChain()
	{

	}

	bool DisplayRenderTargetChain::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = (NULL != pInfo);

		if (false != bResult)
		{
			for (UInt i = 0 ; pInfo->m_uBufferCount > i ; ++i)
			{
				const string strRTName = boost::str(boost::format("%1%_buffer%2%") % pInfo->m_strName % i);
				DisplayRenderTarget::CreateInfo oRTRTCInfo = { strRTName, pInfo->m_uWidth, pInfo->m_uHeight, D3DFORMAT(pInfo->m_pFormats[i]), i };
				DisplayRenderTargetPtr pRT = new DisplayRenderTarget(m_rDisplay);
				bResult = pRT->Create(boost::any(&oRTRTCInfo));
				if (false == bResult)
				{
					pRT->Release();
					delete pRT;
					break;
				}
				m_vGBuffer.push_back(pRT);
			}
		}

		return bResult;
	}

	void DisplayRenderTargetChain::Update()
	{

	}

	void DisplayRenderTargetChain::Release()
	{
		while (m_vGBuffer.end() != m_vGBuffer.begin())
		{
			DisplayRenderTargetPtr pRT = m_vGBuffer.back();
			m_vGBuffer.pop_back();
			pRT->Release();
			delete pRT;
		}
	}

	void DisplayRenderTargetChain::RenderBegin(const DisplayRenderTarget::ERenderMode& _eMode)
	{
		m_rDisplay.GetDevicePtr()->BeginScene();

		DisplayRenderTargetPtrVec::iterator iRT = m_vGBuffer.begin();
		DisplayRenderTargetPtrVec::iterator iEnd = m_vGBuffer.end();
		while (iEnd != iRT)
		{
			DisplayRenderTargetPtr pRT = *iRT;
			if (false != pRT->IsEnabled())
			{
				pRT->RenderBegin(_eMode);
			}
			++iRT;
		}
	}

	void DisplayRenderTargetChain::RenderBeginPass(const UInt _uIndex)
	{
		DisplayRenderTargetPtrVec::iterator iRT = m_vGBuffer.begin();
		DisplayRenderTargetPtrVec::iterator iEnd = m_vGBuffer.end();
		bool bSwap = false;
		while (iEnd != iRT)
		{
			DisplayRenderTargetPtr pRT = *iRT;
			if (false != pRT->IsEnabled())
			{
				pRT->RenderBeginPass(_uIndex);
				bSwap = bSwap || pRT->SwapOccured();
			}
			++iRT;
		}
		// Since SetRenderViewport reset the view port to full screen size we need to force back previously requested view port.
		if (false != bSwap)
		{
			m_rDisplay.GetDevicePtr()->SetViewport(m_rDisplay.GetCurrentCamera()->GetCurrentViewport());
		}
	}

	void DisplayRenderTargetChain::RenderEndPass()
	{
		DisplayRenderTargetPtrVec::iterator iRT = m_vGBuffer.begin();
		DisplayRenderTargetPtrVec::iterator iEnd = m_vGBuffer.end();
		while (iEnd != iRT)
		{
			DisplayRenderTargetPtr pRT = *iRT;
			if (false != pRT->IsEnabled())
			{
				pRT->RenderEndPass();
			}
			++iRT;
		}
	}

	void DisplayRenderTargetChain::RenderEnd()
	{
		DisplayRenderTargetPtrVec::iterator iRT = m_vGBuffer.begin();
		DisplayRenderTargetPtrVec::iterator iEnd = m_vGBuffer.end();
		while (iEnd != iRT)
		{
			DisplayRenderTargetPtr pRT = *iRT;
			if (false != pRT->IsEnabled())
			{
				pRT->RenderEnd();
			}
			++iRT;
		}

		m_rDisplay.GetDevicePtr()->EndScene();
	}

	DisplayTexturePtr DisplayRenderTargetChain::GetTexture(const UInt _uRTIndex)
	{
		DisplayTexturePtr pResult = (_uRTIndex < m_vGBuffer.size()) ? m_vGBuffer[_uRTIndex]->GetTexture() : NULL;
		return pResult;
	}

	DisplayRenderTargetPtr DisplayRenderTargetChain::GetRenderTarget(const UInt _uRTIndex)
	{
		DisplayRenderTargetPtr pResult = (_uRTIndex < m_vGBuffer.size()) ? m_vGBuffer[_uRTIndex] : NULL;
		return pResult;
	}

	void DisplayRenderTargetChain::EnableAllRenderTargets()
	{
		DisplayRenderTargetPtrVec::iterator iRT = m_vGBuffer.begin();
		DisplayRenderTargetPtrVec::iterator iEnd = m_vGBuffer.end();
		UInt uIndex = 0;
		while (iEnd != iRT)
		{
			(*iRT)->SetEnabled(true);
			(*iRT)->SetIndex(uIndex);
			++iRT;
			++uIndex;
		}
		//SetImmediateWrite(false);
	}

	void DisplayRenderTargetChain::DisableAllRenderTargets()
	{
		DisplayRenderTargetPtrVec::iterator iRT = m_vGBuffer.begin();
		DisplayRenderTargetPtrVec::iterator iEnd = m_vGBuffer.end();
		while (iEnd != iRT)
		{
			(*iRT)->SetEnabled(false);
			++iRT;
		}
		//SetImmediateWrite(true);
	}

	void DisplayRenderTargetChain::Clear(const UInt _uClearColor)
	{
		//EnableAllRenderTargets();
		RenderBegin(DisplayRenderTarget::ERenderMode_NORMALPROCESS);
		RenderBeginPass(0);
		m_rDisplay.GetDevicePtr()->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, _uClearColor, 1.0f, 0L);
		RenderEndPass();
		RenderEnd();
   	}

	void DisplayRenderTargetChain::SetImmediateWrite(const bool& _bState)
	{
		if (_bState != m_bImmediateWrite)
		{
			m_bImmediateWrite = _bState;
			DisplayRenderTargetPtrVec::iterator iRT = m_vGBuffer.begin();
			DisplayRenderTargetPtrVec::iterator iEnd = m_vGBuffer.end();
			while (iEnd != iRT)
			{
				(*iRT)->SetImmediateWrite(_bState);
				++iRT;
			}
		}
	}

	bool DisplayRenderTargetChain::GetImmediateWrite()
	{
		return m_bImmediateWrite;
	}
}
