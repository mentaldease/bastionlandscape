#include "stdafx.h"
#include "Landscape.h"
#include "../Display/Camera.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	VertexDefaultRef VertexDefault::operator = (VertexIndependentRef _rVertexIndependent)
	{
		m_oPosition = _rVertexIndependent.m_oPosition;
#if LANDSCAPE_USE_MORPHING
		m_oPosition2 = _rVertexIndependent.m_oPosition2;
#endif // LANDSCAPE_USE_MORPHING
		m_oNormal = _rVertexIndependent.m_oNormal;
		m_oUV = _rVertexIndependent.m_oUV;
		m_oUV2.x = _rVertexIndependent.m_fNormalizedSlope;
		m_oUV2.y = _rVertexIndependent.m_fNormalizedHeight;
		return *this;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	VertexLiquidRef VertexLiquid::operator = (VertexIndependentRef _rVertexIndependent)
	{
		m_oPosition = _rVertexIndependent.m_oPosition;
		m_oNormal = _rVertexIndependent.m_oNormal;
		m_oUV = _rVertexIndependent.m_oUV;
		return *this;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	void VertexIndependent::AddLink(const unsigned int& _uLOD, const unsigned int& _uIndex)
	{
		LODVertexLink oLink = { _uLOD, _uIndex };
		m_vLinks.push_back(oLink);
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	bool Landscape::CreateVertexBufferIndependent()
	{
		bool bResult = true;
		for (unsigned int k = 0 ; m_oGlobalInfo.m_uLODCount > k ; ++k)
		{
			const unsigned int uLODVertexPerRowCount = (m_oGlobalInfo.m_uVertexPerRowCount >> k) | 0x00000001;
			const unsigned int uLODRowCount = (m_oGlobalInfo.m_uRowCount >> k) | 0x00000001;
			const unsigned int uLODVertexCount = uLODVertexPerRowCount * uLODRowCount;
			const unsigned int uLODIncrement = 0x00000001 << k;
			const float fXOffset = float(m_oGlobalInfo.m_uVertexPerRowCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
			const float fZOffset = float(m_oGlobalInfo.m_uRowCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
			VertexIndependentPtr pVertexes = new VertexIndependent[uLODVertexCount];
			VertexIndependentPtr pVertex = pVertexes;
			for (unsigned int j = 0 ; m_oGlobalInfo.m_uRowCount > j ; j += uLODIncrement)
			{
				for (unsigned int i = 0 ; m_oGlobalInfo.m_uVertexPerRowCount > i ; i += uLODIncrement)
				{
					pVertex->m_oPosition.x = float(i) * m_oGlobalInfo.m_fFloorScale - fXOffset;
					pVertex->m_oPosition.y = 0.0f;
					pVertex->m_oPosition.z = -float(j) * m_oGlobalInfo.m_fFloorScale + fZOffset;
#if LANDSCAPE_USE_MORPHING
					pVertex->m_oPosition2 = pVertex->m_oPosition;
#endif // LANDSCAPE_USE_MORPHING
					pVertex->m_oNormal.x = 0.0f;
					pVertex->m_oNormal.y = 0.0f;
					pVertex->m_oNormal.z = 0.0f;
					pVertex->m_oUV.x = float(i) / float(m_oGlobalInfo.m_uVertexPerRowCount - 1);
					pVertex->m_oUV.y = float(j) / float(m_oGlobalInfo.m_uRowCount - 1);
					pVertex->m_fNormalizedHeight = 0.0f;
					pVertex->m_fNormalizedSlope = 0.0f;
					++pVertex;
				}
			}
			m_vVertexesIndependent.push_back(pVertexes);
			m_oGlobalInfo.m_pLODs[k].m_uIncrement = uLODIncrement;
			m_oGlobalInfo.m_pLODs[k].m_pVertexesIndependent = pVertexes;
			m_oGlobalInfo.m_pLODs[k].m_uVertexCount = uLODVertexCount;
			m_oGlobalInfo.m_pLODs[k].m_uVertexPerRowCount = uLODVertexPerRowCount;
			m_oGlobalInfo.m_pLODs[k].m_uRowCount = uLODRowCount;
			m_oGlobalInfo.m_pLODs[k].m_uNumVertices = uLODVertexPerRowCount * (m_oGlobalInfo.m_uQuadSize + 1) - (uLODVertexPerRowCount - (m_oGlobalInfo.m_uQuadSize + 1));
#if LANDSCAPE_USE_MORPHING
			//ComputeVertexIndependentMorphs(m_oGlobalInfo.m_pLODs[k]);
#endif // LANDSCAPE_USE_MORPHING
			//ComputeVertexIndependentNormals(m_oGlobalInfo.m_pLODs[k]);
		}
		return bResult;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	void Landscape::ComputeVertexIndependentMorphs(LODInfoRef _rLODInfo)
	{
		VertexIndependentPtr pVertex = _rLODInfo.m_pVertexesIndependent;
		VertexIndependentPtr pPrevVertex = NULL;
		for (unsigned int j = 0 ; m_oGlobalInfo.m_uRowCount > j ; j += _rLODInfo.m_uIncrement)
		{
			for (unsigned int i = 0 ; m_oGlobalInfo.m_uVertexPerRowCount > i ; i += _rLODInfo.m_uIncrement)
			{
				if (0 != (j % 2))
				{
					if (0 != (i % 2))
					{
						const int sPrevRowIndex = -int(_rLODInfo.m_uVertexPerRowCount) + 1;
						const int sNextRowIndex = int(_rLODInfo.m_uVertexPerRowCount) - 1;
#if LANDSCAPE_USE_MORPHING
						pVertex[0].m_oPosition2 = pVertex[sPrevRowIndex].m_oPosition + (pVertex[sNextRowIndex].m_oPosition - pVertex[sPrevRowIndex].m_oPosition) / 2.0f;
#endif // LANDSCAPE_USE_MORPHING
					}
					else
					{
						const int sPrevRowIndex = -int(_rLODInfo.m_uVertexPerRowCount);
						const int sNextRowIndex = int(_rLODInfo.m_uVertexPerRowCount);
#if LANDSCAPE_USE_MORPHING
						pVertex[0].m_oPosition2 = pVertex[sPrevRowIndex].m_oPosition + (pVertex[sNextRowIndex].m_oPosition - pVertex[sPrevRowIndex].m_oPosition) / 2.0f;
#endif // LANDSCAPE_USE_MORPHING
					}
				}
				else if (0 != (i % 2))
				{
#if LANDSCAPE_USE_MORPHING
					pVertex[0].m_oPosition2 = pVertex[-1].m_oPosition + (pVertex[1].m_oPosition - pVertex[-1].m_oPosition) / 2.0f;
					//pVertex[0].m_oPosition2.y += 50.0f; // for test only
#endif // LANDSCAPE_USE_MORPHING
				}
				pPrevVertex = pVertex;
				++pVertex;
			}
		}
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	void Landscape::ComputeVertexIndependentNormals(LODInfoRef _rLODInfo)
	{
		const int aIndexOffsets[6 * 2] =
		{
			0 - int(_rLODInfo.m_uVertexPerRowCount), -1,
			1 - int(_rLODInfo.m_uVertexPerRowCount), 0 - int(_rLODInfo.m_uVertexPerRowCount),
			1, 1 - int(_rLODInfo.m_uVertexPerRowCount),
			-1, 0 - int(_rLODInfo.m_uVertexPerRowCount),
			-1 + int(_rLODInfo.m_uVertexPerRowCount), 0 + int(_rLODInfo.m_uVertexPerRowCount),
			0 + int(_rLODInfo.m_uVertexPerRowCount), 1
		};
		const Vector3 oUp(0.0f, 1.0f, 0.0f);
		VertexIndependentPtr pVertex = _rLODInfo.m_pVertexesIndependent;
		for (int j = 0 ; int(_rLODInfo.m_uRowCount) > j ; ++j)
		{
			for (int i = 0 ; int(_rLODInfo.m_uVertexPerRowCount) > i ; ++i)
			{
				for (int k = 0 ; 6 > k ; ++k)
				{
					const int sIndex1 = i + j * int(_rLODInfo.m_uVertexPerRowCount) + aIndexOffsets[k * 2 + 0];
					const int sIndex2 = i + j * int(_rLODInfo.m_uVertexPerRowCount) + aIndexOffsets[k * 2 + 1];
					if ((0 <= sIndex1) && (int(_rLODInfo.m_uVertexCount) > sIndex1)
						&& (0 <= sIndex2) && (int(_rLODInfo.m_uVertexCount) > sIndex2))
					{
						Plane oPlane;
						D3DXPlaneFromPoints(&oPlane,
							&pVertex->m_oPosition,
							&_rLODInfo.m_pVertexesIndependent[sIndex1].m_oPosition,
							&_rLODInfo.m_pVertexesIndependent[sIndex2].m_oPosition);
						D3DXPlaneNormalize(&oPlane, &oPlane);
						pVertex->m_oNormal += Vector3(oPlane.a, oPlane.b, oPlane.c);
					}
				}
				D3DXVec3Normalize(&pVertex->m_oNormal, &pVertex->m_oNormal);
				pVertex->m_fNormalizedSlope = (1.0f + D3DXVec3Dot(&pVertex->m_oNormal, &oUp)) / 2.0f;
				++pVertex;
			}
		}
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	bool Landscape::CreateVertexBufferDefault()
	{
		bool bResult = false;
		for (unsigned int k = 0 ; m_oGlobalInfo.m_uLODCount > k ; ++k)
		{
			const unsigned int uLODVertexPerRowCount = (m_oGlobalInfo.m_uVertexPerRowCount >> k) | 0x00000001;
			const unsigned int uLODRowCount = (m_oGlobalInfo.m_uRowCount >> k) | 0x00000001;
			const unsigned int uLODVertexCount = uLODVertexPerRowCount * uLODRowCount;
			DisplayVertexBuffer::CreateInfo oVBCreateInfo = { uLODVertexCount * sizeof(VertexDefault), sizeof(VertexDefault), VertexDefault::s_VertexElement };
			DisplayVertexBufferPtr pVertexBuffer = m_rDisplay.CreateVertexBuffer(oVBCreateInfo);
			bResult = (NULL != pVertexBuffer);
			if (false != bResult)
			{
				const unsigned int uLODIncrement = 0x00000001 << k;
				const float fXOffset = float(m_oGlobalInfo.m_uVertexPerRowCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
				const float fZOffset = float(m_oGlobalInfo.m_uRowCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
				VoidPtr pVertexes = new VertexDefault[uLODVertexCount];
				VertexDefaultPtr pVertex = (VertexDefaultPtr)pVertexes;
				VertexIndependentPtr pVertexIndependent = m_oGlobalInfo.m_pLODs[0].m_pVertexesIndependent;
				for (unsigned int j = 0 ; m_oGlobalInfo.m_uRowCount > j ; j += uLODIncrement)
				{
					for (unsigned int i = 0 ; m_oGlobalInfo.m_uVertexPerRowCount > i ; i += uLODIncrement)
					{
						const unsigned int uLOD0Index = i + j * m_oGlobalInfo.m_uVertexPerRowCount;
						const unsigned int uLODkIndex = (i / uLODIncrement) + (j / uLODIncrement) * m_oGlobalInfo.m_pLODs[k].m_uVertexPerRowCount;
						*pVertex = pVertexIndependent[uLOD0Index];
						const float fLODColor = 1.0f; //0.25f + (float(k) / float(m_oGlobalInfo.m_uLODCount - 1)) * 0.75f;
						pVertex->m_oColor.x = fLODColor;
						pVertex->m_oColor.y = fLODColor;
						pVertex->m_oColor.z = fLODColor;
						pVertex->m_oColor.w = 1.0f;
						pVertexIndependent[uLOD0Index].AddLink(k, uLODkIndex);
						++pVertex;
					}
				}
				bResult = pVertexBuffer->Set(pVertexes);
				if (false == bResult)
				{
					m_rDisplay.ReleaseVertexBuffer(pVertexBuffer);
					delete[] pVertexes;
					break;
				}
				m_vVertexBuffers.push_back(pVertexBuffer);
				m_vVertexes.push_back(pVertexes);
				m_oGlobalInfo.m_pLODs[k].m_pVertexes = pVertexes;
				m_oGlobalInfo.m_pLODs[k].m_pVertexBuffer = pVertexBuffer;
			}
			else
			{
				break;
			}
		}
		return bResult;
	}

	bool Landscape::CreateVertexBufferLiquid()
	{
		bool bResult = false;
		for (unsigned int k = 0 ; m_oGlobalInfo.m_uLODCount > k ; ++k)
		{
			const unsigned int uLODVertexPerRowCount = (m_oGlobalInfo.m_uVertexPerRowCount >> k) | 0x00000001;
			const unsigned int uLODRowCount = (m_oGlobalInfo.m_uRowCount >> k) | 0x00000001;
			const unsigned int uLODVertexCount = uLODVertexPerRowCount * uLODRowCount;
			DisplayVertexBuffer::CreateInfo oVBCreateInfo = { uLODVertexCount * sizeof(VertexLiquid), sizeof(VertexLiquid), VertexLiquid::s_VertexElement };
			DisplayVertexBufferPtr pVertexBuffer = m_rDisplay.CreateVertexBuffer(oVBCreateInfo);
			bResult = (NULL != pVertexBuffer);
			if (false != bResult)
			{
				const unsigned int uLODIncrement = 0x00000001 << k;
				const float fXOffset = float(m_oGlobalInfo.m_uVertexPerRowCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
				const float fZOffset = float(m_oGlobalInfo.m_uRowCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
				VoidPtr pVertexes = new VertexLiquid[uLODVertexCount];
				VertexLiquidPtr pVertex = (VertexLiquidPtr)pVertexes;
				VertexIndependentPtr pVertexIndependent = m_oGlobalInfo.m_pLODs[0].m_pVertexesIndependent;
				for (unsigned int j = 0 ; m_oGlobalInfo.m_uRowCount > j ; j += uLODIncrement)
				{
					for (unsigned int i = 0 ; m_oGlobalInfo.m_uVertexPerRowCount > i ; i += uLODIncrement)
					{
						const unsigned int uLOD0Index = i + j * m_oGlobalInfo.m_uVertexPerRowCount;
						const unsigned int uLODkIndex = (i / uLODIncrement) + (j / uLODIncrement) * m_oGlobalInfo.m_pLODs[k].m_uVertexPerRowCount;
						*pVertex = pVertexIndependent[uLOD0Index];
						pVertex->m_oTangent.x = 0.0f;
						pVertex->m_oTangent.y = 0.0f;
						pVertex->m_oTangent.z = 0.0f;
						pVertexIndependent[uLOD0Index].AddLink(k, uLODkIndex);
						++pVertex;
					}
				}
				bResult = pVertexBuffer->Set(pVertexes);
				if (false == bResult)
				{
					m_rDisplay.ReleaseVertexBuffer(pVertexBuffer);
					delete[] pVertexes;
					break;
				}
				m_vVertexBuffers.push_back(pVertexBuffer);
				m_vVertexes.push_back(pVertexes);
				m_oGlobalInfo.m_pLODs[k].m_pVertexes = pVertexes;
				m_oGlobalInfo.m_pLODs[k].m_pVertexBuffer = pVertexBuffer;
			}
			else
			{
				break;
			}
		}
		return bResult;
	}
}
