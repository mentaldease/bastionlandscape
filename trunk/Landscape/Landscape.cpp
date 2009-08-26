#include "stdafx.h"
#include "Landscape.h"
#include "../Display/Camera.h"
#include "../Display/Effect.h"
#include "../Display/EffectParam.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	VertexElement VertexDefault::s_VertexElement[5] =
	{
		{ 0,	0,						D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
		{ 0,	1 * sizeof(Vector3),	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	1 },
		{ 0,	2 * sizeof(Vector3),	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
		{ 0,	3 * sizeof(Vector3),	D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,		0 },
		D3DDECL_END()
	};

	VertexElement VertexLiquid::s_VertexElement[6] =
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
	:	DisplayObject(_rDisplay),
		m_rLandscape(_rLandscape),
		//m_rDisplay(_rDisplay),
		m_uStartVertexIndex(0),
		m_uLOD(_uLOD),
		m_pParent(NULL),
		m_oCenter(0.0f, 0.0f, 0.0f),
		m_oExtends(0.0f, 0.0f, 0.0f),
		m_pLODInfo(NULL),
		m_fMorphFactor(1.0f)
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
		const unsigned int IndexX = pInfo->m_uX * rGlobalInfo.m_uQuadSize;
		const unsigned int IndexZ = pInfo->m_uZ * rGlobalInfo.m_uQuadSize;
		m_uStartVertexIndex = IndexX + IndexZ * m_pLODInfo->m_uVertexPerRawCount;

		// center
		const unsigned int uStartIndex = m_pLODInfo->m_uStartIndex;
		const unsigned int uStripSize = m_pLODInfo->m_uStripSize;
		Vector3 oTemp[2];
		m_rLandscape.GetVertexPosition(*m_pLODInfo, 0, m_uStartVertexIndex, oTemp[0]);
		m_rLandscape.GetVertexPosition(*m_pLODInfo, uStripSize - 1, m_uStartVertexIndex, oTemp[1]);
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

	void LandscapeChunk::RenderBegin()
	{
		DisplayEffectParamMORPHFACTOR::s_fMorphFactor = &m_fMorphFactor;
	}

	void LandscapeChunk::Render()
	{
		if (m_rLandscape.UseLODVertexBuffer(m_uLOD) && m_rLandscape.SetIndices())
		{
			const unsigned int uVertexCount = m_pLODInfo->m_uNumVertices;
			const unsigned int uStartIndex = m_pLODInfo->m_uStartIndex;
			const unsigned int uStripSize = m_pLODInfo->m_uStripSize - 2;
			m_rDisplay.GetDevicePtr()->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, m_uStartVertexIndex, 0, uVertexCount, uStartIndex, uStripSize);
		}
	}

	void LandscapeChunk::Traverse(LandscapeChunkPtrVecRef _rRenderList, const Vector3& _rCamPos, const float& _fPixelSize)
	{
		const Landscape::GlobalInfo& rGlobalInfo = m_rLandscape.GetGlobalInfo();
		MatrixPtr pWorld = m_rLandscape.GetWorldMatrix();
		const Vector3 oWorld(pWorld->_41, pWorld->_42, pWorld->_43);
		const Vector3 oDelta = oWorld + m_oCenter - _rCamPos;
		const float fDelta = D3DXVec3Length(&oDelta);
		const float fExtends = D3DXVec3Length(&m_oExtends);
		const float fRawDistance = (fDelta - fExtends);
		const float fDistance = (1.0f <= fRawDistance) ? fRawDistance : 1.0f;
		const float fVertexErrorLevel = (m_pLODInfo->m_uGeometricError / fDistance) * _fPixelSize;
		const float fRatio = fDistance / fExtends;

		//if (fVertexErrorLevel <= rGlobalInfo.m_fPixelErrorMax)
		//if ((rGlobalInfo.m_fPixelErrorMax <= fRatio) || (0 == m_uLOD))
		if ((1.0f <= fRatio) || (0 == m_uLOD))
		{
			//m_fMorphFactor = ((2.0f * fVertexErrorLevel) / rGlobalInfo.m_fPixelErrorMax) - 1.0f;
			//m_fMorphFactor = fVertexErrorLevel / rGlobalInfo.m_fPixelErrorMax;
			//m_fMorphFactor = fDistance / fMaxErrorDistance;
			//m_fMorphFactor = ((2.0f * fVertexErrorLevel) / rGlobalInfo.m_fPixelErrorMax) - 1.0f;
			m_fMorphFactor = ((fExtends * 2.0f) / fDistance) - 1.0f;
			m_fMorphFactor = (0.0f > m_fMorphFactor) ? 0.0f : ((1.0f < m_fMorphFactor) ? 1.0f : m_fMorphFactor);
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

	unsigned int LandscapeChunk::GetLODID() const
	{
		return m_uLOD;
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
		m_fPixelErrorMax(0.0f),
		m_fFloorScale(1.0f),
		m_fHeightScale(1.0f)
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
		m_fFloorScale = _rOpenInfo.m_fFloorScale;
		m_fHeightScale = _rOpenInfo.m_fHeightScale;
		m_eFormat = _rOpenInfo.m_eFormat;
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
				pLODInfo->m_uGridSize = m_uGridSize >> i;
				pLODInfo->m_uQuadSize = m_uQuadSize << i;
				pLODInfo->m_uGeometricError = (0 != i) ? (0x00000001 << (i - 1)) : 0;
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
		m_vVertexBuffers(),
		m_vVertexes(),
		m_pCurrentVertexBuffer(NULL),
		m_pIndexBuffer(NULL),
		m_pIndexes(NULL)
	{

	}

	Landscape::~Landscape()
	{

	}

	ELandscapeVertexFormat Landscape::StringToVertexFormat(const string& _strFormat)
	{
		ELandscapeVertexFormat eFormat = ELandscapeVertexFormat_UNKNOWN;

		if ("liquid" == _strFormat)
		{
			eFormat = ELandscapeVertexFormat_LIQUID;
		}
		else if ("default" == _strFormat)
		{
			eFormat = ELandscapeVertexFormat_DEFAULT;
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
		//m_rDisplay.RenderRequest(this);
		const float fPixelSize = m_rDisplay.GetCurrentCamera()->GetPixelSize();
		const Vector3& rCamPos = m_rDisplay.GetCurrentCamera()->GetPosition();
		m_vGrid.back()->Traverse(m_vRenderList, rCamPos, fPixelSize);

		struct LODCompareFunction
		{
			bool operator() (LandscapeChunkPtr pLC1, LandscapeChunkPtr pLC2)
			{
				return (pLC1->GetLODID() < pLC2->GetLODID());
			}
		};
		sort(m_vRenderList.begin(), m_vRenderList.end(), LODCompareFunction());

		LandscapeChunkPtrVec::iterator iChunk = m_vRenderList.begin();
		LandscapeChunkPtrVec::iterator iEnd = m_vRenderList.end();
		while (iEnd != iChunk)
		{
			*((*iChunk)->GetWorldMatrix()) = *(GetWorldMatrix());
			(*iChunk)->SetMaterial(m_pMaterial);
			m_rDisplay.RenderRequest(*iChunk);
			++iChunk;
		}
		m_vRenderList.clear();
	}

	void Landscape::Release()
	{
	}

	void Landscape::Render()
	{
		m_pCurrentVertexBuffer = NULL;
		size_t uCount = m_vRenderList.size();
		if ((false == m_vRenderList.empty()) && (m_pIndexBuffer->Use()))
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
			switch (m_oGlobalInfo.m_eFormat)
			{
				case ELandscapeVertexFormat_DEFAULT:
				{
					bResult = CreateVertexBufferDefault();
					break;
				}
				case ELandscapeVertexFormat_LIQUID:
				{
					bResult = CreateVertexBufferLiquid();
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

		while (false == m_vVertexBuffers.empty())
		{
			m_rDisplay.ReleaseVertexBuffer(m_vVertexBuffers.back());
			m_vVertexBuffers.pop_back();
		}
		while (false == m_vVertexes.empty())
		{
			delete[] m_vVertexes.back();
			m_vVertexes.pop_back();
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

	void Landscape::GetVertexPosition(const LODInfo& _rLODInfo, const unsigned int& _uIndexBufferIndex, const unsigned int& _uVertexStartIndex, Vector3& _rPosition)
	{
		const unsigned int uVertexIndex = _uVertexStartIndex + m_pIndexes[_rLODInfo.m_uStartIndex + _uIndexBufferIndex];
		switch (m_oGlobalInfo.m_eFormat)
		{
			case ELandscapeVertexFormat_DEFAULT:
			{
				VertexDefaultPtr pBuffer = static_cast<VertexDefaultPtr>(_rLODInfo.m_pVertexes);
				_rPosition = pBuffer[uVertexIndex].m_oPosition;
				break;
			}
			case ELandscapeVertexFormat_LIQUID:
			{
				VertexLiquidPtr pBuffer = static_cast<VertexLiquidPtr>(_rLODInfo.m_pVertexes);
				_rPosition = pBuffer[uVertexIndex].m_oPosition;
				break;
			}
		}
	}

	bool Landscape::SetIndices()
	{
		return m_pIndexBuffer->Use();
	}

	bool Landscape::UseLODVertexBuffer(const unsigned int& _uLOD)
	{
		bool bResult = (m_pCurrentVertexBuffer == m_oGlobalInfo.m_pLODs[_uLOD].m_pVertexBuffer);
		if (false == bResult)
		{
			m_pCurrentVertexBuffer = m_oGlobalInfo.m_pLODs[_uLOD].m_pVertexBuffer;
			bResult = m_pCurrentVertexBuffer->Use();
		}
		return bResult;
	}

	bool Landscape::CreateIndexBuffer()
	{
		bool bResult = false;
		m_pIndexes = new unsigned int[m_oGlobalInfo.m_uTotalLODStripSize];
		bResult = (NULL != m_pIndexes);
		if (false != bResult)
		{
			unsigned int* pIndexes = m_pIndexes;

			for (unsigned int k = 0 ; m_oGlobalInfo.m_uLODCount > k ; ++k)
			{
				const unsigned int uLODIncrement = (0x00000001 << 0);
				const unsigned int uLODQuadSize = (m_oGlobalInfo.m_uQuadSize << 0);
				const unsigned int uVertexPerRawCount = (m_oGlobalInfo.m_uVertexPerRawCount >> k) | 0x00000001;
				for (unsigned int j = 0 ; uLODQuadSize > j ; j += uLODIncrement)
				{
					for (unsigned int i = 0 ; (uLODQuadSize + uLODIncrement) > i ; i += uLODIncrement)
					{
						*pIndexes = i + j * uVertexPerRawCount;
						++pIndexes;
						*pIndexes = i + (j + uLODIncrement) * uVertexPerRawCount;
						++pIndexes;
					}
					if ((uLODQuadSize - uLODIncrement) != j)
					{
						*pIndexes = uLODQuadSize + (j + uLODIncrement) * uVertexPerRawCount;
						++pIndexes;
						*pIndexes = 0 + (j + uLODIncrement) * uVertexPerRawCount;
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

		// create chunks recursively
		if (false != bResult)
		{
			LandscapeChunkPtr pLandscapeChunk = new LandscapeChunk(*this, m_rDisplay, m_oGlobalInfo.m_uLODCount - 1);
			LandscapeChunk::CreateInfo oLCCInfo;
			oLCCInfo.m_uX = 0;
			oLCCInfo.m_uZ = 0;
			bResult = pLandscapeChunk->Create(boost::any(&oLCCInfo));
			if (false != bResult)
			{
				m_vGrid.push_back(pLandscapeChunk);
			}
		}

		return bResult;
	}
}
