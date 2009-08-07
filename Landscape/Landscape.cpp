#include "stdafx.h"
#include "Landscape.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	VertexElement Landscape::VertexDefault::s_VertexElement[4] =
	{
		{ 0,	0,						D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
		{ 0,	sizeof(Vector3),		D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
		{ 0,	2 * sizeof(Vector3),	D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,		0 },
		D3DDECL_END()
	};

	VertexElement Landscape::VertexLiquid::s_VertexElement[6] =
	{
		{ 0,	0,						D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
		{ 0,	1 * sizeof(Vector3),	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
		{ 0,	2 * sizeof(Vector3),	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL,	0 },
		{ 0,	3 * sizeof(Vector3),	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,	0 },
		{ 0,	4 * sizeof(Vector3),	D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
		D3DDECL_END()
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	LandscapeChunk::LandscapeChunk(Landscape& _rLandscape, DisplayRef _rDisplay)
	:	CoreObject(),
		m_rLandscape(_rLandscape),
		m_rDisplay(_rDisplay),
		m_uStartVertexIndex(0)
	{

	}

	LandscapeChunk::~LandscapeChunk()
	{

	}

	bool LandscapeChunk::Create(const boost::any& _rConfig)
	{
		bool bResult = true;
		m_uStartVertexIndex = boost::any_cast<const unsigned int>(_rConfig);
		return bResult;
	}

	void LandscapeChunk::Update()
	{

	}

	void LandscapeChunk::Release()
	{

	}

	void LandscapeChunk::Render()
	{
		const Landscape::GlobalInfo& rGlobalInfo = m_rLandscape.GetGlobalInfo();
		const unsigned int uVertexCount = rGlobalInfo.m_uVertexPerRawCount * (rGlobalInfo.m_uQuadSize + 1);
		//m_rDisplay.GetDevicePtr()->DrawIndexedPrimitive(D3DPT_LINESTRIP, m_uStartVertexIndex, 0, uVertexCount, 0, rGlobalInfo.m_uStripSize - 1);
		m_rDisplay.GetDevicePtr()->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, m_uStartVertexIndex, 0, uVertexCount, 0, rGlobalInfo.m_uStripSize - 2);
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Landscape::GlobalInfo::GlobalInfo()
	:	m_strName(),
		m_uQuadSize(0),
		m_uGridWidth(0),
		m_uGridDepth(0),
		m_uChunkCount(0),
		m_uVertexCount(0),
		m_uVertexPerRawCount(0),
		m_uRawCount(0),
		m_uStripSize(0)
	{

	}

	bool Landscape::GlobalInfo::Create(const Landscape::OpenInfo& _rOpenInfo)
	{
		m_uQuadSize = _rOpenInfo.m_uQuadSize;
		m_uGridWidth = _rOpenInfo.m_uGridWidth;
		m_uGridDepth = _rOpenInfo.m_uGridDepth;
		m_uVertexPerRawCount = _rOpenInfo.m_uQuadSize * _rOpenInfo.m_uGridWidth + 1;
		m_uRawCount = _rOpenInfo.m_uQuadSize * _rOpenInfo.m_uGridDepth + 1;
		m_uVertexCount = m_uVertexPerRawCount * m_uRawCount;
		const unsigned int uBandCount = m_uQuadSize;
		const unsigned int uVertexPerBand = (m_uQuadSize + 1) * 2;
		const unsigned int uBandJunctionVertexCount = 2 * (m_uQuadSize - 1);
		m_uStripSize = uBandCount * uVertexPerBand + uBandJunctionVertexCount;
		m_strName = _rOpenInfo.m_strName;
		return true;
	}

	void Landscape::GlobalInfo::Release()
	{
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Landscape::Landscape(DisplayRef _rDisplay)
	:	DisplayObject(_rDisplay),
		m_oGlobalInfo(),
		m_vGrid(),
		m_pVertexBuffer(NULL),
		m_pIndexBuffer(NULL),
		m_pVertexes(NULL),
		m_pIndexes(NULL)
	{

	}

	Landscape::~Landscape()
	{

	}

	Landscape::EVertexFormat Landscape::StringToVertexFormat(const string& _strFormat)
	{
		EVertexFormat eFormat = EFormat_UNKNOWN;

		if ("liquid" == _strFormat)
		{
			eFormat = EFormat_LIQUID;
		}
		else if ("default" == _strFormat)
		{
			eFormat = EFormat_DEFAULT;
		}

		return eFormat;
	}

	bool Landscape::Create(const boost::any& _rConfig)
	{
		bool bResult = true;
		return bResult;
	}

	void Landscape::Update()
	{
		m_rDisplay.RenderRequest(this);
	}

	void Landscape::Release()
	{
	}

	void Landscape::Render()
	{
		if ((m_pVertexBuffer->Use()) && (m_pIndexBuffer->Use()))
		{
			struct RenderFunction
			{
				void operator() (LandscapeChunkPtr pLandscapeChunk)
				{
					pLandscapeChunk->Render();
				}
			};
			for_each(m_vGrid.begin(), m_vGrid.end(), RenderFunction());
		}
	}

	bool Landscape::Open(const OpenInfo& _rOpenInfo)
	{
		bool bResult = m_vGrid.empty() && m_oGlobalInfo.Create(_rOpenInfo);

		if (false != bResult)
		{
			m_pIndexes = new unsigned int[m_oGlobalInfo.m_uStripSize];
			bResult = (NULL != m_pIndexes);
			if (false != bResult)
			{
				unsigned int* pIndexes = m_pIndexes;
				for (unsigned int j = 0 ; m_oGlobalInfo.m_uQuadSize > j ; ++j)
				{
					for (unsigned int i = 0 ; (m_oGlobalInfo.m_uQuadSize + 1) > i ; ++i)
					{
						*pIndexes = i + j * m_oGlobalInfo.m_uVertexPerRawCount;
						++pIndexes;
						*pIndexes = i + (j + 1) * m_oGlobalInfo.m_uVertexPerRawCount;
						++pIndexes;
					}
					if ((m_oGlobalInfo.m_uQuadSize - 1) != j)
					{
						*pIndexes = m_oGlobalInfo.m_uQuadSize + (j + 1) * m_oGlobalInfo.m_uVertexPerRawCount;
						++pIndexes;
						*pIndexes = 0 + (j + 1) * m_oGlobalInfo.m_uVertexPerRawCount;
						++pIndexes;
					}
				}

				DisplayIndexBuffer::CreateInfo oIBCInfo = { m_oGlobalInfo.m_uStripSize, false };
				m_pIndexBuffer = m_rDisplay.CreateIndexBuffer(oIBCInfo);
				bResult = (NULL != m_pIndexBuffer);
				if (false != bResult)
				{
					bResult = m_pIndexBuffer->Set(m_pIndexes);
				}
			}
		}

		if (false != bResult)
		{
			m_eFormat = _rOpenInfo.m_eFormat;
			switch (m_eFormat)
			{
				case Landscape::EFormat_DEFAULT:
				{
					CreateVertexBufferDefault();
					break;
				}
				case Landscape::EFormat_LIQUID:
				{
					CreateVertexBufferLiquid();
					break;
				}
			}
		}

		if (false != bResult)
		{
			// chunks
			m_oGlobalInfo.m_uChunkCount = _rOpenInfo.m_uGridWidth * _rOpenInfo.m_uGridDepth;
			m_vGrid.reserve(m_oGlobalInfo.m_uChunkCount);
			for (unsigned int j = 0 ; m_oGlobalInfo.m_uGridDepth > j ; ++j)
			{
				for (unsigned int i = 0 ; m_oGlobalInfo.m_uGridWidth > i ; ++i)
				{
					LandscapeChunkPtr pLandscapeChunk = new LandscapeChunk(*this, m_rDisplay);
					const unsigned int IndexX = i * m_oGlobalInfo.m_uQuadSize;// + ((0 == i) ? 0 : 1);
					const unsigned int IndexZ = j * m_oGlobalInfo.m_uQuadSize;// + ((0 == j) ? 0 : 1);
					const unsigned int Index = IndexX + IndexZ * m_oGlobalInfo.m_uVertexPerRawCount;
					bResult = pLandscapeChunk->Create(boost::any(Index));
					if (false == bResult)
					{
						break;
					}
					m_vGrid.push_back(pLandscapeChunk);
				}
			}
		}

		return bResult;
	}

	void Landscape::Close()
	{
		struct ReleaseAndDeleteFunction
		{
			void operator() (LandscapeChunkPtr pLandscapeChunk)
			{
				pLandscapeChunk->Release();
				delete pLandscapeChunk;
			}
		};
		for_each(m_vGrid.begin(), m_vGrid.end(), ReleaseAndDeleteFunction());
		m_vGrid.clear();

		if (NULL != m_pVertexBuffer)
		{
			m_rDisplay.ReleaseVertexBuffer(m_pVertexBuffer);
			m_pVertexBuffer = NULL;
		}

		if (NULL != m_pIndexBuffer)
		{
			m_rDisplay.ReleaseIndexBuffer(m_pIndexBuffer);
			m_pIndexBuffer = NULL;
		}

		if (NULL != m_pIndexes)
		{
			delete[] m_pIndexes;
			m_pIndexes = NULL;
		}

		m_oGlobalInfo.Release();
	}

	const Landscape::GlobalInfo& Landscape::GetGlobalInfo() const
	{
		return m_oGlobalInfo;
	}

	bool Landscape::CreateVertexBufferDefault()
	{
		DisplayVertexBuffer::CreateInfo oVBCreateInfo = { m_oGlobalInfo.m_uVertexCount * sizeof(VertexDefault), sizeof(VertexDefault), VertexDefault::s_VertexElement };
		m_pVertexBuffer = m_rDisplay.CreateVertexBuffer(oVBCreateInfo);
		bool bResult = (NULL != m_pVertexBuffer);
		if (false != bResult)
		{
			const float fXOffset = float(m_oGlobalInfo.m_uVertexPerRawCount - 1) / 2.0f;
			const float fZOffset = float(m_oGlobalInfo.m_uRawCount - 1) / 2.0f;
			m_pVertexes = new VertexDefault[m_oGlobalInfo.m_uVertexCount];
			VertexDefaultPtr pVertex = (VertexDefaultPtr)m_pVertexes;
			for (unsigned int j = 0 ; m_oGlobalInfo.m_uRawCount > j ; ++j)
			{
				for (unsigned int i = 0 ; m_oGlobalInfo.m_uVertexPerRawCount > i ; ++i)
				{
					pVertex->m_oPosition.x = float(i) - fXOffset;
					pVertex->m_oPosition.y = 0.0f;
					pVertex->m_oPosition.z = -float(j) + fZOffset;
					pVertex->m_oNormal.x = 0.0f;
					pVertex->m_oNormal.y = 1.0f;
					pVertex->m_oNormal.z = 0.0f;
					pVertex->m_oColor.x = 1.0f;
					pVertex->m_oColor.y = 1.0f;
					pVertex->m_oColor.z = 1.0f;
					pVertex->m_oColor.w = 1.0f;
					++pVertex;
				}
			}
			bResult = m_pVertexBuffer->Set(m_pVertexes);
		}
		return bResult;
	}

	bool Landscape::CreateVertexBufferLiquid()
	{
		DisplayVertexBuffer::CreateInfo oVBCreateInfo = { m_oGlobalInfo.m_uVertexCount * sizeof(VertexLiquid), sizeof(VertexLiquid), VertexLiquid::s_VertexElement };
		m_pVertexBuffer = m_rDisplay.CreateVertexBuffer(oVBCreateInfo);
		bool bResult = (NULL != m_pVertexBuffer);
		if (false != bResult)
		{
			const float fXOffset = float(m_oGlobalInfo.m_uVertexPerRawCount - 1) / 2.0f;
			const float fZOffset = float(m_oGlobalInfo.m_uRawCount - 1) / 2.0f;
			m_pVertexes = new VertexLiquid[m_oGlobalInfo.m_uVertexCount];
			VertexLiquidPtr pVertex = (VertexLiquidPtr)m_pVertexes;
			for (unsigned int j = 0 ; m_oGlobalInfo.m_uRawCount > j ; ++j)
			{
				for (unsigned int i = 0 ; m_oGlobalInfo.m_uVertexPerRawCount > i ; ++i)
				{
					pVertex->m_oPosition.x = float(i) - fXOffset;
					pVertex->m_oPosition.y = 0.0f;
					pVertex->m_oPosition.z = -float(j) + fZOffset;
					pVertex->m_oNormal.x = 0.0f;
					pVertex->m_oNormal.y = 1.0f;
					pVertex->m_oNormal.z = 0.0f;
					pVertex->m_oBiNormal.x = 0.0f;
					pVertex->m_oBiNormal.y = 0.0f;
					pVertex->m_oBiNormal.z = 0.0f;
					pVertex->m_oTangent.x = 0.0f;
					pVertex->m_oTangent.y = 0.0f;
					pVertex->m_oTangent.z = 0.0f;
					pVertex->m_oUV.x = float(i) / float(m_oGlobalInfo.m_uVertexPerRawCount - 1);
					pVertex->m_oUV.y = float(j) / float(m_oGlobalInfo.m_uRawCount - 1);
					++pVertex;
				}
			}
			bResult = m_pVertexBuffer->Set(m_pVertexes);
		}
		return bResult;
	}
}
