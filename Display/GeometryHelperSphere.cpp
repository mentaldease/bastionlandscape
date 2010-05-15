#include "stdafx.h"
#include "../Core/Util.h"
#include "../Display/Display.h"
#include "../Display/Effect.h"
#include "../Display/GeometryHelper.h"
#include "../Display/PointInTriangleTester.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	#define SV2	sizeof(Vector2)
	#define SV3	sizeof(Vector3)
	#define SV4	sizeof(Vector4)

	VertexElement GeometryHelperVertex::s_VertexElement[4] =
	{
		{ 0,	0,						D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
		{ 0,	1 * SV3,				D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
		{ 0,	2 * SV3,				D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
		D3DDECL_END()
	};

	#undef SV2
	#undef SV3
	#undef SV4

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct VertexAccessor
	{
		virtual GeometryHelperVertexPtr Get(UInt _uIndex) = 0;
	};

	template<typename T>
	struct TVertexAccessor : public VertexAccessor
	{
		TVertexAccessor(GeometryHelperVertexPtr _pVertex, T* _pIndex)
		:	m_pVertex(_pVertex),
			m_pIndex(_pIndex)
		{

		}

		virtual GeometryHelperVertexPtr Get(UInt _uIndex)
		{
			assert((NULL != m_pVertex) && (NULL != m_pIndex));
			return m_pVertex + m_pIndex[_uIndex];
		}

		GeometryHelperVertexPtr	m_pVertex;
		T*						m_pIndex;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayGeometrySphere::DisplayGeometrySphere(OctreeRef _rOctree)
	:	DisplayObject(),
		OctreeObject(_rOctree),
		m_f4Color(1.0f, 1.0f, 1.0f, 1.0f),
		m_pIndex16(NULL),
		m_pIndex32(NULL),
		m_pVertex(NULL),
		m_uVertexBuffer(0),
		m_uIndexBuffer(0),
		m_uVertexCount(0),
		m_uIndexCount(0)
	{

	}

	DisplayGeometrySphere::~DisplayGeometrySphere()
	{

	}

	bool DisplayGeometrySphere::Create(const boost::any& _rConfig)
	{
		CreateInfoPtr	pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
		bool bResult = (NULL != pInfo);

		if (false != bResult)
		{
			Release();

			bResult = CreateBuffers(*pInfo);

			if (false != bResult)
			{
				bResult = FillVertexBuffer(*pInfo);
			}
			if (false != bResult)
			{
				bResult = FillIndexBuffer(*pInfo);
			}
			if (false != bResult)
			{
				CreateBoundingMesh(*pInfo);

				Matrix m4Pos;
				D3DXMatrixTranslation(&m4Pos, pInfo->m_f3Pos.x, pInfo->m_f3Pos.y, pInfo->m_f3Pos.z);
				Matrix m4Rot;
				D3DXMatrixRotationYawPitchRoll(&m4Rot, D3DXToRadian(pInfo->m_f3Rot.x), D3DXToRadian(pInfo->m_f3Rot.y), D3DXToRadian(pInfo->m_f3Rot.z));
				Matrix m4World;
				D3DXMatrixMultiply(&m4World, &m4Rot, &m4Pos);
				SetWorldMatrix(m4World);

				m_f4Color = pInfo->m_f4Color;
			}
		}

		return bResult;
	}

	void DisplayGeometrySphere::Update()
	{
	}

	void DisplayGeometrySphere::Release()
	{
		if (0 != m_uIndexBuffer)
		{
			Display::GetInstance()->ReleaseIndexBufferKey(m_uIndexBuffer);
			m_uIndexBuffer = 0;
			m_uIndexCount = 0;
		}
		if (NULL != m_uVertexBuffer)
		{
			Display::GetInstance()->ReleaseVertexBufferKey(m_uVertexBuffer);
			m_uVertexBuffer = 0;
			m_uVertexCount = 0;
		}
		if (NULL != m_pIndex16)
		{
			delete[] m_pIndex16;
			m_pIndex16 = NULL;
		}
		if (NULL != m_pIndex32)
		{
			delete[] m_pIndex32;
			m_pIndex32 = NULL;
		}
		if (NULL != m_pVertex)
		{
			delete[] m_pVertex;
			m_pVertex = NULL;
		}
		DisplayObject::Release();
	}

	void DisplayGeometrySphere::RenderBegin()
	{
		static const Key uDiffuseColorKey = MakeKey(string("DIFFUSECOLOR"));
		DisplayPtr pDisplay = Display::GetInstance();
		pDisplay->GetMaterialManager()->SetVector4BySemantic(uDiffuseColorKey, &m_f4Color);
	}

	void DisplayGeometrySphere::Render()
	{
		DisplayPtr pDisplay = Display::GetInstance();
		if ((false != pDisplay->SetCurrentVertexBufferKey(m_uVertexBuffer)) && (false != pDisplay->SetCurrentIndexBufferKey(m_uIndexBuffer)))
		{
			pDisplay->GetDevicePtr()->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, m_uVertexCount, 0, m_uIndexCount - 2);
		}
	}

	bool DisplayGeometrySphere::RayIntersect(const Vector3& _f3RayBegin, const Vector3& _f3RayEnd, Vector3& _f3Intersect)
	{
		Vector3 aVertices[3];
		VertexAccessor* pVertexAccessor = NULL;
		PITTester3 oPITTester;
		Vector3 f3Out;
		Vector3 f3Delta;
		float fLength;
		float fNearest = FLT_MAX;
		UInt uIndex = 0;
		bool bResult = false;

		if (NULL != m_pIndex16)
		{
			pVertexAccessor = new TVertexAccessor<Word>(m_pVertex, m_pIndex16);
		}
		else
		{
			pVertexAccessor = new TVertexAccessor<UInt>(m_pVertex, m_pIndex32);
		}

		aVertices[0] = pVertexAccessor->Get(0)->m_f3Position;
		aVertices[1] = pVertexAccessor->Get(1)->m_f3Position;

		for (UInt i = 2 ; m_uIndexCount > i ; ++i, ++uIndex)
		{
			const UInt uI0 = ((uIndex + 0) % 3);
			const UInt uI1 = ((uIndex + 1) % 3);
			const UInt uI2 = ((uIndex + 2) % 3);

			aVertices[i % 3] = pVertexAccessor->Get(i)->m_f3Position;

			if ((aVertices[uI0] == aVertices[uI1]) || (aVertices[uI0] == aVertices[uI2]))
			{
				continue;
			}

			if (false != oPITTester.Do(_f3RayBegin, _f3RayEnd,
				aVertices[uI0], aVertices[uI1], aVertices[uI2],
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

		delete pVertexAccessor;

		return bResult;
	}

	bool DisplayGeometrySphere::CreateBuffers(CreateInfoRef _rInfo)
	{
		DisplayPtr pDisplay = Display::GetInstance();
		const UInt uHemisphereCount = (_rInfo.m_bBottomHemisphere ? 1 : 0) + (_rInfo.m_bTopHemisphere ? 1 : 0);
		bool bResult = (0 < uHemisphereCount)
			&& (0 < _rInfo.m_uHorizSlices)
			&& (2 < _rInfo.m_uVertSlices);

		if (false != bResult)
		{
			const UInt uHSVertexCount = _rInfo.m_uVertSlices;
			m_uVertexCount = uHemisphereCount // one vertex for each hemisphere end
				+ (uHemisphereCount * uHSVertexCount * (_rInfo.m_uHorizSlices - 1)) // intermediate
				+ uHSVertexCount; // equator end

			DisplayVertexBuffer::CreateInfo oVBCInfo;
			oVBCInfo.m_pVertexElement = GeometryHelperVertex::s_VertexElement;
			oVBCInfo.m_uVertexSize = sizeof(GeometryHelperVertex);
			oVBCInfo.m_uBufferSize = m_uVertexCount * sizeof(GeometryHelperVertex);
			m_uVertexBuffer = pDisplay->CreateVertexBufferKey(oVBCInfo);
			bResult = (0 != m_uVertexBuffer);
		}

		if (false != bResult)
		{
			const UInt uNormalStripSize = 2 + _rInfo.m_uVertSlices * 2;
			const UInt uStripLink = 2;
			m_uFanToStripSize = 2 * (_rInfo.m_uVertSlices + 1);
			m_uIndexCount = uHemisphereCount * m_uFanToStripSize;
			m_uIndexCount += uHemisphereCount * (uNormalStripSize * (_rInfo.m_uHorizSlices - 1));
			m_uIndexCount += uHemisphereCount * (uStripLink * (_rInfo.m_uHorizSlices - 1));

			DisplayIndexBuffer::CreateInfo oIBCInfo;
			oIBCInfo.m_b16Bits = (m_uIndexCount <= 0xffff);
			oIBCInfo.m_uBufferSize = m_uIndexCount;
			m_uIndexBuffer = pDisplay->CreateIndexBufferKey(oIBCInfo);
			bResult = (0 != m_uVertexBuffer);
		}

		return bResult;
	}

	bool DisplayGeometrySphere::FillVertexBuffer(CreateInfoRef _rInfo)
	{
		bool bResult = (0 != m_uVertexBuffer);

		if (false != bResult)
		{
			const UInt uHemisphereCount = (_rInfo.m_bBottomHemisphere ? 1 : 0) + (_rInfo.m_bTopHemisphere ? 1 : 0);
			const UInt uHSVertexCount = _rInfo.m_uVertSlices;
			const float fVertSliceAngle = D3DXToRadian(360.0f / float(_rInfo.m_uVertSlices));
			const float fHorizSliceAngle = D3DXToRadian(90.0f / float(_rInfo.m_uHorizSlices));

			m_pVertex = new GeometryHelperVertex[m_uVertexCount];
			GeometryHelperVertexPtr pVertex = m_pVertex;

			/*
				phi = Acos(pn.Normal.z)
				pnt.Tv = (phi / PI)

				u = Acos(Max(Min(pnt.Normal.y / Sin(phi), 1.0), -1.0)) / (2.0 * PI)

				If pnt.Normal.x > 0 Then
				pnt.Tu = u
				Else
				pnt.Tu = 1 - u
				End If
			*/
			/*
				u=Nx/2 + 0.5
				v=Ny/2 + 0.5
			*/
			
			if (false != _rInfo.m_bTopHemisphere)
			{
				// top hemisphere end
				pVertex->m_f3Position.x = 0.0f;
				pVertex->m_f3Position.y = 1.0f;
				pVertex->m_f3Position.z = 0.0f;
				++pVertex;

				for (UInt i = 0 ; _rInfo.m_uHorizSlices > i ; ++i)
				{
					const float fHeight = cos((i + 1) * fHorizSliceAngle);
					const float fScale = sin((i + 1) * fHorizSliceAngle);
					if (false == _rInfo.m_bViewFromInside)
					{
						for (UInt j = 0 ; _rInfo.m_uVertSlices > j ; ++j)
						{
							const float fAngle = j * fVertSliceAngle;
							pVertex->m_f3Position.x = cos(fAngle) * fScale;
							pVertex->m_f3Position.y = fHeight;
							pVertex->m_f3Position.z = sin(fAngle) * fScale;
							//vsoutput("%f;%f;%f\n", pVertex->m_f3Position.x, pVertex->m_f3Position.y, pVertex->m_f3Position.z);
							++pVertex;
						}
					}
					else
					{
						for (UInt j = _rInfo.m_uVertSlices - 1 ; _rInfo.m_uVertSlices > j ; --j)
						{
							const float fAngle = j * fVertSliceAngle;
							pVertex->m_f3Position.x = cos(fAngle) * fScale;
							pVertex->m_f3Position.y = fHeight;
							pVertex->m_f3Position.z = sin(fAngle) * fScale;
							//vsoutput("%f;%f;%f\n", pVertex->m_f3Position.x, pVertex->m_f3Position.y, pVertex->m_f3Position.z);
							++pVertex;
						}
					}
				}
			}

			if (false != _rInfo.m_bBottomHemisphere)
			{
				const UInt uStartSlice = (false != _rInfo.m_bTopHemisphere) ? _rInfo.m_uHorizSlices - 1 : _rInfo.m_uHorizSlices;
				for (UInt i = uStartSlice ; 0 < i ; --i)
				{
					const float fHeight = cos(i * fHorizSliceAngle);
					const float fScale = sin(i * fHorizSliceAngle);
					if (false == _rInfo.m_bViewFromInside)
					{
						for (UInt j = 0 ; _rInfo.m_uVertSlices > j ; ++j)
						{
							const float fAngle = j * fVertSliceAngle;
							pVertex->m_f3Position.x = cos(fAngle) * fScale;
							pVertex->m_f3Position.y = -fHeight;
							pVertex->m_f3Position.z = sin(fAngle) * fScale;
							//vsoutput("%f;%f;%f\n", pVertex->m_f3Position.x, pVertex->m_f3Position.y, pVertex->m_f3Position.z);
							++pVertex;
						}
					}
					else
					{
						for (UInt j = _rInfo.m_uVertSlices - 1 ; _rInfo.m_uVertSlices > j ; --j)
						{
							const float fAngle = j * fVertSliceAngle;
							pVertex->m_f3Position.x = cos(fAngle) * fScale;
							pVertex->m_f3Position.y = -fHeight;
							pVertex->m_f3Position.z = sin(fAngle) * fScale;
							//vsoutput("%f;%f;%f\n", pVertex->m_f3Position.x, pVertex->m_f3Position.y, pVertex->m_f3Position.z);
							++pVertex;
						}
					}
				}

				// bottom hemisphere end
				pVertex->m_f3Position.x = 0.0f;
				pVertex->m_f3Position.y = -1.0f;
				pVertex->m_f3Position.z = 0.0f;
			}

			pVertex = m_pVertex;
			const float fNormalDir = (false == _rInfo.m_bViewFromInside) ? 1.0f : -1.0f;
			for (UInt i = 0 ; m_uVertexCount > i ; ++i)
			{
				pVertex->m_f3Position.x *= _rInfo.m_f3Radius.x;
				pVertex->m_f3Position.y *= _rInfo.m_f3Radius.y;
				pVertex->m_f3Position.z *= _rInfo.m_f3Radius.z;
				D3DXVec3Normalize(&pVertex->m_f3Normal, &pVertex->m_f3Position);
				pVertex->m_f3Normal *= fNormalDir;
				++pVertex;
			}

			Display::GetInstance()->SetVertexBufferKeyData(m_uVertexBuffer, m_pVertex);

			fsVector3 fs3TRF(_rInfo.m_f3Radius.x, (false != _rInfo.m_bTopHemisphere) ? _rInfo.m_f3Radius.y : 0.0f, _rInfo.m_f3Radius.z);
			fsVector3 fs3BLN(-_rInfo.m_f3Radius.x, (false != _rInfo.m_bBottomHemisphere) ? -_rInfo.m_f3Radius.y : 0.0f, -_rInfo.m_f3Radius.z);
			SetAABB(fs3TRF, fs3BLN);
		}

		return bResult;
	}

	bool DisplayGeometrySphere::FillIndexBuffer(CreateInfoRef _rInfo)
	{
		bool bResult = (NULL != m_uIndexBuffer);

		if (false != bResult)
		{
			const bool b16Bits = (m_uIndexCount <= 0xffff);

			if (false != b16Bits)
			{
				m_pIndex16 = new Word[m_uIndexCount];
				WordPtr pIndex = m_pIndex16;

				memset(m_pIndex16, 0, m_uIndexCount * sizeof(Word));

				const UInt uHSVertexCount = _rInfo.m_uVertSlices;

				Word uCurrentVertexIndex = 0;

				if (false != _rInfo.m_bTopHemisphere)
				{
					for (Word i = 0 ; _rInfo.m_uVertSlices > i ; ++i, pIndex += 2)
					{
						pIndex[0] = 0;
						pIndex[1] = i + 1;
					}
					pIndex[0] = 0;
					pIndex[1] = 1;
					pIndex += 2;
					uCurrentVertexIndex = 1;
				}

				const UInt uHemisphereCount = (_rInfo.m_bBottomHemisphere ? 1 : 0) + (_rInfo.m_bTopHemisphere ? 1 : 0);
				const UInt uHorizSlicesWOTops = uHemisphereCount * (_rInfo.m_uHorizSlices - 1); // WOTops = WithOut tops
				for (UInt i = 0 ; uHorizSlicesWOTops > i ; ++i)
				{
					const UInt uStartVertexIndex = uCurrentVertexIndex;
					for (UInt j = 0 ; _rInfo.m_uVertSlices > j ; ++j, pIndex += 2, ++uCurrentVertexIndex)
					{
						pIndex[0] = uCurrentVertexIndex;
						pIndex[1] = uCurrentVertexIndex + uHSVertexCount;
					}

					// end of horizontal slice strip (back to starting vertex index)
					uCurrentVertexIndex = uStartVertexIndex;
					pIndex[0] = uCurrentVertexIndex;
					uCurrentVertexIndex += uHSVertexCount;
					pIndex[1] = uCurrentVertexIndex;
					pIndex += 2;

					// link to next strip
					if (((uHorizSlicesWOTops - 1) > i) || (false != _rInfo.m_bBottomHemisphere))
					{
						pIndex[0] = uCurrentVertexIndex;
						pIndex[1] = uCurrentVertexIndex;
						pIndex += 2;
					}
				}

				if (false != _rInfo.m_bBottomHemisphere)
				{
					const UInt uStartVertexIndex = uCurrentVertexIndex;
					for (Word i = 0 ; _rInfo.m_uVertSlices > i ; ++i, pIndex += 2)
					{
						pIndex[0] = uCurrentVertexIndex + i;
						pIndex[1] = m_uVertexCount - 1;
					}
					pIndex[0] = uStartVertexIndex;
					pIndex[1] = m_uVertexCount - 1;
				}

				Display::GetInstance()->SetIndexBufferKeyData(m_uIndexBuffer, m_pIndex16);
			}
			else
			{
				m_pIndex32 = new UInt[m_uIndexCount];
				UIntPtr pIndex = m_pIndex32;

				memset(m_pIndex32, 0, m_uIndexCount * sizeof(UInt));

				const UInt uHSVertexCount = _rInfo.m_uVertSlices;

				UInt uCurrentVertexIndex = 0;

				if (false != _rInfo.m_bTopHemisphere)
				{
					for (UInt i = 0 ; _rInfo.m_uVertSlices > i ; ++i, pIndex += 2)
					{
						pIndex[0] = 0;
						pIndex[1] = i + 1;
					}
					pIndex[0] = 0;
					pIndex[1] = 1;
					pIndex += 2;
					uCurrentVertexIndex = 1;
				}

				const UInt uHemisphereCount = (_rInfo.m_bBottomHemisphere ? 1 : 0) + (_rInfo.m_bTopHemisphere ? 1 : 0);
				const UInt uHorizSlicesWOTops = uHemisphereCount * (_rInfo.m_uHorizSlices - 1); // WOTops = WithOut tops
				for (UInt i = 0 ; uHorizSlicesWOTops > i ; ++i)
				{
					const UInt uStartVertexIndex = uCurrentVertexIndex;
					for (UInt j = 0 ; _rInfo.m_uVertSlices > j ; ++j, pIndex += 2, ++uCurrentVertexIndex)
					{
						pIndex[0] = uCurrentVertexIndex;
						pIndex[1] = uCurrentVertexIndex + uHSVertexCount;
					}

					// end of horizontal slice strip (back to starting vertex index)
					uCurrentVertexIndex = uStartVertexIndex;
					pIndex[0] = uCurrentVertexIndex;
					uCurrentVertexIndex += uHSVertexCount;
					pIndex[1] = uCurrentVertexIndex;
					pIndex += 2;

					// link to next strip
					if (((uHorizSlicesWOTops - 1) > i) || (false != _rInfo.m_bBottomHemisphere))
					{
						pIndex[0] = uCurrentVertexIndex;
						pIndex[1] = uCurrentVertexIndex;
						pIndex += 2;
					}
				}

				if (false != _rInfo.m_bBottomHemisphere)
				{
					const UInt uStartVertexIndex = uCurrentVertexIndex;
					for (UInt i = 0 ; _rInfo.m_uVertSlices > i ; ++i, pIndex += 2)
					{
						pIndex[0] = uCurrentVertexIndex + i;
						pIndex[1] = m_uVertexCount - 1;
					}
					pIndex[0] = uStartVertexIndex;
					pIndex[1] = m_uVertexCount - 1;
				}

				Display::GetInstance()->SetIndexBufferKeyData(m_uIndexBuffer, m_pIndex32);
			}
		}

		return bResult;
	}

	bool DisplayGeometrySphere::CreateBoundingMesh(CreateInfoRef _rInfo)
	{
		m_oBoundingMesh.Clear();

		const float fTop = (false != _rInfo.m_bTopHemisphere) ? _rInfo.m_f3Radius.y : 0.0f;
		const float fBottom = (false != _rInfo.m_bBottomHemisphere) ? -_rInfo.m_f3Radius.y : 0.0f;

		// vertex
		m_oBoundingMesh.AddVertex(-_rInfo.m_f3Radius.x, fTop, _rInfo.m_f3Radius.z);		// TOPLEFTFAR
		m_oBoundingMesh.AddVertex(_rInfo.m_f3Radius.x, fTop, _rInfo.m_f3Radius.z);		// TOPRIGHTTFAR
		m_oBoundingMesh.AddVertex(_rInfo.m_f3Radius.x, fTop, -_rInfo.m_f3Radius.z);		// TOPRIGHTTNEAR
		m_oBoundingMesh.AddVertex(-_rInfo.m_f3Radius.x, fTop, -_rInfo.m_f3Radius.z);	// TOPLEFTNEAR
		m_oBoundingMesh.AddVertex(-_rInfo.m_f3Radius.x, fBottom, _rInfo.m_f3Radius.z);	// BOTTOMLEFTFAR
		m_oBoundingMesh.AddVertex(_rInfo.m_f3Radius.x, fBottom, _rInfo.m_f3Radius.z);	// BOTTOMRIGHTTFAR
		m_oBoundingMesh.AddVertex(_rInfo.m_f3Radius.x, fBottom, -_rInfo.m_f3Radius.z);	// BOTTOMRIGHTTNEAR
		m_oBoundingMesh.AddVertex(-_rInfo.m_f3Radius.x, fBottom, -_rInfo.m_f3Radius.z);	// BOTTOMLEFTTNEAR

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
