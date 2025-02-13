#include "stdafx.h"
#include "../Display/Camera.h"
#include "../Display/Effect.h"
#include "../Display/EffectStateManager.h"
#include "../Core/Scripting.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayCamera::CreateInfo::CreateInfo()
	:	m_oPos(0.0f, 0.0f, 0.0f),
		m_oRot(0.0f, 0.0f, 0.0f),
		m_fX(0.0f),
		m_fY(0.0f),
		m_fWidth(1.0f),
		m_fHeight(1.0f),
		m_fDegreeFovy(45.0f),
		m_fAspectRatio(0.0f),
		m_fZNear(1.0f),
		m_fZFar(1000.0f),
		m_bPerspectiveMode(true)
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
		m_oViewport(),
		m_pPreviousViewport(NULL),
		m_pCurrentViewport(NULL),
		m_uCurrentViewportKey(0),
		m_fFovy(0.0f),
		m_fAspectRatio(0.0f),
		m_fPixelSize(0.0f),
		m_fNear(0.0f),
		m_fFar(0.0f),
		m_uCameraPosKey(MakeKey(string("CAMERAPOS"))),
		m_uFrustumCornersKey(MakeKey(string("FRUSTUMCORNERS"))),
		m_pReflectionPlane(NULL),
		m_bReflection(false),
		m_uClipPlaneCount(0),
		m_pClipPlanes(NULL)
	{

	}

	DisplayCamera::~DisplayCamera()
	{

	}

	bool DisplayCamera::Create(const boost::any& _rConfig)
	{
		CreateInfo* pInfo = boost::any_cast<CreateInfo*>(_rConfig);
		bool bResult = (NULL != pInfo);

		if (false != bResult)
		{
			Release();
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
		}

		if (false != bResult)
		{
			m_bPerspectiveMode = pInfo->m_bPerspectiveMode;

			m_uCurrentViewportKey = 0;
			m_pPreviousViewport = &m_oViewport;
			m_pCurrentViewport = &m_oViewport;
			m_fNear = pInfo->m_fZNear;
			m_fFar = pInfo->m_fZFar;
			m_fFovy = D3DXToRadian(pInfo->m_fDegreeFovy);
			m_fAspectRatio = (0.0f != pInfo->m_fAspectRatio) ? pInfo->m_fAspectRatio : float(m_oViewport.Width) / float(m_oViewport.Height);

			if (false != m_bPerspectiveMode)
			{
				D3DXMatrixPerspectiveFovLH(&m_oMProjection, m_fFovy, m_fAspectRatio, m_fNear, m_fFar);
			}
			else
			{
				D3DXMatrixOrthoLH(&m_oMProjection, float(m_oViewport.Width), float(m_oViewport.Height), m_fNear, m_fFar);
			}
#if CAMERA_LINEARIZED_DEPTH
			// the two following instructions lower z-fighting artifacts
			// also make sure in the shaders to scale z position with w position : Output.Position.z *= Output.Position.w;
			m_oMProjection._33 /= pInfo->m_fZFar;
			m_oMProjection._43 /= pInfo->m_fZFar;
#endif // CAMERA_LINEARIZED_DEPTH

			m_oVPosition = pInfo->m_oPos;
			m_oVRotation = pInfo->m_oRot;

			UpdatePixelSize();
		}

		return bResult;
	}

	void DisplayCamera::Update()
	{
		CoreObjectPtrVec::iterator iListener = m_vListeners.begin();
		CoreObjectPtrVec::iterator iEnd = m_vListeners.end();
		while (iEnd != iListener)
		{
			(*iListener)->Update();
			++iListener;
		}

		// rotation matrix
		{
			D3DXMatrixRotationX(&m_oMXRot, D3DXToRadian(m_oVRotation.x));
			D3DXMatrixRotationY(&m_oMYRot, D3DXToRadian(m_oVRotation.y));
			D3DXMatrixRotationZ(&m_oMZRot, D3DXToRadian(m_oVRotation.z));
			D3DXMatrixMultiply(&m_oMXYRot, &m_oMXRot, &m_oMYRot);
			D3DXMatrixMultiply(&m_oMRotation, &m_oMXYRot, &m_oMZRot);
		}
		// postion matrix
		{
			D3DXMatrixTranslation(&m_oMPosition, m_oVPosition.x, m_oVPosition.y, m_oVPosition.z);
		}

		// final view matrix
		D3DXMatrixMultiply(&m_oMView, &m_oMRotation, &m_oMPosition);

		if (false != m_bReflection)
		{
			Matrix oMReflect;
			D3DXMatrixReflect(&oMReflect, m_pReflectionPlane);
			D3DXMatrixMultiply(&m_oMView, &m_oMView, &oMReflect);
			m_rDisplay.GetStateManagerInterface()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		}
		else
		{
			m_rDisplay.GetStateManagerInterface()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		}

		D3DXMatrixInverse(&m_oMViewInv, NULL, &m_oMView);
#if CAMERA_VIEWINV_AS_VIEW
		D3DXMatrixMultiply(&m_oMViewProj, &m_oMViewInv, &m_oMProjection);
#else // CAMERA_VIEWINV_AS_VIEW
		D3DXMatrixMultiply(&m_oMViewProj, &m_oMView, &m_oMProjection);
#endif // CAMERA_VIEWINV_AS_VIEW

		UInt uFlags = 0;
		if (0 < m_uClipPlaneCount)
		{
			Matrix oVP = m_oMViewProj;
			D3DXMatrixInverse(&oVP, 0, &oVP);
			D3DXMatrixTranspose(&oVP, &oVP);

			for (UInt i = 0 ; m_uClipPlaneCount > i ; ++i)
			{
				uFlags |= (1 << i);
				Plane oClipPlane = m_pClipPlanes[i];
				D3DXPlaneTransform(&oClipPlane, &oClipPlane, &oVP);
				D3DXPlaneNormalize(&oClipPlane, &oClipPlane);
				m_rDisplay.GetDevicePtr()->SetClipPlane(i, (FloatPtr)&oClipPlane);
			}
		}
		m_rDisplay.GetStateManagerInterface()->SetRenderState(D3DRS_CLIPPLANEENABLE, uFlags);

		ExtractFrustumPlanes();
		ExtractFrustumCorners();
		m_rDisplay.GetMaterialManager()->SetVector3BySemantic(m_uCameraPosKey, &m_oVPosition);
		m_rDisplay.GetMaterialManager()->SetVector3BySemantic(m_uFrustumCornersKey, m_aFrustumCorners);

		if (m_pPreviousViewport != m_pCurrentViewport)
		{
			m_rDisplay.GetDevicePtr()->SetViewport(m_pCurrentViewport);
			m_pPreviousViewport = m_pCurrentViewport;
		}
	}

	void DisplayCamera::Release()
	{
		if (NULL != m_pClipPlanes)
		{
			delete[] m_pClipPlanes;
			m_pClipPlanes = NULL;
		}
		m_uClipPlaneCount = 0;
		m_pPreviousViewport = NULL;
		m_pCurrentViewport = NULL;
		m_vListeners.clear();
	}

	ViewportPtr DisplayCamera::GetCurrentViewport()
	{
		return m_pCurrentViewport;
	}

	void DisplayCamera::SetViewport(const Key& _uNameKey)
	{
		// Here we just take notice of a view port change request.
		// This is done this way since we may have SetRenderTarget which reset view port settings to full size.
		if (m_uCurrentViewportKey != _uNameKey)
		{
			if (0 == _uNameKey)
			{
				m_pCurrentViewport = &m_oViewport;
				m_uCurrentViewportKey = _uNameKey;
			}
			else
			{
				ViewportPtr pViewport = m_rDisplay.GetViewport(_uNameKey);
				if (NULL != pViewport)
				{
					m_pCurrentViewport = pViewport;
					m_uCurrentViewportKey = _uNameKey;
				}
			}
		}
	}

	Vector3& DisplayCamera::GetPosition()
	{
		return m_oVPosition;
	}

	Vector3& DisplayCamera::GetRotation()
	{
		return m_oVRotation;
	}

	void DisplayCamera::GetDirs(Vector3& _oFrontDir, Vector3& _oRightDir, Vector3& _oUpDir)
	{
#if CAMERA_VIEWINV_AS_VIEW
		Matrix& rMatrix = m_oMViewInv;
#else // CAMERA_VIEWINV_AS_VIEW
		Matrix& rMatrix = m_oMView;
#endif // CAMERA_VIEWINV_AS_VIEW
		GetDirs(rMatrix, _oFrontDir, _oRightDir, _oUpDir);
	}


	void DisplayCamera::GetDirs(const Matrix& _rMatrix, Vector3& _oFrontDir, Vector3& _oRightDir, Vector3& _oUpDir)
	{
		_oFrontDir.x = _rMatrix._13;
		_oFrontDir.y = _rMatrix._23;
		_oFrontDir.z = _rMatrix._33;

		_oRightDir.x = _rMatrix._11;
		_oRightDir.y = _rMatrix._21;
		_oRightDir.z = _rMatrix._31;

		_oUpDir.x = _rMatrix._12;
		_oUpDir.y = _rMatrix._22;
		_oUpDir.z = _rMatrix._32;
	}

	MatrixPtr DisplayCamera::GetMatrix(const EMatrix& _eMatrix)
	{
		switch (_eMatrix)
		{
			case EMatrix_VIEW:
			{
#if CAMERA_VIEWINV_AS_VIEW
				return &m_oMViewInv;
#else // CAMERA_VIEWINV_AS_VIEW
				return &m_oMView;
#endif // CAMERA_VIEWINV_AS_VIEW
			}
			case EMatrix_VIEWINV:
			{
#if CAMERA_VIEWINV_AS_VIEW
				return &m_oMView;
#else // CAMERA_VIEWINV_AS_VIEW
				return &m_oMViewInv;
#endif // CAMERA_VIEWINV_AS_VIEW
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
		PROFILING(__FUNCTION__);
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
		eResult = (6 == sTotalIn) ? ECollision_IN : eResult;

		return eResult;
	}

	DisplayCamera::ECollision DisplayCamera::CollisionWithAABB(const fsVector3Vec& _rvAABB)
	{
		PROFILING(__FUNCTION__);
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
				const Vector3 f3Corner(_rvAABB[i].x(), _rvAABB[i].y(), _rvAABB[i].z());
				if (PointSideOfPlane(m_aFrustumNormals[p], f3Corner) == DisplayCamera::EHalfSpace_NEGATIVE)
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
		eResult = (6 == sTotalIn) ? ECollision_IN : eResult;

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

	void DisplayCamera::SetReflection(const bool _bState, PlanePtr _pPlane)
	{
		m_bReflection = _bState;
		m_pReflectionPlane = _pPlane;
	}

	void DisplayCamera::SetClipPlanes(const UInt _uCount, PlanePtr _pPlanes)
	{
		m_uClipPlaneCount = _uCount;
		if (NULL != m_pClipPlanes)
		{
			delete[] m_pClipPlanes;
			m_pClipPlanes = NULL;
		}
		if (0 < m_uClipPlaneCount)
		{
			m_pClipPlanes = new Plane[m_uClipPlaneCount];
			memcpy(m_pClipPlanes, _pPlanes, m_uClipPlaneCount * sizeof(Plane));
		}
	}

	Vector3Ptr DisplayCamera::GetFrustumCorners()
	{
		return m_aFrustumCorners;
	}

	void DisplayCamera::GetStateInfo(DisplayCamera::StateInfoRef _rInfo)
	{
		_rInfo.m_pViewport = m_pCurrentViewport;
		_rInfo.m_fFovy = m_fFovy;
		_rInfo.m_fAspectRatio = m_fAspectRatio;
		_rInfo.m_fPixelSize = m_fPixelSize;
		_rInfo.m_fNear = m_fNear;
		_rInfo.m_fFar = m_fFar;
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
		m_aFrustumPlanes[EFrustumPlane_LEFT].a = m_oMViewProj._14 + m_oMViewProj._11;
		m_aFrustumPlanes[EFrustumPlane_LEFT].b = m_oMViewProj._24 + m_oMViewProj._21;
		m_aFrustumPlanes[EFrustumPlane_LEFT].c = m_oMViewProj._34 + m_oMViewProj._31;
		m_aFrustumPlanes[EFrustumPlane_LEFT].d = m_oMViewProj._44 + m_oMViewProj._41;
		// Right clipping plane
		m_aFrustumPlanes[EFrustumPlane_RIGHT].a = m_oMViewProj._14 - m_oMViewProj._11;
		m_aFrustumPlanes[EFrustumPlane_RIGHT].b = m_oMViewProj._24 - m_oMViewProj._21;
		m_aFrustumPlanes[EFrustumPlane_RIGHT].c = m_oMViewProj._34 - m_oMViewProj._31;
		m_aFrustumPlanes[EFrustumPlane_RIGHT].d = m_oMViewProj._44 - m_oMViewProj._41;
		// Top clipping plane
		m_aFrustumPlanes[EFrustumPlane_TOP].a = m_oMViewProj._14 - m_oMViewProj._12;
		m_aFrustumPlanes[EFrustumPlane_TOP].b = m_oMViewProj._24 - m_oMViewProj._22;
		m_aFrustumPlanes[EFrustumPlane_TOP].c = m_oMViewProj._34 - m_oMViewProj._32;
		m_aFrustumPlanes[EFrustumPlane_TOP].d = m_oMViewProj._44 - m_oMViewProj._42;
		// Bottom clipping plane
		m_aFrustumPlanes[EFrustumPlane_BOTTOM].a = m_oMViewProj._14 + m_oMViewProj._12;
		m_aFrustumPlanes[EFrustumPlane_BOTTOM].b = m_oMViewProj._24 + m_oMViewProj._22;
		m_aFrustumPlanes[EFrustumPlane_BOTTOM].c = m_oMViewProj._34 + m_oMViewProj._32;
		m_aFrustumPlanes[EFrustumPlane_BOTTOM].d = m_oMViewProj._44 + m_oMViewProj._42;
		// Near clipping plane
		m_aFrustumPlanes[EFrustumPlane_NEAR].a = m_oMViewProj._13;
		m_aFrustumPlanes[EFrustumPlane_NEAR].b = m_oMViewProj._23;
		m_aFrustumPlanes[EFrustumPlane_NEAR].c = m_oMViewProj._33;
		m_aFrustumPlanes[EFrustumPlane_NEAR].d = m_oMViewProj._43;
		// Far clipping plane
		m_aFrustumPlanes[EFrustumPlane_FAR].a = m_oMViewProj._14 - m_oMViewProj._13;
		m_aFrustumPlanes[EFrustumPlane_FAR].b = m_oMViewProj._24 - m_oMViewProj._23;
		m_aFrustumPlanes[EFrustumPlane_FAR].c = m_oMViewProj._34 - m_oMViewProj._33;
		m_aFrustumPlanes[EFrustumPlane_FAR].d = m_oMViewProj._44 - m_oMViewProj._43;

		const Vector3 oZero(0.0f, 0.0f, 0.0f);
		for (int i = 0 ; 6 > i ; ++i)
		{
			D3DXPlaneNormalize(&m_aFrustumNormals[i], &m_aFrustumPlanes[i]);
			m_aFrustumDistances[i] = DistanceToPoint(m_aFrustumNormals[i], oZero);
		}
	}

	void DisplayCamera::ExtractFrustumCorners()
	{
		//const float fHalfNearQuadHeight = (2.0f * tan(m_fFovy / 2.0f) * m_fNear) / 2.0f;
		//const float fHalfFarQuadHeight = (2.0f * tan(m_fFovy / 2.0f) * m_fFar) / 2.0f;
		const float fHalfNearQuadHeight = tan(m_fFovy / 2.0f) * m_fNear;
		const float fHalfFarQuadHeight = tan(m_fFovy / 2.0f) * m_fFar;
		const float fHalfNearQuadWidth = fHalfNearQuadHeight * m_fAspectRatio;
		const float fHalfFarQuadWidth = fHalfFarQuadHeight * m_fAspectRatio;

		Vector3 oFrontDir;
		Vector3 oRightDir;
		Vector3 oUpDir;
		GetDirs(oFrontDir, oRightDir, oUpDir);
		const Vector3 oFarCenter = m_oVPosition + oFrontDir * m_fFar;
		const Vector3 oNearCenter = m_oVPosition + oFrontDir * m_fNear;

		m_aFrustumCorners[EFrustumCorner_FARTOPLEFT] = oFarCenter + (oUpDir * fHalfFarQuadHeight) - (oRightDir * fHalfFarQuadWidth);
		m_aFrustumCorners[EFrustumCorner_FARTOPRIGHT] = oFarCenter + (oUpDir * fHalfFarQuadHeight) + (oRightDir * fHalfFarQuadWidth);
		m_aFrustumCorners[EFrustumCorner_FARBOTTOMLEFT] = oFarCenter - (oUpDir * fHalfFarQuadHeight) - (oRightDir * fHalfFarQuadWidth);
		m_aFrustumCorners[EFrustumCorner_FARBOTTOMRIGHT] = oFarCenter - (oUpDir * fHalfFarQuadHeight) + (oRightDir * fHalfFarQuadWidth);
		m_aFrustumCorners[EFrustumCorner_NEARTOPLEFT] = oNearCenter + (oUpDir * fHalfNearQuadHeight) - (oRightDir * fHalfNearQuadWidth);
		m_aFrustumCorners[EFrustumCorner_NEARTOPRIGHT] = oNearCenter + (oUpDir * fHalfNearQuadHeight) + (oRightDir * fHalfNearQuadWidth);
		m_aFrustumCorners[EFrustumCorner_NEARBOTTOMLEFT] = oNearCenter - (oUpDir * fHalfNearQuadHeight) - (oRightDir * fHalfNearQuadWidth);
		m_aFrustumCorners[EFrustumCorner_NEARBOTTOMRIGHT] = oNearCenter - (oUpDir * fHalfNearQuadHeight) + (oRightDir * fHalfNearQuadWidth);
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
