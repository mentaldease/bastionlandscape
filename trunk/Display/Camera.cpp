#include "stdafx.h"
#include "../Display/Camera.h"
#include "../Display/Effect.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayCamera::CreateInfo::CreateInfo()
	:	m_fX(0.0f),
		m_fY(0.0f),
		m_fWidth(1.0f),
		m_fHeight(1.0f),
		m_fDegreeFovy(45.0f),
		m_fAspectRatio(0.0f),
		m_fZNear(1.0f),
		m_fZFar(1000.0f)
	{

	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayCamera::DisplayCamera(DisplayRef _rDisplay)
	:	CoreObject(),
		m_rDisplay(_rDisplay),
		m_oMView(),
		m_oMViewInv(),
		m_oMProjection(),
		m_oMViewProj(),
		m_oMPosition(),
		m_oMRotation(),
		m_oVPosition(0.0f, 0.0f, 0.0f),
		m_oVRotation(0.0f, 0.0f, 0.0f),
		m_fFovy(0.0f),
		m_fAspectRatio(0.0f),
		m_fPixelSize(0.0f),
		m_oViewport(),
		m_uCameraPosKey(MakeKey(string("CAMERAPOS"))),
		m_bReflection(false)
	{

	}

	DisplayCamera::~DisplayCamera()
	{

	}

	bool DisplayCamera::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = false;

		unsigned int uTemp[2];
		m_rDisplay.GetResolution(uTemp[0], uTemp[1]);

		m_oViewport.X = (unsigned int)(pInfo->m_fX * uTemp[0]);
		m_oViewport.Y = (unsigned int)(pInfo->m_fY * uTemp[1]);
		m_oViewport.Width = (unsigned int)(pInfo->m_fWidth * uTemp[0]);
		m_oViewport.Height = (unsigned int)(pInfo->m_fHeight * uTemp[1]);
		m_oViewport.MinZ = 0.0f;
		m_oViewport.MaxZ = 1.0f;
		HRESULT hResult = m_rDisplay.GetDevicePtr()->SetViewport(&m_oViewport);
		bResult = SUCCEEDED(hResult);

		if (false != bResult)
		{
			m_fFovy = pInfo->m_fDegreeFovy * (D3DX_PI / 180.0f);
			m_fAspectRatio = (0.0f != pInfo->m_fAspectRatio) ? pInfo->m_fAspectRatio : (float)m_oViewport.Width / (float)m_oViewport.Height;
			D3DXMatrixPerspectiveFovLH(&m_oMProjection, m_fFovy, m_fAspectRatio, pInfo->m_fZNear, pInfo->m_fZFar);
			// the two following instructions lower z-fighting artifacts
			// also make sure in the shaders to scale z position with w position : Output.Position.z *= Output.Position.w;
			m_oMProjection._33 /= pInfo->m_fZFar;
			m_oMProjection._43 /= pInfo->m_fZFar;
			UpdatePixelSize();
		}

		return bResult;
	}

	void DisplayCamera::Update()
	{
		{
			static Matrix oXRot;
			static Matrix oYRot;
			static Matrix oZRot;
			static Matrix oTemp;
			D3DXMatrixRotationX(&oXRot, m_oVRotation.x * s_fDegToRad);
			D3DXMatrixRotationY(&oYRot, m_oVRotation.y * s_fDegToRad);
			D3DXMatrixRotationZ(&oZRot, m_oVRotation.z * s_fDegToRad);
			D3DXMatrixMultiply(&oTemp, &oXRot, &oYRot);
			D3DXMatrixMultiply(&m_oMRotation, &oTemp, &oZRot);
		}
		{
			D3DXMatrixTranslation(&m_oMPosition, m_oVPosition.x, m_oVPosition.y, m_oVPosition.z);
		}

		if (false != m_bReflection)
		{
			D3DXMatrixMultiply(&m_oMView, &m_oMRotation, &m_oMPosition);

			Plane reflect_plane;
			Matrix reflect_matrix;
			reflect_plane.a = 0.0f;
			reflect_plane.b = 1.0f;
			reflect_plane.c = 0.0f;
			reflect_plane.d = 0.0f;
			// Create a reflection matrix and multiply it with the view matrix
			D3DXMatrixReflect(&reflect_matrix, &reflect_plane);
			D3DXMatrixMultiply(&m_oMView, &m_oMView, &reflect_matrix);

			D3DXMatrixMultiply(&m_oMViewProj, D3DXMatrixInverse(&m_oMViewInv, NULL, &m_oMView), &m_oMProjection);

			Plane clip_plane = reflect_plane;
			Matrix oVP = m_oMViewProj;
			D3DXMatrixInverse((D3DXMATRIX*)&oVP,0,(D3DXMATRIX*)&oVP);
			D3DXMatrixTranspose((D3DXMATRIX*)&oVP,(D3DXMATRIX*)&oVP);
			D3DXPlaneTransform(&clip_plane, &clip_plane, &oVP);
			m_rDisplay.GetDevicePtr()->SetClipPlane(0, (FloatPtr)&clip_plane);
			m_rDisplay.GetDevicePtr()->SetRenderState(D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0);
		}
		else
		{
			D3DXMatrixMultiply(&m_oMView, &m_oMRotation, &m_oMPosition);
			D3DXMatrixMultiply(&m_oMViewProj, D3DXMatrixInverse(&m_oMViewInv, NULL, &m_oMView), &m_oMProjection);
			m_rDisplay.GetDevicePtr()->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
		}

		m_rDisplay.GetMaterialManager()->SetVector3BySemantic(m_uCameraPosKey, &m_oVPosition);

		CoreObjectPtrVec::iterator iListener = m_vListeners.begin();
		CoreObjectPtrVec::iterator iEnd = m_vListeners.end();
		while (iEnd != iListener)
		{
			(*iListener)->Update();
			++iListener;
		}

		ExtractFrustumPlanes();
	}

	void DisplayCamera::Release()
	{

	}

	Vector3& DisplayCamera::GetPosition()
	{
		return m_oVPosition;
	}

	Vector3& DisplayCamera::GetRotation()
	{
		return m_oVRotation;
	}

	void DisplayCamera::GetDirs(Vector3& _oFrontDir, Vector3& _oRightDir, Vector3& _oUpDir, const bool _bInv)
	{
		Matrix& rMatrix = ( false == _bInv ) ? m_oMView : m_oMViewInv;

		_oFrontDir.x = rMatrix._13;
		_oFrontDir.y = rMatrix._23;
		_oFrontDir.z = rMatrix._33;

		_oRightDir.x = rMatrix._11;
		_oRightDir.y = rMatrix._21;
		_oRightDir.z = rMatrix._31;

		_oUpDir.x = rMatrix._12;
		_oUpDir.y = rMatrix._22;
		_oUpDir.z = rMatrix._32;
	}

	MatrixPtr DisplayCamera::GetMatrix(const EMatrix& _eMatrix)
	{
		switch (_eMatrix)
		{
			case EMatrix_VIEW:
			{
				return &m_oMView;
			}
			case EMatrix_VIEWINV:
			{
				return &m_oMViewInv;
			}
			case EMatrix_PROJ:
			{
				return &m_oMProjection;
			}
			case EMatrix_VIEWPROJ:
			{
				return &m_oMViewProj;
			}
			case EMatrix_POSITION:
			{
				return &m_oMPosition;
			}
			case EMatrix_ROTATION:
			{
				return &m_oMRotation;
			}
		}
		return &m_oMPosition;
	}

	const float& DisplayCamera::GetPixelSize() const
	{
		return m_fPixelSize;
	}

	DisplayCamera::ECollision DisplayCamera::CollisionWithSphere(const Vector3& _rCenter, const float& _fRadius)
	{
		ECollision eResult = ECollision_IN;
		float fDistance;

		// calculate our distances to each of the planes
		for (int i = 0 ; i < EFrustumPlane_COUNT ; ++i)
		{
			// find the distance to this plane
			const Vector4 Center4f(_rCenter.x, _rCenter.y, _rCenter.z, 0.0f);
			fDistance = D3DXPlaneDot(&m_aFrustumNormals[i], &Center4f) + m_aFrustumDistances[i];

			// if this distance is < -sphere.radius, we are outside
			if (fDistance < -_fRadius)
			{
				eResult = ECollision_OUT;
				break;
			}

			// else if the distance is between +- radius, then we intersect
			if ((float)fabs(fDistance) < _fRadius)
			{
				eResult = ECollision_INTERSECT;
				break;
			}
		}

		return eResult;
	}

	DisplayCamera::ECollision DisplayCamera::CollisionWithAABB(AABBRef _rAABB)
	{
		ECollision eResult = ECollision_INTERSECT;
		int sTotalIn = 0;

		// test all 8 corners against the 6 sides 
		// if all points are behind 1 specific plane, we are out
		// if we are in with all points, then we are fully in
		for (int p = 0 ; p < 6 ; ++p)
		{
			int sInCount = 8;
			int sPtIn = 1;

			for (int i = 0 ; i < 8 ; ++i)
			{
				// test this point against the planes
				if (PointSideOfPlane(m_aFrustumNormals[p], _rAABB.m_aCorners[i]) == DisplayCamera::EHalfSpace_NEGATIVE)
				{
					sPtIn = 0;
					--sInCount;
				}
			}

			// were all the points outside of plane p?
			if (0 == sInCount)
			{
				eResult = DisplayCamera::ECollision_OUT;
				break;
			}

			// check if they were all on the right side of the plane
			sTotalIn += sPtIn;
		}

		// so if sTotalIn is 6, then all are inside the view
		eResult = (6 == sTotalIn) ? ECollision_IN : ECollision_INTERSECT;

		return eResult;
	}

	void DisplayCamera::AddListener(CoreObjectPtr _pListener)
	{
		if (m_vListeners.end() == find(m_vListeners.begin(), m_vListeners.end(), _pListener))
		{
			m_vListeners.push_back(_pListener);
		}
	}

	void DisplayCamera::RemoveListener(CoreObjectPtr _pListener)
	{
		CoreObjectPtrVec::iterator iListerner = find(m_vListeners.begin(), m_vListeners.end(), _pListener);
		if (m_vListeners.end() != iListerner)
		{
			m_vListeners.erase(iListerner);
		}
	}

	void DisplayCamera::SetReflection(const bool _bState)
	{
		m_bReflection = _bState;
	}

	void DisplayCamera::UpdatePixelSize()
	{
		//const float fFovx = m_fFovy * m_fAspectRatio;
		const float fFovx = 2.0f * atan(tan(m_fFovy * 0.5f) * m_fAspectRatio);
		m_fPixelSize = float(m_oViewport.Width) / (2.0f * float(tan(fFovx / 2.0f)));
		//m_fPixelSize = 2.0f * float(tan(m_fFovy / 2.0f)) / float(m_oViewport.Height);
		//m_fPixelSize = float(m_oViewport.Height) / (2.0f * float(tan(m_fFovy / 2.0f)));
	}

	void DisplayCamera::ExtractFrustumPlanes()
	{
		// Left clipping plane
		m_aFrustumPlanes[0].a = m_oMViewProj._14 + m_oMViewProj._11;
		m_aFrustumPlanes[0].b = m_oMViewProj._24 + m_oMViewProj._21;
		m_aFrustumPlanes[0].c = m_oMViewProj._34 + m_oMViewProj._31;
		m_aFrustumPlanes[0].d = m_oMViewProj._44 + m_oMViewProj._41;
		// Right clipping plane
		m_aFrustumPlanes[1].a = m_oMViewProj._14 - m_oMViewProj._11;
		m_aFrustumPlanes[1].b = m_oMViewProj._24 - m_oMViewProj._21;
		m_aFrustumPlanes[1].c = m_oMViewProj._34 - m_oMViewProj._31;
		m_aFrustumPlanes[1].d = m_oMViewProj._44 - m_oMViewProj._41;
		// Top clipping plane
		m_aFrustumPlanes[2].a = m_oMViewProj._14 - m_oMViewProj._12;
		m_aFrustumPlanes[2].b = m_oMViewProj._24 - m_oMViewProj._22;
		m_aFrustumPlanes[2].c = m_oMViewProj._34 - m_oMViewProj._32;
		m_aFrustumPlanes[2].d = m_oMViewProj._44 - m_oMViewProj._42;
		// Bottom clipping plane
		m_aFrustumPlanes[3].a = m_oMViewProj._14 + m_oMViewProj._12;
		m_aFrustumPlanes[3].b = m_oMViewProj._24 + m_oMViewProj._22;
		m_aFrustumPlanes[3].c = m_oMViewProj._34 + m_oMViewProj._32;
		m_aFrustumPlanes[3].d = m_oMViewProj._44 + m_oMViewProj._42;
		// Near clipping plane
		m_aFrustumPlanes[4].a = m_oMViewProj._13;
		m_aFrustumPlanes[4].b = m_oMViewProj._23;
		m_aFrustumPlanes[4].c = m_oMViewProj._33;
		m_aFrustumPlanes[4].d = m_oMViewProj._43;
		// Far clipping plane
		m_aFrustumPlanes[5].a = m_oMViewProj._14 - m_oMViewProj._13;
		m_aFrustumPlanes[5].b = m_oMViewProj._24 - m_oMViewProj._23;
		m_aFrustumPlanes[5].c = m_oMViewProj._34 - m_oMViewProj._33;
		m_aFrustumPlanes[5].d = m_oMViewProj._44 - m_oMViewProj._43;

		const Vector3 oZero(0.0f, 0.0f, 0.0f);
		for (int i = 0 ; 6 > i ; ++i)
		{
			D3DXPlaneNormalize(&m_aFrustumNormals[i], &m_aFrustumPlanes[i]);
			m_aFrustumDistances[i] = DistanceToPoint(m_aFrustumNormals[i], oZero);
		}
	}

	float DisplayCamera::DistanceToPoint(const Plane &_rPlane, const Vector3& _rPoint)
	{
		return _rPlane.a*_rPoint.x + _rPlane.b*_rPoint.y + _rPlane.c*_rPoint.z + _rPlane.d;
	}

	DisplayCamera::EHalfSpace DisplayCamera::PointSideOfPlane(const Plane &_rPlane, const Vector3& _rPoint)
	{
		const float fDistance = DistanceToPoint(_rPlane, _rPoint);
		if (fDistance < 0) return EHalfSpace_NEGATIVE;
		if (fDistance > 0) return EHalfSpace_POSITIVE;
		return EHalfSpace_ON_PLANE;
	}
}
