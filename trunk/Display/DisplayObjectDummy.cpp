#include "stdafx.h"
#include "../Core/Core.h"
#include "../Core/Profiling.h"
#include "../Display/Display.h"
#include "../Display/DisplayObjectDummy.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayObjectDummy::DisplayObjectDummy()
	:	m_pActualObject(NULL)
	{

	}

	DisplayObjectDummy::~DisplayObjectDummy()
	{

	}

	void DisplayObjectDummy::Release()
	{
		m_pActualObject = NULL;
	}

	void DisplayObjectDummy::SetWorldMatrix(MatrixRef _rWorld)
	{
		DisplayObject::SetWorldMatrix(_rWorld);
		if (NULL != m_pActualObject)
		{
			m_pActualObject->GetTransformedBoundingMesh(m_oBoundingMesh, m_m4World);
		}
	}

	void DisplayObjectDummy::RenderBegin()
	{
		assert(NULL != m_pActualObject);
		m_pActualObject->RenderBegin();
	}

	void DisplayObjectDummy::Render()
	{
		assert(NULL != m_pActualObject);
		m_pActualObject->Render();
	}

	void DisplayObjectDummy::RenderEnd()
	{
		assert(NULL != m_pActualObject);
		m_pActualObject->RenderEnd();
	}

	bool DisplayObjectDummy::RayIntersect(const Vector3& _f3RayBegin, const Vector3& _f3RayEnd, Vector3& _f3Intersect)
	{
		assert(NULL != m_pActualObject);

		Matrix m4WorldInv;
		Vector4 f4RB;
		Vector4 f4RE;
		D3DXMatrixInverse(&m4WorldInv, NULL, &m_m4World);
		D3DXVec3Transform(&f4RB, &_f3RayBegin, &m4WorldInv);
		D3DXVec3Transform(&f4RE, &_f3RayEnd, &m4WorldInv);

		const bool bResult = m_pActualObject->RayIntersect(Vector3(f4RB.x, f4RB.y, f4RB.z), Vector3(f4RE.x, f4RE.y, f4RE.z), _f3Intersect);
		if (false != bResult)
		{
			Vector4 f4Intersect;
			D3DXVec3Transform(&f4Intersect, &_f3Intersect, &m_m4World);
			_f3Intersect.x = f4Intersect.x;
			_f3Intersect.y = f4Intersect.y;
			_f3Intersect.z = f4Intersect.z;
		}

		return bResult;
	}

	void DisplayObjectDummy::SetObject(DisplayObjectPtr	_pObject)
	{
		m_pActualObject = _pObject;
		if (NULL != m_pActualObject)
		{
			m_pActualObject->GetTransformedBoundingMesh(m_oBoundingMesh, m_m4World);
		}
		else
		{
			m_oBoundingMesh.Clear();
		}
	}
}
