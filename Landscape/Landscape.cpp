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

	LandscapeChunk::LandscapeChunk(Landscape& _rLandscape, DisplayRef _rDisplay, const unsigned int& _uLOD)
	:	CoreObject(),
		m_rLandscape(_rLandscape),
		m_rDisplay(_rDisplay),
		m_uStartVertexIndex(0),
		m_uLOD(_uLOD),
		m_pParent(NULL)
	{
		m_pChildren[ESubChild_NORTHWEST] =
		m_pChildren[ESubChild_NORTHEAST] =
		m_pChildren[ESubChild_SOUTHWEST] =
		m_pChildren[ESubChild_SOUTHEAST] = NULL;
	}

	LandscapeChunk::~LandscapeChunk()
	{

	}

	bool LandscapeChunk::Create(const boost::any& _rConfig)
	{
		bool bResult = true;
#if 1
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		const Landscape::GlobalInfo& rGlobalInfo = m_rLandscape.GetGlobalInfo();
		Landscape::LODInfoPtr pLODInfo = &(rGlobalInfo.m_pLODs[m_uLOD]);
		const unsigned int IndexX = pInfo->m_uX * pLODInfo->m_uLODQuadSize;
		const unsigned int IndexZ = pInfo->m_uZ * pLODInfo->m_uLODQuadSize;
		m_uStartVertexIndex = IndexX + IndexZ * rGlobalInfo.m_uVertexPerRawCount;

		if (0 < m_uLOD)
		{
			CreateInfo oLCCInfo;
			unsigned int uChild = ESubChild_NORTHWEST;
			for (unsigned int j = 0 ; 2 > j ; ++j)
			{
				for (unsigned int i = 0 ; 2 > i ; ++i)
				{
					LandscapeChunkPtr pLandscapeChunk = new LandscapeChunk(m_rLandscape, m_rDisplay, m_uLOD - 1);
					oLCCInfo.m_uX = pInfo->m_uX * 2 + i;
					oLCCInfo.m_uZ = pInfo->m_uZ * 2 + j;
					bResult = pLandscapeChunk->Create(boost::any(&oLCCInfo));
					if (false == bResult)
					{
						bResult;
					}
					m_pChildren[uChild] = pLandscapeChunk;
					++uChild;
				}
			}
		}
#else
		m_uStartVertexIndex = boost::any_cast<const unsigned int>(_rConfig);
#endif
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
		//m_rDisplay.GetDevicePtr()->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, m_uStartVertexIndex, 0, uVertexCount, 0, rGlobalInfo.m_uStripSize - 2);
		const unsigned int uStartIndex = rGlobalInfo.m_pLODs[m_uLOD].m_uStartIndex;
		const unsigned int uStripSize = rGlobalInfo.m_pLODs[m_uLOD].m_uStripSize - 2;
		m_rDisplay.GetDevicePtr()->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, m_uStartVertexIndex, 0, uVertexCount, uStartIndex, uStripSize);
	}

	void LandscapeChunk::Traverse(LandscapeChunkPtrVecRef _rRenderList)
	{
		const Landscape::GlobalInfo& rGlobalInfo = m_rLandscape.GetGlobalInfo();
		if ((rGlobalInfo.m_uLODCount - 1) == m_uLOD)
		{
			_rRenderList.push_back(this);
		}
		else if (0 != m_uLOD)
		{
			for (unsigned int i = 0 ; ESubChild_COUNT > i ; ++i)
			{
				m_pChildren[i]->Traverse(_rRenderList);
			}
		}
	}


	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Landscape::GlobalInfo::GlobalInfo()
	:	m_strName(),
		m_uQuadSize(0),
		m_uGridSize(0),
		m_uChunkCount(0),
		m_uVertexCount(0),
		m_uVertexPerRawCount(0),
		m_uRawCount(0),
		m_uStripSize(0),
		m_uLODCount(0),
		m_uTotalLODStripSize(0),
		m_pLODs(NULL)
	{

	}

	bool Landscape::GlobalInfo::Create(const Landscape::OpenInfo& _rOpenInfo)
	{
		m_uQuadSize = _rOpenInfo.m_uQuadSize;
		m_uGridSize = _rOpenInfo.m_uGridSize;
		m_uVertexPerRawCount = _rOpenInfo.m_uQuadSize * _rOpenInfo.m_uGridSize + 1;
		m_uRawCount = _rOpenInfo.m_uQuadSize * _rOpenInfo.m_uGridSize + 1;
		m_uVertexCount = m_uVertexPerRawCount * m_uRawCount;
		const unsigned int uBandCount = m_uQuadSize;
		const unsigned int uVertexPerBand = (m_uQuadSize + 1) * 2;
		const unsigned int uBandJunctionVertexCount = 2 * (m_uQuadSize - 1);
		m_uStripSize = uBandCount * uVertexPerBand + uBandJunctionVertexCount;
		m_strName = _rOpenInfo.m_strName;

		bool bResult = IsPowerOf2(m_uGridSize, &m_uLODCount) && IsPowerOf2(m_uQuadSize);

		if (false != bResult)
		{
			m_uTotalLODStripSize = 0;
			m_pLODs = new LODInfo[m_uLODCount];
			LODInfoPtr pLODInfo = m_pLODs;
			for (unsigned int i = 0 ; m_uLODCount > i ; ++i)
			{
				pLODInfo->m_uStripSize = m_uStripSize;
				pLODInfo->m_uStartIndex = m_uTotalLODStripSize;
				pLODInfo->m_uLODGridSize = m_uGridSize >> i;
				pLODInfo->m_uLODQuadSize = m_uGridSize << i;
				m_uTotalLODStripSize += m_uStripSize;
				pLODInfo++;
			}
		}

		return bResult;
	}

	void Landscape::GlobalInfo::Release()
	{
		if (NULL != m_pLODs)
		{
			delete[] m_pLODs;
			m_pLODs = NULL;
		}
	}

	bool Landscape::GlobalInfo::IsPowerOf2(const unsigned int& _uValue, unsigned int* _pPowerLevel)
	{
		unsigned int uTemp = _uValue;
		unsigned int uBitsCount = 0;
		bool bResult = false;

		if (NULL == _pPowerLevel)
		{
			while (0 != uTemp)
			{
				uBitsCount = (0x1 == (0x1 & uTemp)) ? (uBitsCount + 1) : uBitsCount;
				uTemp >>= 1;
			}
		}
		else
		{
			(*_pPowerLevel) = 0;
			while (0 != uTemp)
			{
				uBitsCount = (0x1 == (0x1 & uTemp)) ? (uBitsCount + 1) : uBitsCount;
				uTemp >>= 1;
				++(*_pPowerLevel);
			}
		}

		bResult = (1 == uBitsCount); // is it a poser of 2 number ??

		return bResult;
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
		//m_vRenderList.push_back(m_vGrid.back()); // test : render lowest LOD chunk
		m_vGrid.back()->Traverse(m_vRenderList);
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
			for_each(m_vRenderList.begin(), m_vRenderList.end(), RenderFunction());
			m_vRenderList.clear();
		}
	}

	bool Landscape::Open(const OpenInfo& _rOpenInfo)
	{
		bool bResult = m_vGrid.empty() && m_oGlobalInfo.Create(_rOpenInfo);

		if (false != bResult)
		{
			bResult = CreateIndexBuffer();
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
			bResult = CreateChunks();
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

	bool Landscape::CreateIndexBuffer()
	{

		m_pIndexes = new unsigned int[m_oGlobalInfo.m_uTotalLODStripSize];
		bool bResult = (NULL != m_pIndexes);
		if (false != bResult)
		{
			unsigned int* pIndexes = m_pIndexes;

			for (unsigned int k = 0 ; m_oGlobalInfo.m_uLODCount > k ; ++k)
			{
				const unsigned int uLODIncrement = (0x00000001 << k);
				const unsigned int uLODQuadSize = (m_oGlobalInfo.m_uQuadSize << k);
				for (unsigned int j = 0 ; uLODQuadSize > j ; j += uLODIncrement)
				{
					for (unsigned int i = 0 ; (uLODQuadSize + 1) > i ; i += uLODIncrement)
					{
						*pIndexes = i + j * m_oGlobalInfo.m_uVertexPerRawCount;
						++pIndexes;
						*pIndexes = i + (j + uLODIncrement) * m_oGlobalInfo.m_uVertexPerRawCount;
						++pIndexes;
					}
					if ((uLODQuadSize - uLODIncrement) != j)
					{
						*pIndexes = uLODQuadSize + (j + uLODIncrement) * m_oGlobalInfo.m_uVertexPerRawCount;
						++pIndexes;
						*pIndexes = 0 + (j + uLODIncrement) * m_oGlobalInfo.m_uVertexPerRawCount;
						++pIndexes;
					}
				}
			}

			DisplayIndexBuffer::CreateInfo oIBCInfo = { m_oGlobalInfo.m_uTotalLODStripSize, false };
			m_pIndexBuffer = m_rDisplay.CreateIndexBuffer(oIBCInfo);
			bResult = (NULL != m_pIndexBuffer);
			if (false != bResult)
			{
				bResult = m_pIndexBuffer->Set(m_pIndexes);
			}
		}

		return bResult;
	}

	bool Landscape::CreateChunks()
	{
		// compute total chunks (all LODs)
		unsigned int uLOD = 0;
		unsigned int uGridSize = m_oGlobalInfo.m_uGridSize;
		m_oGlobalInfo.m_uChunkCount = 0;
		while (m_oGlobalInfo.m_uLODCount > uLOD)
		{
			m_oGlobalInfo.m_uChunkCount += uGridSize * uGridSize;
			uGridSize >>= 1;
			++uLOD;
		}
		bool bResult = (0 < m_oGlobalInfo.m_uChunkCount);

		if (false != bResult)
		{
#if 1
			LandscapeChunkPtr pLandscapeChunk = new LandscapeChunk(*this, m_rDisplay, m_oGlobalInfo.m_uLODCount - 1);
			LandscapeChunk::CreateInfo oLCCInfo = { 0, 0 };
			bResult = pLandscapeChunk->Create(boost::any(&oLCCInfo));
			if (false != bResult)
			{
				m_vGrid.push_back(pLandscapeChunk);
			}
#else
			m_vGrid.reserve(m_oGlobalInfo.m_uChunkCount);
			// alloc all chunks
			for (unsigned int uLOD = m_oGlobalInfo.m_uLODCount ; 0 < uLOD ; --uLOD)
			{
				const unsigned int uTrueLOD = uLOD - 1;
				const unsigned int uLODGridSize = m_oGlobalInfo.m_uGridSize >> uTrueLOD;
				const unsigned int uLODQuadSize = m_oGlobalInfo.m_uGridSize << uTrueLOD;
				for (unsigned int j = 0 ; uLODGridSize > j ; ++j)
				{
					for (unsigned int i = 0 ; uLODGridSize > i ; ++i)
					{
						LandscapeChunkPtr pLandscapeChunk = new LandscapeChunk(*this, m_rDisplay, uTrueLOD);
						const unsigned int IndexX = i * uLODQuadSize;
						const unsigned int IndexZ = j * uLODQuadSize;
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
#endif
		}
		// link chunks in parent/child relationship
		if (false != bResult)
		{

		}

		return bResult;
	}
}
