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

	Key LandscapeChunk::s_uMorphFactorKey = MakeKey(string("MORPHFACTOR"));

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	LandscapeChunk::LandscapeChunk(Landscape& _rLandscape, OctreeRef _rOctree, const unsigned int& _uLOD)
	:	DisplayObject(),
		OctreeObject(_rOctree),
		m_rLandscape(_rLandscape),
		m_uStartVertexIndex(0),
		m_uLOD(_uLOD),
		m_pParent(NULL),
		m_oCenter(0.0f, 0.0f, 0.0f),
		m_oExtends(0.0f, 0.0f, 0.0f),
		m_pLODInfo(NULL),
		m_fMorphFactor(1.0f),
		m_uIndexX(0),
		m_uIndexZ(0)
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
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = (NULL != pInfo);

		if (false != bResult)
		{
			Release();
			const Landscape::GlobalInfo& rGlobalInfo = m_rLandscape.GetGlobalInfo();
			m_pLODInfo = &rGlobalInfo.m_pLODs[m_uLOD];
			m_uIndexX = pInfo->m_uX * rGlobalInfo.m_uQuadSize;
			m_uIndexZ = pInfo->m_uZ * rGlobalInfo.m_uQuadSize;
			//{
			//	wchar_t wszBuffer[1024];
			//	wsprintf(wszBuffer, L"%d;%d;%d\n", m_uIndexX, m_uIndexZ, m_uLOD);
			//	OutputDebugString(wszBuffer);
			//}
			m_uStartVertexIndex = m_uIndexX + m_uIndexZ * m_pLODInfo->m_uVertexPerRowCount;

			// center and extend
			const unsigned int uStartIndex = m_pLODInfo->m_uStartIndex;
			const unsigned int uStripSize = m_pLODInfo->m_uStripSize;
			Vector3 oAABB[2] =
			{
				Vector3( FLT_MAX, FLT_MAX, FLT_MAX ),
				Vector3( -FLT_MAX, -FLT_MAX, -FLT_MAX )
			};
			Vector3 oTemp;
			for (unsigned int i = 0 ; uStripSize > i ; ++i)
			{
				m_rLandscape.GetVertexPosition(*m_pLODInfo, i, m_uStartVertexIndex, oTemp);
				if (oAABB[0].x > oTemp.x) oAABB[0].x = oTemp.x;
				if (oAABB[0].y > oTemp.y) oAABB[0].y = oTemp.y;
				if (oAABB[0].z > oTemp.z) oAABB[0].z = oTemp.z;
				if (oAABB[1].x < oTemp.x) oAABB[1].x = oTemp.x;
				if (oAABB[1].y < oTemp.y) oAABB[1].y = oTemp.y;
				if (oAABB[1].z < oTemp.z) oAABB[1].z = oTemp.z;
			}
			m_oExtends = (oAABB[1] - oAABB[0]) / 2.0f;
			m_oCenter = oAABB[0] + m_oExtends;

			if (0 < m_uLOD)
			{
				CreateInfo oLCCInfo;
				unsigned int uChild = ESubChild_NORTHWEST;
				for (unsigned int j = 0 ; 2 > j ; ++j)
				{
					for (unsigned int i = 0 ; 2 > i ; ++i)
					{
						LandscapeChunkPtr pLandscapeChunk = new LandscapeChunk(m_rLandscape, m_rOctree, m_uLOD - 1);
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
					if (false == bResult)
					{
						break;
					}
				}
			}
			else
			{
				SetAABB(fsVector3(oAABB[1].x, oAABB[1].y, oAABB[1].z), fsVector3(oAABB[0].x, oAABB[0].y, oAABB[0].z));
				m_rOctree.AddObject(this);
			}
		}

		return bResult;
	}

	void LandscapeChunk::Update()
	{

	}

	void LandscapeChunk::Release()
	{
		for (int i = 0 ; ESubChild_COUNT > i ; ++i)
		{
			if (NULL != m_pChildren[i])
			{
				m_pChildren[i]->Release();
				delete m_pChildren[i];
				m_pChildren[i] = NULL;
			}
		}

		if (0 == m_uLOD)
		{
			m_rOctree.RemoveObject(this);
		}
	}

	void LandscapeChunk::RenderBegin()
	{
		Display::GetInstance()->GetMaterialManager()->SetFloatBySemantic(s_uMorphFactorKey, &m_fMorphFactor);
		m_rLandscape.UseLayering();
	}

	void LandscapeChunk::Render()
	{
		if (m_rLandscape.UseLODVertexBuffer(m_uLOD) && m_rLandscape.SetIndices())
		{
			const unsigned int uVertexCount = m_pLODInfo->m_uNumVertices;
			const unsigned int uStartIndex = m_pLODInfo->m_uStartIndex;
			const unsigned int uStripSize = m_pLODInfo->m_uStripSize - 2;
			Display::GetInstance()->GetDevicePtr()->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, m_uStartVertexIndex, 0, uVertexCount, uStartIndex, uStripSize);
		}
	}

	void LandscapeChunk::Traverse(LandscapeChunkPtrVecRef _rRenderList, const Vector3& _rCamPos, const float& _fPixelSize)
	{
		const Landscape::GlobalInfo& rGlobalInfo = m_rLandscape.GetGlobalInfo();
		//MatrixPtr pWorld = m_rLandscape.GetWorldMatrix();
		//const Vector3 oWorld(pWorld->_41, pWorld->_42, pWorld->_43);
		const Vector3 oWorld(m_oWorld._41, m_oWorld._42, m_oWorld._43);
		const Vector3 oDelta = oWorld + m_oCenter - _rCamPos;
		const float fExtends = D3DXVec3Length(&m_oExtends);

		if (DisplayCamera::ECollision_OUT != Display::GetInstance()->GetCurrentCamera()->CollisionWithSphere(oWorld + m_oCenter, fExtends))
		{
#if LANDSCAPE_USE_HIGHEST_LOD_ONLY
			if (0 == m_uLOD)
#else // LANDSCAPE_USE_HIGHEST_LOD_ONLY
#if 0
			Vector3 aPoints[8];
			aPoints[0] = oDelta + Vector3(m_oExtends.x, m_oExtends.y, m_oExtends.z);
			aPoints[1] = oDelta + Vector3(-m_oExtends.x, -m_oExtends.y, -m_oExtends.z);
			aPoints[2] = oDelta + Vector3(-m_oExtends.x, m_oExtends.y, m_oExtends.z);
			aPoints[3] = oDelta + Vector3(m_oExtends.x, -m_oExtends.y, -m_oExtends.z);
			aPoints[4] = oDelta + Vector3(m_oExtends.x, -m_oExtends.y, m_oExtends.z);
			aPoints[5] = oDelta + Vector3(-m_oExtends.x, m_oExtends.y, -m_oExtends.z);
			aPoints[6] = oDelta + Vector3(m_oExtends.x, m_oExtends.y, -m_oExtends.z);
			aPoints[7] = oDelta + Vector3(-m_oExtends.x, -m_oExtends.y, m_oExtends.z);
			float fDistance = FLT_MAX;
			for (unsigned int i = 0 ; 8 > i ; ++i)
			{
				const float fDelta = D3DXVec3Length(&aPoints[i]);
				fDistance = (fDistance > fDelta) ? fDelta : fDistance;
			}
			fDistance = (1.0f <= fDistance) ? fDistance : 1.0f;
#else
			const float fDelta = D3DXVec3Length(&oDelta);
			const float fRowDistance = (fDelta - fExtends);
			const float fDistance = (1.0f <= fRowDistance) ? fRowDistance : 1.0f;
#endif
			const float fVertexErrorLevel = (m_pLODInfo->m_uGeometricError / fDistance) * _fPixelSize;

			if (fVertexErrorLevel <= rGlobalInfo.m_fPixelErrorMax)
				//if ((rGlobalInfo.m_fPixelErrorMax <= fRatio) || (0 == m_uLOD))
				//const float fRatio = fDistance / fExtends;
				//if ((1.0f <= fRatio) || (0 == m_uLOD))
#endif
			{
				//m_fMorphFactor = ((2.0f * fVertexErrorLevel) / rGlobalInfo.m_fPixelErrorMax) - 1.0f;
				//m_fMorphFactor = (0.0f != fVertexErrorLevel) ? m_fMorphFactor : 1.0f;
				//m_fMorphFactor = (0.0f > m_fMorphFactor) ? 0.0f : ((1.0f < m_fMorphFactor) ? 1.0f : m_fMorphFactor);
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
		else
		{
			++m_rLandscape.m_uOutOfFrustum;
		}
	}

	unsigned int LandscapeChunk::GetLODID() const
	{
		return m_uLOD;
	}
}
