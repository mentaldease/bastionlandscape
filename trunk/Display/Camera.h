#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "../Display/Display.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	static const float s_fDegToRad = D3DX_PI / 180.0f;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class DisplayCamera : public CoreObject
	{
	public:
		struct CreateInfo
		{
			CreateInfo();

			float	m_fX;
			float	m_fY;
			float	m_fWidth;
			float	m_fHeight;
			float	m_fDegreeFovy;
			float	m_fAspectRatio;
		};

		enum EMatrix
		{
			EMatrix_VIEW,
			EMatrix_VIEWINV,
			EMatrix_PROJ,
			EMatrix_VIEWPROJ,
			EMatrix_POSITION,
			EMatrix_ROTATION,
		};

	public:
		DisplayCamera(DisplayRef _rDisplay);
		virtual ~DisplayCamera();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		Vector3& GetPosition();
		Vector3& GetRotation();
		void GetDirs(Vector3& _oFrontDir, Vector3& _oRightDir, Vector3& _oUpDir, const bool _bInv = false);

		MatrixPtr GetMatrix(const EMatrix& _eMatrix);

	protected:
		DisplayRef	m_rDisplay;

		Matrix		m_oMView;
		Matrix		m_oMViewInv;
		Matrix		m_oMProjection;
		Matrix		m_oMViewProj;
		Matrix		m_oMPosition;
		Matrix		m_oMRotation;
		Vector3		m_oVPosition;
		Vector3		m_oVRotation;

		float		m_fFovy;
		float		m_fAspectRatio;

	private:
	};
}

#endif // __CAMERA_H__
