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
		bool bResult = (NULL != pInfo) && (0 < pInfo->m_uMaxIndex) && (0 < pInfo->m_uMaxVertex);

		Release();

		if (false != bResult)
		{
			Matrix m4Pos;
			D3DXMatrixTranslation(&m4Pos, pInfo->m_oPos.x, pInfo->m_oPos.y, pInfo->m_oPos.z);
			Matrix m4Rot;
			D3DXMatrixRotationYawPitchRoll(&m4Rot, D3DXToRadian(pInfo->m_oRot.x), D3DXToRadian(pInfo->m_oRot.y), D3DXToRadian(pInfo->m_oRot.z));
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

		for (UInt i = 0 ; m_uCurrentLineStripIndex > i ; ++i)
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
		Display::GetInstance()->SetVertexDeclaration(m_uVertDecl);
	}

	void DisplayGeometryLineManager::Render()
	{
		DisplayPtr pDisplay = Display::GetInstance();
		for (UInt i = 0 ; m_uCurrentLineStripIndex > i ; ++i)
		{
			LineStripInfoRef rLS = *m_vLineStrips[i];
			pDisplay->GetDevicePtr()->DrawIndexedPrimitiveUP(D3DPT_LINESTRIP, 0, rLS.m_vVertexBuffer.size(), rLS.m_vIndexBuffer.size() - 1, &rLS.m_vIndexBuffer[0], D3DFMT_INDEX32, &rLS.m_vVertexBuffer[0], sizeof(GeometryHelperVertexColor));
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
}
