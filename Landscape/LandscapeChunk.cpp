#include "stdafx.h"
#include "Landscape.h"
#include "../Display/Camera.h"
#include "../Display/Effect.h"
#include "../Display/EffectParam.h"
#include "../Display/PointInTriangleTester.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Key LandscapeChunk::s_uMorphFactorKey = MakeKey(string("MORPHFACTOR"));

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct LVIAccessor // Landscape Vertex Independent Accessor
	{
		LVIAccessor(Landscape::LODInfoRef _rLODInfo, const UIntPtr _pIndexes, const UInt _uVertexStartIndex)
		:	m_rLODInfo(_rLODInfo),
			m_pIndexes(_pIndexes),
			m_uVertexStartIndex(_uVertexStartIndex)
		{

		}

		LandscapeVertexIndependentPtr Get(const UInt _uIndex)
		{
			const UInt uVertexIndex = m_uVertexStartIndex + m_pIndexes[m_rLODInfo.m_uStartIndex + _uIndex];
			return m_rLODInfo.m_pVertexesIndependent + uVertexIndex;
		}

		Landscape::LODInfoRef	m_rLODInfo;
		const UIntPtr			m_pIndexes;
		UInt					m_uVertexStartIndex;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	LandscapeChunk::LandscapeChunk(Landscape& _rLandscape, OctreeRef _rOctree, const UInt& _uLOD)
	:	DisplayObject(),
		OctreeObject(_rOctree),
		m_rLandscape(_rLandscape),
		m_uStartVertexIndex(0),
		m_uLOD(_uLOD),
		m_pParent(NULL),
		m_f3Center(0.0f, 0.0f, 0.0f),
		m_f3Extends(0.0f, 0.0f, 0.0f),
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

			m_pDisplay = Display::GetInstance();

			const Landscape::GlobalInfo& rGlobalInfo = m_rLandscape.GetGlobalInfo();
			m_pLODInfo = &rGlobalInfo.m_pLODs[m_uLOD];
			m_uIndexX = pInfo->m_uX * rGlobalInfo.m_uQuadSize;
			m_uIndexZ = pInfo->m_uZ * rGlobalInfo.m_uQuadSize;
			m_uStartVertexIndex = m_uIndexX + m_uIndexZ * m_pLODInfo->m_uVertexPerRowCount;

			// center and extend
			const UInt uStartIndex = m_pLODInfo->m_uStartIndex;
			const UInt uStripSize = m_pLODInfo->m_uStripSize;
			m_f3AABB[0] = Vector3( FLT_MAX, FLT_MAX, FLT_MAX );
			m_f3AABB[1] = Vector3( -FLT_MAX, -FLT_MAX, -FLT_MAX );
			Vector3 f3Temp;
			for (UInt i = 0 ; uStripSize > i ; ++i)
			{
				m_rLandscape.GetVertexPosition(*m_pLODInfo, i, m_uStartVertexIndex, f3Temp);
				if (m_f3AABB[0].x > f3Temp.x) m_f3AABB[0].x = f3Temp.x;
				if (m_f3AABB[0].y > f3Temp.y) m_f3AABB[0].y = f3Temp.y;
				if (m_f3AABB[0].z > f3Temp.z) m_f3AABB[0].z = f3Temp.z;
				if (m_f3AABB[1].x < f3Temp.x) m_f3AABB[1].x = f3Temp.x;
				if (m_f3AABB[1].y < f3Temp.y) m_f3AABB[1].y = f3Temp.y;
				if (m_f3AABB[1].z < f3Temp.z) m_f3AABB[1].z = f3Temp.z;
			}
			m_f3Extends = (m_f3AABB[1] - m_f3AABB[0]) / 2.0f;
			m_f3Center = m_f3AABB[0] + m_f3Extends;
			SetAABB(fsVector3(m_f3AABB[1].x, m_f3AABB[1].y, m_f3AABB[1].z), fsVector3(m_f3AABB[0].x, m_f3AABB[0].y, m_f3AABB[0].z));
			//CreateBoundingMesh();

			if (0 < m_uLOD)
			{
				CreateInfo oLCCInfo;
				UInt uChild = ESubChild_NORTHWEST;
				for (UInt j = 0 ; 2 > j ; ++j)
				{
					for (UInt i = 0 ; 2 > i ; ++i)
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

		DisplayObject::Release();
	}

	void LandscapeChunk::SetWorldMatrix(MatrixRef _rWorld)
	{
		DisplayObject::SetWorldMatrix(_rWorld);
		if (0 != m_uLOD)
		{
			for (int i = 0 ; ESubChild_COUNT > i ; ++i)
			{
				m_pChildren[i]->SetWorldMatrix(_rWorld);
			}
		}
	}

	void LandscapeChunk::SetMaterial(DisplayMaterialPtr _pMaterial)
	{
		DisplayObject::SetMaterial(_pMaterial);
		if (0 != m_uLOD)
		{
			for (int i = 0 ; ESubChild_COUNT > i ; ++i)
			{
				m_pChildren[i]->SetMaterial(_pMaterial);
			}
		}
	}

	void LandscapeChunk::SetRenderStage(const Key& _uRenderPass)
	{
		DisplayObject::SetRenderStage(_uRenderPass);
		if (0 != m_uLOD)
		{
			for (int i = 0 ; ESubChild_COUNT > i ; ++i)
			{
				m_pChildren[i]->SetRenderStage(_uRenderPass);
			}
		}
	}

	void LandscapeChunk::RenderBegin()
	{
		m_pDisplay->GetMaterialManager()->SetFloatBySemantic(s_uMorphFactorKey, &m_fMorphFactor);
		m_rLandscape.UseLayering();
	}

	void LandscapeChunk::Render()
	{
		if (m_rLandscape.UseLODVertexBuffer(m_uLOD) && m_rLandscape.SetIndices())
		{
			const UInt uVertexCount = m_pLODInfo->m_uNumVertices;
			const UInt uStartIndex = m_pLODInfo->m_uStartIndex;
			const UInt uStripSize = m_pLODInfo->m_uStripSize - 2;
			m_pDisplay->GetDevicePtr()->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, m_uStartVertexIndex, 0, uVertexCount, uStartIndex, uStripSize);
		}
	}

	bool LandscapeChunk::RayIntersect(const Vector3& _f3RayBegin, const Vector3& _f3RayEnd, Vector3& _f3Intersect)
	{
		LandscapeVertexIndependentPtr ppVertices[3];
		const UIntPtr pIndexes = m_rLandscape.GetIndices();
		LVIAccessor oVertexAccessor(*m_pLODInfo, pIndexes, m_uStartVertexIndex);
		PITTester3 oPITTester;
		Vector3 f3Out;
		Vector3 f3Delta;
		float fLength;
		float fNearest = FLT_MAX;
		UInt uIndex = 0;
		bool bResult = false;

		ppVertices[0] = oVertexAccessor.Get(0);
		ppVertices[1] = oVertexAccessor.Get(1);

		for (UInt i = 2 ; m_pLODInfo->m_uStripSize > i ; ++i, ++uIndex)
		{
			const UInt uI0 = ((uIndex + 0) % 3);
			const UInt uI1 = ((uIndex + 1) % 3);
			const UInt uI2 = ((uIndex + 2) % 3);
			ppVertices[i % 3] = oVertexAccessor.Get(i);

			if ((ppVertices[uI0]->m_f3Position == ppVertices[uI1]->m_f3Position)
				|| (ppVertices[uI0]->m_f3Position == ppVertices[uI2]->m_f3Position))
			{
				continue;
			}

			if (false != oPITTester.Do(_f3RayBegin, _f3RayEnd,
				ppVertices[uI0]->m_f3Position, ppVertices[uI1]->m_f3Position, ppVertices[uI2]->m_f3Position,
				f3Out))
			{
				f3Delta = f3Out - _f3RayBegin;
				fLength = D3DXVec3Length(&f3Delta);
				if (fNearest > fLength)
				{
					_f3Intersect = f3Out;
					fNearest = fLength;
					bResult = true;
				}
			}
		}

		return bResult;
	}

	void LandscapeChunk::Traverse(LandscapeChunkPtrVecRef _rRenderList, const Vector3& _rCamPos, const float& _fPixelSize)
	{
		const Landscape::GlobalInfo& rGlobalInfo = m_rLandscape.GetGlobalInfo();
		//MatrixPtr pWorld = m_rLandscape.GetWorldMatrix();
		//const Vector3 oWorld(pWorld->_41, pWorld->_42, pWorld->_43);
		const Vector3 oWorld(m_m4World._41, m_m4World._42, m_m4World._43);
		const Vector3 oDelta = oWorld + m_f3Center - _rCamPos;
		const float fExtends = D3DXVec3Length(&m_f3Extends);

		if (DisplayCamera::ECollision_OUT != m_pDisplay->GetCurrentCamera()->CollisionWithSphere(oWorld + m_f3Center, fExtends))
		{
#if LANDSCAPE_USE_HIGHEST_LOD_ONLY
			if (0 == m_uLOD)
#else // LANDSCAPE_USE_HIGHEST_LOD_ONLY
#if 0
			Vector3 aPoints[8];
			aPoints[0] = oDelta + Vector3(m_f3Extends.x, m_f3Extends.y, m_f3Extends.z);
			aPoints[1] = oDelta + Vector3(-m_f3Extends.x, -m_f3Extends.y, -m_f3Extends.z);
			aPoints[2] = oDelta + Vector3(-m_f3Extends.x, m_f3Extends.y, m_f3Extends.z);
			aPoints[3] = oDelta + Vector3(m_f3Extends.x, -m_f3Extends.y, -m_f3Extends.z);
			aPoints[4] = oDelta + Vector3(m_f3Extends.x, -m_f3Extends.y, m_f3Extends.z);
			aPoints[5] = oDelta + Vector3(-m_f3Extends.x, m_f3Extends.y, -m_f3Extends.z);
			aPoints[6] = oDelta + Vector3(m_f3Extends.x, m_f3Extends.y, -m_f3Extends.z);
			aPoints[7] = oDelta + Vector3(-m_f3Extends.x, -m_f3Extends.y, m_f3Extends.z);
			float fDistance = FLT_MAX;
			for (UInt i = 0 ; 8 > i ; ++i)
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
				for (UInt i = 0 ; ESubChild_COUNT > i ; ++i)
				{
					m_pChildren[i]->Traverse(_rRenderList, _rCamPos, _fPixelSize);
				}
			}
		}
	}

	UInt LandscapeChunk::GetLODID() const
	{
		return m_uLOD;
	}

	void LandscapeChunk::UpdateObjectLocation(DisplayObjectPtr _pObject)
	{
		const MatrixPtr pWorld = _pObject->GetWorldMatrix();
		const Vector3 f3Pos(pWorld->m[3][0], pWorld->m[3][1], pWorld->m[3][2]);
		const bool bNotInChunk = (f3Pos.x < m_f3AABB[0].x)
			|| (f3Pos.y < m_f3AABB[0].y)
			|| (f3Pos.z < m_f3AABB[0].z)
			|| (f3Pos.x > m_f3AABB[1].x)
			|| (f3Pos.y > m_f3AABB[1].y)
			|| (f3Pos.z > m_f3AABB[1].z);
		const bool bIsChild = IsChild(_pObject);

		if ((false != bNotInChunk) && (false != bIsChild))
		{
			RemoveChild(_pObject);
		}
		else if ((false == bNotInChunk) && (false == bIsChild))
		{
			AddChild(_pObject);
		}

		if (0 != m_uLOD)
		{
			for (UInt i = 0 ; ESubChild_COUNT > i ; ++i)
			{
				m_pChildren[i]->UpdateObjectLocation(_pObject);
			}
		}
	}

	bool LandscapeChunk::GetWaterIndex(const Vector3& _f3Pos, UIntRef _uLevel)
	{
		bool bResult = false;
		const bool bNotInChunk = 
			(_f3Pos.x < m_f3AABB[0].x)
			//|| (_f3Pos.y < m_f3AABB[0].y) // don't use y axis since water level may not be in chunk bounding box
			|| (_f3Pos.z < m_f3AABB[0].z)
			|| (_f3Pos.x > m_f3AABB[1].x)
			//|| (_f3Pos.y > m_f3AABB[1].y)
			|| (_f3Pos.z > m_f3AABB[1].z);

		if (false == bNotInChunk)
		{
			if (0 != m_uLOD)
			{
				for (UInt i = 0 ; ESubChild_COUNT > i ; ++i)
				{
					bResult = m_pChildren[i]->GetWaterIndex(_f3Pos, _uLevel);
					if (false != bResult)
					{
						break;
					}
				}
				assert(false != bResult);
			}
			else
			{
				const Vector3 f3Temp = m_pLODInfo->m_pVertexesIndependent->m_f3Position;
				const float fXZScale = m_rLandscape.GetGlobalInfo().m_fFloorScale;
				const UInt uXIndex = UInt((_f3Pos.x - f3Temp.x) / fXZScale);
				const UInt uZIndex = UInt((f3Temp.z - _f3Pos.z) / fXZScale);
				LandscapeVertexIndependentRef rVertex = m_pLODInfo->m_pVertexesIndependent[uZIndex * m_pLODInfo->m_uVertexPerRowCount + uXIndex];
				_uLevel = rVertex.m_uWaterLevel;
				bResult = true;
			}
		}

		return bResult;
	}

	bool LandscapeChunk::CreateBoundingMesh()
	{
		m_oBoundingMesh.Clear();

		// vertex
		m_oBoundingMesh.AddVertex(m_f3AABB[0].x, m_f3AABB[1].y, m_f3AABB[1].z);	// TOPLEFTFAR
		m_oBoundingMesh.AddVertex(m_f3AABB[1].x, m_f3AABB[1].y, m_f3AABB[1].z);	// TOPRIGHTTFAR
		m_oBoundingMesh.AddVertex(m_f3AABB[1].x, m_f3AABB[1].y, m_f3AABB[0].z);	// TOPRIGHTTNEAR
		m_oBoundingMesh.AddVertex(m_f3AABB[0].x, m_f3AABB[1].y, m_f3AABB[0].z);	// TOPLEFTNEAR
		m_oBoundingMesh.AddVertex(m_f3AABB[0].x, m_f3AABB[0].y, m_f3AABB[1].z);	// BOTTOMLEFTFAR
		m_oBoundingMesh.AddVertex(m_f3AABB[1].x, m_f3AABB[0].y, m_f3AABB[1].z);	// BOTTOMRIGHTTFAR
		m_oBoundingMesh.AddVertex(m_f3AABB[1].x, m_f3AABB[0].y, m_f3AABB[0].z);	// BOTTOMRIGHTTNEAR
		m_oBoundingMesh.AddVertex(m_f3AABB[0].x, m_f3AABB[0].y, m_f3AABB[0].z);	// BOTTOMLEFTTNEAR

		//triangles
		// top
		m_oBoundingMesh.AddTriangle(0, 3, 2);
		m_oBoundingMesh.AddTriangle(0, 2, 1);
		// right
		m_oBoundingMesh.AddTriangle(3, 7, 6);
		m_oBoundingMesh.AddTriangle(3, 6, 2);
		// near
		m_oBoundingMesh.AddTriangle(2, 6, 5);
		m_oBoundingMesh.AddTriangle(2, 5, 5);
		// bottom
		m_oBoundingMesh.AddTriangle(7, 4, 5);
		m_oBoundingMesh.AddTriangle(7, 5, 6);
		// left
		m_oBoundingMesh.AddTriangle(0, 4, 7);
		m_oBoundingMesh.AddTriangle(0, 7, 3);
		// far
		m_oBoundingMesh.AddTriangle(1, 5, 4);
		m_oBoundingMesh.AddTriangle(1, 4, 0);

		return true;
	}
}
