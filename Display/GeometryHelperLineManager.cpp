#include "stdafx.h"
#include "../Display/Display.h"
#include "../Display/Effect.h"
#include "../Display/GeometryHelper.h"
#include "../Display/RenderStage.h"
#include "../Core/Util.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	#define SV2	sizeof(Vector2)
	#define SV3	sizeof(Vector3)
	#define SV4	sizeof(Vector4)

	VertexElement GeometryHelperVertexColor::s_VertexElement[5] =
	{
		{ 0,	0,						D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
		{ 0,	1 * SV3,				D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
		{ 0,	2 * SV3,				D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
		{ 0,	3 * SV3,				D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,		0 },
		D3DDECL_END()
	};

	#undef SV2
	#undef SV3
	#undef SV4

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct AddOffsetFunction
	{
		AddOffsetFunction(const UInt _uOffset)
		:	m_uOffset(_uOffset)
		{

		}

		UInt operator() (const UInt _uValue)
		{
			return (_uValue + m_uOffset);
		}

		UInt	m_uOffset;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayGeometryLineManager::DisplayGeometryLineManager()
	:	DisplayObject(),
		m_vLineStrips(),
		m_uVertDecl(0),
		m_uCurrentLineStripIndex(0)
	{

	}

	DisplayGeometryLineManager::~DisplayGeometryLineManager()
	{

	}

	bool DisplayGeometryLineManager::Create(const boost::any& _rConfig)
	{
		CreateInfoPtr pInfo = boost::any_cast<CreateInfoPtr>(_rConfig);
		bool bResult = (NULL != pInfo);

		Release();

		if (false != bResult)
		{
			Matrix m4Pos;
			D3DXMatrixTranslation(&m4Pos, pInfo->m_f3Pos.x, pInfo->m_f3Pos.y, pInfo->m_f3Pos.z);
			Matrix m4Rot;
			D3DXMatrixRotationYawPitchRoll(&m4Rot, D3DXToRadian(pInfo->m_f3Rot.x), D3DXToRadian(pInfo->m_f3Rot.y), D3DXToRadian(pInfo->m_f3Rot.z));
			Matrix m4World;
			D3DXMatrixMultiply(&m4World, &m4Rot, &m4Pos);
			SetWorldMatrix(m4World);

			m_uVertDecl = Display::GetInstance()->CreateVertexDeclaration(GeometryHelperVertexColor::s_VertexElement);
			bResult = (0 != m_uVertDecl);
		}

		return bResult;
	}

	void DisplayGeometryLineManager::Update()
	{
		DisplayPtr pDisplay = Display::GetInstance();
		pDisplay->RenderRequest(pDisplay->GetCurrentRenderStage()->GetNameKey(), this);
	}

	void DisplayGeometryLineManager::Release()
	{
		if (0 != m_uVertDecl)
		{
			DisplayPtr pDisplay = Display::GetInstance();
			pDisplay->ReleaseVertexDeclaration(m_uVertDecl);
			m_uVertDecl = 0;
		}

		const UInt uSize = UInt(m_vLineStrips.size());
		for (UInt i = 0 ; uSize > i ; ++i)
		{
			delete m_vLineStrips[i];
		}

		m_vLineStrips.clear();
		m_uCurrentLineStripIndex = 0;

		DisplayObject::Release();
	}

	void DisplayGeometryLineManager::RenderBegin()
	{
		static const Key uDiffuseColorKey = MakeKey(string("DIFFUSECOLOR"));
		static Vector4 f4Color(0.5f, 0.5f, 0.5f, 1.0f);
		DisplayPtr pDisplay = Display::GetInstance();
		pDisplay->GetMaterialManager()->SetVector4BySemantic(uDiffuseColorKey, &f4Color);
	}

	void DisplayGeometryLineManager::Render()
	{
		DisplayPtr pDisplay = Display::GetInstance();
		if (false != pDisplay->SetVertexDeclaration(m_uVertDecl))
		{
			for (UInt i = 0 ; m_uCurrentLineStripIndex > i ; ++i)
			{
				LineStripInfoRef rLS = *m_vLineStrips[i];
				pDisplay->GetDevicePtr()->DrawIndexedPrimitiveUP(D3DPT_LINESTRIP,
					0,
					UInt(rLS.m_vVertexBuffer.size()),
					UInt(rLS.m_vIndexBuffer.size() - 1),
					&rLS.m_vIndexBuffer[0],
					D3DFMT_INDEX32,
					&rLS.m_vVertexBuffer[0],
					UInt(sizeof(GeometryHelperVertexColor)));
			}
		}
	}

	void DisplayGeometryLineManager::RenderEnd()
	{
		m_uCurrentLineStripIndex = 0;
	}

	DisplayGeometryLineManager::LineStripInfoRef DisplayGeometryLineManager::NewLineStrip()
	{
		if (m_vLineStrips.size() <= m_uCurrentLineStripIndex)
		{
			m_vLineStrips.push_back(new LineStripInfo);
		}
		LineStripInfoRef rLS = *m_vLineStrips[m_uCurrentLineStripIndex];
		++m_uCurrentLineStripIndex;
		return rLS;
	}

	void DisplayGeometryLineManager::NewTriangle(const Vector3& _f3Point0, const Vector3& _f3Point1, const Vector3& _f3Point2, const Vector4& _f4Color)
	{
		DisplayGeometryLineManager::LineStripInfoRef rLS = NewLineStrip();
		rLS.m_vVertexBuffer.resize(3);
		rLS.m_vIndexBuffer.resize(4);

		rLS.m_vIndexBuffer[0] = 0;
		rLS.m_vIndexBuffer[1] = 1;
		rLS.m_vIndexBuffer[2] = 2;
		rLS.m_vIndexBuffer[3] = 0;

		rLS.m_vVertexBuffer[0].m_f3Position = _f3Point0;
		rLS.m_vVertexBuffer[0].m_f4Color = _f4Color;
		rLS.m_vVertexBuffer[1].m_f3Position = _f3Point1;
		rLS.m_vVertexBuffer[1].m_f4Color = _f4Color;
		rLS.m_vVertexBuffer[2].m_f3Position = _f3Point2;
		rLS.m_vVertexBuffer[2].m_f4Color = _f4Color;
	}

	void DisplayGeometryLineManager::NewAABB(const Vector3& _f3TopRightFar, const Vector3& _f3BottomLeftNear, const Vector4& _f4Color)
	{
		DisplayGeometryLineManager::LineStripInfoRef rLS = NewLineStrip();
		rLS.m_vVertexBuffer.resize(8);
		rLS.m_vIndexBuffer.resize(16);

		rLS.m_vIndexBuffer[0] = 3; rLS.m_vIndexBuffer[1] = 0; rLS.m_vIndexBuffer[2] = 1; rLS.m_vIndexBuffer[3] = 2;
		rLS.m_vIndexBuffer[4] = 3; rLS.m_vIndexBuffer[5] = 7; rLS.m_vIndexBuffer[6] = 6; rLS.m_vIndexBuffer[7] = 5;
		rLS.m_vIndexBuffer[8] = 4; rLS.m_vIndexBuffer[9] = 7; rLS.m_vIndexBuffer[10] = 6; rLS.m_vIndexBuffer[11] = 2;
		rLS.m_vIndexBuffer[12] = 1; rLS.m_vIndexBuffer[13] = 5; rLS.m_vIndexBuffer[14] = 4; rLS.m_vIndexBuffer[15] = 0;

		#define VERTEX(ID, XVEC, YVEC, ZVEC) \
			rLS.m_vVertexBuffer[ID].m_f3Position.x = (XVEC).x; \
			rLS.m_vVertexBuffer[ID].m_f3Position.y = (YVEC).y; \
			rLS.m_vVertexBuffer[ID].m_f3Position.z = (ZVEC).z; \
			rLS.m_vVertexBuffer[ID].m_f4Color = _f4Color;

		VERTEX(0, _f3BottomLeftNear, _f3TopRightFar, _f3TopRightFar);
		VERTEX(1, _f3TopRightFar, _f3TopRightFar, _f3TopRightFar);
		VERTEX(2, _f3TopRightFar, _f3TopRightFar, _f3BottomLeftNear);
		VERTEX(3, _f3BottomLeftNear, _f3TopRightFar, _f3BottomLeftNear);
		VERTEX(4, _f3BottomLeftNear, _f3BottomLeftNear, _f3TopRightFar);
		VERTEX(5, _f3TopRightFar, _f3BottomLeftNear, _f3TopRightFar);
		VERTEX(6, _f3TopRightFar, _f3BottomLeftNear, _f3BottomLeftNear);
		VERTEX(7, _f3BottomLeftNear, _f3BottomLeftNear, _f3BottomLeftNear);
	}

	void DisplayGeometryLineManager::NewBoundingMesh(DisplayObject::BoundingMeshRef _rBoundingMesh, const Vector4& _f4Color)
	{
		Vector3Vec& rvVertex = _rBoundingMesh.m_vVertex;
		UIntVec& rvTriangles = _rBoundingMesh.m_vTriangles;
		const UInt uCount = UInt(rvTriangles.size());

		for (UInt i = 0 ; uCount > i ; i += 3)
		{
			NewTriangle(rvVertex[rvTriangles[i]], rvVertex[rvTriangles[i + 1]], rvVertex[rvTriangles[i + 2]], _f4Color);
		}
	}
}
