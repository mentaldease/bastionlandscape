#include "stdafx.h"
#include "../Display/Camera.h"

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
		m_fAspectRatio(0.0f)
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
		m_oViewport()
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
			D3DXMatrixPerspectiveFovLH(&m_oMProjection, m_fFovy, m_fAspectRatio, 0.0f, 10000.0f);
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
			D3DXMatrixRotationX( &oXRot, m_oVRotation.x * s_fDegToRad );
			D3DXMatrixRotationY( &oYRot, m_oVRotation.y * s_fDegToRad );
			D3DXMatrixRotationZ( &oZRot, m_oVRotation.z * s_fDegToRad );
			D3DXMatrixMultiply( &oTemp, &oXRot, &oYRot );
			D3DXMatrixMultiply( &m_oMRotation, &oTemp, &oZRot );
		}

		{
			D3DXMatrixTranslation( &m_oMPosition, m_oVPosition.x, m_oVPosition.y, m_oVPosition.z );
		}

		{
			D3DXMatrixMultiply( &m_oMView, &m_oMRotation, &m_oMPosition );
			D3DXMatrixMultiply( &m_oMViewProj, D3DXMatrixInverse( &m_oMViewInv, NULL, &m_oMView ), &m_oMProjection );
		}
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

	void DisplayCamera::UpdatePixelSize()
	{
		const float fFovx = m_fFovy * m_fAspectRatio;
		m_fPixelSize = float(m_oViewport.Width) / (2.0f * float(tan(fFovx / 2.0f)));
		//m_fPixelSize = 2.0f * float(tan(m_fFovy / 2.0f)) / float(m_oViewport.Height);
		//m_fPixelSize = float(m_oViewport.Height) / (2.0f * float(tan(m_fFovy / 2.0f)));
	}
}
