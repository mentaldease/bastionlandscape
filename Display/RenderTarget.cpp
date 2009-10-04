#include "stdafx.h"
#include "../Display/RenderTarget.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	const VertexElement	DisplayRenderTargetGeometry::Vertex::s_aDecl[4] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
		{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  1 },
		D3DDECL_END()
	};

	DisplayRenderTargetGeometry::DisplayRenderTargetGeometry(DisplayRef _rDisplay)
	:	DisplayObject(_rDisplay),
		m_pPreviousVertexBuffer(NULL),
		m_pPreviousVertexDecl(NULL),
		m_pVertDeclPP(NULL),
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
		bool bResult = true;

		const Vertex aQuad[4] =
		{
			{ -0.5f,					-0.5f,						0.5f,	1.0f,	0.0f,	0.0f,	0.0f,	0.0f },
			{ -0.5f,					pInfo->m_uHeight - 0.5f,	0.5f,	1.0f,	0.0f,	1.0f,	0.0f,	1.0f },
			{ pInfo->m_uWidth - 0.5f,	-0.5f,						0.5f,	1.0f,	1.0f,	0.0f,	1.0f,	0.0f },
			{ pInfo->m_uWidth - 0.5f,	pInfo->m_uHeight - 0.5f,	0.5f,	1.0f,	1.0f,	1.0f,	1.0f,	1.0f }
		};
		memcpy(m_aQuad, aQuad, 4 * sizeof(Vertex));

		bResult = SUCCEEDED(m_rDisplay.GetDevicePtr()->CreateVertexDeclaration(DisplayRenderTargetGeometry::Vertex::s_aDecl, &m_pVertDeclPP));

		return bResult;
	}

	void DisplayRenderTargetGeometry::Update()
	{

	}

	void DisplayRenderTargetGeometry::Release()
	{
		if (NULL != m_pVertDeclPP)
		{
			m_pVertDeclPP->Release();
			m_pVertDeclPP = NULL;
		}
		m_pPreviousVertexBuffer = NULL;
		m_pPreviousVertexDecl = NULL;
		m_uPreviousVBOffset = 0;
		m_uPreviousVBStride = 0;
	}

	void DisplayRenderTargetGeometry::RenderBegin()
	{
		m_rDisplay.GetDevicePtr()->GetStreamSource(0, &m_pPreviousVertexBuffer, &m_uPreviousVBOffset, &m_uPreviousVBStride);
		m_rDisplay.GetDevicePtr()->GetVertexDeclaration(&m_pPreviousVertexDecl);
		if (m_pPreviousVertexDecl != m_pVertDeclPP)
		{
			m_rDisplay.GetDevicePtr()->SetVertexDeclaration(m_pVertDeclPP);
		}
	}

	void DisplayRenderTargetGeometry::Render()
	{
		m_rDisplay.GetDevicePtr()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, m_aQuad, sizeof(Vertex));
	}

	void DisplayRenderTargetGeometry::RenderEnd()
	{
		if (m_pPreviousVertexDecl != m_pVertDeclPP)
		{
			m_rDisplay.GetDevicePtr()->SetStreamSource(0, m_pPreviousVertexBuffer, m_uPreviousVBOffset, m_uPreviousVBStride);
			m_rDisplay.GetDevicePtr()->SetVertexDeclaration(m_pPreviousVertexDecl);
		}
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
		m_uCurrentBuffer(0),
		m_uRTIndex(0),
		m_uPassIndex(0),
		m_uRTSemanticNameKey(0),
		m_uORTSemanticNameKey(0),
		m_eRenderState(ERenderState_UNKNOWN),
		m_eMode(ERenderMode_UNKNOWNPROCESS),
		m_bFirstRender(true),
		m_bImmediateWrite(false)
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
		bool bResult = false;

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

		return bResult;
	}

	void DisplayRenderTarget::Update()
	{

	}

	void DisplayRenderTarget::Release()
	{
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
		if ((ERenderState_UNKNOWN == m_eRenderState) || (ERenderState_RENDEREND == m_eRenderState))
		{
			if (m_eMode != _eMode)
			{
				m_eRenderState = ERenderState_RENDERBEGIN;
				if (false == m_bImmediateWrite)
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
	}

	void DisplayRenderTarget::RenderBeginPass(UIntRef _uIndex)
	{
		if ((ERenderState_RENDERBEGIN == m_eRenderState) || (ERenderState_RENDERENDPASS == m_eRenderState))
		{
			m_uPassIndex = _uIndex;
			m_eRenderState = ERenderState_RENDERBEGINPASS;
			if ((ERenderMode_POSTPROCESS == m_eMode) || (NULL == m_pCurrentBufferTex))
			{
				if (ERenderMode_POSTPROCESS == m_eMode)
				{
					m_rDisplay.GetTextureManager()->SetBySemantic(m_uRTSemanticNameKey, m_pCurrentBufferTex);
					m_rDisplay.GetTextureManager()->SetBySemantic(m_uORTSemanticNameKey, m_pDoubleBufferTex[c_uOriginalBuffer]);
					if (1 == m_uRTIndex)
					{
						return;
					}
				}
				if (false == m_bImmediateWrite)
				{
					const UInt uNewIndex = (false != m_bFirstRender) ? c_uOriginalBuffer : m_uCurrentBuffer;
					if (m_pDoubleBufferTex[uNewIndex] != m_pCurrentBufferTex)
					{
						m_pCurrentBufferTex = m_pDoubleBufferTex[uNewIndex];
						m_rDisplay.GetDevicePtr()->SetRenderTarget(m_uRTIndex, m_pDoubleBufferSurf[uNewIndex]);
					}
				}
			}
		}
	}

	void DisplayRenderTarget::RenderEndPass()
	{
		if (ERenderState_RENDERBEGINPASS == m_eRenderState)
		{
			m_eRenderState = ERenderState_RENDERENDPASS;
			if ((ERenderMode_POSTPROCESS == m_eMode) && (1 == m_uRTIndex))
			{
				return;
			}
			if (ERenderMode_POSTPROCESS == m_eMode)
			{
				//m_rDisplay.GetTextureManager()->SetBySemantic(m_uRTSemanticNameKey, NULL);
				//m_rDisplay.GetTextureManager()->SetBySemantic(m_uORTSemanticNameKey, NULL);
				m_uCurrentBuffer = 1 - m_uCurrentBuffer;
			}
		}
	}

	void DisplayRenderTarget::RenderEnd()
	{
		if ((ERenderState_RENDERBEGIN == m_eRenderState) || (ERenderState_RENDERENDPASS == m_eRenderState))
		{
			m_eRenderState = ERenderState_RENDEREND;
			m_bFirstRender = false;
			if (false == m_bImmediateWrite)
			{
				m_rDisplay.GetDevicePtr()->SetRenderTarget(m_uRTIndex, m_pPreviousBufferSurf);
				if (NULL != m_pPreviousBufferSurf)
				{
					m_pPreviousBufferSurf->Release();
					m_pPreviousBufferSurf = NULL;
				}
				if ((ERenderMode_NORMALPROCESS == m_eMode) && (1 == m_uRTIndex))
				{
					//D3DXSaveTextureToFile(L"data/Debug.jpg", D3DXIFF_JPG, m_pCurrentBufferTex->GetBase(), NULL);
				}
			}
		}
	}

	DisplayTexturePtr DisplayRenderTarget::GetTexture()
	{
		return m_pCurrentBufferTex;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayRenderTargetChain::DisplayRenderTargetChain(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_vGBuffer()
	{

	}

	DisplayRenderTargetChain::~DisplayRenderTargetChain()
	{

	}

	bool DisplayRenderTargetChain::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = false;

		for (UInt i = 0 ; pInfo->m_uBufferCount > i ; ++i)
		{
			const string strRTName = boost::str(boost::format("%1%_buffer%2%") % pInfo->m_strName % i);
			DisplayRenderTarget::CreateInfo oRTRTCInfo = { strRTName, pInfo->m_uWidth, pInfo->m_uHeight, pInfo->m_uFormat, i };
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
			(*iRT)->RenderBegin(_eMode);
			++iRT;
		}
	}

	void DisplayRenderTargetChain::RenderBeginPass(UIntRef _uIndex)
	{
		DisplayRenderTargetPtrVec::iterator iRT = m_vGBuffer.begin();
		DisplayRenderTargetPtrVec::iterator iEnd = m_vGBuffer.end();
		while (iEnd != iRT)
		{
			(*iRT)->RenderBeginPass(_uIndex);
			++iRT;
		}
	}

	void DisplayRenderTargetChain::RenderEndPass()
	{
		DisplayRenderTargetPtrVec::iterator iRT = m_vGBuffer.begin();
		DisplayRenderTargetPtrVec::iterator iEnd = m_vGBuffer.end();
		while (iEnd != iRT)
		{
			(*iRT)->RenderEndPass();
			++iRT;
		}
	}

	void DisplayRenderTargetChain::RenderEnd()
	{
		DisplayRenderTargetPtrVec::iterator iRT = m_vGBuffer.begin();
		DisplayRenderTargetPtrVec::iterator iEnd = m_vGBuffer.end();
		while (iEnd != iRT)
		{
			(*iRT)->RenderEnd();
			++iRT;
		}

		m_rDisplay.GetDevicePtr()->EndScene();
	}

	DisplayTexturePtr DisplayRenderTargetChain::GetTexture(UIntRef _uBufferIndex)
	{
		DisplayTexturePtr pResult = (_uBufferIndex < m_vGBuffer.size()) ? m_vGBuffer[_uBufferIndex]->GetTexture() : NULL;
		return pResult;
	}
}
