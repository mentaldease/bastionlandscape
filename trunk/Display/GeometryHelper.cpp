#include "stdafx.h"
#include "../Display/Display.h"
#include "../Display/GeometryHelper.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	#define SV2	sizeof(Vector2)
	#define SV3	sizeof(Vector3)
	#define SV4	sizeof(Vector4)

	VertexElement GeometryHelperVertex::s_VertexElement[5] =
	{
		{ 0,	0,						D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
		{ 0,	1 * SV3,				D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
		{ 0,	2 * SV3,				D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,		0 },
		{ 0,	2 * SV3 + SV4,			D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
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
		m_pVertexBuffer(NULL),
		m_pIndexBuffer(NULL),
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
		}

		return bResult;
	}

	void DisplayGeometrySphere::Update()
	{

	}

	void DisplayGeometrySphere::Release()
	{
		if (NULL != m_pIndexBuffer)
		{
			Display::GetInstance()->ReleaseIndexBuffer(m_pIndexBuffer);
			m_pIndexBuffer = NULL;
			m_uIndexCount = 0;
		}
		if (NULL != m_pVertexBuffer)
		{
			Display::GetInstance()->ReleaseVertexBuffer(m_pVertexBuffer);
			m_pVertexBuffer = NULL;
			m_uVertexCount = 0;
		}
	}

	void DisplayGeometrySphere::Render()
	{
		DisplayPtr pDisplay = Display::GetInstance();
		if ((false != pDisplay->SetCurrentVertexBuffer(m_pVertexBuffer)) && (false != pDisplay->SetCurrentIndexBuffer(m_pIndexBuffer)))
		{
			//pDisplay->GetDevicePtr()->DrawPrimitive(D3DPT_POINTLIST, 0, m_uVertexCount);
			//pDisplay->GetDevicePtr()->DrawPrimitive(D3DPT_POINTLIST, 0, 182);//m_uVertexCount - 1);
			pDisplay->GetDevicePtr()->DrawPrimitive(D3DPT_LINESTRIP, 0, 181);//m_uVertexCount - 1);
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
			m_pVertexBuffer = pDisplay->CreateVertexBuffer(oVBCInfo);
			bResult = (NULL != m_pVertexBuffer);
		}

		if (false != bResult)
		{
			const UInt uEndStripSize = 2 * ((_rInfo.m_uVertSlices + 1) / 3) + _rInfo.m_uVertSlices + 2; // thanks to msimon for the formula
			const UInt uNormalStripSize = 2 + _rInfo.m_uVertSlices;
			const UInt uStripLink = 2;
			m_uIndexCount = uHemisphereCount * uEndStripSize;
			m_uIndexCount += uHemisphereCount * (uNormalStripSize * (_rInfo.m_uHorizSlices - 1));
			m_uIndexCount += uHemisphereCount * (uStripLink * (_rInfo.m_uHorizSlices - 1));

			DisplayIndexBuffer::CreateInfo oIBCInfo;
			oIBCInfo.m_b16Bits = (m_uIndexCount <= 0xffff);
			oIBCInfo.m_uBufferSize = m_uIndexCount;
			m_pIndexBuffer = pDisplay->CreateIndexBuffer(oIBCInfo);
			bResult = (NULL != m_pVertexBuffer);
		}

		return bResult;
	}

	bool DisplayGeometrySphere::FillVertexBuffer(CreateInfoRef _rInfo)
	{
		bool bResult = (NULL != m_pVertexBuffer);

		if (false != bResult)
		{
			const UInt uHemisphereCount = (_rInfo.m_bBottomHemisphere ? 1 : 0) + (_rInfo.m_bTopHemisphere ? 1 : 0);
			const UInt uHSVertexCount = _rInfo.m_uVertSlices;
			const float fVertSliceAngle = D3DXToRadian(360.0f / float(_rInfo.m_uVertSlices));
			const float fHorizSliceAngle = D3DXToRadian(90.0f / float(_rInfo.m_uHorizSlices));

			GeometryHelperVertexPtr pVertexData = new GeometryHelperVertex[m_uVertexCount];
			GeometryHelperVertexPtr pVertex = pVertexData;

			if (false != _rInfo.m_bTopHemisphere)
			{
				// top hemisphere end
				pVertex->m_oPosition.x = 0.0f;
				pVertex->m_oPosition.y = 1.0f;
				pVertex->m_oPosition.z = 0.0f;
				pVertex->m_oColor = Vector4(0.0f, 1.0f, 1.0f, 1.0f);
				++pVertex;

				for (UInt i = 0 ; _rInfo.m_uHorizSlices > i ; ++i)
				{
					const float fHeight = cos((i + 1) * fHorizSliceAngle);
					const float fScale = sin((i + 1) * fHorizSliceAngle);
					for (UInt j = 0 ; _rInfo.m_uVertSlices > j ; ++j)
					{
						const float fAngle = j * fVertSliceAngle;
						pVertex->m_oPosition.x = cos(fAngle) * fScale;
						pVertex->m_oPosition.y = fHeight;
						pVertex->m_oPosition.z = sin(fAngle) * fScale;
						pVertex->m_oColor = Vector4(0.0f, 1.0f, 1.0f, 1.0f);
						//{
						//	char szBuffer[1024];
						//	const char* pBuffer = &szBuffer[0];
						//	sprintf(szBuffer, "%f;%f;%f\n", pVertex->m_oPosition.x, pVertex->m_oPosition.y, pVertex->m_oPosition.z);
						//	wchar_t wszBuffer[1024];
						//	mbsrtowcs(wszBuffer, &pBuffer, strlen(szBuffer) + 1, NULL);
						//	OutputDebugString(wszBuffer);
						//}
						++pVertex;
					}
				}
			}


			if (false != _rInfo.m_bBottomHemisphere)
			{
				for (UInt i = _rInfo.m_uHorizSlices - 2 ; 0 < i ; --i)
				{
					const float fHeight = cos((i + 1) * fHorizSliceAngle);
					const float fScale = sin((i + 1) * fHorizSliceAngle);
					for (UInt j = 0 ; _rInfo.m_uVertSlices > j ; ++j)
					{
						const float fAngle = j * fVertSliceAngle;
						pVertex->m_oPosition.x = cos(fAngle) * fScale;
						pVertex->m_oPosition.y = -fHeight;
						pVertex->m_oPosition.z = sin(fAngle) * fScale;
						pVertex->m_oColor = Vector4(0.0f, 1.0f, 1.0f, 1.0f);
						//{
						//	char szBuffer[1024];
						//	const char* pBuffer = &szBuffer[0];
						//	sprintf(szBuffer, "%f;%f;%f\n", pVertex->m_oPosition.x, pVertex->m_oPosition.y, pVertex->m_oPosition.z);
						//	wchar_t wszBuffer[1024];
						//	mbsrtowcs(wszBuffer, &pBuffer, strlen(szBuffer) + 1, NULL);
						//	OutputDebugString(wszBuffer);
						//}
						++pVertex;
					}
				}

				// bottom hemisphere end
				pVertex->m_oPosition.x = 0.0f;
				pVertex->m_oPosition.y = -1.0f;
				pVertex->m_oPosition.z = 0.0f;
				pVertex->m_oColor = Vector4(0.0f, 1.0f, 1.0f, 1.0f);
			}

			pVertex = pVertexData;
			for (UInt i = 0 ; m_uVertexCount > i ; ++i)
			{
				D3DXVec3Normalize(&pVertex->m_oNormal, &pVertex->m_oPosition);
				pVertex->m_oPosition.x *= _rInfo.m_oRadius.x;
				pVertex->m_oPosition.y *= _rInfo.m_oRadius.y;
				pVertex->m_oPosition.z *= _rInfo.m_oRadius.z;
				++pVertex;
			}

			m_pVertexBuffer->Set(pVertexData);
			delete[] pVertexData;
		}

		return bResult;
	}

	bool DisplayGeometrySphere::FillIndexBuffer(CreateInfoRef _rInfo)
	{
		bool bResult = (NULL != m_pIndexBuffer);

		if (false != bResult)
		{
			const bool b16Bits = (m_uIndexCount <= 0xffff);

			// temp
			if (false != b16Bits)
			{
				WordPtr pIndexData = new Word[m_uIndexCount];
				for (UInt i = 0 ; m_uIndexCount > i ; ++i)
				{
					pIndexData[i] = i;
				}
				m_pIndexBuffer->Set(pIndexData);
				delete[] pIndexData;
			}
			else
			{

			}
		}

		return bResult;
	}
}
