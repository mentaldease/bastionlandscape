#include "stdafx.h"
#include "Landscape.h"
#include "../Display/Camera.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	LandscapeVertexDefaultRef LandscapeVertexDefault::operator = (LandscapeVertexIndependentRef _rVertexIndependent)
	{
		m_f3Position = _rVertexIndependent.m_f3Position;
#if LANDSCAPE_USE_MORPHING
		m_f3Position2 = _rVertexIndependent.m_f3Position2;
#endif // LANDSCAPE_USE_MORPHING
		m_f3Normal = _rVertexIndependent.m_f3Normal;
		m_f2UV = _rVertexIndependent.m_f2UV;
		m_f3UV2.x = _rVertexIndependent.m_fNormalizedSlope;
		m_f3UV2.y = _rVertexIndependent.m_fNormalizedHeight;
		m_f3UV2.z = float(_rVertexIndependent.m_uWaterLevel);
		return *this;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	LandscapeVertexLiquidRef LandscapeVertexLiquid::operator = (LandscapeVertexIndependentRef _rVertexIndependent)
	{
		m_f3Position = _rVertexIndependent.m_f3Position;
		m_f3Normal = _rVertexIndependent.m_f3Normal;
		m_f2UV = _rVertexIndependent.m_f2UV;
		return *this;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	void LandscapeVertexIndependent::AddLink(const UInt& _uLOD, const UInt& _uIndex)
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
		for (UInt k = 0 ; m_oGlobalInfo.m_uLODCount > k ; ++k)
		{
			const UInt uLODVertexPerRowCount = (m_oGlobalInfo.m_uVertexPerRowCount >> k) | 0x00000001;
			const UInt uLODRowCount = (m_oGlobalInfo.m_uRowCount >> k) | 0x00000001;
			const UInt uLODVertexCount = uLODVertexPerRowCount * uLODRowCount;
			const UInt uLODIncrement = 0x00000001 << k;
			const float fXOffset = float(m_oGlobalInfo.m_uVertexPerRowCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
			const float fZOffset = float(m_oGlobalInfo.m_uRowCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
			LandscapeVertexIndependentPtr pVertexes = new LandscapeVertexIndependent[uLODVertexCount];
			LandscapeVertexIndependentPtr pVertex = pVertexes;
			for (UInt j = 0 ; m_oGlobalInfo.m_uRowCount > j ; j += uLODIncrement)
			{
				for (UInt i = 0 ; m_oGlobalInfo.m_uVertexPerRowCount > i ; i += uLODIncrement)
				{
					pVertex->m_f3Position.x = float(i) * m_oGlobalInfo.m_fFloorScale - fXOffset;
					pVertex->m_f3Position.y = 0.0f;
					pVertex->m_f3Position.z = -float(j) * m_oGlobalInfo.m_fFloorScale + fZOffset;
#if LANDSCAPE_USE_MORPHING
					pVertex->m_f3Position2 = pVertex->m_f3Position;
#endif // LANDSCAPE_USE_MORPHING
					pVertex->m_f3Normal.x = 0.0f;
					pVertex->m_f3Normal.y = 0.0f;
					pVertex->m_f3Normal.z = 0.0f;
					pVertex->m_f2UV.x = float(i) / float(m_oGlobalInfo.m_uVertexPerRowCount - 1);
					pVertex->m_f2UV.y = float(j) / float(m_oGlobalInfo.m_uRowCount - 1);
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
		LandscapeVertexIndependentPtr pVertex = _rLODInfo.m_pVertexesIndependent;
		LandscapeVertexIndependentPtr pPrevVertex = NULL;
		for (UInt j = 0 ; m_oGlobalInfo.m_uRowCount > j ; j += _rLODInfo.m_uIncrement)
		{
			for (UInt i = 0 ; m_oGlobalInfo.m_uVertexPerRowCount > i ; i += _rLODInfo.m_uIncrement)
			{
				if (0 != (j % 2))
				{
					if (0 != (i % 2))
					{
						const int sPrevRowIndex = -int(_rLODInfo.m_uVertexPerRowCount) + 1;
						const int sNextRowIndex = int(_rLODInfo.m_uVertexPerRowCount) - 1;
#if LANDSCAPE_USE_MORPHING
						pVertex[0].m_f3Position2 = pVertex[sPrevRowIndex].m_f3Position + (pVertex[sNextRowIndex].m_f3Position - pVertex[sPrevRowIndex].m_f3Position) / 2.0f;
#endif // LANDSCAPE_USE_MORPHING
					}
					else
					{
						const int sPrevRowIndex = -int(_rLODInfo.m_uVertexPerRowCount);
						const int sNextRowIndex = int(_rLODInfo.m_uVertexPerRowCount);
#if LANDSCAPE_USE_MORPHING
						pVertex[0].m_f3Position2 = pVertex[sPrevRowIndex].m_f3Position + (pVertex[sNextRowIndex].m_f3Position - pVertex[sPrevRowIndex].m_f3Position) / 2.0f;
#endif // LANDSCAPE_USE_MORPHING
					}
				}
				else if (0 != (i % 2))
				{
#if LANDSCAPE_USE_MORPHING
					pVertex[0].m_f3Position2 = pVertex[-1].m_f3Position + (pVertex[1].m_f3Position - pVertex[-1].m_f3Position) / 2.0f;
					//pVertex[0].m_f3Position2.y += 50.0f; // for test only
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
		LandscapeVertexIndependentPtr pVertex = _rLODInfo.m_pVertexesIndependent;
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
							&pVertex->m_f3Position,
							&_rLODInfo.m_pVertexesIndependent[sIndex1].m_f3Position,
							&_rLODInfo.m_pVertexesIndependent[sIndex2].m_f3Position);
						D3DXPlaneNormalize(&oPlane, &oPlane);
						pVertex->m_f3Normal += Vector3(oPlane.a, oPlane.b, oPlane.c);
					}
				}
				D3DXVec3Normalize(&pVertex->m_f3Normal, &pVertex->m_f3Normal);
				pVertex->m_fNormalizedSlope = (1.0f + D3DXVec3Dot(&pVertex->m_f3Normal, &oUp)) / 2.0f;
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
		for (UInt k = 0 ; m_oGlobalInfo.m_uLODCount > k ; ++k)
		{
			const UInt uLODVertexPerRowCount = (m_oGlobalInfo.m_uVertexPerRowCount >> k) | 0x00000001;
			const UInt uLODRowCount = (m_oGlobalInfo.m_uRowCount >> k) | 0x00000001;
			const UInt uLODVertexCount = uLODVertexPerRowCount * uLODRowCount;
			DisplayVertexBuffer::CreateInfo oVBCreateInfo = { uLODVertexCount * sizeof(LandscapeVertexDefault), sizeof(LandscapeVertexDefault), LandscapeVertexDefault::s_VertexElement };
			Key uVertexBuffer = m_pDisplay->CreateVertexBufferKey(oVBCreateInfo);
			bResult = (NULL != uVertexBuffer);
			if (false != bResult)
			{
				const UInt uLODIncrement = 0x00000001 << k;
				const float fXOffset = float(m_oGlobalInfo.m_uVertexPerRowCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
				const float fZOffset = float(m_oGlobalInfo.m_uRowCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
				VoidPtr pVertexes = new LandscapeVertexDefault[uLODVertexCount];
				LandscapeVertexDefaultPtr pVertex = (LandscapeVertexDefaultPtr)pVertexes;
				LandscapeVertexIndependentPtr pVertexIndependent = m_oGlobalInfo.m_pLODs[0].m_pVertexesIndependent;
				for (UInt j = 0 ; m_oGlobalInfo.m_uRowCount > j ; j += uLODIncrement)
				{
					for (UInt i = 0 ; m_oGlobalInfo.m_uVertexPerRowCount > i ; i += uLODIncrement)
					{
						const UInt uLOD0Index = i + j * m_oGlobalInfo.m_uVertexPerRowCount;
						const UInt uLODkIndex = (i / uLODIncrement) + (j / uLODIncrement) * m_oGlobalInfo.m_pLODs[k].m_uVertexPerRowCount;
						*pVertex = pVertexIndependent[uLOD0Index];
						const float fLODColor = 1.0f; //0.25f + (float(k) / float(m_oGlobalInfo.m_uLODCount - 1)) * 0.75f;
						pVertex->m_f4Color.x = fLODColor;
						pVertex->m_f4Color.y = fLODColor;
						pVertex->m_f4Color.z = fLODColor;
						pVertex->m_f4Color.w = 1.0f;
						pVertexIndependent[uLOD0Index].AddLink(k, uLODkIndex);
						++pVertex;
					}
				}
				m_pDisplay->SetVertexBufferKeyData(uVertexBuffer, pVertexes);
				if (false == bResult)
				{
					m_pDisplay->ReleaseVertexBufferKey(uVertexBuffer);
					delete[] pVertexes;
					break;
				}
				m_vVertexBuffers.push_back(uVertexBuffer);
				m_vVertexes.push_back(pVertexes);
				m_oGlobalInfo.m_pLODs[k].m_pVertexes = pVertexes;
				m_oGlobalInfo.m_pLODs[k].m_uVertexBuffer = uVertexBuffer;
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
		for (UInt k = 0 ; m_oGlobalInfo.m_uLODCount > k ; ++k)
		{
			const UInt uLODVertexPerRowCount = (m_oGlobalInfo.m_uVertexPerRowCount >> k) | 0x00000001;
			const UInt uLODRowCount = (m_oGlobalInfo.m_uRowCount >> k) | 0x00000001;
			const UInt uLODVertexCount = uLODVertexPerRowCount * uLODRowCount;
			DisplayVertexBuffer::CreateInfo oVBCreateInfo = { uLODVertexCount * sizeof(LandscapeVertexLiquid), sizeof(LandscapeVertexLiquid), LandscapeVertexLiquid::s_VertexElement };
			Key uVertexBuffer = m_pDisplay->CreateVertexBufferKey(oVBCreateInfo);
			bResult = (NULL != uVertexBuffer);
			if (false != bResult)
			{
				const UInt uLODIncrement = 0x00000001 << k;
				const float fXOffset = float(m_oGlobalInfo.m_uVertexPerRowCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
				const float fZOffset = float(m_oGlobalInfo.m_uRowCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
				VoidPtr pVertexes = new LandscapeVertexLiquid[uLODVertexCount];
				LandscapeVertexLiquidPtr pVertex = (LandscapeVertexLiquidPtr)pVertexes;
				LandscapeVertexIndependentPtr pVertexIndependent = m_oGlobalInfo.m_pLODs[0].m_pVertexesIndependent;
				for (UInt j = 0 ; m_oGlobalInfo.m_uRowCount > j ; j += uLODIncrement)
				{
					for (UInt i = 0 ; m_oGlobalInfo.m_uVertexPerRowCount > i ; i += uLODIncrement)
					{
						const UInt uLOD0Index = i + j * m_oGlobalInfo.m_uVertexPerRowCount;
						const UInt uLODkIndex = (i / uLODIncrement) + (j / uLODIncrement) * m_oGlobalInfo.m_pLODs[k].m_uVertexPerRowCount;
						*pVertex = pVertexIndependent[uLOD0Index];
						pVertex->m_f3Tangent.x = 0.0f;
						pVertex->m_f3Tangent.y = 0.0f;
						pVertex->m_f3Tangent.z = 0.0f;
						pVertexIndependent[uLOD0Index].AddLink(k, uLODkIndex);
						++pVertex;
					}
				}
				m_pDisplay->SetVertexBufferKeyData(uVertexBuffer, pVertexes);
				if (false == bResult)
				{
					m_pDisplay->ReleaseVertexBufferKey(uVertexBuffer);
					delete[] pVertexes;
					break;
				}
				m_vVertexBuffers.push_back(uVertexBuffer);
				m_vVertexes.push_back(pVertexes);
				m_oGlobalInfo.m_pLODs[k].m_pVertexes = pVertexes;
				m_oGlobalInfo.m_pLODs[k].m_uVertexBuffer = uVertexBuffer;
			}
			else
			{
				break;
			}
		}
		return bResult;
	}
}
