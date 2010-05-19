#include "stdafx.h"
#include "../Display/Display.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	void DisplayObject::BoundingMesh::Clear()
	{
		m_vVertex.clear();
		m_vTriangles.clear();
	}

	void DisplayObject::BoundingMesh::AddVertex(const float _fX, const float _fY, const float _fZ)
	{
		m_vVertex.push_back(Vector3(_fX, _fY, _fZ));
	}

	void DisplayObject::BoundingMesh::AddTriangle(const UInt _uI1, const UInt _uI2, const UInt _uI3)
	{
		m_vTriangles.push_back(_uI1);
		m_vTriangles.push_back(_uI2);
		m_vTriangles.push_back(_uI3);
	}

	void DisplayObject::BoundingMesh::Transform(DisplayObject::BoundingMeshRef _rBoundingMesh, MatrixRef _rm4Transform)
	{
		Vector4 f4Transform;
		_rBoundingMesh.Clear();
		_rBoundingMesh.m_vTriangles = m_vTriangles;
		for (UInt i = 0 ; UInt(m_vVertex.size()) > i ; ++i)
		{
			D3DXVec3Transform(&f4Transform, &m_vVertex[i], &_rm4Transform);
			_rBoundingMesh.AddVertex(f4Transform.x, f4Transform.y, f4Transform.z);
		}
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayObject::DisplayObject()
	:	CoreObject(),
		m_m4World(),
		m_pMaterial(NULL),
		m_pComponent(NULL),
		m_oBoundingMesh(),
		m_uRenderPass(0)
	{
		D3DXMatrixIdentity(&m_m4World);
	}

	DisplayObject::~DisplayObject()
	{

	}

	void DisplayObject::Release()
	{
		D3DXMatrixIdentity(&m_m4World);
		m_oBoundingMesh.Clear();
		m_pMaterial = NULL;
		m_pComponent = NULL;
		m_uRenderPass = 0;
	}

	void DisplayObject::SetWorldMatrix(MatrixRef _rWorld)
	{
		m_m4World = _rWorld;
	}

	MatrixPtr DisplayObject::GetWorldMatrix()
	{
		return &m_m4World;
	}

	void DisplayObject::SetMaterial(DisplayMaterialPtr _pMaterial)
	{
		m_pMaterial = _pMaterial;
	}

	DisplayMaterialPtr DisplayObject::GetMaterial()
	{
		return m_pMaterial;
	}

	void DisplayObject::SetRenderStage(const Key& _uRenderPass)
	{
		m_uRenderPass = _uRenderPass;
	}

	const Key& DisplayObject::GetRenderStage() const
	{
		return m_uRenderPass;
	}

	bool DisplayObject::RayIntersect(const Vector3& _f3RayBegin, const Vector3& _f3RayEnd, Vector3& _f3Intersect)
	{
		return false;
	}

	DisplayObject::BoundingMeshRef DisplayObject::GetBoundingMesh()
	{
		return m_oBoundingMesh;
	}

	void DisplayObject::GetTransformedBoundingMesh(DisplayObject::BoundingMeshRef _rBoundingMesh, MatrixRef _rm4Transform)
	{
		m_oBoundingMesh.Transform(_rBoundingMesh, _rm4Transform);
	}

	void DisplayObject::SetComponent(ComponentPtr _pComponent)
	{
		m_pComponent = _pComponent;
	}

	ComponentPtr DisplayObject::GetComponent()
	{
		return m_pComponent;
	}
}