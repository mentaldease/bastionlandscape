#include "stdafx.h"
#include "Landscape.h"
#include "../Display/Camera.h"
#include "../Display/Effect.h"
#include "../Display/EffectParam.h"

#define LANDSCAPE_CHUNK_PITTESTER 3

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	Key LandscapeChunk::s_uMorphFactorKey = MakeKey(string("MORPHFACTOR"));
	Vector3Ptr LandscapeChunk::s_af3PickVertices[3] = { NULL, NULL, NULL };

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

	struct PITTester // Point-In-Triangle Tester
	{
		bool Do(const Vector3Ptr _pPoint, const Vector3Ptr _pV0, const Vector3Ptr _pV1, const Vector3Ptr _pV2, const Vector3Ptr _pNormal)
		{
			#define VEC3TOFLOAT3(VEC3) { (VEC3).x, (VEC3).y, (VEC3).z }
			const float I[3] = VEC3TOFLOAT3(*_pPoint);
			const float V0[3] = VEC3TOFLOAT3(*_pV0);
			const float V1[3] = VEC3TOFLOAT3(*_pV1);
			const float V2[3] = VEC3TOFLOAT3(*_pV2);
			const float N[3] = VEC3TOFLOAT3(*_pNormal);
			#undef VEC3TOFLOAT3

			// Collapse the largest component of 
			// the normal in order to convert to 2D
			int i1, i2;
			if( fabs(N[0]) > fabs(N[1]) && fabs(N[0]) > fabs(N[2]) )
			{
				// x is the largest component, ignore it in further calculations
				i1 = 1;
				i2 = 2;
			}
			else if( fabs(N[1]) > fabs(N[0]) && fabs(N[1]) > fabs(N[2]))
			{
				// y is the largest
				i1 = 0;
				i2 = 2;
			}
			else
			{
				// z is the largest
				i1 = 0;
				i2 = 1;
			}


			// Compute the barycentric coordinates, by computing 
			// the signed area of each subtriangle made up of the
			// intersection point and two vertices. The signed area
			// of a triangle is computed as the crossproduct of two 
			// edges divided by two. 
			float areaTimes2 = (V1[i1] - V0[i1]) * (V2[i2] - V0[i2]) - (V1[i2] - V0[i2]) * (V2[i1] - V0[i1]);
			float u = ((V2[i1] - I[i1]) * (V0[i2] - I[i2]) - (V2[i2] - I[i2]) * (V0[i1] - I[i1])) / areaTimes2;
			float v = ((V0[i1] - I[i1]) * (V1[i2] - I[i2]) - (V0[i2] - I[i2]) * (V1[i1] - I[i1])) / areaTimes2;


			// The triangle can be described with barycentric 
			// coordinates with the following formula:
			// T(u,v) = (1-u-v)*_pV0 + u*_pV1 + v*_pV2
			// Restrictions: u >= 0, v >= 0, u + v <= 1


			// Test if the any of the restrictions are violated, if
			// they are the point is outside the triangle
			if ((u < 0) || (v < 0) || ((u + v) > 1))
				return false;


			// The point is inside the area
			return true;
		}
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct PITTester2
	{
		bool Do(const Vector3& orig, const Vector3& dir,
			const Vector3& v0, const Vector3& v1, const Vector3& v2,
			Vector3& _f3PosResult, Vector3& _f3TUVResult)
		{
			// Find vectors for two edges sharing vert0
			const Vector3 edge1 = v1 - v0;
			const Vector3 edge2 = v2 - v0;

			// Begin calculating determinant - also used to calculate U parameter
			Vector3 pvec;
			D3DXVec3Cross( &pvec, &dir, &edge2 );

			// If determinant is near zero, ray lies in plane of triangle
			FLOAT det = D3DXVec3Dot( &edge1, &pvec );

			Vector3 tvec;
			if( det > 0 )
			{
				tvec = orig - v0;
			}
			else
			{
				tvec = v0 - orig;
				det = -det;
			}

			if( det < 0.0001f )
				return false;

			// Calculate U parameter and test bounds
			_f3TUVResult.y = D3DXVec3Dot( &tvec, &pvec );
			if( _f3TUVResult.y < 0.0f || _f3TUVResult.y > det )
				return false;

			// Prepare to test V parameter
			Vector3 qvec;
			D3DXVec3Cross( &qvec, &tvec, &edge1 );

			// Calculate V parameter and test bounds
			_f3TUVResult.z = D3DXVec3Dot( &dir, &qvec );
			if( _f3TUVResult.z < 0.0f || _f3TUVResult.y + _f3TUVResult.z > det )
				return false;

			// Calculate t, scale parameters, ray intersects triangle
			_f3TUVResult.x = D3DXVec3Dot( &edge2, &qvec );
			FLOAT fInvDet = 1.0f / det;
			_f3TUVResult.x *= fInvDet; // t
			_f3TUVResult.y *= fInvDet; // u
			_f3TUVResult.z *= fInvDet; // v

			_f3PosResult = orig + _f3TUVResult.x * dir;

			return true;
		}
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct PITTester3 
	{
		struct Ray
		{
			const Vector3&	P0;
			const Vector3&	P1;
		};

		struct Triangle
		{
			const Vector3&	V0;
			const Vector3&	V1;
			const Vector3&	V2;
		};

		bool Do(const Vector3& _f3RayP0, const Vector3& _f3RayP1, const Vector3& _f3V0, const Vector3& _f3V1, const Vector3& _f3V2, Vector3& _f3Out)
		{
			// Copyright 2001, softSurfer (www.softsurfer.com)
			// This code may be freely used and modified for any purpose
			// providing that this copyright notice is included with it.
			// SoftSurfer makes no warranty for this code, and cannot be held
			// liable for any real or imagined damage resulting from its use.
			// Users of this code must verify correctness for their application.

			// Assume that classes are already given for the objects:
			//    Point and Vector with
			//        coordinates {float x, y, z;}
			//        operators for:
			//            == to test equality
			//            != to test inequality
			//            (Vector)0 = (0,0,0)         (null vector)
			//            Point  = Point ± Vector
			//            Vector = Point - Point
			//            Vector = Scalar * Vector    (scalar product)
			//            Vector = Vector * Vector    (cross product)
			//    Line and Ray and Segment with defining points {Point P0, P1;}
			//        (a Line is infinite, Rays and Segments start at P0)
			//        (a Ray extends beyond P1, but a Segment ends at P1)
			//    Plane with a point and a normal {Point V0; Vector n;}
			//    Triangle with defining vertices {Point V0, V1, V2;}
			//    Polyline and Polygon with n vertices {int n; Point *V;}
			//        (a Polygon has V[n]=V[0])
			//===================================================================

			Ray R = { _f3RayP0, _f3RayP1 };
			Triangle T = { _f3V0, _f3V1, _f3V2 };
			bool bResult = false;

			#define SMALL_NUM  0.00000001 // anything that avoids division overflow
			// dot product (3D) which allows vector operations in arguments
			//#define dot(u,v)   ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)
			#define dot(u,v)   D3DXVec3Dot(&(u), &(v))

			// intersect_RayTriangle(): intersect a ray with a 3D triangle
			//    Input:  a ray R, and a triangle T
			//    Output: *I = intersection point (when it exists)
			//    Return: -1 = triangle is degenerate (a segment or point)
			//             0 = disjoint (no intersect)
			//             1 = intersect in unique point I1
			//             2 = are in the same plane
			Vector3    u, v, n;             // triangle vectors
			Vector3    dir, w0, w;          // ray vectors
			float     r, a, b;             // params to calc ray-plane intersect

			// get triangle edge vectors and plane normal
			u = T.V1 - T.V0;
			v = T.V2 - T.V0;
			//n = u * v;             // cross product
			D3DXVec3Cross(&n, &u, &v);
			if (n == Vector3(0.0f, 0.0f, 0.0f))            // triangle is degenerate
				//return -1;                 // do not deal with this case
				return false;

			dir = R.P1 - R.P0;             // ray direction vector
			w0 = R.P0 - T.V0;
			a = -dot(n,w0);
			b = dot(n,dir);
			if (fabs(b) < SMALL_NUM) {     // ray is parallel to triangle plane
				//if (a == 0)                // ray lies in triangle plane
				//	return 2;
				//else return 0;             // ray disjoint from plane
				return false;
			}

			// get intersect point of ray with triangle plane
			r = a / b;
			if (r < 0.0)                   // ray goes away from triangle
				//return 0;                  // => no intersect
				return false;
			// for a segment, also test if (r > 1.0) => no intersect
			if (r > 1.0)                   // ray goes away from triangle
				//return 0;                  // => no intersect
				return false;

			_f3Out = R.P0 + r * dir;           // intersect point of ray and plane

			// is I inside T?
			float    uu, uv, vv, wu, wv, D;
			uu = dot(u,u);
			uv = dot(u,v);
			vv = dot(v,v);
			w = _f3Out - T.V0;
			wu = dot(w,u);
			wv = dot(w,v);
			D = uv * uv - uu * vv;

			// get and test parametric coords
			float s, t;
			s = (uv * wv - vv * wu) / D;
			if (s < 0.0 || s > 1.0)        // I is outside T
				//return 0;
				return false;
			t = (uv * wu - uu * wv) / D;
			if (t < 0.0 || (s + t) > 1.0)  // I is outside T
				//return 0;
				return false;

			//return 1;                      // I is in T
			return true;
		}
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

			m_pDisplay = Display::GetInstance();

			const Landscape::GlobalInfo& rGlobalInfo = m_rLandscape.GetGlobalInfo();
			m_pLODInfo = &rGlobalInfo.m_pLODs[m_uLOD];
			m_uIndexX = pInfo->m_uX * rGlobalInfo.m_uQuadSize;
			m_uIndexZ = pInfo->m_uZ * rGlobalInfo.m_uQuadSize;
			m_uStartVertexIndex = m_uIndexX + m_uIndexZ * m_pLODInfo->m_uVertexPerRowCount;

			// center and extend
			const UInt uStartIndex = m_pLODInfo->m_uStartIndex;
			const UInt uStripSize = m_pLODInfo->m_uStripSize;
			Vector3 oAABB[2] =
			{
				Vector3( FLT_MAX, FLT_MAX, FLT_MAX ),
				Vector3( -FLT_MAX, -FLT_MAX, -FLT_MAX )
			};
			Vector3 oTemp;
			for (UInt i = 0 ; uStripSize > i ; ++i)
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

		DisplayObject::Release();
	}

	void LandscapeChunk::SetWorldMatrix(MatrixRef _rWorld)
	{
		DisplayObject::SetWorldMatrix(_rWorld);
		for (int i = 0 ; ESubChild_COUNT > i ; ++i)
		{
			if (NULL != m_pChildren[i])
			{
				m_pChildren[i]->SetWorldMatrix(_rWorld);
			}
		}
	}

	void LandscapeChunk::SetMaterial(DisplayMaterialPtr _pMaterial)
	{
		DisplayObject::SetMaterial(_pMaterial);
		for (int i = 0 ; ESubChild_COUNT > i ; ++i)
		{
			if (NULL != m_pChildren[i])
			{
				m_pChildren[i]->SetMaterial(_pMaterial);
			}
		}
	}

	void LandscapeChunk::SetRenderStage(const Key& _uRenderPass)
	{
		DisplayObject::SetRenderStage(_uRenderPass);
		for (int i = 0 ; ESubChild_COUNT > i ; ++i)
		{
			if (NULL != m_pChildren[i])
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
#if (1 == LANDSCAPE_CHUNK_PITTESTER)
		PITTester oPITTester;
#elif (2 == LANDSCAPE_CHUNK_PITTESTER)
		PITTester2 oPITTester;
		Vector3 f3OutTUV;
		Vector3 f3Dir = _f3RayEnd - _f3RayBegin;
		D3DXVec3Normalize(&f3Dir, &f3Dir);
#elif (3 == LANDSCAPE_CHUNK_PITTESTER)
		PITTester3 oPITTester;
#endif // !LANDSCAPE_CHUNK_PITTESTER
		Vector3 f3Out;
		Vector3 f3Delta;
		float fLength;
		float fNearest = FLT_MAX;
		UInt uIndex = 0;
		bool bResult = false;

		s_af3PickVertices[0] = NULL;
		s_af3PickVertices[1] = NULL;
		s_af3PickVertices[2] = NULL;

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

#if (1 == LANDSCAPE_CHUNK_PITTESTER)
			Vector3 f3Normal = (ppVertices[uI0]->m_f3Normal + ppVertices[uI1]->m_f3Normal + ppVertices[uI2]->m_f3Normal) / 3.0f;
			const float fD = D3DXVec3Dot(&f3Normal, &ppVertices[uI0]->m_f3Position);
			const Vector3 f3Ray = _f3RayEnd - _f3RayBegin;
			const float fT = - (D3DXVec3Dot(&f3Normal, &_f3RayBegin) - fD) / D3DXVec3Dot(&f3Normal, &f3Ray);

			f3Out = _f3RayBegin + f3Ray * fT;

			if (false != oPITTester.Do(&f3Out,
				&ppVertices[uI0]->m_f3Position, &ppVertices[uI1]->m_f3Position, &ppVertices[uI2]->m_f3Position,
				&f3Normal))
#elif (2 == LANDSCAPE_CHUNK_PITTESTER)
			if (false != oPITTester.Do(_f3RayBegin, f3Dir,
				ppVertices[uI0]->m_f3Position, ppVertices[uI1]->m_f3Position, ppVertices[uI2]->m_f3Position,
				f3Out, f3OutTUV))
#elif (3 == LANDSCAPE_CHUNK_PITTESTER)
			if (false != oPITTester.Do(_f3RayBegin, _f3RayEnd,
				ppVertices[uI0]->m_f3Position, ppVertices[uI1]->m_f3Position, ppVertices[uI2]->m_f3Position,
				f3Out))
#endif // !LANDSCAPE_CHUNK_PITTESTER
			{
				f3Delta = f3Out - _f3RayBegin;
				fLength = D3DXVec3Length(&f3Delta);
				if (fNearest > fLength)
				{
					_f3Intersect = f3Out;
					fNearest = fLength;
					s_af3PickVertices[0] = &ppVertices[uI0]->m_f3Position;
					s_af3PickVertices[1] = &ppVertices[uI1]->m_f3Position;
					s_af3PickVertices[2] = &ppVertices[uI2]->m_f3Position;
					bResult = true;
				}
			}
		}

		//vsoutput(__FUNCTION__" : y=%f\n", _f3Intersect.y);

		return bResult;
	}

	void LandscapeChunk::Traverse(LandscapeChunkPtrVecRef _rRenderList, const Vector3& _rCamPos, const float& _fPixelSize)
	{
		const Landscape::GlobalInfo& rGlobalInfo = m_rLandscape.GetGlobalInfo();
		//MatrixPtr pWorld = m_rLandscape.GetWorldMatrix();
		//const Vector3 oWorld(pWorld->_41, pWorld->_42, pWorld->_43);
		const Vector3 oWorld(m_oWorld._41, m_oWorld._42, m_oWorld._43);
		const Vector3 oDelta = oWorld + m_oCenter - _rCamPos;
		const float fExtends = D3DXVec3Length(&m_oExtends);

		if (DisplayCamera::ECollision_OUT != m_pDisplay->GetCurrentCamera()->CollisionWithSphere(oWorld + m_oCenter, fExtends))
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
}
