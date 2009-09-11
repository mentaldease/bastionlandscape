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
		m_oPosition2 = _rVertexIndependent.m_oPosition2;
		m_oNormal = _rVertexIndependent.m_oNormal;
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
			const unsigned int uLODVertexPerRawCount = (m_oGlobalInfo.m_uVertexPerRawCount >> k) | 0x00000001;
			const unsigned int uLODRawCount = (m_oGlobalInfo.m_uRawCount >> k) | 0x00000001;
			const unsigned int uLODVertexCount = uLODVertexPerRawCount * uLODRawCount;
			const unsigned int uLODIncrement = 0x00000001 << k;
			const float fXOffset = float(m_oGlobalInfo.m_uVertexPerRawCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
			const float fZOffset = float(m_oGlobalInfo.m_uRawCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
			VertexIndependentPtr pVertexes = new VertexIndependent[uLODVertexCount];
			VertexIndependentPtr pVertex = pVertexes;
			for (unsigned int j = 0 ; m_oGlobalInfo.m_uRawCount > j ; j += uLODIncrement)
			{
				for (unsigned int i = 0 ; m_oGlobalInfo.m_uVertexPerRawCount > i ; i += uLODIncrement)
				{
					pVertex->m_oPosition.x = float(i) * m_oGlobalInfo.m_fFloorScale - fXOffset;
					pVertex->m_oPosition.y = 0.0f;
					pVertex->m_oPosition.z = -float(j) * m_oGlobalInfo.m_fFloorScale + fZOffset;
					pVertex->m_oPosition2 = pVertex->m_oPosition;
					pVertex->m_oNormal.x = 0.0f;
					pVertex->m_oNormal.y = 1.0f;
					pVertex->m_oNormal.z = 0.0f;
					++pVertex;
				}
			}
			ComputeVertexIndependentMorph(pVertexes, uLODIncrement, uLODVertexPerRawCount);
			//pVertex = (VertexIndependentPtr)pVertexes;
			//VertexIndependentPtr pPrevVertex = NULL;
			//int sVertexIndex = 0;
			//for (unsigned int j = 0 ; m_oGlobalInfo.m_uRawCount > j ; j += uLODIncrement)
			//{
			//	for (unsigned int i = 0 ; m_oGlobalInfo.m_uVertexPerRawCount > i ; i += uLODIncrement, ++sVertexIndex)
			//	{
			//		if (0 != (j % 2))
			//		{
			//			if (0 != (i % 2))
			//			{
			//				const int sPrevRawIndex = -int(uLODVertexPerRawCount) + 1;
			//				const int sNextRawIndex = int(uLODVertexPerRawCount) - 1;
			//				pVertex[0].m_oPosition2 = pVertex[sPrevRawIndex].m_oPosition + (pVertex[sNextRawIndex].m_oPosition - pVertex[sPrevRawIndex].m_oPosition) / 2.0f;
			//			}
			//			else
			//			{
			//				const int sPrevRawIndex = -int(uLODVertexPerRawCount);
			//				const int sNextRawIndex = int(uLODVertexPerRawCount);
			//				pVertex[0].m_oPosition2 = pVertex[sPrevRawIndex].m_oPosition + (pVertex[sNextRawIndex].m_oPosition - pVertex[sPrevRawIndex].m_oPosition) / 2.0f;
			//			}
			//		}
			//		else if (0 != (i % 2))
			//		{
			//			pVertex[0].m_oPosition2 = pVertex[-1].m_oPosition + (pVertex[1].m_oPosition - pVertex[-1].m_oPosition) / 2.0f;
			//			//pVertex[0].m_oPosition2.y += 50.0f; // for test only
			//		}
			//		pPrevVertex = pVertex;
			//		++pVertex;
			//	}
			//}
			m_vVertexesIndependent.push_back(pVertexes);
			m_oGlobalInfo.m_pLODs[k].m_pVertexesIndependent = pVertexes;
			m_oGlobalInfo.m_pLODs[k].m_uVertexCount = uLODVertexCount;
			m_oGlobalInfo.m_pLODs[k].m_uVertexPerRawCount = uLODVertexPerRawCount;
			m_oGlobalInfo.m_pLODs[k].m_uNumVertices = uLODVertexPerRawCount * (m_oGlobalInfo.m_uQuadSize + 1);
		}
		return bResult;
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	void Landscape::ComputeVertexIndependentMorph(VertexIndependentPtr _pVertexes, const unsigned int _uLODIncrement, const unsigned int _uLODVertexPerRawCount)
	{
		VertexIndependentPtr pVertex = _pVertexes;
		VertexIndependentPtr pPrevVertex = NULL;
		int sVertexIndex = 0;
		for (unsigned int j = 0 ; m_oGlobalInfo.m_uRawCount > j ; j += _uLODIncrement)
		{
			for (unsigned int i = 0 ; m_oGlobalInfo.m_uVertexPerRawCount > i ; i += _uLODIncrement, ++sVertexIndex)
			{
				if (0 != (j % 2))
				{
					if (0 != (i % 2))
					{
						const int sPrevRawIndex = -int(_uLODVertexPerRawCount) + 1;
						const int sNextRawIndex = int(_uLODVertexPerRawCount) - 1;
						pVertex[0].m_oPosition2 = pVertex[sPrevRawIndex].m_oPosition + (pVertex[sNextRawIndex].m_oPosition - pVertex[sPrevRawIndex].m_oPosition) / 2.0f;
					}
					else
					{
						const int sPrevRawIndex = -int(_uLODVertexPerRawCount);
						const int sNextRawIndex = int(_uLODVertexPerRawCount);
						pVertex[0].m_oPosition2 = pVertex[sPrevRawIndex].m_oPosition + (pVertex[sNextRawIndex].m_oPosition - pVertex[sPrevRawIndex].m_oPosition) / 2.0f;
					}
				}
				else if (0 != (i % 2))
				{
					pVertex[0].m_oPosition2 = pVertex[-1].m_oPosition + (pVertex[1].m_oPosition - pVertex[-1].m_oPosition) / 2.0f;
					//pVertex[0].m_oPosition2.y += 50.0f; // for test only
				}
				pPrevVertex = pVertex;
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
			const unsigned int uLODVertexPerRawCount = (m_oGlobalInfo.m_uVertexPerRawCount >> k) | 0x00000001;
			const unsigned int uLODRawCount = (m_oGlobalInfo.m_uRawCount >> k) | 0x00000001;
			const unsigned int uLODVertexCount = uLODVertexPerRawCount * uLODRawCount;
			DisplayVertexBuffer::CreateInfo oVBCreateInfo = { uLODVertexCount * sizeof(VertexDefault), sizeof(VertexDefault), VertexDefault::s_VertexElement };
			DisplayVertexBufferPtr pVertexBuffer = m_rDisplay.CreateVertexBuffer(oVBCreateInfo);
			bResult = (NULL != pVertexBuffer);
			if (false != bResult)
			{
				const unsigned int uLODIncrement = 0x00000001 << k;
				const float fXOffset = float(m_oGlobalInfo.m_uVertexPerRawCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
				const float fZOffset = float(m_oGlobalInfo.m_uRawCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
				VoidPtr pVertexes = new VertexDefault[uLODVertexCount];
				VertexDefaultPtr pVertex = (VertexDefaultPtr)pVertexes;
				VertexIndependentPtr pVertexIndependent = m_oGlobalInfo.m_pLODs[0].m_pVertexesIndependent;
				for (unsigned int j = 0 ; m_oGlobalInfo.m_uRawCount > j ; j += uLODIncrement)
				{
					for (unsigned int i = 0 ; m_oGlobalInfo.m_uVertexPerRawCount > i ; i += uLODIncrement)
					{
						const unsigned int uLOD0Index = i + j * m_oGlobalInfo.m_uVertexPerRawCount;
						const unsigned int uLODkIndex = (i / uLODIncrement) + (j / uLODIncrement) * m_oGlobalInfo.m_pLODs[k].m_uVertexPerRawCount;
						*pVertex = pVertexIndependent[uLOD0Index];
						const float fLODColor = 0.25f + (float(k) / float(m_oGlobalInfo.m_uLODCount - 1)) * 0.75f;
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
			const unsigned int uLODVertexPerRawCount = (m_oGlobalInfo.m_uVertexPerRawCount >> k) | 0x00000001;
			const unsigned int uLODRawCount = (m_oGlobalInfo.m_uRawCount >> k) | 0x00000001;
			const unsigned int uLODVertexCount = uLODVertexPerRawCount * uLODRawCount;
			DisplayVertexBuffer::CreateInfo oVBCreateInfo = { uLODVertexCount * sizeof(VertexLiquid), sizeof(VertexLiquid), VertexLiquid::s_VertexElement };
			DisplayVertexBufferPtr pVertexBuffer = m_rDisplay.CreateVertexBuffer(oVBCreateInfo);
			bResult = (NULL != pVertexBuffer);
			if (false != bResult)
			{
				const unsigned int uLODIncrement = 0x00000001 << k;
				const float fXOffset = float(m_oGlobalInfo.m_uVertexPerRawCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
				const float fZOffset = float(m_oGlobalInfo.m_uRawCount - 1) * m_oGlobalInfo.m_fFloorScale / 2.0f;
				VoidPtr pVertexes = new VertexLiquid[uLODVertexCount];
				VertexLiquidPtr pVertex = (VertexLiquidPtr)pVertexes;
				VertexIndependentPtr pVertexIndependent = m_oGlobalInfo.m_pLODs[0].m_pVertexesIndependent;
				for (unsigned int j = 0 ; m_oGlobalInfo.m_uRawCount > j ; j += uLODIncrement)
				{
					for (unsigned int i = 0 ; m_oGlobalInfo.m_uVertexPerRawCount > i ; i += uLODIncrement)
					{
						const unsigned int uLOD0Index = i + j * m_oGlobalInfo.m_uVertexPerRawCount;
						const unsigned int uLODkIndex = (i / uLODIncrement) + (j / uLODIncrement) * m_oGlobalInfo.m_pLODs[k].m_uVertexPerRawCount;
						*pVertex = pVertexIndependent[uLOD0Index];
						pVertex->m_oTangent.x = 0.0f;
						pVertex->m_oTangent.y = 0.0f;
						pVertex->m_oTangent.z = 0.0f;
						pVertex->m_oUV.x = float(i) / float(m_oGlobalInfo.m_uVertexPerRawCount - 1);
						pVertex->m_oUV.y = float(j) / float(m_oGlobalInfo.m_uRawCount - 1);
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
