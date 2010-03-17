#include "stdafx.h"
#include "../Display/Display.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayVertexBuffer::DisplayVertexBuffer(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_uBufferSize(0),
		m_uVertexSize(0),
		m_pVertexDecl(NULL),
		m_pVertexBuffer(NULL)
	{

	}

	DisplayVertexBuffer::~DisplayVertexBuffer()
	{

	}

	bool DisplayVertexBuffer::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = (NULL != pInfo) && (0 != pInfo->m_uBufferSize) && (NULL != pInfo->m_pVertexElement);

		if (false != bResult)
		{
			Release();
			m_uBufferSize = pInfo->m_uBufferSize;
			m_uVertexSize = pInfo->m_uVertexSize;
			HRESULT hResult =  m_rDisplay.GetDevicePtr()->CreateVertexBuffer(
				m_uBufferSize,
				D3DUSAGE_WRITEONLY,
				0,
				D3DPOOL_DEFAULT,
				&m_pVertexBuffer,
				NULL);
			bResult = SUCCEEDED(hResult);
		}

		if (false != bResult)
		{
			HRESULT hResult = m_rDisplay.GetDevicePtr()->CreateVertexDeclaration(pInfo->m_pVertexElement, &m_pVertexDecl);
			bResult = SUCCEEDED(hResult);
		}

		return bResult;
	}

	void DisplayVertexBuffer::Update()
	{

	}

	void DisplayVertexBuffer::Release()
	{
		if (NULL != m_pVertexDecl)
		{
			m_pVertexDecl->Release();
			m_pVertexDecl = NULL;
		}
		if (NULL != m_pVertexBuffer)
		{
			m_pVertexBuffer->Release();
			m_pVertexBuffer = NULL;
		}
	}

	bool DisplayVertexBuffer::Set(const VoidPtr _pData)
	{
		VoidPtr pLockedData = NULL;
		HRESULT hResult = m_pVertexBuffer->Lock(0, m_uBufferSize, &pLockedData, 0);
		if (SUCCEEDED(hResult))
		{
			memcpy(pLockedData, _pData, m_uBufferSize);
			m_pVertexBuffer->Unlock();
		}
		return (SUCCEEDED(hResult));
	}

	bool DisplayVertexBuffer::Use()
	{
		VertexBufferPtr pCurrentVertexBuffer;
		unsigned int uOffset;
		unsigned int uStride;
		HRESULT hResult = m_rDisplay.GetDevicePtr()->GetStreamSource(0, &pCurrentVertexBuffer, &uOffset, &uStride);
		if ((SUCCEEDED(hResult)) && (pCurrentVertexBuffer != m_pVertexBuffer))
		{
			hResult = m_rDisplay.GetDevicePtr()->SetStreamSource(0, m_pVertexBuffer, 0, m_uVertexSize);
		}
		if (SUCCEEDED(hResult))
		{
			VertexDeclPtr pCurrentVertexDecl;
			hResult = m_rDisplay.GetDevicePtr()->GetVertexDeclaration(&pCurrentVertexDecl);
			if ((SUCCEEDED(hResult)) && (pCurrentVertexDecl != m_pVertexDecl))
			{
				hResult = m_rDisplay.GetDevicePtr()->SetVertexDeclaration(m_pVertexDecl);
			}
		}
		return (SUCCEEDED(hResult));
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayIndexBuffer::DisplayIndexBuffer(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_uBufferSize(0),
		m_b16Bits(false),
		m_pIndexBuffer(NULL)
	{

	}

	DisplayIndexBuffer::~DisplayIndexBuffer()
	{

	}

	bool DisplayIndexBuffer::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = (NULL != pInfo) && (0 != pInfo->m_uBufferSize);

		if (false != bResult)
		{
			Release();
			m_b16Bits = pInfo->m_b16Bits;
			m_uBufferSize = pInfo->m_uBufferSize * ((false != m_b16Bits) ? sizeof(Word) : sizeof(UInt));
			HRESULT hResult =  m_rDisplay.GetDevicePtr()->CreateIndexBuffer(
				m_uBufferSize,
				D3DUSAGE_WRITEONLY,
				(false != m_b16Bits) ? D3DFMT_INDEX16 : D3DFMT_INDEX32,
				D3DPOOL_DEFAULT,
				&m_pIndexBuffer,
				NULL);
			bResult = SUCCEEDED(hResult);
		}

		return bResult;
	}

	void DisplayIndexBuffer::Update()
	{

	}

	void DisplayIndexBuffer::Release()
	{
		if (NULL != m_pIndexBuffer)
		{
			m_pIndexBuffer->Release();
			m_pIndexBuffer = NULL;
		}
	}

	bool DisplayIndexBuffer::Set(const VoidPtr _pData)
	{
		VoidPtr pLockedData = NULL;
		HRESULT hResult = m_pIndexBuffer->Lock(0, m_uBufferSize, &pLockedData, 0);
		if (SUCCEEDED(hResult))
		{
			memcpy(pLockedData, _pData, m_uBufferSize);
			m_pIndexBuffer->Unlock();
		}
		return (SUCCEEDED(hResult));
	}

	bool DisplayIndexBuffer::Use()
	{
		IndexBufferPtr pCurrentIndexBuffer;
		HRESULT hResult = m_rDisplay.GetDevicePtr()->GetIndices(&pCurrentIndexBuffer);
		if ((SUCCEEDED(hResult)) && (pCurrentIndexBuffer != m_pIndexBuffer))
		{
			hResult = m_rDisplay.GetDevicePtr()->SetIndices(m_pIndexBuffer);
		}
		return (SUCCEEDED(hResult));
	}
}