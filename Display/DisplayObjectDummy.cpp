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
		m_m4WorldSave = *m_pActualObject->GetWorldMatrix();
		m_pActualObject->SetWorldMatrix(m_m4World);
		const bool bResult = m_pActualObject->RayIntersect(_f3RayBegin, _f3RayEnd, _f3Intersect);
		m_pActualObject->SetWorldMatrix(m_m4WorldSave);
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
