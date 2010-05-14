#include "../Display/DisplayTypes.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct PITTester // Point-In-Triangle Tester
	{
		bool Do(const Vector3Ptr _pPoint, const Vector3Ptr _pV0, const Vector3Ptr _pV1, const Vector3Ptr _pV2, const Vector3Ptr _pNormal)
		{
			#define VEC3TOFLOAT3(VEC3) { (VEC3).x, (VEC3).y, (VEC3).z }
			const float I[3] = VEC3TOFLOAT3(*_pPoint);
			const float V0[3] = VEC3TOFLOAT3(*_pV0);
			const float V1[3] = VEC3TOFLOAT3(*_pV1);
			const float V2[3] = VEC3TOFLOAT3(*_pV2);
			const float N[3] = VEC3TOFLOAT3(*_pNormal);
			#undef VEC3TOFLOAT3

			// Collapse the largest component of 
			// the normal in order to convert to 2D
			int i1, i2;
			if( fabs(N[0]) > fabs(N[1]) && fabs(N[0]) > fabs(N[2]) )
			{
				// x is the largest component, ignore it in further calculations
				i1 = 1;
				i2 = 2;
			}
			else if( fabs(N[1]) > fabs(N[0]) && fabs(N[1]) > fabs(N[2]))
			{
				// y is the largest
				i1 = 0;
				i2 = 2;
			}
			else
			{
				// z is the largest
				i1 = 0;
				i2 = 1;
			}


			// Compute the barycentric coordinates, by computing 
			// the signed area of each subtriangle made up of the
			// intersection point and two vertices. The signed area
			// of a triangle is computed as the crossproduct of two 
			// edges divided by two. 
			float areaTimes2 = (V1[i1] - V0[i1]) * (V2[i2] - V0[i2]) - (V1[i2] - V0[i2]) * (V2[i1] - V0[i1]);
			float u = ((V2[i1] - I[i1]) * (V0[i2] - I[i2]) - (V2[i2] - I[i2]) * (V0[i1] - I[i1])) / areaTimes2;
			float v = ((V0[i1] - I[i1]) * (V1[i2] - I[i2]) - (V0[i2] - I[i2]) * (V1[i1] - I[i1])) / areaTimes2;


			// The triangle can be described with barycentric 
			// coordinates with the following formula:
			// T(u,v) = (1-u-v)*_pV0 + u*_pV1 + v*_pV2
			// Restrictions: u >= 0, v >= 0, u + v <= 1


			// Test if the any of the restrictions are violated, if
			// they are the point is outside the triangle
			if ((u < 0) || (v < 0) || ((u + v) > 1))
				return false;


			// The point is inside the area
			return true;
		}
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct PITTester2
	{
		bool Do(const Vector3& orig, const Vector3& dir,
			const Vector3& v0, const Vector3& v1, const Vector3& v2,
			Vector3& _f3PosResult, Vector3& _f3TUVResult)
		{
			// Find vectors for two edges sharing vert0
			const Vector3 edge1 = v1 - v0;
			const Vector3 edge2 = v2 - v0;

			// Begin calculating determinant - also used to calculate U parameter
			Vector3 pvec;
			D3DXVec3Cross( &pvec, &dir, &edge2 );

			// If determinant is near zero, ray lies in plane of triangle
			FLOAT det = D3DXVec3Dot( &edge1, &pvec );

			Vector3 tvec;
			if( det > 0 )
			{
				tvec = orig - v0;
			}
			else
			{
				tvec = v0 - orig;
				det = -det;
			}

			if( det < 0.0001f )
				return false;

			// Calculate U parameter and test bounds
			_f3TUVResult.y = D3DXVec3Dot( &tvec, &pvec );
			if( _f3TUVResult.y < 0.0f || _f3TUVResult.y > det )
				return false;

			// Prepare to test V parameter
			Vector3 qvec;
			D3DXVec3Cross( &qvec, &tvec, &edge1 );

			// Calculate V parameter and test bounds
			_f3TUVResult.z = D3DXVec3Dot( &dir, &qvec );
			if( _f3TUVResult.z < 0.0f || _f3TUVResult.y + _f3TUVResult.z > det )
				return false;

			// Calculate t, scale parameters, ray intersects triangle
			_f3TUVResult.x = D3DXVec3Dot( &edge2, &qvec );
			FLOAT fInvDet = 1.0f / det;
			_f3TUVResult.x *= fInvDet; // t
			_f3TUVResult.y *= fInvDet; // u
			_f3TUVResult.z *= fInvDet; // v

			_f3PosResult = orig + _f3TUVResult.x * dir;

			return true;
		}
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	struct PITTester3 
	{
		struct Ray
		{
			const Vector3&	P0;
			const Vector3&	P1;
		};

		struct Triangle
		{
			const Vector3&	V0;
			const Vector3&	V1;
			const Vector3&	V2;
		};

		bool Do(const Vector3& _f3RayP0, const Vector3& _f3RayP1, const Vector3& _f3V0, const Vector3& _f3V1, const Vector3& _f3V2, Vector3& _f3Out)
		{
			// Copyright 2001, softSurfer (www.softsurfer.com)
			// This code may be freely used and modified for any purpose
			// providing that this copyright notice is included with it.
			// SoftSurfer makes no warranty for this code, and cannot be held
			// liable for any real or imagined damage resulting from its use.
			// Users of this code must verify correctness for their application.

			// Assume that classes are already given for the objects:
			//    Point and Vector with
			//        coordinates {float x, y, z;}
			//        operators for:
			//            == to test equality
			//            != to test inequality
			//            (Vector)0 = (0,0,0)         (null vector)
			//            Point  = Point ± Vector
			//            Vector = Point - Point
			//            Vector = Scalar * Vector    (scalar product)
			//            Vector = Vector * Vector    (cross product)
			//    Line and Ray and Segment with defining points {Point P0, P1;}
			//        (a Line is infinite, Rays and Segments start at P0)
			//        (a Ray extends beyond P1, but a Segment ends at P1)
			//    Plane with a point and a normal {Point V0; Vector n;}
			//    Triangle with defining vertices {Point V0, V1, V2;}
			//    Polyline and Polygon with n vertices {int n; Point *V;}
			//        (a Polygon has V[n]=V[0])
			//===================================================================

			Ray R = { _f3RayP0, _f3RayP1 };
			Triangle T = { _f3V0, _f3V1, _f3V2 };
			bool bResult = false;

			#define SMALL_NUM  0.00000001 // anything that avoids division overflow
			// dot product (3D) which allows vector operations in arguments
			//#define dot(u,v)   ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)
			#define dot(u,v)   D3DXVec3Dot(&(u), &(v))

			// intersect_RayTriangle(): intersect a ray with a 3D triangle
			//    Input:  a ray R, and a triangle T
			//    Output: *I = intersection point (when it exists)
			//    Return: -1 = triangle is degenerate (a segment or point)
			//             0 = disjoint (no intersect)
			//             1 = intersect in unique point I1
			//             2 = are in the same plane
			Vector3    u, v, n;             // triangle vectors
			Vector3    dir, w0, w;          // ray vectors
			float     r, a, b;             // params to calc ray-plane intersect

			// get triangle edge vectors and plane normal
			u = T.V1 - T.V0;
			v = T.V2 - T.V0;
			//n = u * v;             // cross product
			D3DXVec3Cross(&n, &u, &v);
			if (n == Vector3(0.0f, 0.0f, 0.0f))            // triangle is degenerate
				//return -1;                 // do not deal with this case
				return false;

			dir = R.P1 - R.P0;             // ray direction vector
			w0 = R.P0 - T.V0;
			a = -dot(n,w0);
			b = dot(n,dir);
			if (fabs(b) < SMALL_NUM) {     // ray is parallel to triangle plane
				//if (a == 0)                // ray lies in triangle plane
				//	return 2;
				//else return 0;             // ray disjoint from plane
				return false;
			}

			// get intersect point of ray with triangle plane
			r = a / b;
			if (r < 0.0)                   // ray goes away from triangle
				//return 0;                  // => no intersect
				return false;
			// for a segment, also test if (r > 1.0) => no intersect
			if (r > 1.0)                   // ray goes away from triangle
				//return 0;                  // => no intersect
				return false;

			_f3Out = R.P0 + r * dir;           // intersect point of ray and plane

			// is I inside T?
			float    uu, uv, vv, wu, wv, D;
			uu = dot(u,u);
			uv = dot(u,v);
			vv = dot(v,v);
			w = _f3Out - T.V0;
			wu = dot(w,u);
			wv = dot(w,v);
			D = uv * uv - uu * vv;

			// get and test parametric coords
			float s, t;
			s = (uv * wv - vv * wu) / D;
			if (s < 0.0 || s > 1.0)        // I is outside T
				//return 0;
				return false;
			t = (uv * wu - uu * wv) / D;
			if (t < 0.0 || (s + t) > 1.0)  // I is outside T
				//return 0;
				return false;

			//return 1;                      // I is in T
			return true;
		}
	};
}