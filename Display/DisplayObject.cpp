#include "stdafx.h"
#include "../Display/Display.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayObject::DisplayObject()
	:	CoreObject(),
		m_oWorld(),
		m_pMaterial(NULL),
		m_uRenderPass(0)
	{
		D3DXMatrixIdentity(&m_oWorld);
	}

	DisplayObject::~DisplayObject()
	{

	}

	void DisplayObject::Release()
	{
		D3DXMatrixIdentity(&m_oWorld);
		m_pMaterial = NULL;
		m_uRenderPass = 0;
	}

	void DisplayObject::SetWorldMatrix(MatrixRef _rWorld)
	{
		m_oWorld = _rWorld;
	}

	MatrixPtr DisplayObject::GetWorldMatrix()
	{
		return &m_oWorld;
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

	DisplayObjectPtr DisplayObject::Clone(const boost::any& _rConfig)
	{
		return NULL;
	}
}