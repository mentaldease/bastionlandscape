#include "stdafx.h"
#include "Landscape.h"
#include "../Display/Camera.h"

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
		m_pParent(NULL),
		m_pLODInfo(NULL)
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
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		const Landscape::GlobalInfo& rGlobalInfo = m_rLandscape.GetGlobalInfo();
		m_pLODInfo = &(rGlobalInfo.m_pLODs[m_uLOD]);
		const unsigned int IndexX = pInfo->m_uX * m_pLODInfo->m_uLODQuadSize;
		const unsigned int IndexZ = pInfo->m_uZ * m_pLODInfo->m_uLODQuadSize;
		m_uStartVertexIndex = IndexX + IndexZ * rGlobalInfo.m_uVertexPerRawCount;

		// center
		const unsigned int uStartIndex = m_pLODInfo->m_uStartIndex;
		const unsigned int uStripSize = m_pLODInfo->m_uStripSize;
		Vector3 oTemp[2];
		m_rLandscape.GetVertexPosition(uStartIndex, m_uStartVertexIndex, oTemp[0]);
		m_rLandscape.GetVertexPosition(uStartIndex + uStripSize - 1, m_uStartVertexIndex, oTemp[1]);
		//m_oCenter = oTemp[0] + ((oTemp[0] - oTemp[1]) / 2.0f);
		//m_oCenter = (oTemp[0] + oTemp[1]) / 2.0f;
		m_oExtends.x = (oTemp[1].x - oTemp[0].x) / 2.0f;
		m_oExtends.y = 0.0f;
		m_oExtends.z = (oTemp[0].z - oTemp[1].z) / 2.0f;
		m_oCenter.x = oTemp[0].x + m_oExtends.x;
		m_oCenter.y = 0.0f;
		m_oCenter.z = oTemp[1].z + m_oExtends.z;

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
						break;
					}
					m_pChildren[uChild] = pLandscapeChunk;
					++uChild;
				}
			}
		}

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
		const unsigned int uVertexCount = rGlobalInfo.m_uVertexCount;
		const unsigned int uStartIndex = m_pLODInfo->m_uStartIndex;
		const unsigned int uStripSize = m_pLODInfo->m_uStripSize - 2;
		m_rDisplay.GetDevicePtr()->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, m_uStartVertexIndex, 0, uVertexCount, uStartIndex, uStripSize);
	}

	void LandscapeChunk::Traverse(LandscapeChunkPtrVecRef _rRenderList, const Vector3& _rCamPos, const float& _fPixelSize)
	{
		MatrixPtr pWorld = m_rLandscape.GetWorldMatrix();
		const Vector3 oWorld(pWorld->_41, pWorld->_42, pWorld->_43);
		const Vector3 oDelta = oWorld + m_oCenter - _rCamPos;
		const float fDistance0 = D3DXVec3Length(&oDelta) - D3DXVec3Length(&m_oExtends);
		const float fDistance = (0.0f < fDistance0) ? fDistance0 : 1.0f;
		const float fVertexErrorLevel = m_pLODInfo->m_uGeometricError / fDistance * _fPixelSize;
		const Landscape::GlobalInfo& rGlobalInfo = m_rLandscape.GetGlobalInfo();
		if (fVertexErrorLevel <= rGlobalInfo.m_fPixelErrorMax)
		{
			_rRenderList.push_back(this);
		}
		else if (0 != m_uLOD)
		{
			for (unsigned int i = 0 ; ESubChild_COUNT > i ; ++i)
			{
				m_pChildren[i]->Traverse(_rRenderList, _rCamPos, _fPixelSize);
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
		m_pLODs(NULL),
		m_fPixelErrorMax(0.0f)
	{

	}

	bool Landscape::GlobalInfo::Create(const Landscape::OpenInfo& _rOpenInfo)
	{
		m_strName = _rOpenInfo.m_strName;
		m_uQuadSize = _rOpenInfo.m_uQuadSize;
		m_uGridSize = _rOpenInfo.m_uGridSize;
		m_uVertexPerRawCount = _rOpenInfo.m_uQuadSize * _rOpenInfo.m_uGridSize + 1;
		m_uRawCount = _rOpenInfo.m_uQuadSize * _rOpenInfo.m_uGridSize + 1;
		m_uVertexCount = m_uVertexPerRawCount * m_uRawCount;
		m_fPixelErrorMax = _rOpenInfo.m_fPixelErrorMax;
		const unsigned int uBandCount = m_uQuadSize;
		const unsigned int uVertexPerBand = (m_uQuadSize + 1) * 2;
		const unsigned int uBandJunctionVertexCount = 2 * (m_uQuadSize - 1);
		m_uStripSize = uBandCount * uVertexPerBand + uBandJunctionVertexCount;

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
				pLODInfo->m_uLODQuadSize = m_uQuadSize << i;
				pLODInfo->m_uGeometricError = (0 != i) ? (0x00000001 << (i - 1)) : 0;
				//pLODInfo->m_uGeometricError = 0x00000001 << ((m_uLODCount - 1) - i);
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
		m_pIndexes(NULL),
		m_eFormat(EFormat_UNKNOWN)
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
		const float fPixelSize = m_rDisplay.GetCurrentCamera()->GetPixelSize();
		const Vector3& rCamPos = m_rDisplay.GetCurrentCamera()->GetPosition();
		m_vGrid.back()->Traverse(m_vRenderList, rCamPos, fPixelSize);
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

	void Landscape::GetVertexPosition(const unsigned int& _uIndexBufferIndex, const unsigned int& _uVertexStartIndex, Vector3& _rPosition)
	{
		const unsigned int uVertexIndex = _uVertexStartIndex + m_pIndexes[_uIndexBufferIndex];
		switch (m_eFormat)
		{
			case Landscape::EFormat_DEFAULT:
			{
				VertexDefaultPtr pBuffer = static_cast<VertexDefaultPtr>(m_pVertexes);
				_rPosition = pBuffer[uVertexIndex].m_oPosition;
				break;
			}
			case Landscape::EFormat_LIQUID:
			{
				VertexLiquidPtr pBuffer = static_cast<VertexLiquidPtr>(m_pVertexes);
				_rPosition = pBuffer[uVertexIndex].m_oPosition;
				break;
			}
		}
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
					for (unsigned int i = 0 ; (uLODQuadSize + uLODIncrement) > i ; i += uLODIncrement)
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

		// create chunk recursively
		if (false != bResult)
		{
			LandscapeChunkPtr pLandscapeChunk = new LandscapeChunk(*this, m_rDisplay, m_oGlobalInfo.m_uLODCount - 1);
			LandscapeChunk::CreateInfo oLCCInfo = { 0, 0 };
			bResult = pLandscapeChunk->Create(boost::any(&oLCCInfo));
			if (false != bResult)
			{
				m_vGrid.push_back(pLandscapeChunk);
			}
		}

		return bResult;
	}
}
