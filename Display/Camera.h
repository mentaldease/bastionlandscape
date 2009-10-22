#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "../Display/Display.h"

#define CAMERA_VIEWINV_AS_VIEW	1
#define CAMERA_LINEARIZED_DEPTH	0

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	static const unsigned int s_uCameraFrustumPlanesCount = 6;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct AABB
	{
		Vector3	m_aCorners[8];
	};

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
			float	m_fZNear;
			float	m_fZFar;
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

		enum EFrustumPlane
		{
			EFrustumPlane_LEFT,
			EFrustumPlane_RIGHT,
			EFrustumPlane_TOP,
			EFrustumPlane_BOTTOM,
			EFrustumPlane_NEAR,
			EFrustumPlane_FAR,
			EFrustumPlane_COUNT // last enum member
		};

		enum EFrustumCorner
		{
			EFrustumCorner_FARTOPLEFT,
			EFrustumCorner_FARTOPRIGHT,
			EFrustumCorner_FARBOTTOMLEFT,
			EFrustumCorner_FARBOTTOMRIGHT,
			EFrustumCorner_NEARTOPLEFT,
			EFrustumCorner_NEARTOPRIGHT,
			EFrustumCorner_NEARBOTTOMLEFT,
			EFrustumCorner_NEARBOTTOMRIGHT,
			EFrustumCorner_COUNT // last enum member
		};

		enum ECollision
		{
			ECollision_OUT,
			ECollision_IN,
			ECollision_INTERSECT,
		};

		enum EHalfSpace
		{
			EHalfSpace_NEGATIVE = -1,
			EHalfSpace_ON_PLANE = 0,
			EHalfSpace_POSITIVE = 1,
		};

	public:
		DisplayCamera(DisplayRef _rDisplay);
		virtual ~DisplayCamera();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		Vector3& GetPosition();
		Vector3& GetRotation();
		void GetDirs(Vector3& _oFrontDir, Vector3& _oRightDir, Vector3& _oUpDir);
		void GetDirs(const Matrix& _rMatrix, Vector3& _oFrontDir, Vector3& _oRightDir, Vector3& _oUpDir);
		MatrixPtr GetMatrix(const EMatrix& _eMatrix);

		const float& GetPixelSize() const;
		ECollision CollisionWithSphere(const Vector3& _rCenter, const float& _fRadius);
		ECollision CollisionWithAABB(AABBRef _rAABB);

		void AddListener(CoreObjectPtr _pListener);
		void RemoveListener(CoreObjectPtr _pListener);

		void SetReflection(const bool _bState, PlanePtr _pPlane = NULL);
		void SetClipPlanes(const UInt _uCount, PlanePtr _pPlanes);

		Vector3Ptr GetFrustumCorners();

	protected:
		void UpdatePixelSize();
		void ExtractFrustumPlanes();
		void ExtractFrustumCorners();
		float DistanceToPoint(const Plane &_rPlane, const Vector3& _rPoint);
		EHalfSpace PointSideOfPlane(const Plane &_rPlane, const Vector3& _rPoint);

	protected:
		DisplayRef			m_rDisplay;

		Matrix				m_oMView;
		Matrix				m_oMViewInv;
		Matrix				m_oMProjection;
		Matrix				m_oMViewProj;
		Matrix				m_oMPosition;
		Matrix				m_oMRotation;
		Vector3				m_oVPosition;
		Vector3				m_oVRotation;

		Matrix				m_oMXRot;
		Matrix				m_oMYRot;
		Matrix				m_oMZRot;
		Matrix				m_oMXYRot;

		Viewport			m_oViewport;
		float				m_fFovy;
		float				m_fAspectRatio;
		float				m_fPixelSize;
		float				m_fNear;
		float				m_fFar;

		Plane				m_aFrustumPlanes[EFrustumPlane_COUNT];
		Plane				m_aFrustumNormals[EFrustumPlane_COUNT];
		float				m_aFrustumDistances[EFrustumPlane_COUNT];
		Vector3				m_aFrustumCorners[EFrustumCorner_COUNT];

		Key					m_uCameraPosKey;
		Key					m_uFrustumCornersKey;

		CoreObjectPtrVec	m_vListeners;

		PlanePtr			m_pReflectionPlane;
		bool				m_bReflection;

		UInt				m_uClipPlaneCount;
		PlanePtr			m_pClipPlanes;

	private:
	};
}

#endif // __CAMERA_H__
