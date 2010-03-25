#include "stdafx.h"
#include "../Display/Display.h"
#include "../Display/Effect.h"
#include "../Display/GeometryHelper.h"
#include "../Core/Util.h"

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

	DisplayGeometrySphere::DisplayGeometrySphere()
	:	DisplayObject(),
		m_uVertexBuffer(0),
		m_uIndexBuffer(0),
		m_uVertexCount(0),
		m_uIndexCount(0),
		m_f4Color(1.0f, 1.0f, 1.0f, 1.0f)
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
				Matrix m4Pos;
				D3DXMatrixTranslation(&m4Pos, pInfo->m_oPos.x, pInfo->m_oPos.y, pInfo->m_oPos.z);
				Matrix m4Rot;
				D3DXMatrixRotationYawPitchRoll(&m4Rot, D3DXToRadian(pInfo->m_oRot.x), D3DXToRadian(pInfo->m_oRot.y), D3DXToRadian(pInfo->m_oRot.z));
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

			GeometryHelperVertexPtr pVertexData = new GeometryHelperVertex[m_uVertexCount];
			GeometryHelperVertexPtr pVertex = pVertexData;

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
				pVertex->m_oPosition.x = 0.0f;
				pVertex->m_oPosition.y = 1.0f;
				pVertex->m_oPosition.z = 0.0f;
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
							pVertex->m_oPosition.x = cos(fAngle) * fScale;
							pVertex->m_oPosition.y = fHeight;
							pVertex->m_oPosition.z = sin(fAngle) * fScale;
							//vsoutput("%f;%f;%f\n", pVertex->m_oPosition.x, pVertex->m_oPosition.y, pVertex->m_oPosition.z);
							++pVertex;
						}
					}
					else
					{
						for (UInt j = _rInfo.m_uVertSlices - 1 ; _rInfo.m_uVertSlices > j ; --j)
						{
							const float fAngle = j * fVertSliceAngle;
							pVertex->m_oPosition.x = cos(fAngle) * fScale;
							pVertex->m_oPosition.y = fHeight;
							pVertex->m_oPosition.z = sin(fAngle) * fScale;
							//vsoutput("%f;%f;%f\n", pVertex->m_oPosition.x, pVertex->m_oPosition.y, pVertex->m_oPosition.z);
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
							pVertex->m_oPosition.x = cos(fAngle) * fScale;
							pVertex->m_oPosition.y = -fHeight;
							pVertex->m_oPosition.z = sin(fAngle) * fScale;
							//vsoutput("%f;%f;%f\n", pVertex->m_oPosition.x, pVertex->m_oPosition.y, pVertex->m_oPosition.z);
							++pVertex;
						}
					}
					else
					{
						for (UInt j = _rInfo.m_uVertSlices - 1 ; _rInfo.m_uVertSlices > j ; --j)
						{
							const float fAngle = j * fVertSliceAngle;
							pVertex->m_oPosition.x = cos(fAngle) * fScale;
							pVertex->m_oPosition.y = -fHeight;
							pVertex->m_oPosition.z = sin(fAngle) * fScale;
							//vsoutput("%f;%f;%f\n", pVertex->m_oPosition.x, pVertex->m_oPosition.y, pVertex->m_oPosition.z);
							++pVertex;
						}
					}
				}

				// bottom hemisphere end
				pVertex->m_oPosition.x = 0.0f;
				pVertex->m_oPosition.y = -1.0f;
				pVertex->m_oPosition.z = 0.0f;
			}

			pVertex = pVertexData;
			const float fNormalDir = (false == _rInfo.m_bViewFromInside) ? 1.0f : -1.0f;
			for (UInt i = 0 ; m_uVertexCount > i ; ++i)
			{
				D3DXVec3Normalize(&pVertex->m_oNormal, &pVertex->m_oPosition);
				pVertex->m_oNormal *= fNormalDir;
				pVertex->m_oPosition.x *= _rInfo.m_oRadius.x;
				pVertex->m_oPosition.y *= _rInfo.m_oRadius.y;
				pVertex->m_oPosition.z *= _rInfo.m_oRadius.z;
				++pVertex;
			}


			Display::GetInstance()->SetVertexBufferKeyData(m_uVertexBuffer, pVertexData);
			delete[] pVertexData;
		}

		return bResult;
	}

	bool DisplayGeometrySphere::FillIndexBuffer(CreateInfoRef _rInfo)
	{
		bool bResult = (NULL != m_uIndexBuffer);

		if (false != bResult)
		{
			const bool b16Bits = (m_uIndexCount <= 0xffff);

			// temp
			if (false != b16Bits)
			{
				WordPtr pIndexData = new Word[m_uIndexCount];
				WordPtr pIndex = pIndexData;

				memset(pIndexData, 0, m_uIndexCount * sizeof(Word));

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

				Display::GetInstance()->SetIndexBufferKeyData(m_uIndexBuffer, pIndexData);
				delete[] pIndexData;
			}
			else
			{

			}
		}

		return bResult;
	}
}
