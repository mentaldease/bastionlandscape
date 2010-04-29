#include "stdafx.h"
#include "../Core/Core.h"
#include "../Core/Octree.h"
#include "../Core/Util.h"
#include "../Core/Profiling.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	OctreeObject::OctreeObject(OctreeRef _rOctree)
	:	//CoreObject(),
		m_vPoints(),
		m_fs3Center(0.0f, 0.0f, 0.0f),
		m_rOctree(_rOctree),
		m_fRadius(0.0f)
	{
		m_vPoints.resize(EOctreeAABB_COUNT);
	}

	OctreeObject::~OctreeObject()
	{

	}

	void OctreeObject::SetAABB(const fsVector3& _rf3TopRightFar, const fsVector3& _rf3BottomLeftNear)
	{
		//vsoutput("aabb (%f %f %f) (%f %f %f)\n",
		//	_rf3BottomLeftNear.x(), _rf3BottomLeftNear.y(), _rf3BottomLeftNear.z(),
		//	_rf3TopRightFar.x(), _rf3TopRightFar.y(), _rf3TopRightFar.z());

#define SETPOINT(ID, XVEC3, YVEC3, ZVEC3) \
	m_vPoints[ID].x() = XVEC3.x(); \
	m_vPoints[ID].y() = YVEC3.y(); \
	m_vPoints[ID].z() = ZVEC3.z();

		SETPOINT(EOctreeAABB_TOPLEFTFAR, _rf3TopRightFar, _rf3BottomLeftNear, _rf3TopRightFar);
		SETPOINT(EOctreeAABB_TOPRIGHTTFAR, _rf3TopRightFar, _rf3TopRightFar, _rf3TopRightFar);
		SETPOINT(EOctreeAABB_TOPRIGHTTNEAR, _rf3TopRightFar, _rf3TopRightFar, _rf3BottomLeftNear);
		SETPOINT(EOctreeAABB_TOPLEFTNEAR, _rf3TopRightFar, _rf3BottomLeftNear, _rf3BottomLeftNear);

		SETPOINT(EOctreeAABB_BOTTOMLEFTFAR, _rf3BottomLeftNear, _rf3BottomLeftNear, _rf3TopRightFar);
		SETPOINT(EOctreeAABB_BOTTOMRIGHTTFAR, _rf3BottomLeftNear, _rf3TopRightFar, _rf3TopRightFar);
		SETPOINT(EOctreeAABB_BOTTOMRIGHTTNEAR, _rf3BottomLeftNear, _rf3TopRightFar, _rf3BottomLeftNear);
		SETPOINT(EOctreeAABB_BOTTOMLEFTTNEAR, _rf3BottomLeftNear, _rf3BottomLeftNear, _rf3BottomLeftNear);

#undef SETPOINT

		m_fs3Center.x() = (_rf3TopRightFar.x() - _rf3BottomLeftNear.x()) / 2.0f;
		m_fs3Center.y() = (_rf3TopRightFar.y() - _rf3BottomLeftNear.y()) / 2.0f;
		m_fs3Center.z() = (_rf3TopRightFar.z() - _rf3BottomLeftNear.z()) / 2.0f;
		m_fRadius = m_fs3Center.length();

		m_fs3Center.x() += _rf3BottomLeftNear.x();
		m_fs3Center.y() += _rf3BottomLeftNear.y();
		m_fs3Center.z() += _rf3BottomLeftNear.z();
	}

	void OctreeObject::GetAABB(fsVector3Vec& _vPoints)
	{
		_vPoints = m_vPoints;
	}

	const fsVector3Vec& OctreeObject::GetAABB() const
	{
		return m_vPoints;
	}

	const fsVector3& OctreeObject::GetCenter() const
	{
		return m_fs3Center;
	}

	const float OctreeObject::GetRadius() const
	{
		return m_fRadius;
	}

	bool OctreeObject::RayIntersect(const fsVector3& _f3RayBegin, const fsVector3& _f3RayEnd, fsVector3& _f3Intersect1, fsVector3& _f3Intersect2)
	{
		_f3Intersect1 = _f3RayBegin;
		_f3Intersect2 = _f3RayEnd;
		return ClipSegment(_f3Intersect1, _f3Intersect2, m_vPoints[EOctreeAABB_BOTTOMLEFTTNEAR], m_vPoints[EOctreeAABB_TOPRIGHTTFAR]);
	}


	/*

	based on oliii code, see http://www.gamedev.net/community/forums/topic.asp?topic_id=309689

	missed intersection.
	--------------------
	intervals [tymin, tymax] and [txmin, txmax] do not overlap (here, txmin > tymax)


	|     |
	|     |
	*P0      |     |
	\       |     |
	\tymin |     |
	--------*-----+-----+-----------------
	\    |     |
	\   |     |
	-----------*--+-----+-----------------
	tymax \ |     | 
	\|     | 
	*txmin| 
	|\    | 
	| \   | 
	|  \  | 
	|   \ | 
	|    \| 
	|     *txmax 
	|     |\ 
	|     | \ 
	|     |  *P1 


	we have  intersection.
	----------------------
	intervals [tymin, tymax] and [txmin, txmax] overlap (tymin < txmax and tymax > txmin)

	*P0 |      |
	\  |      |
	\ |      |
	\|txmin |
	*      |
	|\ tymin
	--------------+-*----+-----------------
	|  \   |
	|   \ tymax
	--------------+----*-+-----------------
	|     \|
	|      *txmax 
	|      |\ 
	|      | \ 
	|      |  \  
	|      |   *P1 
	|      | 


	in code, you start off with a ray where tfirst = 0.0f, and tlast = |P1 - P0|
	- first, you test intersection with the two X planes
	=> will give you [txmin, txmax]
	=> also, you notice that txmin > tfirst, and txmax < tlast, so then 
	tfirst = txmin, tlast = txmax
	- second, you test with the Y planes
	=> will give you [tymin, tymax]
	=> here also, (tymin > tfirst), so tfirst = tymin
	=>            (tymax < tlast),  so tlast  = tymax

	- finally, (tlast > tfirst). this is consistent. so we have an intersection.


	in case of no intersection
	-------------------------
	- first, you test intersection with the two X planes
	=> will give you [txmin, txmax]
	=> also, you notice that txmin > tfirst, and txmax < tlast, so then 
	tfirst = txmin, tlast = txmax
	- second, you test with the Y planes
	=> will give you [tymin, tymax]
	=> but here, tymin is not > tfirst
	=>            (tymax < tlast),  so tlast  = tymax
	- finally, now that we have tfirst = txmin, tlast = tymax
	but, (tfirst > tlast). discrepency, the interseciton is invalid.


	- bottom line
	-------------
	you keep testing intersections with pairs of planes along axes. 
	-> you keep reducing the initial interval [tfirst, tlast] with the small values
	for both. 
	-> As soon as the interval becomes invalidated, it means that the ray missed the box and there
	is no intersection.
	/**/
	bool OctreeObject::ClipSegment(float min, float max, float a, float b, float d, float& t0, float& t1)
	{
		const float threshold = 1.0e-6f;

		if (abs(d) < threshold)
		{
			if (d > 0.0f)
			{
				return !(b < min || a > max);
			}
			else
			{
				return !(a < min || b > max);
			}
		}

		float u0, u1;

		u0 = (min - a) / (d);
		u1 = (max - a) / (d);

		if (u0 > u1) 
		{
			swap(u0, u1);
		}

		if (u1 < t0 || u0 > t1)
		{
			return false;
		}

		t0 = max(u0, t0);
		t1 = min(u1, t1);

		if (t1 < t0)
		{
			return false;
		}

		return true; 
	}

	bool OctreeObject::ClipSegment(fsVector3& A, fsVector3& B, const fsVector3& Min, const fsVector3& Max)
	{
		fsVector3 S = A;
		fsVector3 D = (B - A);

		float t0 = 0.0f, t1 = 1.0f;

		if (!ClipSegment(Min.x(), Max.x(), A.x(), B.x(), D.x(), t0, t1)) 
		{
			return false;
		}

		if (!ClipSegment(Min.y(), Max.y(), A.y(), B.y(), D.y(), t0, t1)) 
		{
			return false;
		}

		if (!ClipSegment(Min.z(), Max.z(), A.z(), B.z(), D.z(), t0, t1)) 
		{
			return false;
		}

		A = S + D * t0;
		B = S + D * t1;

		return true;
	}
}
